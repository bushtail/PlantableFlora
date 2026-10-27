// Plant limits (Claude.md §23), server side.
// - Per-player pools outside the player's own bases (MaxTreesPerPlayer / MaxBushesPerPlayer): planting is never blocked;
//   when a plant of a full pool reaches its final stage, the owner's oldest other plant of that kind outside their bases is deleted.
// - Per-player limit inside the player's own bases, all their bases together (MaxTreesPerPlayerInBase / ...): blocks.
//   "Inside" = within FlagPlantRadius of a flag the owner is a member of, decided when counting (flag lost = outside).
// - Per-base cap (MaxTreesPerBase / MaxBushesPerBase) within FlagPlantRadius of any flag, all owners together: blocks.
// - Server cap (MaxPlantsPerServer): the oldest plant outside every active flag radius is deleted to make room;
//   blocks only when every plant sits in an active base.
// Stumps count for the server cap only. "Oldest" = PF_Bush_Base.GetFirstPlantedAt (0 for plants from older saves).
class PF_Caps
{
	static const int KIND_NONE = 0;	// stump: not in the pools or base caps
	static const int KIND_TREE = 1;
	static const int KIND_BUSH = 2;

	static int KindOf(PF_Bush_Base plant)
	{
		if (!plant || plant.IsStump())
			return KIND_NONE;
		if (plant.IsTreePlant())
			return KIND_TREE;
		return KIND_BUSH;
	}

	static string KindName(int kind, bool plural)
	{
		if (kind == KIND_TREE)
		{
			if (plural)
				return "trees";
			return "tree";
		}
		if (plural)
			return "bushes";
		return "bush";
	}

	static bool IsInOwnerBase(PF_Bush_Base plant)
	{
		return PF_Territory.IsOwnerBase(plant.GetOwnerId(), plant.GetPosition());
	}

	// the owner's plants of a kind inside (inBase) or outside their own bases
	static int CountOwnerKind(string ownerId, int kind, bool inBase)
	{
		if (ownerId == "")
			return 0;
		int count;
		array<PF_Bush_Base> plants = PF_Registry.GetAll();
		foreach (PF_Bush_Base plant : plants)
		{
			if (plant.GetOwnerId() == ownerId && KindOf(plant) == kind && IsInOwnerBase(plant) == inBase)
				count++;
		}
		return count;
	}

	// rolling pool outside the player's bases
	static int PoolMax(int kind)
	{
		if (kind == KIND_TREE)
			return PF_Config.Get().MaxTreesPerPlayer;
		return PF_Config.Get().MaxBushesPerPlayer;
	}

	// blocking limit inside the player's bases
	static int InBaseMax(int kind)
	{
		if (kind == KIND_TREE)
			return PF_Config.Get().MaxTreesPerPlayerInBase;
		return PF_Config.Get().MaxBushesPerPlayerInBase;
	}

	// server: planting at pos inside the player's own base, their base limit full? "" or the player-facing reason
	static string CheckPlayerBase(Man player, vector pos, int kind)
	{
		if (!player || !player.GetIdentity())
			return "";
		string ownerId = player.GetIdentity().GetId();
		if (!PF_Territory.IsOwnerBase(ownerId, pos))
			return "";
		int max = InBaseMax(kind);
		int count = CountOwnerKind(ownerId, kind, true);
		if (count >= max)
			return "Your base limit reached (" + count + "/" + max + " " + KindName(kind, true) + ")";
		return "";
	}

	static int BaseMax(int kind)
	{
		if (kind == KIND_TREE)
			return PF_Config.Get().MaxTreesPerBase;
		return PF_Config.Get().MaxBushesPerBase;
	}

