# SDL2 Top-Down Asset Coverage

Last generated: by `python tools/report_topdown_asset_coverage.py`

This report maps the current SDL2 top-down tile contract to legacy game data in
`lib/edit/terrain.txt`, `lib/edit/object.txt`, and `lib/edit/monster.txt`.

## Current Atlas

The checked-in placeholder atlas is `lib/xtra/graf/sdl2_topdown_24.bmp`.
It is project-generated and currently provides the `topdown-v1` contract with
36 broad tiles:

- darkness
- floor
- wall
- door
- up stairs
- down stairs
- trap
- glyph
- shop
- General Store shop
- Clothing store shop
- Gun shop
- Machinist shop
- Alchemy shop
- Magic shop
- Black Market shop
- Home
- rubble
- ore vein
- object
- food/anodyne object
- scroll/book object
- potion/flask object
- weapon/tool object
- armor object
- ray gun/launcher object
- ammo object
- money object
- jewelry object
- device/chest object
- generic monster
- automata
- undead/demon
- beast
- humanoid
- player

## Terrain Coverage

Terrain is matched to current broad tiles, including shops, glyphs, rubble, and
ore veins. Store types and individual trap kinds still share broad family tiles.

- `darkness`: 1 entries. Examples: 0: <darkness>
- `door`: 18 entries. Examples: 4: open door; 5: broken door; 32: door; 33: locked door
- `down stairs`: 1 entries. Examples: 7: down staircase
- `floor`: 2 entries. Examples: 1: open floor; 2: invisible trap
- `glyph`: 1 entries. Examples: 3: glyph of warding
- `ore`: 6 entries. Examples: 50: magma vein; 51: quartz vein; 52: magma vein; 53: quartz vein
- `rubble`: 1 entries. Examples: 49: pile of rubble
- `shop`: 8 entries. Examples: 8: General Store; 9: Clothing store; 10: Gun shop; 11: Machineist shop
- `trap`: 16 entries. Examples: 16: trap door; 17: pit; 18: pit; 19: pit
- `up stairs`: 1 entries. Examples: 6: up staircase
- `wall`: 9 entries. Examples: 48: secret door; 56: granite wall; 57: granite wall; 58: granite wall

## Object Coverage

Objects are now matched to broad item-family tiles in the live renderer when
bounded `cave_o_idx -> o_list` data is available. Unknown or invalid object data
falls back to terrain or the generic object tile. The groups below are the
coverage buckets used to guide future art polish.

- `ammo`: 6 entries. Examples: 78: & Rifle bullet~; 79: & Hollow-point rifle bullet~; 80: & Shotgun buckshot; 81: & Steel shotgun slug~
- `amulet`: 13 entries. Examples: 163: Wisdom; 164: Charisma; 165: Searching; 166: Teleportation
- `armor`: 77 entries. Examples: 91: & Pair~ of Soft Leather Shoes; 92: & Pair~ of Hard Leather Shoes; 93: & Pair~ of Metal Shod Boots; 94: & Hard Leather Cap~
- `device/light`: 18 entries. Examples: 338: & Small wooden chest~; 339: & Large wooden chest~; 340: & Small iron chest~; 341: & Large iron chest~
- `food/anodyne`: 36 entries. Examples: 1: Blinding; 2: Bad Tallow weed; 3: Ether; 4: Peyote
- `launcher/ray gun`: 15 entries. Examples: 73: & .22 Bolt Action Rifle~; 74: & .30 Lever-Action Carbine~; 75: & 20 Gauge Shotgun~; 76: & 16 Gauge Shotgun~
- `money`: 18 entries. Examples: 480: copper; 481: copper; 482: copper; 483: silver
- `pile`: 1 entries. Examples: 0: <pile>
- `potion/flask`: 58 entries. Examples: 222: Slime Mold Juice; 223: Apple Juice; 224: Water; 225: Strength
- `ring`: 35 entries. Examples: 132: Muscle; 133: Agility; 134: Constitution; 135: Mental Aptitude
- `scroll/book`: 61 entries. Examples: 173: Enchant Weapon To-Hit; 174: Enchant Weapon To-Dam; 175: Enchant Armor; 176: Identify
- `tval 35`: 3 entries. Examples: 123: & Cloak~; 124: & Shadow Cloak~; 525: & Fur Cloak~
- `tval 36`: 7 entries. Examples: 101: & Robe~; 102: & Filthy Rag~; 103: & Leather Waistcoat~; 104: & Leather Waistcoat~ and Vest~
- `tval 55`: 30 entries. Examples: 300: Trap Location; 301: Treasure Location; 302: Object Location; 303: Teleportation
- `tval 65`: 29 entries. Examples: 269: Photic Beam; 270: Voltic Blast; 271: Icy Blast; 272: Flame
- `tval 66`: 27 entries. Examples: 351: Door/Stair Location; 352: Trap Location; 353: Probing; 354: Recall
- `weapon/tool`: 54 entries. Examples: 30: & Broken Dagger~; 31: & Bastard Sword~; 32: & Scimitar~; 33: & Cane~

## Monster Coverage

Monsters are partially matched. The renderer uses live `r_info` metadata to map
visible monsters into broad family tiles; unknown or unmapped races use the
generic monster tile.

- `automata`: 66 entries. Examples: 27: Strange terminal; 31: Maintance automaton - Type I; 49: Maintenance automaton - Type II; 75: Luminary terminal
- `beast`: 148 entries. Examples: 2: Scrawny cat; 3: Scruffy little dog; 17: Giant yellow centipede; 18: Giant white centipede
- `generic monster`: 86 entries. Examples: 15: Grey mold; 16: Grey mushroom patch; 19: White icky thing; 20: Clear icky thing
- `humanoid`: 89 entries. Examples: 1: Filthy urchin; 4: Nellie Bly; 5: Blubbering idiot; 6: Boil-covered wretch
- `player`: 1 entries. Examples: 0: <player>
- `undead/demon`: 64 entries. Examples: 107: Skeleton dog-man; 147: Zombified Dog-man; 161: Lot No. 249; 165: Skeleton pig-man

## Gaps To Close Before A Real Tileset Claim

- Split shop/town facades by store type after the base shop tile is visually proven.
- Split object-family placeholders into higher-quality final art for food,
  scroll/books, potions, weapons/tools, armor, ray guns/launchers, ammo, money,
  devices/lights, rings, and amulets.
- Consider separate monster tiles for dragon, alien, animal, brute, undead,
  demon, automata, townsfolk/humanoid, and generic oddities instead of the
  current broad buckets.
- Add tile contract versioning before changing atlas geometry again.
- Keep all external art out of the repo until `ASSETS.md` records source,
  author, exact license, and commercial-use status.
