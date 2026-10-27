modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		PF_Config.Get();	// load (or create) config.json at start
		if (PF_Territory.UsesExpansion())
			Print("[PlantableFlora] territories: Expansion BaseBuilding");
		else
			Print("[PlantableFlora] territories: own (territory.json)");
		PF_StressTest.Start();	// DEV: only acts when config StressTestCount > 0 or test plants exist
		PF_Wipe.Start();		// admin: only acts when config WipeAllPlants is true
		PF_Stats.Start();		// admin: stats.txt every StatsMinutes
		PF_DevTune.Start();		// DEV: live ground offset overrides (DebugLogs only)
		PF_MapSurvey.StartIfRequested();	// DEV: map survey (docs/maps.md), only with survey_request.txt
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		PF_Perf.OnFrame(timeslice);
	}

	// same place vanilla syncs cfggameplay.json: before the player entity exists
	override void OnClientPrepareEvent(PlayerIdentity identity, out bool useDB, out vector pos, out float yaw, out int preloadTimeout)
	{
		super.OnClientPrepareEvent(identity, useDB, pos, yaw, preloadTimeout);
		PF_Config.SendTo(identity);
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);
		if (identity)
		{
			PF_Registry.NotifyOwner(identity.GetId());
			PF_Territory.SendTo(identity);
			PF_Frozen.SendTo(identity);
		}
		PF_DevTune.OnPlayerConnected(player);	// DEV: TreeLineup
	}
}
