// Foreign-base protection (Claude.md §21): no planting near a territory flag the player is not a member of.
// Vanilla flags have no owner, so membership is tracked here: building any part of the flag pole, raising the
// flag, or building any base part within FlagPlantRadius makes a player a member (covers clans without lists).
// Stored in $profile:PlantableFlora/territory.json, not in the vanilla save. Flags never move, so a flag is
// identified by its rounded position.
// With Expansion BaseBuilding (EXPANSIONMODBASEBUILDING) its territory system is used instead: only claimed
// territories count as bases, members = Expansion territory members, radius = Expansion TerritorySize.
// territory.json is not touched then.

class PF_FlagEntry
{
	string Key;
	ref array<string> Members = new array<string>();
}

class PF_TerritoryData
{
	ref array<ref PF_FlagEntry> Flags = new array<ref PF_FlagEntry>();
}

class PF_Territory
{
	static const string PATH = "$profile:PlantableFlora/territory.json";
	protected static ref PF_TerritoryData s_Data;

	static string KeyOf(Object flag)
	{
		vector p = flag.GetPosition();
		return Math.Round(p[0]).ToString() + "_" + Math.Round(p[2]).ToString();
	}

	protected static PF_TerritoryData Data()
	{
		if (!s_Data)
		{
			s_Data = new PF_TerritoryData();
			string error;
			if (FileExist(PATH) && !JsonFileLoader<PF_TerritoryData>.LoadFile(PATH, s_Data, error))
			{
				ErrorEx("[PlantableFlora] territory.json could not be read: " + error);
				s_Data = new PF_TerritoryData();
			}
		}
		return s_Data;
	}

	protected static PF_FlagEntry Find(string key)
	{
		foreach (PF_FlagEntry entry : Data().Flags)
		{
			if (entry.Key == key)
				return entry;
		}
		return null;
	}

	// server
	static bool IsMember(string key, string playerId)
	{
		PF_FlagEntry entry = Find(key);
		return entry && entry.Members.Find(playerId) != -1;
	}

	static bool UsesExpansion()
	{
	#ifdef EXPANSIONMODBASEBUILDING
		return true;
	#else
		return false;
	#endif
	}

	// server + client: Expansion safezone (Core). Planting is already blocked there by Expansion's deploy rule,
	// so our dig/water/fertilize/cut/fell/dig-up actions follow it
	static bool InSafeZone(PlayerBase player)
	{
	#ifdef EXPANSIONMODCORE
		return player && player.Expansion_IsInSafeZone();
	#else
		return false;
	#endif
	}

	// server + client: base radius around a flag
	static float Radius()
	{
	#ifdef EXPANSIONMODBASEBUILDING
		ExpansionTerritorySettings settings = GetExpansionSettings().GetTerritory(false);
		if (settings && settings.IsLoaded() && settings.TerritorySize > 0)
			return settings.TerritorySize;
	#endif
		return PF_Config.Get().FlagPlantRadius;
	}

	// server + client: does this flag make a base? Expansion: only claimed territories
	static bool IsBase(TerritoryFlag flag)
	{
		if (!flag)
			return false;
	#ifdef EXPANSIONMODBASEBUILDING
		return flag.HasExpansionTerritoryInformation();
	#else
		return true;
	#endif
	}

	// server: is this player id a member of the flag? (works for offline players)
	static bool FlagHasMember(TerritoryFlag flag, string playerId)
	{
	#ifdef EXPANSIONMODBASEBUILDING
		ExpansionTerritory territory = flag.GetTerritory();
		return territory && territory.IsMember(playerId);
	#else
		return IsMember(KeyOf(flag), playerId);
	#endif
	}

	// server: player built a part of this flag pole, raised its flag or built near it
	static void AddMember(Object flag, Man player)
	{
		if (UsesExpansion())
			return;	// Expansion invites decide
		if (!g_Game.IsServer() || !flag || !player || !player.GetIdentity())
			return;
		string id = player.GetIdentity().GetId();
		string key = KeyOf(flag);
		PF_FlagEntry entry = Find(key);
		if (!entry)
		{
			entry = new PF_FlagEntry();
			entry.Key = key;
			Data().Flags.Insert(entry);
		}
		if (entry.Members.Find(id) != -1)
			return;
		entry.Members.Insert(id);
		PF_Log.Debug("territory: " + id + " joined flag " + key);
		string error;
		if (!JsonFileLoader<PF_TerritoryData>.SaveFile(PATH, Data(), error))
			ErrorEx("[PlantableFlora] territory.json could not be written: " + error);
		SendTo(player.GetIdentity());
	}

	// server: a base part was built at pos -> member of every flag whose radius covers it
	static void AddMemberNear(vector pos, Man player)
	{
		array<TerritoryFlag> flags = FlagsAround(pos);
		foreach (TerritoryFlag flag : flags)
			AddMember(flag, player);
	}

