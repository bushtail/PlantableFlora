# Changelog

Format follows [Keep a Changelog](https://keepachangelog.com/). The version is `version` in `release/mod.cpp`.
Config changes: new keys are added to an existing `config.json` automatically, existing values stay.

## [0.4.0] - 2026-10-31

### Changed
- Felling a planted tree works like the same wild tree: one log or firewood per chopping cycle with the wild tree's amounts and tool wear, then the tree falls over away from you (vanilla fall sound) and lies on the ground without collision until the next server restart. New config keys `FellLogs`, `FellSticks` (trees without a wild version), `FallenTreeMinutes`, `FallenTreeCollision` (also gives felled wild trees their collision back).

## [0.3.1] - 2026-10-27

### Added
- Expansion BaseBuilding support (beta, tested solo): with Expansion the base rules use Expansion's claimed territories, their members and `TerritorySize` instead of the own flag tracking. An unclaimed flag pole is no base. Digging a planting hole, watering, fertilizing, cutting, digging up and felling are not possible in Expansion safezones (searching wild trees for seeds still is). Admin guide 4.2.

### Changed
- New logo.
- The planting preview repeats the vanilla placement checks itself (step off the spot, too far above/below you, under water, aim at the ground). With Expansion's `CanBuildAnywhere` these were skipped; without Expansion the existing rejections now show a reason.

## [0.3.0] - 2026-10-23

### Added
- Cutting small wild trees and saplings that the game treats as bushes (small spruces and pines, beech/oak/birch saplings) now can give the matching tree seed (forest models give the forest seed). Livonia's dwarf birch and Manchurian hazel give the dwarf birch / hazel seed. Works on every map with these models (Chernarus, Livonia, Frostline, Deer Isle).
- Black poplar is plantable again (the tall, narrow roadside tree; grows to 34.5 m, the tallest planted tree). "Search for seeds" on a wild black poplar gives its seed; loot entry in `types_PlantableFlora.xml`.
- Snowy areas: a planted spruce, birch or dwarf birch takes the look of the wild plants around it. If most wild trees nearby are snow-covered (Frostline winter models, Deer Isle snow spruces) it grows the snowy version, next to frozen spruces the frozen one. Same seed as before. Species without a snowy model refuse to grow there ("Too cold here").
- Frozen ground on winter maps: snow ground needs a fire before you can dig a planting hole or plant. A fireplace or fire barrel burning hot for 5 minutes thaws the ground within 2 m, and it stays thawed for 30 minutes after the fire is out. Frostline: the snow surfaces (brown grass stays plantable as before); Namalsk: snow and snowy grass; other winter maps (e.g. Winter Chernarus): every surface with the vanilla melt patch under fires. New settings `FrozenGround`, `ThawBurnMinutes`, `ThawMinutes`, `ThawRadius`, `FrozenSurfaces` (admin guide 3.6).

### Changed
- Every plant gets a random rotation when planted. Before, plants of one species planted facing the same way grew into identical trees.
- Forest oak grows into the full forest oak (28.7 m) instead of the small one, which looked like the middle stage.

### Fixed
- Some grown trees stood up to 4 m away from where the seedling was planted (forest oak, fig, olive, hornbeam, Turkish pine, date palm, and up to 0.8 m for several others): the game's tree models don't all have the trunk in the middle. Every stage and the stump now stand on the planted spot. Already grown trees stay where they are.

### Config
- New keys: `FrozenGround` (true), `ThawBurnMinutes` (5), `ThawMinutes` (30), `ThawRadius` (2), `FrozenSurfaces` ([]). Added to an existing `config.json` automatically.

## [0.2.0] - 2026-10-22

### Added
- **Real shade on Badlands.** Vanilla Badlands adds sun heat to players in the open (up to +8 °C at noon, +9.6 °C without a hat) but never counted trees as shade. Now the crown of any tree, wild or planted, is shade: no sun heat underneath. The shade follows the sun (under the crown at noon, away from the sun in the morning and evening). Other maps have no sun heat and are not affected. Off with `TreeShade` false.
- Shade overlay for admins: `ShadeDebug` + `AdminIds` (Steam64 IDs). Shows bottom left whether you stand in tree shade and how much sun heat it removes; Numpad 9 shows/hides it.

### Changed
- Fewer seeds: searching a tree, cutting a wild bush and "Get seeds" from fruit now give 0-2 seeds (nothing 50 %, 1 seed 40 %, 2 seeds 10 %) instead of 0-3. The same odds for all three.

### Fixed
- Conflict with ScopeGlint: both mods used the same network message ids, so with both installed one mod could swallow the other's messages (e.g. seedlings not showing or growing). PlantableFlora now uses its own ids.

### Config
- New keys: `TreeShade` (true), `ShadeDebug` (false), `AdminIds` ([]), `SeedOneChance` (0.4), `SeedTwoChance` (0.1). Added to an existing `config.json` automatically.
- Removed: `TreeSeedChance`, `WildBushSeedChance` (replaced by `SeedOneChance` / `SeedTwoChance`). If you had changed them, set the new keys instead.

## [0.1.0] - 2026-10-20

First Workshop test build: https://steamcommunity.com/sharedfiles/filedetails/?id=3823449997

### Added
- Planting: dig a planting hole (shovel), water it (required by default), optionally fertilize it (garden lime, plant material: faster growth), plant a seed. Empty vanilla garden plots work too.
- Own seedling model on a soil heap as the first stage of every tree and bush; it keeps the hole's look (limed, compost, wet).
- Trees: seedling → young tree → full-grown, fell with an axe (logs, sticks, stump where vanilla has one). Bushes: seedling → young → full-grown, cut (sticks, grows back) or dig up.
- Species: Chernarus / Livonia: spruce, oak, pine, beech, birch, larch, ash, walnut, hornbeam, white poplar, willow, rowan, acacia, apple, pear, plum (+ forest variants of spruce, oak, pine, beech, birch, larch, acacia); rose, hazel, hawthorn, blackthorn, elder, dwarf birch. Badlands: fig, Turkish pine, olive, date palm, poplar, ash, plum; oleander, fig bush, prickly pear, cycas, Syrian rue.
- Seeds from loot (one loot file per map), wild tree search (0-3), wild bush cut (0-3), fruit ("Search for fruit", then "Get seeds" with a knife, 0-3). Grown planted trees give seeds and fruit like wild ones.
- Seeds only grow on their own map ("This seed won't grow here"); seed names hidden by default (`ShowSeedSpecies`).
- Placement rules: fertile ground, distance to roads and buildings, tree spacing, no planting in other players' bases or on stashes (silent), in caves or admin no-plant zones; optional strict mode `RequireOwnFlag`.
- Limits: per player outside the own bases (rolling: the oldest disappears when a new one is grown), per player inside the own bases, per base (all members), per server.
- Decay like base building: plants within an active flag radius live, others disappear after `PlantLifetimeDays`.
- Admin tools: `stats.txt`, `WipeAllPlants`, `NoPlantZones`; messages as notification popups.
