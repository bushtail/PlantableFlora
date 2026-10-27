// Ground preparation (Claude.md §15): seeds are planted onto prepared ground, either an own planting mound
// (dug from the seed's placement hologram) or an empty vanilla garden plot. Fertilizer and water are bonuses:
// each makes every growth step of the plant faster (config GardenLimeBonus / PlantMaterialBonus / WaterBonus).

class PF_PlantingMound extends ItemBase
{
	static const int STORAGE_VERSION = 2;	// 2: m_Compost
	protected bool m_Fertilized;
	protected bool m_Compost;	// fertilized with plant material (compost look) instead of garden lime
	protected bool m_Watered;

	void PF_PlantingMound()
	{
		RegisterNetSyncVariableBool("m_Fertilized");
		RegisterNetSyncVariableBool("m_Compost");
		RegisterNetSyncVariableBool("m_Watered");
	}

	// server: every existing mound (admin wipe, rain)
	static ref array<PF_PlantingMound> s_All = new array<PF_PlantingMound>();

	// rain waters holes like vanilla garden plots (GardenBase.CheckRainTick: every 15 s, rain above 0.05)
	static const int RAIN_TICK_SECONDS = 15;
	static const float RAIN_THRESHOLD = 0.05;
	protected static bool s_RainRunning;
	protected float m_RainSeconds;	// rain collected so far (server only, not saved; starts over after a restart)
	protected Object m_ClutterCutter;	// removes the grass around the hole while it exists (like vanilla garden plot / fireplace)

	static void RainTick()
	{
		float minutes = PF_Config.Get().RainWaterMinutes;
		if (minutes <= 0 || g_Game.GetWeather().GetRain().GetActual() <= RAIN_THRESHOLD)
			return;
		foreach (PF_PlantingMound mound : s_All)
		{
			if (!mound || mound.IsWatered())
				continue;
			mound.m_RainSeconds += RAIN_TICK_SECONDS;
			PF_Log.Debug(string.Format("rain on planting hole at %1: %2/%3 s", mound.GetPosition().ToString(), mound.m_RainSeconds, minutes * 60));
			if (mound.m_RainSeconds >= minutes * 60)
				mound.SetWatered();
		}
	}

	override void EEInit()
	{
		super.EEInit();
		// client: map soil look without waiting for a change (OnVariablesSynchronized only runs once a value changes);
		// set in EEInit itself it was lost (test 2026-10-20), so a moment later
		if (!g_Game.IsDedicatedServer())
			g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(RefreshSoilLook, 200, false);
		if (!g_Game.IsServer())
			return;
		s_All.Insert(this);
		if (!s_RainRunning)
		{
			s_RainRunning = true;
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(RainTick, RAIN_TICK_SECONDS * 1000, true);
		}
		AlignToTerrain();
		// created for new and loaded holes (not saved), deleted with the hole: grass grows back around the plant
		if (!m_ClutterCutter)
			m_ClutterCutter = g_Game.CreateObjectEx("ClutterCutterFireplace", GetPosition(), ECE_PLACE_ON_SURFACE);
		dBodyActive(this, ActiveState.INACTIVE);
		dBodyDynamic(this, false);
		// an unused mound is removed by the Central Economy after MoundLifetimeMinutes (once no player is near)
		float lifetime = PF_Config.Get().MoundLifetimeMinutes * 60;
		SetLifetimeMax(lifetime);
		SetLifetime(lifetime);
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		s_All.RemoveItem(this);
		if (m_ClutterCutter && g_Game)
		{
			g_Game.ObjectDelete(m_ClutterCutter);
			m_ClutterCutter = null;
		}
	}

