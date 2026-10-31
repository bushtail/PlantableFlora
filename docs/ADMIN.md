# PlantableFlora – Admin Guide

For server admins. Work in progress. Sections 1-6 explain the mod, section 7 lists every setting with its default, section 8 has example presets. Defaults are the ones in `PF_Config.c`. If a value here differs from your generated `config.json`, the file is right.

---

## 1. What the mod does

Players plant trees and bushes from seeds: to hide a base, grow cover or hide stash with a bush. On Badlands every tree, wild or planted, also gives real shade from the hot sun.

- **Seeds** come from loot, from searching wild trees, from cutting wild bushes and from fruit (cut with a knife). Fully grown planted trees give seeds and fruit too.
- **Planting** takes effort: dig a planting hole with a digging tool, water it (fertilizer is optional and makes it grow faster), then plant the seed. An empty vanilla garden plot works too.
- **Growth** runs on real time (offline time counts): a small seedling on its soil heap grows into a young and then a full-grown plant. Defaults: bushes 1 hour, trees 12 hours.
- **Bushes** can be cut (they grow back) or dug up (gone). **Trees** are felled with an axe: logs, sticks and, for some species, a stump. Felled trees never grow back.
- **Planted plants look like wild ones.** No name panel, no inventory entry, the same actions and prompts as wild plants. From the young stage on a player can't tell a planted tree from a wild one.
- **Bases are protected.** By default nobody can plant inside another player's base (60 m around a territory flag they are not a member of). Trees can't be planted on stashes, but a **bush on top of a stash hides it** (4.3).
- **Maps:** Chernarus and Livonia share their species; **Badlands has its own species and soil look**; seeds only grow on their own map (2.6).
- **Real shade on Badlands.** Vanilla Badlands heats up a player standing in the sun (up to +8 °C at noon, +9.6 °C without a hat), but only buildings, roofs, rocks and terrain gave shade, never a tree. With the mod the crown of any tree, wild or planted, gives shade too (3.5).
- **Limits** per player (inside and outside the own base), per base and per server keep the number of plants under control (section 5).
- **Plants decay like base building:** they live 45 days unless an active territory flag nearby refreshes them.
- Messages to players are vanilla notification popups (top right).

---

## 2. How to use

### 2.1 Installation

1. Add the mod to the server and the client mod list (`-mod=...`). It has to be loaded on **both** sides. The server holds the config and the plant data, the client needs the models and the placement preview.
2. Copy `PlantableFlora.bikey` into the server's `keys` folder.
3. Add the seed loot: copy the `<type>` entries from the mod's `ServerFiles/types_PlantableFlora.xml` (Badlands: `types_PlantableFlora_nasdara.xml`) into your mission's `db/types.xml` (see 2.5).
4. Start the server once. It creates `<server>/profiles/PlantableFlora/config.json` with the defaults.
5. Edit the file, restart the server.

### 2.2 The config file

`<server>/profiles/PlantableFlora/config.json`

- Created with defaults when missing. Every setting is a plain JSON key.
- **Edits need a server restart.** The server reads the file at start and sends the values to every client when it connects. Nobody needs to repack or update the mod to change a value.
- **The server rewrites the file on every start**, so keys added by newer mod versions show up. Your values are kept. The file is written with one value per line, that is normal.
- DayZ writes floats with a long tail (`0.25000000596046449`) and booleans as `1`/`0`. You can type `0.25` and `true`.

If the file has a JSON error the server logs `config.json could not be read, using defaults` and runs with the defaults. Validate the file in any JSON checker after editing.

### 2.3 Files the mod writes

All in `<server>/profiles/PlantableFlora/`:

| File | What |
|---|---|
| `config.json` | Settings (section 7) |
| `territory.json` | Who is a member of which territory flag (section 4.2). Not part of the vanilla save. |
| `stats.txt` | Plant counts per kind and owner with positions, rewritten every `StatsMinutes` (section 6.1) |

The plants themselves are saved in the normal server storage like any other object.

### 2.4 Removing the mod

Planted plants are mod objects. Without the mod they disappear from the world on the next start, nothing else breaks. To clean up first, use `WipeAllPlants` (section 6.2).

