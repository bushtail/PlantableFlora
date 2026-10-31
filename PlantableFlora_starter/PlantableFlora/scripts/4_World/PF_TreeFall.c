// Falling tree (felling step 1, vanilla style): the tree tips over around its cut point like a falling rod
// (angular acceleration 3g / 2L * sin(tilt)) and comes to rest on the terrain. Clients animate it locally, the
// server only sets the end pose (synced to everyone, also to players who stream in later).
// Works on any Object (step 2 idea: the same fall for wild map trees).
class PF_TreeFall
{
	static const float CUT_HEIGHT = 0.4;	// m above the terrain at the trunk: pivot of the fall (about the stump top)
	static const float START_TILT = 0.035;	// rad (~2 deg), the cut tree already leans a little when it starts to go
	static const float START_SPEED = 0.1;	// rad/s
	static const float REST_LIFT = 0.07;	// rad (~4 deg) above the terrain line: the crown rests on its branches, not on the trunk axis
	static const float MIN_TILT = 0.8;		// rad (~45 deg), steep uphill: stops early
	static const float MAX_TILT = 2.0;		// rad (~115 deg), steep downhill
	static const float STEP = 0.0166;		// s, simulation step of Duration()

	static vector Pivot(Object tree, vector trunk)
	{
		trunk[1] = g_Game.SurfaceY(trunk[0], trunk[2]) + CUT_HEIGHT;
		return trunk;
	}

	// tree height above the pivot (model bounding box top)
	static float Length(Object tree, vector pivot)
	{
		vector minMax[2];
		tree.ClippingInfo(minMax);
		return Math.Max(tree.GetPosition()[1] + minMax[1][1] - pivot[1], 2);
	}

	static vector FallDirection(float heading)
	{
		return Vector(Math.Sin(heading), 0, Math.Cos(heading));
	}

	// rotation axis that turns "up" into the fall direction
	static vector Axis(float heading)
	{
		return vector.Up * FallDirection(heading);
	}

	// tilt (rad from upright) at which the tree lies on the terrain along the fall direction:
	// the highest terrain elevation seen from the pivot along the tree, plus REST_LIFT
	static float RestTilt(Object tree, vector pivot, float heading)
	{
		float length = Length(tree, pivot);
		vector dir = FallDirection(heading);
		float maxElevation = -1.5;
		for (int i = 1; i <= 8; i++)
		{
			float dist = length * i / 8;
			vector p = pivot + dir * dist;
			float elevation = Math.Atan2(g_Game.SurfaceY(p[0], p[2]) - pivot[1], dist);
			maxElevation = Math.Max(maxElevation, elevation);
		}
		return Math.Clamp(Math.PI_HALF - maxElevation - REST_LIFT, MIN_TILT, MAX_TILT);
	}

	static float Accel(float length, float tilt)
	{
		return 1.5 * 9.81 / length * Math.Sin(tilt);
	}

	// seconds the fall takes (same integration as PF_TreeFallAnim)
	static float Duration(float length, float endTilt)
	{
		float tilt = START_TILT;
		float speed = START_SPEED;
		float t = 0;
		while (tilt < endTilt && t < 20)
		{
			speed += Accel(length, tilt) * STEP;
			tilt += speed * STEP;
			t += STEP;
		}
		return t;
	}

	// transform of the tree tilted by tilt around the pivot (start = upright transform)
	static void Pose(vector start[4], vector pivot, vector axis, float tilt, out vector mat[4])
	{
		for (int i = 0; i < 3; i++)
			mat[i] = vector.RotateAroundZeroRad(start[i], axis, tilt);
		mat[3] = pivot + vector.RotateAroundZeroRad(start[3] - pivot, axis, tilt);
	}

	// server: put the tree straight into its lying pose (from upright)
	static void SetLying(Object tree, vector pivot, float heading, float tilt)
	{
		vector start[4];
		tree.GetTransform(start);
		vector mat[4];
		Pose(start, pivot, Axis(heading), tilt, mat);
		tree.SetTransform(mat);
	}

	// vanilla PlantType sounds (EntityAI.SoundHardTreeFallingPlay): TreeHard / TreeSoft of the model's wild class
	static void PlaySound(vector pos, bool hard)
	{
		string soundSet = "hardTreeFall_SoundSet";
		if (!hard)
			soundSet = "softTreeFall_SoundSet";
		EffectSound sound = SEffectManager.PlaySound(soundSet, pos);
		if (sound)
			sound.SetAutodestroy(true);
	}
}

// What felling a tree model gives, from the vanilla wild tree class of the same model (CfgNonAIVehicles
// TreeHard_<model> / TreeSoft_<model>, read like CAContinuousMineWood: primaryDropsAmount cycles of primaryOutput,
// then secondaryDropsAmount of secondaryOutput, toolDamage per cycle). Winter/spring/frozen variants without an
// entry use their plain model; models without any (Badlands) use config FellLogs / FellSticks.
class PF_FellYield
{
	string Primary = "WoodenLog";
	int PrimaryCount;
	string Secondary = "LongWoodenStick";
	int SecondaryCount;
	float ToolDamage = -1;	// per cycle; -1 = 20 per tree spread over the cycles
	bool Hard = true;		// fall sound

	protected static ref map<string, ref PF_FellYield> s_Cache = new map<string, ref PF_FellYield>();