	// show the state on the soil ("camo" selection): vanilla garden soil variants (limed / wet)
	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		if (g_Game.IsDedicatedServer())
			return;
		ApplySoilLook(this, GetFertilizer(), m_Watered);
	}

	void RefreshSoilLook()
	{
		ApplySoilLook(this, GetFertilizer(), m_Watered);
	}

	// soil look of the hole, also used by the seedling standing in it (PF_Bush_Base.SetSoil).
	// Same files as vanilla GardenBase (SLOT_MATERIAL_*); only works on a binarized model with a "camo" section (data\model.cfg).
	static void ApplySoilLook(EntityAI obj, int fertilizer, bool watered)
	{
		string soil = "soil_cultivated";
		if (fertilizer == PF_PreparedGround.FERT_COMPOST)
			soil = "soil_cultivated_compost";
		else if (fertilizer == PF_PreparedGround.FERT_LIME)
			soil = "soil_cultivated_limed";
		string texture = "dz\\gear\\cultivation\\data\\" + soil + "_co.paa";
		// plain and limed soil can have a map look (config CfgPlantableFlora Soil <world>; Badlands: light dug earth,
		// limed = with light gravel)
		string mapTexture = "";
		if (fertilizer == PF_PreparedGround.FERT_NONE)
			mapTexture = PF_World.SoilTexture();
		else if (fertilizer == PF_PreparedGround.FERT_LIME)
			mapTexture = PF_World.LimedSoilTexture();
		if (mapTexture != "")
			texture = mapTexture;
		obj.SetObjectTexture(0, texture);
		if (watered)
			obj.SetObjectMaterial(0, "dz\\gear\\cultivation\\data\\" + soil + "_wet.rvmat");
		else
			obj.SetObjectMaterial(0, "dz\\gear\\cultivation\\data\\" + soil + ".rvmat");
	}

	void AlignToTerrain()
	{
		AlignObjectToTerrain(this, 0);
	}

	// sit on the terrain (+ height) and tilt with the slope (terrain normal as up), keeping the current heading
	static void AlignObjectToTerrain(Object obj, float height)
	{
		vector pos = obj.GetPosition();
		pos[1] = g_Game.SurfaceY(pos[0], pos[2]) + height;
		vector up = g_Game.SurfaceGetNormal(pos[0], pos[2]);
		vector heading = obj.GetDirection();
		heading[1] = 0;
		if (heading.Length() < 0.01)
			heading = "0 0 1";
		// forward = heading projected onto the slope plane; the engine builds a proper (not mirrored) basis from it
		vector forward = heading - up * vector.Dot(heading, up);
		forward.Normalize();
		vector mat[4];
		Math3D.DirectionAndUpMatrix(forward, up, mat);
		mat[3] = pos;
		obj.SetTransform(mat);
	}

	bool IsFertilized()
	{
		return m_Fertilized;
	}

	bool IsWatered()
	{
		return m_Watered;
	}

	// PF_PreparedGround.FERT_*
	int GetFertilizer()
	{
		if (!m_Fertilized)
			return PF_PreparedGround.FERT_NONE;
		if (m_Compost)
			return PF_PreparedGround.FERT_COMPOST;
		return PF_PreparedGround.FERT_LIME;
	}

	void SetFertilized(bool compost)
	{
		m_Fertilized = true;
		m_Compost = compost;
		SetSynchDirty();
		PF_Log.Debug("planting hole fertilized (compost=" + compost + ") at " + GetPosition().ToString());
	}

	void SetWatered()
	{
		m_Watered = true;
		SetSynchDirty();
		PF_Log.Debug("planting hole watered at " + GetPosition().ToString());
	}

	float GetGrowFactor()
	{
		return PF_PreparedGround.Factor(GetFertilizer(), m_Watered);
	}

	// "Planting Mound (fertilized, watered)"
	override string GetDisplayName()
	{
		string name = super.GetDisplayName();
		string state;
		if (m_Fertilized)
			state = "fertilized";
		if (m_Watered)
		{
			if (state != "")
				state = state + ", ";
			state = state + "watered";
		}
		if (state != "")
			name = name + " (" + state + ")";
		return name;
	}

	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		ctx.Write(STORAGE_VERSION);
		ctx.Write(m_Fertilized);
		ctx.Write(m_Watered);
		ctx.Write(m_Compost);
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;
		int storedVersion;
		if (!ctx.Read(storedVersion) || !ctx.Read(m_Fertilized) || !ctx.Read(m_Watered))
			return false;
		if (storedVersion >= 2 && !ctx.Read(m_Compost))
			return false;
		SetSynchDirty();
		return true;
	}

	override bool IsInventoryVisible() { return false; }
	override bool CanPutInCargo(EntityAI parent) { return false; }
	override bool CanPutIntoHands(EntityAI parent) { return false; }
	override bool CanReceiveItemIntoCargo(EntityAI item) { return false; }
}

