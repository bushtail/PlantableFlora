// Own RPC ids (far above vanilla's range)
class PF_RPC
{
	static const int SYNC_CONFIG = 714310101;	// server -> client: config
	static const int SYNC_COUNT = 714310102;	// server -> owner client: how many trees and bushes they own (outside / inside their bases)
	static const int SYNC_FLAGS = 714310103;	// server -> client: territory flags the player is a member of
	static const int SHADE_INFO = 714310104;	// server -> client: shade debug text (config ShadeDebug), every 2 s
	static const int SYNC_THAW = 714310105;	// server -> clients: positions of thawed ground on winter maps (PF_Frozen)
	static const int SYNC_FALLEN = 714310106;	// server -> clients: hidden colliders of felled wild trees (PF_FallenColliders)
}

// Admin zone without planting (traders, safe zones, events): world position (Y is ignored) + radius in m
class PF_NoPlantZone
{
	float X;
	float Y;
	float Z;
	float Radius;
}

// DEV: extra ground offset for one plant class (m, + raises), for dialing in tree heights live (PF_DevTune)
class PF_OffsetOverride
{
	string PlantClass;	// class name ("Class" and "Plant" are type names, not allowed as field names)
	float Offset;
}

// Server settings: $profile:PlantableFlora/config.json (created with defaults, rewritten on start so new keys appear).
// Synced to every client on connect, because placement rules are also evaluated client-side (hologram, prompt).
class PF_Config
{
	static const string DIR = "$profile:PlantableFlora";
	static const string PATH = "$profile:PlantableFlora/config.json";

	// growth (seconds, wall-clock)
	int BushGrowSeconds = 3600;		// young bush -> mature bush
	int TreeGrowSeconds = 43200;	// young tree -> mature tree
	int BushRegrowSeconds = 3600;	// cut bush -> mature bush again
	int GrowthTickSeconds = 60;		// how often the server checks all planted flora
	float PlantLifetimeDays = 45;	// Central Economy lifetime of planted flora: a plant disappears after this unless an active territory flag within 60 m refreshes it (like base building)

	// actions (seconds)
	float PlantSeconds = 3;
	float CutSeconds = 5;
	float DigUpSeconds = 20;
	float FellSeconds = 45;			// chopping a planted tree, all cycles together
	int FellLogs = 3;				// wooden logs from a felled planted tree, one per chopping cycle; only for models without a vanilla wild tree entry (Badlands), others give what the same wild tree gives
	int FellSticks = 4;				// long wooden sticks, dropped when such a tree falls
	float FallenTreeMinutes = 0;	// a felled tree lies on the ground this long; 0 = until the next server restart (vanilla)
	bool FallenTreeCollision = false;	// a felled tree lying on the ground blocks players and vehicles, planted and wild ones (vanilla: no collision)
	float SearchSeedSeconds = 6;

	// ground preparation: seeds are planted onto a planting mound (dug from the seed hologram) or an empty garden plot
	bool RequirePreparedGround = true;	// false: plant anywhere with a digging tool on you (no mound/plot needed)
	bool AllowGardenPlot = true;	// an empty vanilla garden plot also counts as prepared ground (removed when planting)
	float SnapDistance = 2;			// m, the seed hologram snaps to a mound/plot this close
	float MoundDigSeconds = 8;
	float MoundLifetimeMinutes = 60;	// an unused mound disappears after this (Central Economy lifetime)
	float FertilizeSeconds = 4;
	float WaterSeconds = 4;
	float WaterAmountMl = 500;		// water used per mound
	float RainWaterMinutes = 10;	// an unwatered planting hole counts as watered after this many minutes of rain (any rain above the vanilla garden threshold); 0 = off
	bool RequireWater = true;		// a mound/plot must be watered before a seed can be planted
	bool RequireFertilizer = false;	// same for fertilizer, trees only (bushes only need water)
	float GardenLimeBonus = 0.3;	// mound/plot fertilized with garden lime: every growth step takes this much less time
	float PlantMaterialBonus = 0.15;	// same for plant material (compost), weaker than lime
	float WaterBonus = 0;			// watered mound/plot slot: same, adds up with fertilizer (0 by default: water is required, grow times stay exact)
	float PlotSlopeTolerance = 1.0;	// m, garden plot corners may differ this much in height from the centre (vanilla 0.3)
	float PlotMaxTiltDegrees = 25;	// garden plot hologram tilt limit (vanilla 10)