### 2.5 Loot

Use the loot file of your map:
- **Chernarus, Livonia, Frostline and other maps:** `server_files/types_PlantableFlora.xml` (29 seeds).
- **Badlands:** `server_files/types_PlantableFlora_nasdara.xml` (12 Badlands seeds).

Entries are usage Farm/Village, tier 1-2, nominal 3-4. Tune `nominal`/`min` like any other item. Seeds also come from wild bushes and wild trees (section 3.1), so loot is only one source.

**Maps (2.6):** Badlands has its own species and only grows those; seeds of the other maps don't grow there and Badlands seeds don't grow elsewhere ("This seed won't grow here"). On Frostline the Chernarus species grow; wild Frostline birches and spruces give seeds, but planted deciduous trees keep their summer look. Only add seeds of species that fit your map.

### 2.6 Maps

| Map | Species |
|---|---|
| Chernarus, Livonia | Trees: spruce, oak, pine, beech, birch, larch, ash, walnut, hornbeam, white poplar, willow, rowan, acacia, apple, pear, plum (+ forest variants). Bushes: rose, hazel, hawthorn, blackthorn, elder, dwarf birch |
| Badlands | Trees: fig, Turkish pine, olive, date palm, poplar, ash, plum. Bushes: oleander, fig bush, prickly pear, cycas, Syrian rue. Wild Badlands trees and bushes give these seeds; cutting a wild Turkish pine sapling gives a Turkish pine seed |
| Frostline | the Chernarus species. Snow ground needs a fire first (3.6). Near snowy or frozen wild trees, spruce, birch and dwarf birch grow in their snowy/frozen look; other species refuse there ("Too cold here") |
| Namalsk | the Chernarus species; snow ground needs a fire first (3.6). Not tested |
| Winter Chernarus (map mod) | the Chernarus species; the mod's snowy tree models are used automatically. All its ground is snow-covered, so every planting spot needs a fire first (3.6). Not tested |
| Other maps | the Chernarus species; wild trees only give seeds if the map uses vanilla tree models |

**Tested (0.1.0):**

| Map | Status |
|---|---|
| Chernarus | fully tested |
| Livonia | all growth stages and heights checked; same species and mechanics as Chernarus |
| Badlands | fully tested: own species, heights, soil look, wild seeds, fruit, caves |
| Frostline | works (planting, growth, wild seeds from birch and spruce). Tested 2026-10-23: thawing snow ground with a fire, spruce growing in the winter look |
| Namalsk, Deer Isle and other maps | not tested. Vanilla tree models give seeds, own models of a map mod don't (see 9) |

Badlands soil: only soil, green grass, mud and the pine forest floor are fertile (vanilla), about 40 % of the land.
On Badlands planting holes and seedling heaps use the map's own light earth (limed: with light gravel) instead of the dark garden soil.

---

## 3. For players: how planting works

### 3.1 Getting seeds

| Source | How | Setting |
|---|---|---|
| Loot | Farms and villages | `types.xml` |
| Wild bush | Cut a wild rose, hazel, hawthorn, blackthorn or elder bush with a knife/axe (vanilla action). The wild bush stays. 0-2 seeds drop (popup with the count). | `SeedOneChance` 0.4, `SeedTwoChance` 0.1 |
| Wild tree | "Search for seeds" on a wild tree, bare hands. Each player can search each tree once. Forest trees give the forest seed. 0-2 seeds (popup with the count). Planted trees count too once they are fully grown (players can start their own orchard). | `SeedOneChance` 0.4, `SeedTwoChance` 0.1 |
| Fruit | "Search for fruit" on a wild apple/pear/plum tree (Badlands: plum, fig) gives 1-3 fruits (popup hint: "Cut it with a knife to get seeds"). Crafting "Get seeds" (knife + fruit) gives 0-2 seeds (same odds, popup with the count) of the tree that grows on this map. Olive and date palm have no fruit item: their wild trees give the seed directly. | `FruitSearchChance`, `FruitMaxPerSearch` |

### 3.2 Planting

**Winter maps (Frostline, Namalsk, Winter Chernarus):** snow ground is frozen. Thaw it first: keep a fireplace or fire barrel burning hot for 5 minutes, then dig and plant within 2 m of it (details in 3.6). Without that the seed preview says "Frozen ground: make a fire here first".