	// model = config model path of the tree class
	static PF_FellYield ForModel(string model)
	{
		PF_FellYield yield;
		if (s_Cache.Find(model, yield))
			return yield;

		yield = new PF_FellYield();
		string name = model;
		name.ToLower();
		name.Replace("/", "\\");
		array<string> parts = new array<string>();
		name.Split("\\", parts);
		if (parts.Count() > 0)
			name = parts[parts.Count() - 1];
		name.Replace(".p3d", "");

		bool found = yield.Read(name);
		if (!found)
		{
			array<string> suffixes = {"_winter", "_spring", "_frozen", "_latefall", "_dark"};
			foreach (string suffix : suffixes)
			{
				if (!found && name.Contains(suffix))
				{
					string plain = name;
					plain.Replace(suffix, "");
					found = yield.Read(plain);
				}
			}
		}
		if (!found)
		{
			PF_Config cfg = PF_Config.Get();
			yield.PrimaryCount = cfg.FellLogs;
			yield.SecondaryCount = cfg.FellSticks;
		}
		PF_Log.Debug(string.Format("fell yield %1: %2x %3 + %4x %5, tool %6, vanilla %7", name, yield.PrimaryCount, yield.Primary, yield.SecondaryCount, yield.Secondary, yield.ToolDamage, found));
		s_Cache.Insert(model, yield);
		return yield;
	}

	protected bool Read(string name)
	{
		if (ReadClass("TreeHard_" + name))
		{
			Hard = true;
			return true;
		}
		if (ReadClass("TreeSoft_" + name))
		{
			Hard = false;
			return true;
		}
		return false;
	}

	protected bool ReadClass(string cls)
	{
		string path = "CfgNonAIVehicles " + cls;
		if (!g_Game.ConfigIsExisting(path))
			return false;
		string primary = "";
		g_Game.ConfigGetText(path + " primaryOutput", primary);
		if (primary == "")
			return false;
		Primary = primary;
		// vanilla: no primaryDropsAmount = 3 drops with an axe (WoodBase.GetAmountOfDropsEx)
		PrimaryCount = g_Game.ConfigGetInt(path + " primaryDropsAmount");
		if (PrimaryCount <= 0)
			PrimaryCount = 3;
		string secondary = "";
		g_Game.ConfigGetText(path + " secondaryOutput", secondary);
		Secondary = secondary;
		SecondaryCount = 0;
		if (secondary != "")
			SecondaryCount = g_Game.ConfigGetInt(path + " secondaryDropsAmount");
		ToolDamage = -1;
		if (g_Game.ConfigIsExisting(path + " toolDamage"))
			ToolDamage = g_Game.ConfigGetFloat(path + " toolDamage");
		return true;
	}

	int Cycles()
	{
		if (PrimaryCount < 1)
			return 1;
		return PrimaryCount;
	}

	float ToolDamagePerCycle()
	{
		if (ToolDamage > -1)
			return ToolDamage;
		return 20.0 / Cycles();
	}
}

// Client: running fall animations, stepped every frame
class PF_TreeFallAnim
{
	protected static ref array<ref PF_TreeFallAnim> s_Running = new array<ref PF_TreeFallAnim>();

	protected Object m_Tree;
	protected vector m_Start[4];
	protected vector m_Pivot;
	protected vector m_Axis;
	protected float m_Length;
	protected float m_EndTilt;
	protected float m_Tilt;
	protected float m_Speed;

	static void Start(Object tree, vector trunk, float heading, float endTilt, bool hard)
	{
		if (!tree || IsRunning(tree))
			return;
		PF_TreeFallAnim anim = new PF_TreeFallAnim();
		anim.m_Tree = tree;
		tree.GetTransform(anim.m_Start);
		anim.m_Pivot = PF_TreeFall.Pivot(tree, trunk);
		anim.m_Axis = PF_TreeFall.Axis(heading);
		anim.m_Length = PF_TreeFall.Length(tree, anim.m_Pivot);
		anim.m_EndTilt = endTilt;
		anim.m_Tilt = PF_TreeFall.START_TILT;
		anim.m_Speed = PF_TreeFall.START_SPEED;
		PF_TreeFall.PlaySound(anim.m_Pivot, hard);
		if (s_Running.Count() == 0)
			g_Game.GetUpdateQueue(CALL_CATEGORY_SYSTEM).Insert(PF_TreeFallAnim.Update);
		s_Running.Insert(anim);
	}

	static bool IsRunning(Object tree)
	{
		foreach (PF_TreeFallAnim anim : s_Running)
		{
			if (anim.m_Tree == tree)
				return true;
		}
		return false;
	}

	// the server pose arrived (or the tree is gone): stop animating it
	static void Stop(Object tree)
	{
		for (int i = s_Running.Count() - 1; i >= 0; i--)
		{
			if (s_Running[i].m_Tree == tree)
				s_Running.Remove(i);
		}
		if (s_Running.Count() == 0)
			g_Game.GetUpdateQueue(CALL_CATEGORY_SYSTEM).Remove(PF_TreeFallAnim.Update);
	}

	static void Update(float dt)
	{
		for (int i = s_Running.Count() - 1; i >= 0; i--)
		{
			if (!s_Running[i].Step(dt))
				s_Running.Remove(i);
		}
		if (s_Running.Count() == 0)
			g_Game.GetUpdateQueue(CALL_CATEGORY_SYSTEM).Remove(PF_TreeFallAnim.Update);
	}

	// false = finished
	protected bool Step(float dt)
	{
		if (!m_Tree)
			return false;
		m_Speed += PF_TreeFall.Accel(m_Length, m_Tilt) * dt;
		m_Tilt = Math.Min(m_Tilt + m_Speed * dt, m_EndTilt);
		vector mat[4];
		PF_TreeFall.Pose(m_Start, m_Pivot, m_Axis, m_Tilt, mat);
		m_Tree.SetTransform(mat);
		return m_Tilt < m_EndTilt;
	}
}
