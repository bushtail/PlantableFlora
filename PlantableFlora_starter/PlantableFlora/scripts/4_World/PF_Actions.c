class PF_Tools
{
	protected static ref array<string> s_CutTools;

	// knives, machetes, axes: same set that can cut wild bushes
	static bool CanCutBush(ItemBase item)
	{
		if (!item)
			return false;

		if (!s_CutTools)
		{
			s_CutTools = {"KitchenKnife", "SteakKnife", "HuntingKnife", "CombatKnife", "BoneKnife", "FangeKnife", "KukriKnife", "StoneKnife", "Cleaver", "Machete", "CrudeMachete", "OrientalMachete", "Sickle", "Hatchet", "WoodAxe", "FirefighterAxe", "Sword"};
		}
		foreach (string toolClass : s_CutTools)
		{
			if (item.IsKindOf(toolClass))
				return true;
		}
		return false;
	}

	static bool CanFell(ItemBase item)
	{
		return item && (item.IsKindOf("Hatchet") || item.IsKindOf("WoodAxe") || item.IsKindOf("FirefighterAxe"));
	}

	static bool CanDig(ItemBase item)
	{
		return item && item.CanMakeGardenplot();
	}

	// first digging tool anywhere on the player (for tool wear)
	static ItemBase FindDigTool(PlayerBase player)
	{
		array<EntityAI> items = new array<EntityAI>();
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI e : items)
		{
			ItemBase ib = ItemBase.Cast(e);
			if (ib && ib.CanMakeGardenplot())
				return ib;
		}
		return null;
	}
}

class PF_ActionCutBushCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(PF_Config.Get().CutSeconds);
	}
}

class PF_ActionCutBush : ActionContinuousBase
{
	void PF_ActionCutBush()
	{
		m_CallbackClass = PF_ActionCutBushCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_HACKBUSH;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_HIGH;
		m_Text = "Cut bush";
	}

	// same prompt as vanilla ActionMineBush on wild bushes ("Harvest <first yield>"), so planted bushes can't be told apart
	override void OnActionInfoUpdate(PlayerBase player, ActionTarget target, ItemBase item)
	{
		m_Text = "#harvest" + " " + MiscGameplayFunctions.GetItemDisplayName("LongWoodenStick");
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined();
		m_ConditionTarget = new CCTCursor(UAMaxDistances.SMALL);
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		PF_Bush_Base bush = PF_Bush_Base.Cast(target.GetObject());
		if (!bush || PF_Territory.InSafeZone(player))
			return false;

		bool canCut = bush.CanBeCut();
		bool tool = PF_Tools.CanCutBush(item);
		string state = "target=" + bush.GetType() + " canBeCut=" + canCut + " tool=" + tool + " item=" + item.GetType();
		if (state != m_LastState)
		{
			m_LastState = state;
			PF_Log.Debug("CutBush condition: " + state);
		}
		return canCut && tool;
	}

	protected string m_LastState;

	override void OnFinishProgressServer(ActionData action_data)
	{
		PF_Bush_Base bush = PF_Bush_Base.Cast(action_data.m_Target.GetObject());
		if (!bush || !bush.CanBeCut())
			return;

		bush.Cut();
		MiscGameplayFunctions.DealAbsoluteDmg(action_data.m_MainItem, 5);
	}
}

class PF_ActionDigUpPlantCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(PF_Config.Get().DigUpSeconds);
	}
}

class PF_ActionDigUpPlant : ActionContinuousBase
{
	void PF_ActionDigUpPlant()
	{
		m_CallbackClass = PF_ActionDigUpPlantCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DIGMANIPULATE;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_LOW;
		m_Text = "Dig up";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined();
		m_ConditionTarget = new CCTCursorNoRuinCheck();
	}

	// also fills in an unused planting mound
	override void OnActionInfoUpdate(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (PF_PlantingMound.Cast(target.GetObject()))
			m_Text = "Fill in planting hole";
		else
			m_Text = "Dig up";
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (player.GetBrokenLegs() == eBrokenLegs.BROKEN_LEGS || !PF_Tools.CanDig(item) || PF_Territory.InSafeZone(player))
			return false;

		if (PF_PlantingMound.Cast(target.GetObject()))
			return true;
		PF_Bush_Base plant = PF_Bush_Base.Cast(target.GetObject());
		return plant && plant.CanBeDugUp();
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		PF_PlantingMound mound = PF_PlantingMound.Cast(action_data.m_Target.GetObject());
		if (mound)
		{
			PF_Log.Debug("filled in mound at " + mound.GetPosition().ToString());
			mound.Delete();
			MiscGameplayFunctions.DealAbsoluteDmg(action_data.m_MainItem, 2);
			return;
		}

		PF_Bush_Base plant = PF_Bush_Base.Cast(action_data.m_Target.GetObject());
		if (!plant || !plant.CanBeDugUp())
			return;

		PF_Log.Debug("dug up " + plant.GetType() + " at " + plant.GetPosition().ToString());
		plant.Delete();
		MiscGameplayFunctions.DealAbsoluteDmg(action_data.m_MainItem, 10);
	}
}

class PF_ActionFellTreeCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(PF_Config.Get().FellSeconds);
	}
}

class PF_ActionFellTree : ActionContinuousBase
{
	void PF_ActionFellTree()
	{
		m_CallbackClass = PF_ActionFellTreeCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_HACKTREE;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_HIGH;
		m_Text = "Fell tree";
	}

	// same prompt as vanilla ActionMineTree on wild trees ("Harvest <first yield>")
	override void OnActionInfoUpdate(PlayerBase player, ActionTarget target, ItemBase item)
	{
		m_Text = "#harvest" + " " + MiscGameplayFunctions.GetItemDisplayName("WoodenLog");
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined();
		m_ConditionTarget = new CCTCursor(UAMaxDistances.SMALL);
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		PF_Bush_Base tree = PF_Bush_Base.Cast(target.GetObject());
		return tree && tree.CanBeFelled() && PF_Tools.CanFell(item) && !PF_Territory.InSafeZone(player);
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		PF_Bush_Base tree = PF_Bush_Base.Cast(action_data.m_Target.GetObject());
		if (!tree || !tree.CanBeFelled())
			return;

		tree.Fell();
		MiscGameplayFunctions.DealAbsoluteDmg(action_data.m_MainItem, 20);
	}
}

// cut and dig actions are decided in their ActionCondition (target must be a planted bush), so every item gets them
modded class ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(PF_ActionCutBush);
		AddAction(PF_ActionDigUpPlant);
		AddAction(PF_ActionFellTree);
		AddAction(PF_ActionDigMoundWithTool);
		AddAction(PF_ActionFertilizeMound);
		AddAction(PF_ActionWaterMound);
	}
}