	// server -> client: the flags this player is a member of (for the placement prompt)
	static void SendTo(PlayerIdentity identity)
	{
		if (!identity)
			return;
		string id = identity.GetId();
		array<string> keys = new array<string>();
		foreach (PF_FlagEntry entry : Data().Flags)
		{
			if (entry.Members.Find(id) != -1)
				keys.Insert(entry.Key);
		}
		g_Game.RPCSingleParam(null, PF_RPC.SYNC_FLAGS, new Param1<ref array<string>>(keys), true, identity);
	}

	static array<TerritoryFlag> FlagsAround(vector pos)
	{
		array<TerritoryFlag> flags = new array<TerritoryFlag>();
		array<Object> objs = new array<Object>();
		array<CargoBase> proxies = new array<CargoBase>();
		g_Game.GetObjectsAtPosition(pos, Radius(), objs, proxies);
		foreach (Object obj : objs)
		{
			TerritoryFlag flag = TerritoryFlag.Cast(obj);
			if (IsBase(flag))
				flags.Insert(flag);
		}
		return flags;
	}

	// server: every flag pole in the world (registered in TerritoryFlag.EEInit). Vanilla's
	// GetActiveRefresherLocations() is empty after a restart until a flag changes state, so it can't be used.
	protected static ref array<TerritoryFlag> s_Flags = new array<TerritoryFlag>();

	static void RegisterFlag(TerritoryFlag flag)
	{
		if (s_Flags.Find(flag) == -1)
			s_Flags.Insert(flag);
	}

	static void UnregisterFlag(TerritoryFlag flag)
	{
		s_Flags.RemoveItem(flag);
	}

	static array<TerritoryFlag> AllFlags()
	{
		return s_Flags;
	}

	// server: inside the radius of a flag that currently refreshes lifetimes (raised, not run out)
	static bool IsInActiveBase(vector pos)
	{
		float radius = Radius();
		float radiusSq = radius * radius;
		foreach (TerritoryFlag flag : s_Flags)
		{
			if (IsBase(flag) && flag.m_RefresherActive && vector.DistanceSq(flag.GetPosition(), pos) <= radiusSq)
				return true;
		}
		return false;
	}

	// server: is pos inside the radius of a flag this player id is a member of? (per-player base limits, PF_Caps;
	// works for offline owners too)
	static bool IsOwnerBase(string ownerId, vector pos)
	{
		if (ownerId == "")
			return false;
		float radius = Radius();
		float radiusSq = radius * radius;
		foreach (TerritoryFlag flag : s_Flags)
		{
			if (IsBase(flag) && vector.DistanceSq(flag.GetPosition(), pos) <= radiusSq && FlagHasMember(flag, ownerId))
				return true;
		}
		return false;
	}

	// server + client: is pos inside the radius of a flag this player is not a member of?
	static bool IsForeignBase(Man player, vector pos)
	{
		if (!PF_Config.Get().ProtectForeignBases)
			return false;
		array<TerritoryFlag> flags = FlagsAround(pos);
		foreach (TerritoryFlag flag : flags)
		{
			if (!PlayerIsMember(player, flag))
				return true;
		}
		return false;
	}

	// server + client: is pos inside the radius of a flag this player is a member of? (strict mode RequireOwnFlag)
	static bool IsOwnBase(Man player, vector pos)
	{
		array<TerritoryFlag> flags = FlagsAround(pos);
		foreach (TerritoryFlag flag : flags)
		{
			if (PlayerIsMember(player, flag))
				return true;
		}
		return false;
	}

	// server: membership from territory.json; client: the flag keys the server synced (SYNC_FLAGS)
	// Expansion: server asks the territory, client has the territories it is a member of (synced by Expansion)
	protected static bool PlayerIsMember(Man player, TerritoryFlag flag)
	{
		if (!g_Game.IsServer())
		{
		#ifdef EXPANSIONMODBASEBUILDING
			ExpansionTerritoryModule module = ExpansionTerritoryModule.s_Instance;
			return module && module.GetTerritory(flag.GetTerritoryID()) != null;
		#else
			return PF_ClientState.MyFlags.Find(KeyOf(flag)) != -1;
		#endif
		}
		if (!player || !player.GetIdentity())
			return false;
		return FlagHasMember(flag, player.GetIdentity().GetId());
	}
}

modded class BaseBuildingBase
{
	override void OnPartBuiltServer(notnull Man player, string part_name, int action_id)
	{
		super.OnPartBuiltServer(player, part_name, action_id);
		if (TerritoryFlag.Cast(this))
			PF_Territory.AddMember(this, player);
		else
			PF_Territory.AddMemberNear(GetPosition(), player);
	}
}

modded class TerritoryFlag
{
	override void EEInit()
	{
		super.EEInit();
		if (g_Game.IsServer())
			PF_Territory.RegisterFlag(this);
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		if (g_Game.IsServer())
			PF_Territory.UnregisterFlag(this);
	}

	override void AnimateFlagEx(float delta, PlayerBase player = null)
	{
		super.AnimateFlagEx(delta, player);
		if (player && g_Game.IsServer())
			PF_Territory.AddMember(this, player);
	}
}