	// a plant outside its owner's bases reached its final stage by growing: if the owner's outside pool is over the
	// limit, their oldest other plant outside their bases goes (inside a base the limit blocks planting instead)
	static void OnMatured(PF_Bush_Base plant)
	{
		string ownerId = plant.GetOwnerId();
		int kind = KindOf(plant);
		if (ownerId == "" || kind == KIND_NONE || IsInOwnerBase(plant))
			return;
		int max = PoolMax(kind);
		if (CountOwnerKind(ownerId, kind, false) <= max)
			return;

		// only a plant beyond the pool evicts: with equal grow times the oldest plant matures first, and it used to
		// remove the second oldest (test 2026-10-20). A plant with fewer than max older ones is inside the pool.
		PF_Bush_Base oldest;
		int older;
		array<PF_Bush_Base> plants = PF_Registry.GetAll();
		foreach (PF_Bush_Base other : plants)
		{
			if (other == plant || other.GetOwnerId() != ownerId || KindOf(other) != kind || IsInOwnerBase(other))
				continue;
			if (other.GetFirstPlantedAt() <= plant.GetFirstPlantedAt())
				older++;
			if (!oldest || other.GetFirstPlantedAt() < oldest.GetFirstPlantedAt())
				oldest = other;
		}
		if (older < max)
			return;
		if (!oldest)
			return;
		PF_Log.Debug("pool full: " + ownerId + " " + KindName(kind, true) + ", oldest " + oldest.GetType() + " at " + oldest.GetPosition().ToString() + " removed");
		g_Game.ObjectDelete(oldest);
		PF_Notify.SendToOwner(ownerId, "Your oldest " + KindName(kind, false) + " has died", "Limit outside your base: " + max + " " + KindName(kind, true) + ". A new one has grown up.");
	}

	// base cap at pos: "" or the player-facing reason. Cached briefly, CanBePlaced runs every frame while placing.
	protected static vector s_BasePos;
	protected static int s_BaseKind;
	protected static int s_BaseTime;
	protected static string s_BaseReason;

	static string CheckBase(vector pos, int kind)
	{
		int now = g_Game.GetTime();
		if (kind == s_BaseKind && vector.DistanceSq(pos, s_BasePos) < 0.01 && now - s_BaseTime < 1000)
			return s_BaseReason;
		s_BasePos = pos;
		s_BaseKind = kind;
		s_BaseTime = now;
		s_BaseReason = "";

		float radius = PF_Territory.Radius();
		float radiusSq = radius * radius;
		int limit = BaseMax(kind);
		array<PF_Bush_Base> plants = PF_Registry.GetAll();
		array<TerritoryFlag> flags = PF_Territory.AllFlags();
		foreach (TerritoryFlag flag : flags)
		{
			if (!PF_Territory.IsBase(flag) || vector.DistanceSq(flag.GetPosition(), pos) > radiusSq)
				continue;
			int count = 0;
			foreach (PF_Bush_Base plant : plants)
			{
				if (KindOf(plant) == kind && vector.DistanceSq(flag.GetPosition(), plant.GetPosition()) <= radiusSq)
					count++;
			}
			if (count >= limit)
			{
				s_BaseReason = "Base full (" + count + "/" + limit + " " + KindName(kind, true) + ")";
				return s_BaseReason;
			}
		}
		return s_BaseReason;
	}

	// server cap: the oldest plant outside every active flag radius, or null
	static PF_Bush_Base FindServerEvictable()
	{
		PF_Bush_Base oldest;
		array<PF_Bush_Base> plants = PF_Registry.GetAll();
		foreach (PF_Bush_Base plant : plants)
		{
			if (oldest && plant.GetFirstPlantedAt() >= oldest.GetFirstPlantedAt())
				continue;
			if (!PF_Territory.IsInActiveBase(plant.GetPosition()))
				oldest = plant;
		}
		return oldest;
	}

	protected static int s_EvictCheckTime = -100000;
	protected static bool s_EvictCheckResult;

	// cached: CanBePlaced runs every frame while placing
	static bool CanFreeServerSlot()
	{
		int now = g_Game.GetTime();
		if (now - s_EvictCheckTime > 30000)
		{
			s_EvictCheckTime = now;
			s_EvictCheckResult = FindServerEvictable() != null;
		}
		return s_EvictCheckResult;
	}

	// called right before a new plant is created
	static void FreeServerSlot()
	{
		if (PF_Registry.CountAll() < PF_Config.Get().MaxPlantsPerServer)
			return;
		PF_Bush_Base oldest = FindServerEvictable();
		if (!oldest)
			return;
		Print("[PlantableFlora] server plant limit reached: oldest plant outside bases removed, " + oldest.GetType() + " at " + oldest.GetPosition().ToString());
		g_Game.ObjectDelete(oldest);
		s_EvictCheckTime = -100000;
	}
}
