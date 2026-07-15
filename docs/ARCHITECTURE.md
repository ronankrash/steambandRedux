# Architecture — Brassdeep

## Layers

### Simulation (`game/core/`)

| Type | Role |
|------|------|
| `BrassRng` | Seeded xorshift RNG |
| `GridPos` | Integer grid coordinates |
| `ContentDB` | Loads/validates JSON content by string ID |
| `SimWorld` | Tiles, FOV, ground items, features |
| `SimActor` / `SimItem` | Entities and inventory objects |
| `CombatResolver` | Melee/ranged/LOS |
| `InventorySystem` / `CraftingSystem` | Items and recipes |
| `DungeonGenerator` | Procedural ruin layout |
| `GameSim` | Turn loop and player/enemy actions |
| `SaveSystem` | Schema-versioned JSON saves |

Simulation types are `RefCounted` / plain data. They do not reference sprites, AnimationPlayer, or UI nodes.

### Presentation (`game/presentation/`)

- `IsoMath` — 2:1 projection and depth keys
- `WorldView` — draws explored/visible tiles and actors
- `InputRouter` — keyboard + joypad → command signals
- `main.tscn` / `main.gd` — menus, play loop, HUD

Presentation calls into `GameServices.sim` and reacts to results. Animation may be added later to tween between committed grid positions; animation must not author game state.

### Content (`game/content/`)

JSON folders: `races`, `classes`, `skills`, `monsters`, `items`, `recipes`, `affixes`. Stable `id` fields are the only persistent identities.

## Turn flow

1. Player issues a command (move/attack/interact/craft/use).
2. Simulation resolves the action and mutates state.
3. Living enemies receive actions.
4. Status/environment ticks.
5. FOV updates.
6. Presentation redraws / will later play resolution tweens while `awaiting_visual` blocks input.

## RNG policy

- One `BrassRng` instance per run, seeded at `new_game`.
- Save files store `rng_state` for exact resume.
- Content loading is non-random.

## Save architecture

- Path: `user://brassdeep_save.json`
- `schema_version: 1`
- Unsupported versions fail closed with an explicit reason.

## Autoloads

Only `GameServices` — narrow locator for `content`, `sim`, input device hint, and save path. World state lives on `GameSim`, not in globals.