// a planting mound or an empty vanilla garden plot
class PF_PreparedGround
{
	static const int FERT_NONE = 0;
	static const int FERT_LIME = 1;		// garden lime
	static const int FERT_COMPOST = 2;	// plant material
	// nearest mound or outdoor garden plot within radius, measured horizontally
	// (tree holograms float above the ground by their model offset)
	static EntityAI FindNear(vector pos, float radius)
	{
		pos[1] = g_Game.SurfaceY(pos[0], pos[2]);
		array<Object> objs = new array<Object>();
		array<CargoBase> proxies = new array<CargoBase>();
		g_Game.GetObjectsAtPosition(pos, radius + 2, objs, proxies);
		EntityAI best;
		float bestDist = radius * radius;
		bool allowPlots = PF_Config.Get().AllowGardenPlot;
		foreach (Object obj : objs)
		{
			EntityAI ground;
			if (PF_PlantingMound.Cast(obj))
				ground = EntityAI.Cast(obj);
			else if (allowPlots && PF_GardenPlot.IsOutdoorPlot(obj))
				ground = EntityAI.Cast(obj);
			if (!ground)
				continue;
			vector groundPos = ground.GetPosition();
			groundPos[1] = pos[1];
			float dist = vector.DistanceSq(groundPos, pos);
			if (dist <= bestDist)
			{
				best = ground;
				bestDist = dist;
			}
		}
		return best;
	}

	// a garden plot must be empty (planting removes it, so it must not destroy someone's crops)
	static bool IsUsable(EntityAI ground)
	{
		GardenPlot plot = GardenPlot.Cast(ground);
		return !plot || PF_GardenPlot.IsEmpty(plot);
	}

	static float GrowFactor(EntityAI ground)
	{
		PF_PlantingMound mound = PF_PlantingMound.Cast(ground);
		if (mound)
			return mound.GetGrowFactor();
		GardenPlot plot = GardenPlot.Cast(ground);
		if (plot)
			return PF_GardenPlot.GrowFactor(plot);
		return 1;
	}

	// fertilizer (FERT_*) and water of a mound or garden plot
	static void GetCare(EntityAI ground, out int fertilizer, out bool watered)
	{
		fertilizer = FERT_NONE;
		watered = false;
		PF_PlantingMound mound = PF_PlantingMound.Cast(ground);
		GardenPlot plot = GardenPlot.Cast(ground);
		if (mound)
		{
			fertilizer = mound.GetFertilizer();
			watered = mound.IsWatered();
		}
		else if (plot)
		{
			PF_GardenPlot.GetCare(plot, fertilizer, watered);
		}
	}

	// config RequireWater / RequireFertilizer: "" = ready for a seed, otherwise the prompt reason.
	// Fertilizer is only required for trees; bushes only need water (user, 2026-10-20).
	static string MissingCare(EntityAI ground, bool tree)
	{
		PF_Config cfg = PF_Config.Get();
		if (!cfg.RequireWater && !cfg.RequireFertilizer)
			return "";
		int fertilizer;
		bool watered;
		GetCare(ground, fertilizer, watered);
		bool needFertilizer = cfg.RequireFertilizer && tree && fertilizer == FERT_NONE;
		bool needWater = cfg.RequireWater && !watered;
		if (needFertilizer && needWater)
			return "Needs fertilizer and water first";
		if (needFertilizer)
			return "Needs fertilizer first";
		if (needWater)
			return "Needs water first";
		return "";
	}

	static float Factor(int fertilizer, bool watered)
	{
		PF_Config cfg = PF_Config.Get();
		float factor = 1;
		if (fertilizer == FERT_LIME)
			factor -= cfg.GardenLimeBonus;
		else if (fertilizer == FERT_COMPOST)
			factor -= cfg.PlantMaterialBonus;
		if (watered)
			factor -= cfg.WaterBonus;
		return Math.Max(factor, 0.1);
	}
}

// ---------------------------------------------------------------- dig (from the seed's placement hologram)

class PF_DigMoundActionData : ActionData
{
	vector m_Position;
}

