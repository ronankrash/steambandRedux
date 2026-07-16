# Architecture — Brassdeep

## Layers

### Simulation (`game/core/`)

| Type | Role |
|------|------|
| `BrassRng` | Seeded xorshift RNG |
| `GridPos` | Integer grid coordinates |
| `ContentDB` | Loads/validates JSON content by string ID |
| `SimWorld` | Tiles, FOV, ground items, feature meta |
| `SimActor` / `SimItem` | Entities, skills, equipment, encumbrance |
| `CombatResolver` | Melee/ranged/LOS/crits |
| `InventorySystem` / `CraftingSystem` | Items, stations, recipes |
| `SkillSystem` | Ranks, costs, affinities, abilities |
| `EconomySystem` | Merchants, prices, restock |
| `AiController` | Deterministic monster decisions |
| `ItemGenerator` | Seeded affix rolls |
| `DungeonGenerator` | Town hub + multi-level expeditions |
| `GameSim` | Turn loop, hub/expedition flow |
| `SaveSystem` | Schema v2 JSON (+ v1 migration) |

Simulation types are `RefCounted` / plain data. They do not reference sprites, AnimationPlayer, or UI nodes.

### Presentation (`game/presentation/`)

- `IsoMath` — 2:1 projection and depth keys
- `WorldView` — isometric draw + camera tracking
- `AnimSequencer` — event-driven move/attack/death visuals; sets `awaiting_visual`
- `InputRouter` — keyboard + joypad → command signals
- `main.tscn` / `main.gd` — menus, HUD, hub/expedition UI

Presentation calls into `GameServices.sim` and reacts to results. Animation never authors combat outcomes.

### Content (`game/content/`)

JSON folders: `races`, `classes`, `skills`, `monsters`, `items`, `recipes`, `affixes`, `merchants`, `environments`. Stable `id` fields are the only persistent identities.

## Turn flow

1. Player issues a command.
2. Simulation resolves and mutates state; emits signals.
3. Enemies act (expedition only); statuses/environment tick; FOV updates.
4. `AnimSequencer` plays committed results while input is locked.

## RNG policy

- One `BrassRng` per run, seeded at `new_game`.
- Saves store `rng_state`.
- Affix rolls and AI RNG draws use the same stream.

## Save architecture

- Path: `user://brassdeep_save.json`
- `schema_version: 2` (hub depth, merchants, storage, skills, settings)
- Schema 1 migrates when practical; unknown versions fail with an explicit message.

## Autoloads

Only `GameServices` — locator for `content`, `sim`, input device hint, and save path.