1. **Dig a planting hole.** Hold a digging tool and use "Dig planting hole" on the ground. Digging tools: shovel, field shovel, farming hoe (everything vanilla lets you dig a garden plot with). All placement rules (section 4) are checked here.
2. **Water the hole** (required by default) **and optionally fertilize it.** A container with at least 500 ml of water → "Water". Garden lime or plant material (compost) in hands → "Fertilize". Rain also waters a hole: after `RainWaterMinutes` (10) of rain it counts as watered. Without water the seed can't be planted ("Needs water first"); `RequireWater` switches that off. `RequireFertilizer` makes fertilizer required for **trees** (bushes only ever need water). Fertilizer makes every growth step faster: garden lime 30 %, plant material 15 % (water gives no bonus by default, `WaterBonus`).
3. **Plant.** The seed preview snaps to the hole, "Plant" uses up the hole; the seedling stands on its soil heap.
4. An unused hole disappears after 60 minutes. A digging tool in hands can "Fill in planting hole".

An empty vanilla garden plot also works as a mound (`AllowGardenPlot`); one fertilized/watered slot counts for the whole plot (bonus and requirements; a limed slot beats a composted one). With `RequirePreparedGround` off, seeds are planted straight into the ground (a digging tool must be in the inventory) and `RequireWater` / `RequireFertilizer` have no effect.

### 3.3 Growth

- Every plant starts as a small seedling (about 20 cm) on the soil heap of its planting hole; the heap keeps the hole's look (limed, compost, wet).
- Bush: seedling → young bush → full-grown, together `BushGrowSeconds` (1 h, half per step).
- Tree: seedling → young tree (size 1, or a sapling for species with only one model) → full-grown, together `TreeGrowSeconds` (12 h, half per step).
- Time is real time and keeps running while the server is off or the player is away.
- Planted trees grow to a "middle-aged" size (about 18-20 m), like most trees on the map.
- Growth does not check for objects on the spot. A car parked on a sapling ends up inside the tree; it can be pushed out (tested, harmless).

### 3.4 Cutting, felling, digging up

| Action | Tool | Time | Result |
|---|---|---|---|
| Cut bush ("Harvest Long Wooden Stick") | knife, machete, sickle, axe, sword | `CutSeconds` 5 | Sticks, bush vanishes and grows back after `BushRegrowSeconds` (1 h) |
| Fell tree ("Harvest Wooden Log") | hatchet, wood axe, firefighter axe | `FellSeconds` 45 | Like the same wild tree: one log or firewood per chopping cycle, as many as the wild tree gives (e.g. spruce 2 logs + 4 firewood, pine 3 logs + 2 firewood, birch 6 firewood + 1 stick); on the last cycle the extra items drop and the tree falls over away from you. It lies there without collision until the next server restart (`FallenTreeMinutes`, `FallenTreeCollision`). Trees without a vanilla wild version (Badlands) give `FellLogs` logs + `FellSticks` sticks. Stump for spruce, oak, pine, beech and Turkish pine. Never grows back. |
| Dig up | digging tool in hands | `DigUpSeconds` 20 | Removes a bush, young tree or stump for good. Full-grown trees must be felled first. |

Anyone can cut, fell or dig up any planted plant, owner or not.

### 3.5 Shade (Badlands)

Badlands is the only vanilla map with **sun heat**: by day a player in the open gets extra temperature (dawn +2, morning +6, noon +8, afternoon +4, evening +2 °C; x1.2 without a hat, less with one; clouds fade it out). Vanilla takes it away in buildings, under roofs and in the shadow of rocks or terrain, but never under a tree: tree crowns have no collision, so the game didn't see them.

With the mod (`TreeShade`, on by default) a tree crown is shade:

