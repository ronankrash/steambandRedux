# Brassdeep Handoff

## Current state

- Branch: `cursor/brassdeep-phase2-verify-7ff6` (Phase 2 verification / hardening)
- Replaces dirty PR #15 ancestry (`cursor/brassdeep-phase2-7ff6` @ `0fc8320`)
- Engine: Godot **4.7.1-stable**, compatibility renderer
- Legacy Steamband under `reference/steamband/` (reference only)
- Playable loop: create operative → Brassharbor hub → buy/sell/craft/storage → 5-depth expedition → extract or die → save/load

## Implemented (Phase 2) — player-reachable

- Event-driven `AnimSequencer` (move/attack/death FX, normal/fast)
- 6 races × 6 classes, skill system (8 families, 14 abilities)
- 9 equipment slots, encumbrance, inventory ops, HUD
- 3 crafting stations, 20+ recipes, 3 merchants + healer + storage
- Multi-level expeditions (foundry/mine), 13 monsters incl. boss
- XP/level + skill points; progression model ADR-007
- Save schema **v2** with v1 migration
- Presentation E2E via `InputRouter` (`--e2e-test` / `run_presentation_e2e.gd`)
- Content reachability CI (`tools/content_reachability.py`) — 100% required
- Dungeon connectivity suite (500 seeds + regressions)
- Windows export + **windows-latest** `--smoke-test` launch job

## Verification

```bash
python3 tools/validate_content.py
python3 tools/content_reachability.py
python3 tools/focus_graph_audit.py
./tools/run_ci_local.sh
BRASSDEEP_EXPORT=1 ./tools/run_ci_local.sh
BRASSDEEP_SCREENSHOTS=1 ./tools/run_ci_local.sh
```

| Suite | Count / result |
|-------|----------------|
| Unit / sim | **86 passed** |
| Stability | **130 passed** |
| Presentation E2E | **PASS** |
| Dungeon seeds | **500 + regressions, 0 failures** |
| Reachability | **132/132 (100%)** |

See `docs/PHASE2_VERIFICATION_REPORT.md` and `docs/ROG_ALLY_PLAYTEST.md`.

## Risks / limitations

- Placeholder geometric art (original, not polished)
- No quests/dialogue; town is functional hub only
- Controller remapping not yet a full settings editor
- Steamband parity remains partial (see feature matrix)
- Licensing: do not reuse Steamband data/prose in production content
- Local Wine 9 cannot launch Godot 4.7.1 Windows builds (CI uses native Windows runner)
- ROG Ally playtest not executed in this environment

## Next

See `NEXT_TASKS.md`. Do **not** begin Phase 3 feature expansion until verification PR is accepted.