	// placement rules
	float BuildingClearance = 3;	// m, free box around the plant: houses, base objects, vehicles, players
	float RoadClearance = 5;		// m, no road (road piece or road surface) within this distance
	float TreeRootSinkPercent = 6;	// % of the tree model height sunk below the bottom of its bounding box, so the roots are in the ground (vanilla map trees: ~3-10 %)
	float TreeGroundOffset = 0;		// m, extra fine tuning on top: + raises, - sinks all planted trees
	float TreeSlopeSampleRadius = 1;	// m, trees stand on the lowest terrain within this ring around the trunk (no floating downhill roots on slopes); 0 = trunk centre only
	float TreeSpacing = 1.5;		// m, min distance between planting holes and planted trees (holes: at least 1 m)
	bool ProtectForeignBases = true;	// no planting near a territory flag the player is not a member of (built the pole, raised the flag or built near it)
	bool RequireOwnFlag = false;	// strict mode: planting only within FlagPlantRadius of a flag the player is a member of (nowhere else in the world)
	bool ShowSeedSpecies = false;	// false: seeds are only called "Tree Seed" / "Bush Seed", players have to remember what they found
	float FlagPlantRadius = 60;		// m around a territory flag (vanilla flag refresh radius is 60 m)
	ref array<string> RoadSurfaces;	// surface names that count as road
	ref array<ref PF_NoPlantZone> NoPlantZones;	// admin zones without planting: {"X": 6500, "Y": 0, "Z": 2500, "Radius": 150}, any number
	// limits (Claude.md §23)
	int MaxPlantsPerServer = 1500;	// all plants incl. stumps; when reached, the oldest plant outside every active flag radius is removed for a new one
	int MaxTreesPerPlayer = 10;		// outside the player's own bases, rolling: planting is never blocked; when a new tree there is fully grown, the owner's oldest tree outside their bases disappears
	int MaxBushesPerPlayer = 10;	// same for bushes
	int MaxTreesPerPlayerInBase = 4;	// inside the player's own bases (all together, any flag they are a member of); full = no more planting there
	int MaxBushesPerPlayerInBase = 6;
	int MaxTreesPerBase = 10;		// within FlagPlantRadius of a flag, all players together; full = no more planting there
	int MaxBushesPerBase = 15;

	// seeds
	float SeedOneChance = 0.4;		// chance to find exactly 1 seed: wild or grown planted tree search (each player can search each tree once), wild bush cut, "Get seeds" from fruit
	float SeedTwoChance = 0.1;		// chance to find 2 seeds (the maximum); nothing = the rest (default 50 %)
	float FruitSearchChance = 0.5;		// chance per search of a wild apple/pear/plum tree
	int FruitMaxPerSearch = 2;		// 1 + up to this many extra fruits

	// world
	bool FrozenGround = true;		// winter maps: planting holes and seeds on frozen ground (snow) only where a fire has thawed it (PF_Frozen); false = snow ground behaves like any other ground (vanilla: not fertile)
	float ThawBurnMinutes = 5;		// a fireplace or fire barrel must burn hot (at least 40 % of its max temperature, the vanilla melt patch under it mostly grown) this long on frozen ground
	float ThawMinutes = 30;			// the thawed ground stays plantable this long after the fire is out
	float ThawRadius = 2;			// m around the fire (user 2026-10-23: 2 m)
	ref array<string> FrozenSurfaces;	// surface names that count as frozen; empty = map default (Frostline: sakhal_snow, sakhal_snow_forest; Namalsk: nam_snow, nam_snowgrass; other maps: every surface with the vanilla snow melt patch, e.g. Winter Chernarus)
	bool TreeShade = true;			// a player under a tree crown (wild or planted) is in the shade: no vanilla sun heat (only Badlands has it, up to +9.6 °C at noon); false = vanilla (trees cast no shade)

