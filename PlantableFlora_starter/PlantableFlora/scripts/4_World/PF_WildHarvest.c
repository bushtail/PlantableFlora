// Cutting a wild map bush (vanilla ActionMineBush) can yield a PlantableFlora seed.
class PF_WildHarvest
{
	// wild bush model -> seed class (config CfgPlantableFlora Wild, source "bush")
	static string SeedFor(string shapeName)
	{
		return PF_Species.BushSeedFor(shapeName);
	}

	static string TreeSeedFor(string shapeName)
	{
		return PF_Species.TreeSeedFor(shapeName);
	}

	static string FruitFor(string shapeName)
	{
		return PF_Species.FruitFor(shapeName);
	}

	// server: "<player id>:<tree position>" of every tree a player already searched (in memory only, resets on restart).
	// Wild trees are stateless map objects, so the tree is identified by its position.
	protected static ref map<string, bool> s_Searched = new map<string, bool>();

	static void SearchTree(PlayerBase player, Object tree)
	{
		if (!g_Game.IsServer() || !player || !tree)
			return;
		PF_Bush_Base planted = PF_Bush_Base.Cast(tree);
		if (!planted && !tree.IsTree())
			return;

		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return;

		// wild fruit trees give fruit, the other wild trees give seeds
		string fruitClass = FruitFor(tree.GetShapeName());
		string seedClass = TreeSeedFor(tree.GetShapeName());
		string itemClass = seedClass;
		if (fruitClass != "")
			itemClass = fruitClass;
		if (itemClass == "")
			return;

		vector treePos = tree.GetPosition();
		string key = identity.GetId() + ":" + Math.Round(treePos[0] * 10) + "_" + Math.Round(treePos[2] * 10);
		if (s_Searched.Contains(key))
		{
			PF_Notify.Send(player, "You already searched this tree");
			return;
		}
		s_Searched.Set(key, true);

		PF_Config cfg = PF_Config.Get();
		// planted trees bear like wild ones once they are mature (user 2026-10-20; before: never anything);
		// seedling and middle stage have nothing yet
		bool bears = !planted || planted.GetNextStageConfig() == "";
		// seeds: 0-2 (RollSeeds); fruit: the chance, then 1 + up to FruitMaxPerSearch
		int count = 0;
		if (bears && fruitClass != "" && Math.RandomFloat01() < cfg.FruitSearchChance)
			count = 1 + Math.RandomInt(0, cfg.FruitMaxPerSearch);
		else if (bears && fruitClass == "")
			count = RollSeeds();
		bool found = count > 0;
		PF_Log.Debug("tree search: " + tree.GetShapeName() + " item=" + itemClass + " found=" + found);
		if (!found)
		{
			PF_Notify.Send(player, "You found nothing");
			return;
		}

		string itemName;
		for (int i = 0; i < count; i++)
		{
			vector pos = player.GetPosition() + player.GetDirection() * 0.7 + Vector(Math.RandomFloat(-0.2, 0.2), 0, Math.RandomFloat(-0.2, 0.2));
			EntityAI found_item = EntityAI.Cast(g_Game.CreateObjectEx(itemClass, pos, ECE_PLACE_ON_SURFACE));
			if (found_item)
				itemName = found_item.GetDisplayName();
		}
		if (fruitClass != "")
			PF_Notify.Send(player, "You found: " + itemName, "Cut it with a knife to get seeds");
		else
			PF_Notify.Send(player, FoundSeedsText(count, itemName));
	}

	static const int MAX_SEEDS = 2;

	// number of seeds, 0-2 (user 2026-10-22: none 50 %, 1 seed 40 %, 2 seeds 10 %; config SeedOneChance / SeedTwoChance)
	static int RollSeeds()
	{
		PF_Config cfg = PF_Config.Get();
		float roll = Math.RandomFloat01();
		if (roll < cfg.SeedTwoChance)
			return 2;
		if (roll < cfg.SeedTwoChance + cfg.SeedOneChance)
			return 1;
		return 0;
	}

