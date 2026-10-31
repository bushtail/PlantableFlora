// Client: receives the synced config. DayZGame lives in the 3_Game module, so this must too (as must PF_Config).
modded class DayZGame
{
	override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
	{
		if (!target && rpc_type == PF_RPC.SYNC_CONFIG)
		{
			Param1<PF_Config> data = new Param1<PF_Config>(null);
			if (ctx.Read(data) && data.param1)
				PF_Config.Set(data.param1);
			else
				ErrorEx("[PlantableFlora] failed to read config from server");
			return;
		}

		if (!target && rpc_type == PF_RPC.SYNC_COUNT)
		{
			Param4<int, int, int, int> count = new Param4<int, int, int, int>(0, 0, 0, 0);
			if (ctx.Read(count))
			{
				PF_ClientState.OwnTrees = count.param1;
				PF_ClientState.OwnBushes = count.param2;
				PF_ClientState.OwnTreesInBase = count.param3;
				PF_ClientState.OwnBushesInBase = count.param4;
			}
			return;
		}

		if (!target && rpc_type == PF_RPC.SYNC_FLAGS)
		{
			Param1<ref array<string>> flags = new Param1<ref array<string>>(null);
			if (ctx.Read(flags) && flags.param1)
				PF_ClientState.MyFlags = flags.param1;
			return;
		}

		if (!target && rpc_type == PF_RPC.SYNC_THAW)
		{
			Param1<ref array<vector>> spots = new Param1<ref array<vector>>(null);
			if (ctx.Read(spots) && spots.param1)
				PF_ClientState.ThawSpots = spots.param1;
			return;
		}

		if (!target && rpc_type == PF_RPC.SYNC_FALLEN)
		{
			Param2<ref array<string>, ref array<vector>> fallen = new Param2<ref array<string>, ref array<vector>>(null, null);
			if (ctx.Read(fallen) && fallen.param1 && fallen.param2)
				PF_FallenColliders.OnReceive(fallen.param1, fallen.param2);
			return;
		}

		if (!target && rpc_type == PF_RPC.SHADE_INFO)
		{
			Param1<string> shade = new Param1<string>("");
			if (ctx.Read(shade))
				PF_ClientState.ShadeInfo = shade.param1;
			return;
		}

		super.OnRPC(sender, target, rpc_type, ctx);
	}
}
