// Collision for felled wild map trees (config FallenTreeCollision; default off = vanilla, a fallen wild tree blocks
// nobody). The engine keeps a cut map tree lying (the server has the lying pose a frame after the cut, clients animate into
// the same pose in ~1.5 s, probe 2026-10-31) but drops its collision. A hidden static copy of the same model (its own
// Geometry LOD) is put in that pose. Objects made from a p3d are not networked, so the server and every client make
// their own (PF_RPC.SYNC_FALLEN, also sent on connect). Memory only: a restart stands the wild trees up again.
class PF_FallenColliders
{
	static const int CLIENT_DELAY_MS = 1600;	// client: the collider appears once the fall animation is done
	static const int SERVER_DELAY_MS = 300;		// server: the engine sets the lying pose a frame after the cut, read it then

	// server: every collider so far (for players who connect later), transform = 4 vectors per tree
	protected static ref array<string> s_Models = new array<string>();
	protected static ref array<vector> s_Transforms = new array<vector>();
	// both sides: the hidden copies; they are vanilla tree objects themselves, so actions on them are blocked (ActionBase.Can)
	protected static ref array<Object> s_Colliders = new array<Object>();

	static bool IsCollider(Object obj)
	{
		return obj && s_Colliders.Find(obj) != -1;
	}

	// server: a wild tree was cut down
	static void OnCut(Object tree)
	{
		if (tree && PF_Config.Get().FallenTreeCollision)
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Add, SERVER_DELAY_MS, false, tree);
	}

	// server: the cut tree lies in its final pose
	protected static void Add(Object tree)
	{
		if (!tree)
			return;
		string model = tree.GetShapeName();
		if (model == "")
			return;
		vector mat[4];
		tree.GetTransform(mat);
		array<string> models = new array<string>();
		array<vector> transforms = new array<vector>();
		models.Insert(model);
		for (int i = 0; i < 4; i++)
			transforms.Insert(mat[i]);
		s_Models.Insert(model);
		s_Transforms.InsertAll(transforms);
		Spawn(model, mat);
		PF_Log.Debug("fallen collider " + model + " at " + mat[3].ToString());
		g_Game.RPCSingleParam(null, PF_RPC.SYNC_FALLEN, new Param2<ref array<string>, ref array<vector>>(models, transforms), true, null);
	}

	// server -> one client on connect: all colliders so far
	static void SendTo(PlayerIdentity identity)
	{
		if (s_Models.Count() == 0)
			return;
		g_Game.RPCSingleParam(null, PF_RPC.SYNC_FALLEN, new Param2<ref array<string>, ref array<vector>>(s_Models, s_Transforms), true, identity);
	}

	// client
	static void OnReceive(array<string> models, array<vector> transforms)
	{
		for (int i = 0; i < models.Count(); i++)
		{
			if (transforms.Count() < i * 4 + 4)
				return;
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SpawnAt, CLIENT_DELAY_MS, false, models[i], transforms[i * 4], transforms[i * 4 + 1], transforms[i * 4 + 2], transforms[i * 4 + 3]);
		}
	}

	protected static void SpawnAt(string model, vector m0, vector m1, vector m2, vector m3)
	{
		vector mat[4];
		mat[0] = m0;
		mat[1] = m1;
		mat[2] = m2;
		mat[3] = m3;
		Spawn(model, mat);
	}

	protected static void Spawn(string model, vector mat[4])
	{
		Object obj = g_Game.CreateStaticObjectUsingP3D(model, mat[3], vector.Zero, 1.0, true);
		if (!obj)
		{
			PF_Log.Debug("fallen collider: could not create " + model);
			return;
		}
		obj.SetTransform(mat);
		s_Colliders.Insert(obj);
		Hide(obj);
		// static objects may get their flags back after the first frame
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Hide, 100, false, obj);
	}

	protected static void Hide(Object obj)
	{
		if (!obj)
			return;
		obj.ClearFlags(EntityFlags.VISIBLE, true);
		Entity entity = Entity.Cast(obj);
		if (entity)
			entity.SetInvisible(true);
		string info = "fallen collider hide: " + obj.ClassName() + " entity " + (entity != null);
		info += " visible " + obj.IsFlagSet(EntityFlags.VISIBLE);
		PF_Log.Debug(info);
	}
}
