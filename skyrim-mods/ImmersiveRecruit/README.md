# Immersive Recruit

SKSE plugin, no ESP. Any NPC whose relationship with you reaches **Friend** can be recruited as a follower
with the vanilla "Follow me" line. Once they follow you, they **level with you** and become **Protected**.

## Features

### Recruit your friends
- The game only offers "Follow me" when an NPC is in `PotentialFollowerFaction` **and** is your Friend.
  Many NPCs (e.g. Aeri in Anga's Mill) become your Friend through quests or favors, but are not in that faction.
- After every conversation, Immersive Recruit checks the NPC you talked to. If the relationship is at least `iMinRank`
  (Friend by default), they are added to `PotentialFollowerFaction` and you get a notification.
- Talk to them again and "Follow me" is there. Works with Nether's Follower Framework and other frameworks that
  use the vanilla follower dialogue.
- NPCs who were already your Friend before installing: talk once, close the dialogue, talk again.
- Skipped: dead NPCs, children, hostile NPCs and anyone in `[Exclude]` (all configurable).

### Followers that keep up with you
Ordinary NPCs have a fixed level, so they fall behind quickly as followers. While an NPC from
`PotentialFollowerFaction` is following you (checked a few seconds after recruiting, and on every save load):
- **Fixed-level NPCs** level with the player (up to `iMaxLevel`, 80 by default), like NFF's `nfflevel` batch file (`setlevel 1000 0 1 0`),
  but automatic and without making them weaker: their original level becomes the minimum.
- **NPCs that already level with you but stop early** (e.g. capped at level 25) get their cap raised to `iMaxLevel` (80 by default).
- **Protected**: only the player can kill them. Essential NPCs stay Essential.
- Only unique NPCs are changed, so generic NPCs sharing one base record (guards, raised thralls...) are never affected.
- NPCs that take their stats from a template cannot be changed (logged).
- Health and other stats may need a cell change or a save reload to update to the new level.

## Requirements
- [SKSE64](https://skse.silverlock.org/)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
- Tested on Skyrim AE 1.6.1170. Built with CommonLibSSE-NG (SE and AE), 1.5.97 should work but is untested.

## Installation
Install with your mod manager, or copy `SKSE/Plugins/ImmersiveRecruit.dll` and `SKSE/Plugins/ImmersiveRecruit.ini` into `Data/`.
Safe to install mid-playthrough.

## Configuration
Everything is in `SKSE/Plugins/ImmersiveRecruit.ini`; see the comments in the file.

| Setting | Default | |
|---|---|---|
| `iMinRank` | 1 | Minimum relationship: 1 Friend, 2 Confidant, 3 Ally, 4 Lover |
| `bNotify` | 1 | On-screen notifications |
| `bSkipChildren`, `bSkipHostile` | 1 | Ignore children / hostile NPCs |
| `bScaleWithPlayer` | 1 | Fixed-level followers level with the player |
| `iLevelMultPercent` | 100 | Follower level = player level × this % |
| `bKeepCurrentLevelAsMin` | 1 | Never drop below the original level |
| `iMaxLevel` | 80 | Level cap, 0 = none. NPCs whose own level is higher keep it |
| `bRemoveLevelCap` | 1 | Raise the cap of NPCs that already level with you |
| `bProtected` | 1 | Only the player can kill followers |
| `bOnlyUnique` | 1 | Only change unique NPCs |

### Excluding NPCs
Add a line under `[Exclude]`: `AnyName=Plugin.esp|0xFormID`. To get the form ID, open the console, click the NPC and
type `GetBaseObject` (or use More Informative Console). Drop the load order prefix: `0A012345` from `Skyrim.esm`
becomes `Skyrim.esm|0x012345`. For ESL plugins (`FExxxyyy`) use the last 3 digits.

Consider excluding NPCs with important quest roles until their questline is done (e.g. Brynjolf before the
Thieves Guild questline ends).

## Compatibility
- **Relationship Dialogue Overhaul (RDO):** compatible. NPCs that RDO already made recruitable are left alone.
- **Nether's Follower Framework:** compatible, recruited NPCs show up in NFF as usual.
- No ESP and no edited records, so it does not conflict with other plugins.

## Uninstalling
Remove the DLL and INI. NPCs that were already made recruitable stay recruitable, and the level and Protected
changes stay in your save, the same way console commands such as `setlevel` are stored. Nothing breaks: they are
ordinary game values. New NPCs are no longer changed.

## Changelog
- 1.0.0: Default level cap 80 (`iMaxLevel`).
- 0.3.0: Remove the level cap of followers that already level with the player (`bRemoveLevelCap`).
- 0.2.0: Followers level with the player and become Protected (`[Follower]` section).
- 0.1.0: Initial release.