class PF_DigMoundReciveData : ActionReciveData
{
	vector m_Position;
}

class PF_ActionDigPlantingMoundCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(PF_Config.Get().MoundDigSeconds);
	}
}

// seed in hands, placement hologram out, no prepared ground there, digging tool anywhere on the player:
// dig a planting mound under the hologram. Placement mode stays on, so "Plant" follows right after.
class PF_ActionDigPlantingMound : ActionContinuousBase
{
	void PF_ActionDigPlantingMound()
	{
		m_CallbackClass = PF_ActionDigPlantingMoundCB;
		// the dig animations expect a shovel in hand and never loop with a seed (progress never finished);
		// use the deploy animation like planting does (set per item in SetupAction)
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DEPLOY_2HD;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_HIGH;
		m_Text = "Dig planting hole";
	}

	// shows the player's tree/bush pools
	override void OnActionInfoUpdate(PlayerBase player, ActionTarget target, ItemBase item)
	{
		m_Text = "Dig planting hole" + PF_ActionDigPlantingMound.CountsAt(player, target);
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined();
		m_ConditionTarget = new CCTNone();
	}

	override bool HasTarget()
	{
		return false;
	}

	// keeps placement mode on (vanilla ActionBase.OnStart cancels placing for other actions)
	override bool IsDeploymentAction()
	{
		return true;
	}

	override ActionData CreateActionData()
	{
		return new PF_DigMoundActionData();
	}

	// ground point under the hologram
	static bool HologramGround(PlayerBase player, out vector pos)
	{
		Hologram hologram = player.GetHologramLocal();
		if (!hologram)
			return false;
		pos = hologram.GetProjectionPosition();
		pos[1] = g_Game.SurfaceY(pos[0], pos[2]);
		return true;
	}

	protected static string s_LastDigFail;

	// the shared rules for a new mound (client prompt and server check); seed = dug from the seed hologram
	// (a digging tool must be on the player), null = dug with the tool in hands
	// planted tree near pos (server: registry, client: object search)
	static bool IsTreeWithin(vector pos, float radius)
	{
		if (g_Game.IsServer())
			return PF_Registry.HasTreeWithin(pos, radius);
		return PF_Placement.HasTreeNear(pos, radius);
	}

	// prompt suffix with the player's limits for the spot: inside their bases the base limits, else the outside pools
	static string CountsAt(PlayerBase player, ActionTarget target)
	{
		vector pos = player.GetPosition();
		Hologram hologram = player.GetHologramLocal();
		if (target && target.GetCursorHitPos() != vector.Zero)
			pos = target.GetCursorHitPos();
		else if (hologram)
			pos = hologram.GetProjectionPosition();
		return PF_ClientState.CountsText(PF_Territory.IsOwnBase(player, pos));
	}

	static bool CanDigAt(PlayerBase player, PF_Seed_Base seed, vector pos)
	{
		if (PF_Territory.InSafeZone(player))
			return false;
		PF_Config cfg = PF_Config.Get();
		string fail;
		string surface;
		g_Game.SurfaceGetType(pos[0], pos[2], surface);
		bool isTree = seed && seed.IsTreeSeed();
		// every hole keeps the tree distance (user, 2026-10-19): any hole may get a tree later.
		// 1 m floor so mounds can't overlap; the seed preview snaps to the nearest hole, so SnapDistance may be larger
		float holeSpacing = Math.Max(1, cfg.TreeSpacing);
		if (cfg.DebugLogs)
			PF_Placement.LogSurfaceAt(pos[0], pos[2]);
		if (!cfg.RequirePreparedGround)
			fail = "mounds off";
		else if (seed && !seed.PlayerHasDigTool(player))
			fail = "no digging tool";
		else if (PF_Placement.IsUnderground(player, pos))
			fail = "underground (cave)";	// before the surface check: SurfaceGetType is the terrain above the cave
		else if (PF_PreparedGround.FindNear(pos, holeSpacing))
			fail = "mound/plot within " + holeSpacing + " m";
		else if (IsTreeWithin(pos, cfg.TreeSpacing))
			fail = "planted tree within " + cfg.TreeSpacing + " m";
		else if (PF_Frozen.GroundBlock(pos, surface) != "")
			fail = PF_Frozen.GroundBlock(pos, surface) + ": " + surface;
		else
			fail = PF_Placement.Check(player, pos, isTree);
		if (fail != s_LastDigFail)
		{
			s_LastDigFail = fail;
			PF_Log.Debug("dig mound check at " + pos.ToString() + ": " + fail);
		}
		return fail == "";
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (g_Game.IsDedicatedServer())
			return true;	// checked again in OnFinishProgressServer
		if (!player.IsPlacingLocal())
			return false;
		vector pos;
		if (!HologramGround(player, pos))
			return false;
		return CanDigAt(player, PF_Seed_Base.Cast(item), pos);
	}