- **Any tree**, wild or planted, from about 4 m height. Seedlings and young saplings don't shade, bushes don't.
- **The shade follows the sun.** It is where the crown blocks the line from the player's head to the sun: at noon under the crown, in the morning and evening on the side away from the sun, like the real shadow (a bit rougher, box-shaped).
- **How much:** exactly the vanilla sun heat, nothing invented. Under a tree the player gets the same temperature as in the shadow of a building or rock.
- **Timing:** the game checks the shade every 30 s (vanilla) and heat comfort changes gradually, so cooling down takes a moment.
- **Other maps** have no sun heat, nothing changes there.
- Planted trees shade exactly like wild ones, so the shade gives nothing away.

A grove next to the base is a cool spot to wait out the midday sun.

### 3.6 Frozen ground (winter maps)

On winter maps the snow ground is frozen: no planting hole, no seed. A fire thaws it:

1. Build a fireplace (or a fire barrel) on the snow and keep it burning hot (at least 40 % of its maximum temperature; the vanilla melt patch under the fire is clearly visible by then).
2. After `ThawBurnMinutes` (5) of hot fire, the ground within `ThawRadius` (2 m) of the fire is thawed.
3. Dig planting holes and plant there. The ground stays thawed for `ThawMinutes` (30) after the fire is out or gone; a hole that is already dug keeps its state.

Without a fire the seed preview says "Frozen ground: make a fire here first" and the digging tool offers no "Dig planting hole". A server restart refreezes thawed ground (holes stay).

Which ground is frozen:

| Map | Frozen surfaces |
|---|---|
| Frostline | `sakhal_snow`, `sakhal_snow_forest`. The brown grass (`sakhal_grass_brown`) stays plantable without a fire |
| Namalsk | `nam_snow`, `nam_snowgrass` |
| Any other map | every surface with the vanilla snow melt patch under fires. Covers Winter Chernarus (all its ground); normal Chernarus, Livonia and Badlands have none |

`FrozenSurfaces` in `config.json` replaces the list (surface names, e.g. for a map mod with own snow surfaces). `FrozenGround: false` switches the rule off; snow ground then behaves like vanilla (Frostline and Namalsk snow is not fertile, so nothing grows there).

---

## 4. Placement rules

Checked when digging the mound (on the server, the client shows the reason in the preview prompt). Every hole keeps `TreeSpacing` to other holes and planted trees, because any hole may get a tree later; so bushes planted into holes are also at least 1.5 m apart. Planting onto a mound only checks the stash rule and the limits.

| Rule | Default | Prompt |
|---|---|---|
| Seed from another map (2.6) | – | "This seed won't grow here" |
| Fertile ground (grass, soil, forest floor) | – | (preview red, dig prompt hidden) |
| Caves and underground (more than 1.5 m below the terrain surface) | – | "Not enough light to grow here" |
| Admin no-plant zones | none | "No planting in this area" |
| Strict mode: own flag nearby | off | "Needs your own territory flag nearby" |
| Road: no road within `RoadClearance` | 5 m | "Too close to a road" |
| Foreign base | on, 60 m | (silent, see 4.2) |
| Underground stash: no tree within 2 m | – | (silent) |
| Buildings, base parts, vehicles, players within `BuildingClearance` | 3 m | "Too close to …" |
| Trees on a garden plot or without prepared ground: another planted tree within `TreeSpacing` (planting holes already keep this distance when dug) | 1.5 m | "Too close to another tree" |
| Planting holes: another hole/garden plot or a planted tree within `TreeSpacing` (every hole, trees and bushes) | 1.5 m | (dig prompt hidden) |

### 4.1 No-plant zones

`NoPlantZones` is a list of circles (X/Z world position + radius in m, Y is ignored). Use it for traders, safe zones, event areas:

```json
"NoPlantZones": [
    { "X": 11850, "Y": 0, "Z": 12450, "Radius": 300 },
    { "X": 6500, "Y": 0, "Z": 2500, "Radius": 150 }
]
```

### 4.2 Bases and territory flags

Vanilla territory flags have no owner, so the mod tracks **members** itself. A player becomes a member of a flag when they:

- build any part of the flag pole,
- raise or lower a flag on it, or
- build any base part within `FlagPlantRadius` of it (covers clans without member lists).

Membership is stored in `territory.json` and never expires.

**With DayZ Expansion BaseBuilding** the mod uses Expansion's territory system instead (the server log shows `[PlantableFlora] territories: Expansion BaseBuilding` at start):

