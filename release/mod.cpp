// Mod metadata for the DayZ launcher and the Steam Workshop upload.
// Lives in the root of the packed mod folder: @PlantableFlora\mod.cpp (next to Addons\ and keys\), not inside the addon source.
// pack.bat copies it there. The game resolves image paths in its file system (inside the PBOs, like CF does), not in
// this folder: the images live in the PBO under PlantableFlora\data\gui (loose copies next to this file are only a fallback).
// The Workshop preview image is set separately on the Steam page (Artwork\logo_large.png).
name = "PlantableFlora - Plant Trees and Bushes";
author = "ollio";
version = "0.4.0";
tooltip = "PlantableFlora";
overview = "Dig a planting hole, water it and plant tree and bush seeds found in the world. Seedlings grow into real map trees and bushes over hours: hide your base, grow cover, plant your own orchard. Chernarus, Livonia and Badlands species, real tree shade on Badlands, base protection, limits and decay, fully configurable by server admins. Test build.";
picture = "PlantableFlora\data\gui\picture.paa";
logo = "PlantableFlora\data\gui\logo.paa";
logoSmall = "PlantableFlora\data\gui\logo_small.paa";
logoOver = "PlantableFlora\data\gui\logo_hover.paa";
action = "";
actionURL = "";