	override bool SetupAction(PlayerBase player, ActionTarget target, ItemBase item, out ActionData action_data, Param extra_data = NULL)
	{
		if (!super.SetupAction(player, target, item, action_data, extra_data))
			return false;
		// same choice as vanilla ActionDeployObject.SetupAnimation
		if (item.IsHeavyBehaviour())
			m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DEPLOY_HEAVY;
		else if (item.IsOneHandedBehaviour())
			m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DEPLOY_1HD;
		else
			m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DEPLOY_2HD;
		if (!g_Game.IsDedicatedServer())
		{
			PF_DigMoundActionData data = PF_DigMoundActionData.Cast(action_data);
			vector pos;
			if (!data || !HologramGround(player, pos))
				return false;
			data.m_Position = pos;
		}
		return true;
	}

	override void WriteToContext(ParamsWriteContext ctx, ActionData action_data)
	{
		super.WriteToContext(ctx, action_data);
		PF_DigMoundActionData data = PF_DigMoundActionData.Cast(action_data);
		if (data)
			ctx.Write(data.m_Position);
	}

	override bool ReadFromContext(ParamsReadContext ctx, out ActionReciveData action_recive_data)
	{
		if (!action_recive_data)
			action_recive_data = new PF_DigMoundReciveData;
		super.ReadFromContext(ctx, action_recive_data);
		PF_DigMoundReciveData recive = PF_DigMoundReciveData.Cast(action_recive_data);
		vector pos;
		if (!ctx.Read(pos))
			return false;
		recive.m_Position = pos;
		return true;
	}

	override void HandleReciveData(ActionReciveData action_recive_data, ActionData action_data)
	{
		super.HandleReciveData(action_recive_data, action_data);
		PF_DigMoundActionData data = PF_DigMoundActionData.Cast(action_data);
		PF_DigMoundReciveData recive = PF_DigMoundReciveData.Cast(action_recive_data);
		if (data && recive)
			data.m_Position = recive.m_Position;
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		PF_DigMoundActionData data = PF_DigMoundActionData.Cast(action_data);
		PF_Seed_Base seed = PF_Seed_Base.Cast(action_data.m_MainItem);
		if (!data || !seed)
			return;
		vector pos = data.m_Position;
		pos[1] = g_Game.SurfaceY(pos[0], pos[2]);
		// the client sends the position: must be within reach, and the rules must hold on the server too
		if (vector.Distance(pos, action_data.m_Player.GetPosition()) > 6 || !CanDigAt(action_data.m_Player, seed, pos))
			return;
		g_Game.CreateObjectEx("PF_PlantingMound", pos, ECE_PLACE_ON_SURFACE);
		ItemBase tool = PF_Tools.FindDigTool(action_data.m_Player);
		if (tool)
			MiscGameplayFunctions.DealEvinronmentAdjustedDmg(tool, action_data.m_Player, 4);
		PF_Log.Debug("planting mound dug at " + pos.ToString());
	}
}

// ---------------------------------------------------------------- dig (digging tool in hands)

class PF_ActionDigMoundWithToolCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(PF_Config.Get().MoundDigSeconds);
	}
}

// digging tool in hands (normal mode, not the vanilla garden plot placement), aim at fertile ground:
// dig a planting mound there. Same rules as from the seed hologram.
class PF_ActionDigMoundWithTool : ActionContinuousBase
{
	void PF_ActionDigMoundWithTool()
	{
		m_CallbackClass = PF_ActionDigMoundWithToolCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DIGMANIPULATE;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_HIGH;
		m_Text = "Dig planting hole";
	}