- Only **claimed** Expansion territories count as bases. A flag pole without a territory created in the flag menu is no base (no protection, no base limits).
- Members = the territory's members (owner + invited players). Building parts or raising the flag no longer makes anyone a member; `territory.json` is not used.
- The radius is Expansion's `TerritorySize` (default 150 m) instead of `FlagPlantRadius`, for protection, `RequireOwnFlag` and all base limits below.
- Expansion's own deploy rules also apply to seeds (no planting in Expansion safezones, and no digging, watering, fertilizing, cutting, digging up or felling there either; in enemy territory unless the seed class is whitelisted in `DeployableInsideAEnemyTerritory`).

**`ProtectForeignBases`** (default on): no planting within `FlagPlantRadius` (60 m, the vanilla flag refresh radius) of a flag the player is not a member of. If several flags overlap, every one of them must be the player's own. This rule is **silent**: the prompt shows no reason, so players can't use a seed preview to find hidden bases. A pole without a raised flag counts as a flag.

**`RequireOwnFlag`** (strict mode, default off): players may plant **only** within `FlagPlantRadius` of a flag they are a member of, nowhere else in the world. Use it if you want planting to be base decoration only (fewer plants in the open world, no tree walls on roads or loot spots). Players without a flag can't plant at all, and planting a bush on a stash in the woods is no longer possible. The prompt tells the player why ("Needs your own territory flag nearby"); this reveals nothing about other players' bases.

| Spot | default | `RequireOwnFlag` on |
|---|---|---|
| Open world, no flag nearby | allowed | blocked |
| Own base | allowed | allowed |
| Someone else's base | blocked | blocked |

If a flag pole is dismantled or destroyed, the plants around it stay. Nobody can plant there until a flag stands again.

### 4.3 Underground stashes

Trees can't be planted within 2 m of a buried stash (the trunk would block digging it out). Silent, like the base rule.

**Bushes may sit right on top of a stash**: planting a bush over your stash in the woods hides it. The stash can still be dug out next to the bush, and anyone can cut or dig up the bush.

### 4.4 Roads

Roads of the map's road network (asphalt, concrete and dirt roads) count, plus painted paved surfaces (`RoadSurfaces`: concrete, gravel). Painted dirt (`cp_dirt`) is not a road: it is fertile and also covers farmyards and field edges. `RoadClearance` 0 switches the rule off.

---

## 5. Limits

### 5.1 Per player outside their bases (rolling)

`MaxTreesPerPlayer` 10, `MaxBushesPerPlayer` 10 count a player's plants **outside** their own bases. Planting is **never blocked** by these. When a new plant of a full pool is fully grown, the owner's **oldest** plant of that kind outside their bases disappears, and the owner gets a message ("Your oldest tree has died"). The prompts show the counts ("Dig planting hole (Trees 3/10, Bushes 7/10)") and warn when the pool is full ("your oldest tree outside your base disappears when this one is grown"). Stumps, mounds and wild plants don't count.

A group that moves to a new base empties the old one plant by plant.

### 5.2 Per player inside their bases (blocking)

`MaxTreesPerPlayerInBase` 4, `MaxBushesPerPlayerInBase` 6 count a player's plants **inside** their own bases (within `FlagPlantRadius` of any flag they are a member of, all their bases together). When full, planting there is blocked ("Your base limit reached (4/4 trees)"); nothing is removed. The prompts show "(Base: Trees 1/4, Bushes 0/6)" inside a base. Inside or outside is decided when counting: if the flag is lost, those plants count as outside from then on (they stay, but the outside pool may remove the oldest when a new one grows).

### 5.3 Per base

`MaxTreesPerBase` 10, `MaxBushesPerBase` 15 within `FlagPlantRadius` of a flag, counting every plant there, no matter who planted it (clans share it). A full base blocks planting ("Base full"). With 10 members and 4 trees each the base still holds only `MaxTreesPerBase` trees. It never removes plants, otherwise one member could kill another's plant.

### 5.4 Per server

`MaxPlantsPerServer` 1500 (all plants including stumps). When reached, the oldest plant **outside every active base** is removed to make room. If there is none, planting is blocked ("Server plant limit reached").

### 5.5 Decay

