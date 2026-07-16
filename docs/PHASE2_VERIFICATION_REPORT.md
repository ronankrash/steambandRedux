# Phase 2 Verification Report

**Branch:** `cursor/brassdeep-phase2-verify-7ff6`  
**Date:** 2026-07-16  
**Engine:** Godot 4.7.1-stable (GL Compatibility)  
**Scope:** Integration / hardening only — no Phase 3 content expansion.

## Ancestry cleanup

| Ref | SHA | Notes |
|-----|-----|-------|
| Dirty PR #15 head (`cursor/brassdeep-phase2-7ff6`) | `0fc8320e302236298425bb24164da127f1ef4cc1` | Contained unrelated C64/DENZI/SDL2 history |
| Clean verify tip (this branch) | (see `git rev-parse HEAD` on PR) | Brassdeep-only commits rebased onto `steambranch` |
| Base | `origin/steambranch` | Preferred merge base |

Clean commit series (onto relocate + Brassdeep):

1. `chore: relocate Steamband codebase under reference/steamband`
2. `feat(brassdeep): bootstrap Godot vertical slice and CI`
3. `feat(content): add Phase 2 data pack`
4. `feat(core): Phase 2 simulation systems and save schema v2`
5. `feat(presentation): event-driven animation and Phase 2 UI`
6. `test(docs): Phase 2 headless coverage and documentation`
7. `feat: add Phase 2 content reachability checker and fix gaps`
8. Verification commits on this branch (CI honesty, E2E, dungeon repair, docs)

Unrelated legacy renderer/tileset commits were **not** rewritten into this history.

## Classification of Phase 2 systems

Evidence uses production paths (`game/core/*`, `game/presentation/*`, content JSON) exercised by presentation E2E (`AutomationDriver` → `InputRouter.command`) unless noted.

| System | Classification | Evidence |
|--------|----------------|----------|
| Title / new game / load / quit | Fully integrated | `main.gd` Mode.TITLE; E2E steps 1–3, save/load |
| Character creation (6×6) | Fully integrated | `main.gd` `_show_create`; content races/classes; E2E |
| Brassharbor hub | Fully integrated | `DungeonGenerator.generate_town`; E2E town play |
| Merchants buy/sell | Fully integrated | `EconomySystem` + `_open_merchant`; E2E buy/sell |
| Storage deposit/withdraw | Fully integrated | `GameSim.storage_*`; E2E |
| Crafting (3 stations) | Fully integrated | `CraftingSystem` + `_open_craft`; E2E crafts |
| Inventory / 9-slot equip / ammo | Fully integrated | `InventorySystem` + `_open_inventory`; E2E equip |
| Skills + spend SP | Fully integrated | `SkillSystem` + `_open_skills`; E2E |
| Expedition start / descend | Fully integrated | `GameSim.start_expedition` / `descend_to`; E2E |
| Melee / ranged combat | Fully integrated | `try_player_move` / `try_player_ranged`; E2E |
| Consumables / quick item | Fully integrated | `InventorySystem.use_item` effects; E2E |
| Pickup / drop | Fully integrated | inventory drop actions; E2E |
| Save schema v2 + load | Fully integrated | `SaveSystem`; E2E pause save → title load |
| Boss on depth 5 | Fully integrated | `_spawn_enemies` boss placement; E2E + seed tests |
| Death / restart | Fully integrated | `_show_dead` / restart → CREATE; E2E |
| Procedural connectivity | Fully integrated (hardened) | `DungeonGenerator._repair_playable_paths`; 500-seed suite |
| Controller input path | Fully integrated | `InputRouter` + menu navigation; E2E + focus audit |
| Content reachability | Fully integrated | `tools/content_reachability.py` → 100% required |
| AnimSequencer FX | Integrated but superficial | Real events, geometric flashes; not polished art |
| Isometric world art | Superficial / placeholder | Colored primitives in `world_view.gd` |
| Ability HUD previously showing `abilstat_*` | Integrated but defective → fixed | Filtered in `_refresh_status` |
| Wine launch of Windows EXE on Linux | Not verifiable (Wine 9) | Stock Godot win64 also crashes; CI uses `windows-latest` |
| ROG Ally hardware playtest | Not verifiable here | See `docs/ROG_ALLY_PLAYTEST.md` — do not claim pass |
| Quests / dialogue / sex-history birth | Missing | Deferred (Phase 3+) |
| Full Steamband parity | Missing | Feature matrix remains Partial |

## Defects found

1. **PR ancestry pollution** — Phase 2 PR carried unrelated C64/DENZI/SDL2 commits.
2. **Unreachable content** — `item_leather`, `item_engineer_goggles`, foundry tags for drones/lamps.
3. **Consumable `use_effect` mismatch** — JSON effects not handled; silent wrong heal path.
4. **Unwinnable maps** — locked gates / destructibles treated as hard blockers in connectivity.
5. **CI import suppression** — historical `|| true` on Godot import (removed).
6. **Presentation E2E gaps** — equip stale IDs, player death mid-path, FOV for ranged, menu step limits.
7. **HUD raw IDs** — `abilstat_*` and `item_*` def ids shown to players.
8. **Death screen prompts** — play prompts lingered after `_show_dead`.
9. **Wine smoke impossible locally** — Godot 4.7.1 PE crashes under Wine 9 before init.

## Defects fixed

1. Clean branch rebased onto `steambranch` (Brassdeep-only).
2. Content reachability gaps closed; checker fails CI on unreachable required IDs.
3. Consumable handlers aligned; unknown effects fail closed.
4. `DungeonGenerator` playable-path repair + seed suite (500 + regression seeds) → **0 failures**.
5. Honest CI: import/tests/export fail hard; screenshots required; Windows smoke on `windows-latest`.
6. `--smoke-test` / `--e2e-test` via `Automation` + `AutomationDriver` on production `main.tscn`.
7. Presentation E2E green (`AUTOMATION_RESULT: PASS`).
8. HUD status/quick-item labels humanized; death prompts refreshed.
9. Stability suite (130) + strengthened unit content assertions (86).

## Systems still superficial

- Placeholder geometric tiles/actors (`world_view.gd` colors).
- No polished audio, particles, or tween telegraphs.
- Town has no ambient NPCs / quests.
- Controller remapping UI not a full settings editor.

## Verification metrics

| Metric | Result |
|--------|--------|
| Unit / sim tests | **86 passed** (`run_headless_tests.gd`) |
| Stability / invariant tests | **130 passed** (`run_stability_tests.gd`) |
| Presentation E2E | **PASS** (InputRouter-driven) |
| Dungeon seeds | **500 + 10 regression**, **0 failures** |
| Content reachability | **100%** (132/132 required) |
| Focus graph audit | **PASS** |
| Screenshots (1280×720) | **17 PNGs** under `build/reports/screenshots/` |
| Windows ZIP | `build/Brassdeep-windows.zip` (exe + pck + console) |
| Windows launch | **CI `windows-latest` job** runs `--smoke-test` on exported EXE; local Wine 9 **not** viable |
| ROG Ally | Manual guide only — **not run** |

## Screenshot inspection notes

Captured from live Godot (not mockups). Observations:

- Usable controller legends on most screens.
- Large unused black margins around iso view (placeholder art budget).
- Fixed: raw `abilstat_*` / `item_*` HUD strings.
- Character creation shows both race and class `>` markers (selection state clarity could improve later).
- Contrast of white-on-black menus is readable at 1280×720.

## Remaining manual checks

Follow `docs/ROG_ALLY_PLAYTEST.md` on real Windows handheld hardware. Do not mark Ally verification complete until that procedure is executed on-device.