	// "You found 2 seeds (Tree Seed)"
	static string FoundSeedsText(int count, string name)
	{
		if (count == 1)
			return "You found 1 seed (" + name + ")";
		return "You found " + count + " seeds (" + name + ")";
	}

	static void OnBushCut(ActionData action_data)
	{
		if (!g_Game.IsServer())
			return;

		Object bush = action_data.m_Target.GetObject();
		PlayerBase player = action_data.m_Player;
		if (!bush || !player || !bush.IsBush() || PF_Bush_Base.Cast(bush))
			return;

		// a completed cut can end as UA_FINISHED or UA_CANCEL (player still holding the button); count it as done when the bush is used up
		PF_Log.Debug("wild bush action ended, state=" + action_data.m_State + " destroyed=" + bush.IsDamageDestroyed());
		if (action_data.m_State != UA_FINISHED && !bush.IsDamageDestroyed())
			return;

		string shape = bush.GetShapeName();
		string seedClass = SeedFor(shape);
		PF_Log.Debug("wild bush cut: " + shape + " seed=" + seedClass);
		if (seedClass == "")
			return;

		int count = RollSeeds();
		if (count == 0)
			return;

		// the seeds drop on the ground in front of the player (like the tree search), the player has to pick them up.
		// Not at the bush: its origin sits ~2 m up in the crown of big bushes, 2-3 m away, the seeds were never found
		string seedName;
		vector pos;
		for (int i = 0; i < count; i++)
		{
			pos = player.GetPosition() + player.GetDirection() * 0.7 + Vector(Math.RandomFloat(-0.2, 0.2), 0, Math.RandomFloat(-0.2, 0.2));
			EntityAI seed = EntityAI.Cast(g_Game.CreateObjectEx(seedClass, pos, ECE_PLACE_ON_SURFACE));
			if (seed)
				seedName = seed.GetDisplayName();
		}
		PF_Log.Debug("seeds dropped: " + count + " x " + seedClass + " at " + pos);
		if (seedName != "")
			PF_Notify.Send(player, FoundSeedsText(count, seedName));
	}
}

modded class ActionMineBush
{
	override void OnEndServer(ActionData action_data)
	{
		super.OnEndServer(action_data);
		PF_WildHarvest.OnBushCut(action_data);
	}
}

class PF_ActionSearchTreeSeedCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(PF_Config.Get().SearchSeedSeconds);
	}
}

// bare hands on a wild spruce, oak or pine: chance for a seed, each tree once per player (server)
class PF_ActionSearchTreeSeed : ActionContinuousBase
{
	void PF_ActionSearchTreeSeed()
	{
		m_CallbackClass = PF_ActionSearchTreeSeedCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_LOW;
		m_Text = "Search for seeds";
	}

	override void CreateConditionComponents()
	{
		m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
		m_ConditionItem = new CCINotPresent;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		Object tree = target.GetObject();
		if (!tree)
			return false;
		// planted trees offer the search too (mature ones find seeds/fruit like wild trees; a missing prompt would give them away)
		PF_Bush_Base planted = PF_Bush_Base.Cast(tree);
		if (planted)
		{
			if (planted.IsProjection() || !planted.IsTreePlant())
				return false;
		}
		else if (!tree.IsTree())
			return false;
		string shape = tree.GetShapeName();
		return PF_WildHarvest.TreeSeedFor(shape) != "" || PF_WildHarvest.FruitFor(shape) != "";
	}

	// prompt text: fruit trees say "fruit", the others "seeds"
	override void OnActionInfoUpdate(PlayerBase player, ActionTarget target, ItemBase item)
	{
		Object tree = target.GetObject();
		if (tree && PF_WildHarvest.FruitFor(tree.GetShapeName()) != "")
			m_Text = "Search for fruit";
		else
			m_Text = "Search for seeds";
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		PF_WildHarvest.SearchTree(action_data.m_Player, action_data.m_Target.GetObject());
	}
}

modded class PlayerBase
{
	override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);
		AddAction(PF_ActionSearchTreeSeed, InputActionMap);
	}
}