Planted plants have a Central Economy lifetime of `PlantLifetimeDays` (45). An active territory flag within 60 m refreshes it (vanilla flag refresh, like base building). So plants in a kept-up base live as long as the base, plants in the open world vanish after 45 days.

---

## 6. Admin tools

### 6.1 Stats

Every `StatsMinutes` (10) the server writes `stats.txt`: totals (trees, bushes, stumps, growing, limit), unused mounds, and per owner ID the counts and every plant with X/Z. One summary line also goes into the script log. `0` switches it off.

### 6.2 Wipe all plants

Set `WipeAllPlants` to `true` and restart. 30 s after the start every planted plant, stump and planting mound is deleted, then the setting switches back to `false` by itself. Seeds in inventories and loot, and `territory.json`, are kept.

### 6.3 Debug logs

`DebugLogs` writes detailed `[PlantableFlora]` lines into the script logs (placement decisions, growth, lifetimes). It also shows plant names in the look-at panel ("Planted Beech"), so **keep it off on live servers**. With it on, a red garden plot preview logs why (ground, blocking object, surface) in the client log.

### 6.4 Shade overlay

To see the shade at work: put your Steam64 ID into `AdminIds`, set `ShadeDebug` to `true` and restart. For those admins the shade check runs every 2 s and a text box bottom left shows the result: in the shade of a planted tree, a wild tree, vanilla shade (trunk, roof, building, terrain) or in the sun; the tree's class and model; the sun heat removed or added; the air temperature. **Numpad 9** shows/hides the box. Other players see nothing and get no extra checks. Switch it off when done.

---

## 7. Settings reference

### 7.1 Growth

| Key | Default | What |
|---|---|---|
| `BushGrowSeconds` | 3600 | Seedling → full-grown bush (two steps, half each) |
| `TreeGrowSeconds` | 43200 | Seedling → full-grown tree (12 h, two steps, half each) |
| `BushRegrowSeconds` | 3600 | Cut bush → grown back |
| `GrowthTickSeconds` | 60 | How often the server checks all plants |
| `PlantLifetimeDays` | 45 | Lifetime without a flag refresh (5.5) |

### 7.2 Actions (seconds)

| Key | Default |
|---|---|
| `PlantSeconds` | 3 |
| `CutSeconds` | 5 |
| `DigUpSeconds` | 20 |
| `FellSeconds` | 45 |
| `FellLogs` | 3 |
| `FellSticks` | 4 |
| `FallenTreeMinutes` | 0 (= until the next restart) |
| `FallenTreeCollision` | false (lying tree blocks nobody, like vanilla); true = felled planted **and wild** trees keep their collision while they lie there |
| `SearchSeedSeconds` | 6 |

### 7.3 Ground preparation (3.2)

| Key | Default | What |
|---|---|---|
| `RequirePreparedGround` | true | Seeds need a mound or an empty garden plot |
| `AllowGardenPlot` | true | An empty vanilla garden plot counts as a mound |
| `SnapDistance` | 2 | m, the seed preview snaps to a mound this close |
| `MoundDigSeconds` | 8 | |
| `MoundLifetimeMinutes` | 60 | Unused mound disappears |
| `FertilizeSeconds` | 4 | |
| `WaterSeconds` | 4 | |
| `WaterAmountMl` | 500 | Water used per mound |
| `RainWaterMinutes` | 10 | Minutes of rain until an unwatered hole counts as watered. 0 = rain doesn't water holes. Garden plots get rain the vanilla way. |
| `RequireWater` | true | Mound/plot must be watered before planting |
| `RequireFertilizer` | false | Mound/plot must be fertilized before planting a tree (bushes only need water) |
| `GardenLimeBonus` | 0.3 | Fertilized with garden lime: every growth step this much faster |
| `PlantMaterialBonus` | 0.15 | Same for plant material (compost) |
| `WaterBonus` | 0 | Watered: same, adds up with the fertilizer bonus. 0 because water is required by default; set it (e.g. 0.25) when you switch `RequireWater` off |
| `PlotSlopeTolerance` | 1.0 | m, garden plot height difference allowed (vanilla 0.3). Also affects vegetable gardens. |
| `PlotMaxTiltDegrees` | 25 | Garden plot tilt limit (vanilla 10). Also affects vegetable gardens. |

