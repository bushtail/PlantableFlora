class PF_Seed_Base extends ItemBase
{
	protected int m_ToolCheckTime;
	protected bool m_ToolCheckResult;

	override void SetActions()
	{
		super.SetActions();
		AddAction(PF_ActionTogglePlaceSeed);
		AddAction(PF_ActionPlantSeed);
		// no "Dig planting mound" from the seed preview (user, 2026-10-19: confusing); holes are dug with a tool in hands
	}

	// needed so ActionDeployObject picks the deploy animation
	override bool IsDeployable()
	{
		return true;
	}

	override bool CanMakeGardenplot()
	{
		return false;
	}

	// the class planted at pos: pf_plantClass, or its snowy/frozen variant there (PF_Climate); "" = too cold
	string GetPlantClass(vector pos)
	{
		return PF_Climate.PlantClassFor(GetType(), pos);
	}

	bool IsTreeSeed()
	{
		int isTree = g_Game.ConfigGetInt("CfgVehicles " + GetType() + " pf_isTree");
		return isTree == 1;
	}

	// config ShowSeedSpecies false: hide the species (inventory, inspect, hands, "You found a seed" message)
	override string GetDisplayName()
	{
		if (PF_Config.Get().ShowSeedSpecies)
			return super.GetDisplayName();
		if (IsTreeSeed())
			return "Tree Seed";
		return "Bush Seed";
	}

	// any digging tool (shovel, hoe, pickaxe) anywhere on the player; cached, called every frame while placing
	bool PlayerHasDigTool(Man player)
	{
		int now = g_Game.GetTime();
		if (now - m_ToolCheckTime < 1000)
			return m_ToolCheckResult;

		m_ToolCheckTime = now;
		m_ToolCheckResult = false;

		array<EntityAI> items = new array<EntityAI>();
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI e : items)
		{
			ItemBase ib = ItemBase.Cast(e);
			if (ib && ib.CanMakeGardenplot())
			{
				m_ToolCheckResult = true;
				break;
			}
		}
		return m_ToolCheckResult;
	}

	protected string m_LastReason;
	protected vector m_CachePos;
	protected int m_CacheTime;
	protected string m_CacheReason;

	protected string m_BlockReason;		// player-facing reason of the last CanBePlaced, "" when allowed

	// shown in the placement prompt (PF_ActionTogglePlaceSeed)
	string GetBlockReason()
	{
		return m_BlockReason;
	}

	protected bool PlaceResult(Man player, bool ok, string reason)
	{
		// silent reasons (PF_Placement.SILENT) are never shown: they would let players probe for hidden bases/stashes
		bool silent = reason.IndexOf(PF_Placement.SILENT) == 0;
		if (ok || silent)
			m_BlockReason = "";
		else
			m_BlockReason = reason;
		if (reason != m_LastReason)
		{
			m_LastReason = reason;
			PF_Log.Debug("CanBePlaced " + ok + " (" + reason + ") server=" + g_Game.IsServer());
			// the server is the authority (caps are only known there): tell the player why it failed
			PlayerBase pb = PlayerBase.Cast(player);
			if (!ok && !silent && g_Game.IsServer() && pb)
				PF_Notify.Send(pb, "Can't plant here", reason);
		}
		return ok;
	}

	// vanilla EvaluateCollision checks that apply to plant projections (the others are switched off in PF_Hologram)
	protected string HologramBlock(PlayerBase pb, Hologram hologram, PF_Bush_Base projection)
	{
		if (!pb.CanPlaceItem(projection))
			return "Can't plant right now";
		if (hologram.IsCollidingPlayer())
			return "Step off the planting spot";
		if (!hologram.HeightPlacementCheck())
			return "Too far above or below you";
		if (hologram.IsUnderwater())
			return "Under water";
		if (hologram.IsFloating() || hologram.IsHidden() || hologram.IsCollidingZeroPos())
			return "Aim at the ground";
		return "";
	}

	// called every frame while placing (hologram + prompt): cheap checks first, the world queries are cached
	override bool CanBePlaced(Man player, vector position)
	{
		// a seed from another map (Badlands seed elsewhere, Chernarus seed on Badlands; PF_World)
		if (!PF_World.IsNativeClass(GetType()))
			return PlaceResult(player, false, "This seed won't grow here");
		// snowy/frozen surroundings and no snowy model for this species (PF_Climate, cached per 10 m cell)
		if (GetPlantClass(position) == "")
			return PlaceResult(player, false, "Too cold here");
		PF_Config cfg = PF_Config.Get();
		// the projection origin sits above the terrain by the model's ground offset (trees: half their height)
		PlayerBase pb = PlayerBase.Cast(player);
		Hologram hologram;
		if (pb && g_Game.IsServer())
			hologram = pb.GetHologramServer();
		else if (pb)
			hologram = pb.GetHologramLocal();
		PF_Bush_Base projection;
		if (hologram && Class.CastTo(projection, hologram.GetProjectionEntity()))
			position[1] = position[1] - projection.GetGroundOffset();
		// Expansion CanBuildAnywhere (default on) skips Hologram.EvaluateCollision, only CanBePlaced still runs:
		// repeat the vanilla hologram checks that matter for plants (client only, like vanilla)
		if (projection && !g_Game.IsDedicatedServer())
		{
			string holoBlock = HologramBlock(pb, hologram, projection);
			if (holoBlock != "")
				return PlaceResult(player, false, holoBlock);
		}
		if (PF_Config.Get().DebugLogs)
			PF_Placement.LogSurfaceAt(position[0], position[2]);

		// a planting mound was checked against all placement rules when it was dug (PF_ActionDigPlantingMound.CanDigAt),
		// so planting onto it only checks what depends on the seed. Garden plots are dug without our rules: full check.
		bool onMound;
		if (cfg.RequirePreparedGround)
		{
			// the hologram snaps onto a mound/plot (PF_Hologram), so prepared ground must be right here
			EntityAI ground = PF_PreparedGround.FindNear(position, 0.5);
			// frozen ground: the hole can't be dug yet, say why instead of asking for a hole
			if (!ground && PF_Frozen.IsFrozenUnthawed(position))
				return PlaceResult(player, false, "Frozen ground: make a fire here first");
			if (!ground)
				return PlaceResult(player, false, "Needs a planting hole (dig one with a shovel) or an empty garden plot");
			if (!PF_PreparedGround.IsUsable(ground))
				return PlaceResult(player, false, "The garden plot must be empty");
			string care = PF_PreparedGround.MissingCare(ground, IsTreeSeed());
			if (care != "")
				return PlaceResult(player, false, care);
			onMound = PF_PlantingMound.Cast(ground) != null;
		}
		else if (!PlayerHasDigTool(player))
			return PlaceResult(player, false, "Needs a digging tool");

		string surfaceType;
		if (!onMound)
		{
			float surfaceHeight = g_Game.SurfaceGetType3D(position[0], position[1], position[2], surfaceType);
			if (Math.AbsFloat(position[1] - surfaceHeight) > 0.3)
				return PlaceResult(player, false, "Must be planted in the ground");

			string groundBlock = PF_Frozen.GroundBlock(position, surfaceType);
			if (groundBlock != "")
				return PlaceResult(player, false, groundBlock);
		}

		// a planting mound already kept TreeSpacing to holes and trees when it was dug (CanDigAt);
		// garden plots (dug with vanilla rules) and planting without prepared ground still need the check
		if (IsTreeSeed() && !onMound)
		{
			bool treeNear;
			if (g_Game.IsServer())
				treeNear = PF_Registry.HasTreeWithin(position, cfg.TreeSpacing);
			else
				treeNear = PF_Placement.HasTreeNear(position, cfg.TreeSpacing);
			if (treeNear)
				return PlaceResult(player, false, "Too close to another tree (min " + cfg.TreeSpacing + " m)");
		}

		// caps (PF_Caps): the server is the authority. The player pool never blocks (the oldest plant goes when
		// the new one is grown, the prompt warns); the server cap only blocks when every plant sits in an active base.
		if (g_Game.IsServer())
		{
			if (PF_Registry.CountAll() >= cfg.MaxPlantsPerServer && !PF_Caps.CanFreeServerSlot())
				return PlaceResult(player, false, "Server plant limit reached");

			int kind = PF_Caps.KIND_BUSH;
			if (IsTreeSeed())
				kind = PF_Caps.KIND_TREE;
			string baseReason = PF_Caps.CheckBase(position, kind);
			if (baseReason == "")
				baseReason = PF_Caps.CheckPlayerBase(player, position, kind);
			if (baseReason != "")
				return PlaceResult(player, false, baseReason);
		}

		if (onMound)
		{
			// a mound dug with a tool in hands doesn't know whether a tree goes in: stash rule here (silent)
			if (IsTreeSeed() && PF_Placement.IsNearStash(position))
				return PlaceResult(player, false, PF_Placement.SILENT + "stash");
			return PlaceResult(player, true, "ok on mound");
		}

		int now = g_Game.GetTime();
		if (vector.DistanceSq(position, m_CachePos) > 0.01 || now - m_CacheTime > 500)
		{
			m_CachePos = position;
			m_CacheTime = now;
			m_CacheReason = PF_Placement.Check(player, position, IsTreeSeed());
		}
		if (m_CacheReason != "")
			return PlaceResult(player, false, m_CacheReason);

		return PlaceResult(player, true, "ok on " + surfaceType);
	}

	override void OnPlacementComplete(Man player, vector position = "0 0 0", vector orientation = "0 0 0")
	{
		super.OnPlacementComplete(player, position, orientation);

		if (!g_Game.IsServer())
			return;

		string plantClass = GetPlantClass(position);
		if (plantClass == "")
			return;	// too cold (CanBePlaced refused it already)

		// the mound/plot is used up; fertilizer and water in it speed up every growth step of this plant
		float growFactor = 1;
		int soilFertilizer = PF_PreparedGround.FERT_NONE;	// the seedling's soil heap keeps the hole's look
		bool soilWatered;
		if (PF_Config.Get().RequirePreparedGround)
		{
			EntityAI ground = PF_PreparedGround.FindNear(position, PF_Config.Get().SnapDistance);
			if (ground)
			{
				growFactor = PF_PreparedGround.GrowFactor(ground);
				PF_PreparedGround.GetCare(ground, soilFertilizer, soilWatered);
				g_Game.ObjectDelete(ground);
			}
		}

		// server cap reached: the oldest plant outside every active base makes room
		PF_Caps.FreeServerSlot();

		// keep the hologram height (terrain + model ground offset): a surface trace would land on top of the
		// planting hole, which is only deleted at the end of the frame (plant floated ~15-20 cm)
		PF_Bush_Base plant = PF_Bush_Base.Cast(g_Game.CreateObjectEx(plantClass, position, ECE_CREATEPHYSICS | ECE_UPDATEPATHGRAPH | ECE_KEEPHEIGHT | ECE_NOSURFACEALIGN));
		if (plant)
		{
			plant.SetGrowFactor(growFactor);
			plant.SetSoil(soilFertilizer, soilWatered);
			// random heading (user 2026-10-23): with the player's heading, plants of one species planted facing the same
			// way all grew into the identical tree; the seedling is round, so nobody sees the difference at planting
			plant.SetOrientation(Vector(Math.RandomFloat(0, 360), 0, 0));
			// seedling: tilt with the slope like the hole (SnapToGround aligns it for pf_alignToTerrain)
			plant.SnapToGround();
			PlayerIdentity identity = player.GetIdentity();
			if (identity)
				plant.SetOwnerId(identity.GetId());
			PF_Log.Debug("planted " + plantClass + " at " + position.ToString());
		}

		// consumed; deferred so the action can finish with the item first
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Delete, 200, false);
	}
}

// seed species (PF_Seed_*) are config-only and use this script class