	// admin
	int StatsMinutes = 10;			// write plant counts per kind/owner with positions to PlantableFlora/stats.txt (+ one line in the script log) this often; 0 = off
	bool WipeAllPlants = false;
	ref array<string> AdminIds;	// Steam64 IDs of admins who get the admin debug tools (ShadeDebug overlay); not sent to clients		// true: 30 s after the next server start every planted plant, stump and planting mound is deleted, then this switches back to false

	// debugging / development
	bool DebugLogs = false;			// verbose [PlantableFlora] lines in the script logs
	bool PerfLog = false;			// server FPS + growth tick time every 10 s in the server script log
	int StressTestCount = 0;		// DEV: spawn this many plants (owner "stress") in a grid 60 s after start; 0 removes them again
	float StressTestX = 2400;		// DEV: grid centre (world X / Z)
	float StressTestZ = 7920;
	bool TreeLineup = false;		// DEV: when the first player joins, one stage of every tree species is planted in a grid in front of them; switches back to false
	string TreeLineupStage = "Medium";	// DEV: stage for TreeLineup: "Young" (seedling), "Medium" or "Mature"
	bool FreezeGrowth = false;		// DEV: no plant grows while true (keep a lineup at its stage)
	bool ShadeDebug = false;		// admin tool: for players in AdminIds the shade check runs every 2 s (vanilla 30 s) and the result is shown bottom left (planted/wild tree, sun heat, air temperature); Numpad 9 shows/hides it
	ref array<ref PF_OffsetOverride> GroundOffsetOverrides;	// DEV: [{"PlantClass": "PF_Tree_Hornbeam", "Offset": 1.2}]; with DebugLogs re-read every 10 s, plants move live

	// extra ground offset of a plant class from GroundOffsetOverrides (0 if none)
	float OffsetOverrideFor(string plantClass)
	{
		if (!GroundOffsetOverrides)
			return 0;
		foreach (PF_OffsetOverride entry : GroundOffsetOverrides)
		{
			if (entry.PlantClass == plantClass)
				return entry.Offset;
		}
		return 0;
	}

	void PF_Config()
	{
		RoadSurfaces = DefaultRoadSurfaces();
		NoPlantZones = new array<ref PF_NoPlantZone>();
		GroundOffsetOverrides = new array<ref PF_OffsetOverride>();
		AdminIds = new array<string>();
		FrozenSurfaces = new array<string>();
	}

	// server: player is in AdminIds (Steam64 ID, or the hashed DayZ ID)
	bool IsAdmin(PlayerIdentity identity)
	{
		if (!identity || !AdminIds)
			return false;
		return AdminIds.Find(identity.GetPlainId()) >= 0 || AdminIds.Find(identity.GetId()) >= 0;
	}

	// Chernarus terrain surfaces of paved ground. Not cp_dirt (2026-10-20): it is fertile and painted on farmyards and
	// field edges too, so holes there were refused while vanilla allowed a garden plot; real roads are found as road
	// objects (PF_Placement.IsNearRoadPiece)
	static array<string> DefaultRoadSurfaces()
	{
		array<string> list = new array<string>();
		list.Insert("cp_concrete1");
		list.Insert("cp_concrete2");
		list.Insert("cp_gravel");
		return list;
	}

	protected static ref PF_Config s_Instance;

	static PF_Config Get()
	{
		if (!s_Instance)
			Load();
		return s_Instance;
	}

	// client: config received from the server
	static void Set(PF_Config config)
	{
		if (config)
			s_Instance = config;
	}