### 7.4 Placement (4)

| Key | Default | What |
|---|---|---|
| `BuildingClearance` | 3 | m around buildings, base parts, vehicles, players |
| `RoadClearance` | 5 | m, 0 = rule off |
| `TreeSpacing` | 1.5 | m between planting holes and planted trees (holes: at least 1 m) |
| `ProtectForeignBases` | true | No planting in other players' bases |
| `RequireOwnFlag` | false | Strict mode: planting only in your own base |
| `ShowSeedSpecies` | false | true: seeds show their species ("Spruce Seed (forest)"); false: seeds are only called "Tree Seed" / "Bush Seed" (inventory, inspect, found message); players have to remember which species they found. The seed pack print already only shows tree or bush |
| `FlagPlantRadius` | 60 | m around a territory flag (with Expansion: its `TerritorySize`) |
| `RoadSurfaces` | cp_concrete1, cp_concrete2, cp_gravel | Terrain surfaces that count as road (roads of the road network are found as objects anyway). An empty list falls back to the defaults. `cp_dirt` is left out on purpose: it is fertile and also painted on farmyards and field edges. Configs from before 0.1.0 still list it, remove it there. |
| `NoPlantZones` | [] | Admin zones (4.1) |
| `TreeRootSinkPercent` | 6 | Fallback ground height for tree models without measured data. Don't change. |
| `TreeGroundOffset` | 0 | m, raises (+) or sinks (−) all planted trees. Only for fine tuning. |
| `TreeSlopeSampleRadius` | 1 | m, on slopes a tree stands on the lowest ground within this ring around the trunk, so the downhill roots don't float. 0 = trunk centre only. |

### 7.5 Limits (5)

| Key | Default |
|---|---|
| `MaxPlantsPerServer` | 1500 |
| `MaxTreesPerPlayer` | 10 |
| `MaxBushesPerPlayer` | 10 |
| `MaxTreesPerPlayerInBase` | 4 |
| `MaxBushesPerPlayerInBase` | 6 |
| `MaxTreesPerBase` | 10 |
| `MaxBushesPerBase` | 15 |

### 7.6 Seeds (3.1)

| Key | Default | What |
|---|---|---|
| `SeedOneChance` | 0.4 | Chance to find exactly 1 seed: searching a wild or grown planted tree, cutting a wild bush, "Get seeds" from fruit |
| `SeedTwoChance` | 0.1 | Chance to find 2 seeds (the maximum). Nothing = the rest (default 50 %) |
| `FruitSearchChance` | 0.5 | Per search of a wild fruit tree |
| `FruitMaxPerSearch` | 2 | 1 + up to this many extra fruits |

### 7.7 Admin and debugging (6)

| Key | Default | What |
|---|---|---|
| `StatsMinutes` | 10 | `stats.txt` interval, 0 = off |
| `WipeAllPlants` | false | Delete everything on the next start |
| `AdminIds` | [] | Steam64 IDs of admins who get the admin tools (6.4), e.g. `["76561198000000000"]`. Stays on the server, clients never receive it. |
| `DebugLogs` | false | Verbose logs, plant names visible. Off on live servers. |
| `PerfLog`, `StressTestCount`, `StressTestX`, `StressTestZ`, `TreeLineup`, `TreeLineupStage`, `FreezeGrowth`, `GroundOffsetOverrides` | off/empty | Development tools. Leave them alone. |

### 7.8 Shade (3.5)

| Key | Default | What |
|---|---|---|
| `TreeShade` | true | A tree crown (wild or planted, from about 4 m) is shade: no vanilla sun heat underneath. Only maps with sun heat (Badlands). false = vanilla, trees give no shade. |
| `ShadeDebug` | false | Shade overlay for the admins in `AdminIds` (6.4). |

### 7.9 Frozen ground (3.6)

