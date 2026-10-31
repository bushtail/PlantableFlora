// Felled wild map trees: TreeHard / TreeSoft are engine classes (cannot be modded), so the cut is caught in the
// vanilla chop component (CAContinuousMineWood.Execute calls OnTreeCutDown when the tree's health reaches 0).
modded class CAContinuousMineWood
{
	override int Execute(ActionData action_data)
	{
		Object target = action_data.m_Target.GetObject();
		bool wasDestroyed = false;
		if (target)
			wasDestroyed = target.IsDamageDestroyed();
		int result = super.Execute(action_data);
		if (g_Game.IsServer() && target && !wasDestroyed && target.IsDamageDestroyed() && target.IsTree() && !PF_Bush_Base.Cast(target))
			PF_FallenColliders.OnCut(target);
		return result;
	}
}

// the hidden collider copy of a felled wild tree (PF_FallenColliders) is a vanilla tree object: no "Gather wood",
// "Harvest", seed search or any other action on it
modded class ActionBase
{
	override bool Can(PlayerBase player, ActionTarget target, ItemBase item, int condition_mask)
	{
		if (target && PF_FallenColliders.IsCollider(target.GetObject()))
			return false;
		return super.Can(player, target, item, condition_mask);
	}
}