	static void Load()
	{
		s_Instance = new PF_Config();
		if (!g_Game.IsServer())
			return;

		string error;
		if (FileExist(PATH))
		{
			if (!JsonFileLoader<PF_Config>.LoadFile(PATH, s_Instance, error))
			{
				ErrorEx("[PlantableFlora] config.json could not be read, using defaults: " + error);
				s_Instance = new PF_Config();
				return;
			}
			Print("[PlantableFlora] config loaded from " + PATH);
			// an empty list (old configs had one) would silently switch the road rule off; RoadClearance 0 does that on purpose
			if (!s_Instance.RoadSurfaces || s_Instance.RoadSurfaces.Count() == 0)
				s_Instance.RoadSurfaces = DefaultRoadSurfaces();
			if (!s_Instance.NoPlantZones)
				s_Instance.NoPlantZones = new array<ref PF_NoPlantZone>();
			if (!s_Instance.GroundOffsetOverrides)
				s_Instance.GroundOffsetOverrides = new array<ref PF_OffsetOverride>();
			if (!s_Instance.AdminIds)
				s_Instance.AdminIds = new array<string>();
			if (!s_Instance.FrozenSurfaces)
				s_Instance.FrozenSurfaces = new array<string>();
		}
		else
		{
			if (!FileExist(DIR))
				MakeDirectory(DIR);
			Print("[PlantableFlora] writing default config to " + PATH);
		}

		// (re)write so keys added by newer versions show up in the file
		Save();
	}

	static void Save()
	{
		string error;
		if (!JsonFileLoader<PF_Config>.SaveFile(PATH, s_Instance, error))
			ErrorEx("[PlantableFlora] config.json could not be written: " + error);
	}

	// server -> one client (called when the client connects)
	static void SendTo(PlayerIdentity identity)
	{
		// the admin list stays on the server
		PF_Config cfg = Get();
		array<string> admins = cfg.AdminIds;
		cfg.AdminIds = new array<string>();
		g_Game.RPCSingleParam(null, PF_RPC.SYNC_CONFIG, new Param1<PF_Config>(cfg), true, identity);
		cfg.AdminIds = admins;
	}

	int GrowSecondsFor(string kind)
	{
		if (kind == "treeStep")
			return TreeGrowSeconds / 2;	// 3-stage trees: the two steps add up to TreeGrowSeconds
		if (kind == "tree")
			return TreeGrowSeconds;
		if (kind == "bushStep")
			return BushGrowSeconds / 2;	// seedling -> young -> mature bush: the two steps add up to BushGrowSeconds
		if (kind == "bushRegrow")
			return BushRegrowSeconds;
		return BushGrowSeconds;
	}
}

// Verbose logging, off unless config DebugLogs (clients get the flag with the synced config)
class PF_Log
{
	static void Debug(string msg)
	{
		if (PF_Config.Get().DebugLogs)
			Print("[PlantableFlora] " + msg);
	}
}

// Client: what the server told us about our own plants (for the placement prompt)
class PF_ClientState
{
	static int OwnTrees;		// outside own bases
	static int OwnBushes;
	static int OwnTreesInBase;	// inside own bases
	static int OwnBushesInBase;
	static string ShadeInfo;	// DEV: last shade debug text from the server (config ShadeDebug)

	// " (Trees 3/10, Bushes 7/10)" or " (Base: Trees 1/4, Bushes 0/6)" for the dig/plant prompts
	static string CountsText(bool inBase)
	{
		PF_Config cfg = PF_Config.Get();
		if (inBase)
			return " (Base: Trees " + OwnTreesInBase + "/" + cfg.MaxTreesPerPlayerInBase + ", Bushes " + OwnBushesInBase + "/" + cfg.MaxBushesPerPlayerInBase + ")";
		return " (Trees " + OwnTrees + "/" + cfg.MaxTreesPerPlayer + ", Bushes " + OwnBushes + "/" + cfg.MaxBushesPerPlayer + ")";
	}
	static ref array<string> MyFlags = new array<string>();
	static ref array<vector> ThawSpots = new array<vector>();	// thawed ground on winter maps (PF_Frozen, synced by the server)	// keys of territory flags this player is a member of (PF_Territory.KeyOf)
}