| Key | Default | What |
|---|---|---|
| `FrozenGround` | true | Winter maps: planting holes and seeds on frozen ground only where a fire has thawed it. false = snow ground behaves like vanilla. |
| `ThawBurnMinutes` | 5 | How long a fireplace or fire barrel must burn hot on frozen ground. |
| `ThawMinutes` | 30 | How long the ground stays thawed after the fire is out. |
| `ThawRadius` | 2 | m around the fire. |
| `FrozenSurfaces` | [] | Surface names that count as frozen. Empty = map default (3.6). |

---

## 8. Example presets

### 8.1 Default (survival PvP)

Leave everything as generated.

### 8.2 Base decoration only (roleplay, PvE)

Planting only inside your own base, nothing in the open world.

```json
"RequireOwnFlag": true,
"MaxTreesPerBase": 20,
"MaxBushesPerBase": 30
```


### 8.3 Casual

Faster growth, no mound needed.

```json
"RequirePreparedGround": false,
"RequireWater": false,
"BushGrowSeconds": 1800,
"TreeGrowSeconds": 14400
```

### 8.4 Hardcore

Slow growth, fewer seeds.

```json
"TreeGrowSeconds": 86400,
"SeedOneChance": 0.2,
"SeedTwoChance": 0.05,
"MaxTreesPerPlayer": 5
```

---

## 9. Adding your own species (modders)

There is no `config.json` key for new species: a plant needs a model, and a model is only reachable through a config class. Species can be added with **a small extra addon** (own PBO, loaded after PlantableFlora), **without any script**:

```cpp
class CfgPatches
{
	class MyPlants
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = { "PlantableFlora" };
	};
};
class CfgVehicles
{
	class PF_Seedling;
	class PF_Bush_Base;
	class PF_Seed_Base;
	// tree: seedling -> middle -> full-grown
	class PF_Tree_MyMaple_Young: PF_Seedling
	{
		scope = 2;
		pf_nextStage = "PF_Tree_MyMaple_Medium";
		pf_kind = "treeStep";
		pf_isTree = 1;
	};
	class PF_Tree_MyMaple_Medium: PF_Bush_Base
	{
		scope = 2;
		model = "\MyMap\plants\maple_small.p3d";
		pf_groundOffset = 2.0;		// height of the model origin above the ground (m)
		pf_nextStage = "PF_Tree_MyMaple";
		pf_kind = "treeStep";
		pf_isTree = 1;
	};
	class PF_Tree_MyMaple: PF_Bush_Base
	{
		scope = 2;
		model = "\MyMap\plants\maple_big.p3d";
		pf_groundOffset = 5.5;
		pf_fellClass = "none";		// or a stump class
		pf_isTree = 1;
	};
	class PF_Tree_MyMaple_Placing: PF_Tree_MyMaple_Young
	{
		scope = 1;
		storageCategory = 10;
		pf_isProjection = 1;
	};
	class PF_Seed_MyMaple: PF_Seed_Base
	{
		scope = 2;
		displayName = "Maple Seed";
		projectionTypename = "PF_Tree_MyMaple_Placing";
		pf_plantClass = "PF_Tree_MyMaple_Young";
		pf_isTree = 1;
	};
};
class CfgPlantableFlora
{
	class Wild
	{
		// wild map model (part of its path, lower case) -> seed; the longest match wins
		class MyMaple { source = "tree"; match[] = { "myplants\maple" }; seed = "PF_Seed_MyMaple"; };
	};
};
```

- Bushes work the same way: `PF_Bush_<n>_Seedling` (from `PF_Seedling`, `pf_kind = "bushStep"`) -> `_Young` -> full-grown with `pf_cutClass` and a `_Cut` class (see the Chernarus bushes in the mod's `config.cpp`).
- `pf_world = "<world name>"` on every class makes the species grow only on that map; a map listed in `CfgPlantableFlora exclusiveWorlds[]` only grows its own species.
- `CfgPlantableFlora Soil <world> { texture; limedTexture; }` gives planting holes the look of the map's soil.
- Add the seed to the mission `types.xml` like any item.
- Dial in `pf_groundOffset` in game: with `DebugLogs`, `TreeLineup` and `GroundOffsetOverrides` (development keys) the heights can be changed live.
- A wild model of a map mod that should simply give an **existing** seed (e.g. the map's own spruce -> `PF_Seed_Spruce`) only needs the `CfgPlantableFlora Wild` entry.