	// shows the player's tree/bush pools
	override void OnActionInfoUpdate(PlayerBase player, ActionTarget target, ItemBase item)
	{
		m_Text = "Dig planting hole" + PF_ActionDigPlantingMound.CountsAt(player, target);
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined();
		m_ConditionTarget = new CCTSurface(UAMaxDistances.DEFAULT);
	}

	override bool HasTarget()
	{
		return true;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!PF_Tools.CanDig(item) || player.IsPlacingLocal() || target.GetObject())
			return false;
		return PF_ActionDigPlantingMound.CanDigAt(player, null, target.GetCursorHitPos());
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		vector pos = action_data.m_Target.GetCursorHitPos();
		pos[1] = g_Game.SurfaceY(pos[0], pos[2]);
		if (!PF_ActionDigPlantingMound.CanDigAt(action_data.m_Player, null, pos))
			return;
		g_Game.CreateObjectEx("PF_PlantingMound", pos, ECE_PLACE_ON_SURFACE);
		MiscGameplayFunctions.DealEvinronmentAdjustedDmg(action_data.m_MainItem, action_data.m_Player, 4);
		PF_Log.Debug("planting mound dug with tool at " + pos.ToString());
	}
}

// ---------------------------------------------------------------- fertilize

class PF_ActionFertilizeMoundCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(PF_Config.Get().FertilizeSeconds);
	}
}

// garden lime or compost in hands, aim at an unfertilized mound. Uses the vanilla amount per garden slot
// (config Horticulture ConsumedQuantity: compost 100, garden lime 250)
class PF_ActionFertilizeMound : ActionContinuousBase
{
	void PF_ActionFertilizeMound()
	{
		m_CallbackClass = PF_ActionFertilizeMoundCB;
		// like vanilla ActionFertilizeSlot; the dig animation never finishes with lime/compost in hands
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_LOW;
		m_Text = "#fertilize_slot";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined();
		m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
	}

	static float ConsumedQuantity(ItemBase item)
	{
		return g_Game.ConfigGetFloat("cfgVehicles " + item.GetType() + " Horticulture ConsumedQuantity");
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		PF_PlantingMound mound = PF_PlantingMound.Cast(target.GetObject());
		if (!mound || mound.IsFertilized() || PF_Territory.InSafeZone(player))
			return false;
		if (!item.IsInherited(GardenLime) && !item.IsInherited(PlantMaterial))
			return false;
		return item.GetQuantity() >= ConsumedQuantity(item);
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		PF_PlantingMound mound = PF_PlantingMound.Cast(action_data.m_Target.GetObject());
		ItemBase item = action_data.m_MainItem;
		if (!mound || mound.IsFertilized() || item.GetQuantity() < ConsumedQuantity(item))
			return;
		item.AddQuantity(-ConsumedQuantity(item));
		mound.SetFertilized(item.IsInherited(PlantMaterial));
	}
}

// ---------------------------------------------------------------- water

class PF_ActionWaterMoundCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(PF_Config.Get().WaterSeconds);
	}
}

// container with (unfrozen) water in hands, aim at an unwatered mound
class PF_ActionWaterMound : ActionContinuousBase
{
	void PF_ActionWaterMound()
	{
		m_CallbackClass = PF_ActionWaterMoundCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_EMPTY_VESSEL;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_LOW;
		m_Text = "#water_slot";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined();
		m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		PF_PlantingMound mound = PF_PlantingMound.Cast(target.GetObject());
		if (!mound || mound.IsWatered() || PF_Territory.InSafeZone(player))
			return false;
		if (item.GetLiquidType() != LIQUID_WATER || item.GetIsFrozen())
			return false;
		return item.GetQuantity() >= PF_Config.Get().WaterAmountMl;
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		PF_PlantingMound mound = PF_PlantingMound.Cast(action_data.m_Target.GetObject());
		ItemBase item = action_data.m_MainItem;
		float amount = PF_Config.Get().WaterAmountMl;
		if (!mound || mound.IsWatered() || item.GetQuantity() < amount)
			return;
		item.AddQuantity(-amount);
		mound.SetWatered();
	}
}
