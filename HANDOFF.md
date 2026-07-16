# Brassdeep Handoff

## Current state

- Branch: `cursor/brassdeep-phase2-7ff6` (Phase 2 demo)
- Engine: Godot **4.7.1-stable**, compatibility renderer
- Legacy Steamband under `reference/steamband/` (reference only)
- Playable loop: create operative → Brassharbor hub → buy/sell/craft/storage → 5-depth expedition → extract or die → save/load

## Implemented (Phase 2)

- Event-driven `AnimSequencer` (move/attack/death FX, normal/fast)
- 6 races × 6 classes, skill system (8 families, 14 abilities)
- 9 equipment slots, encumbrance, inventory ops, HUD
- 3 crafting stations, 20+ recipes, 3 merchants + healer + storage
- Multi-level expeditions (foundry/mine), 13 monsters incl. boss
- XP/level + skill points; progression model ADR-007
- Save schema **v2** with v1 migration

## Verification

```bash
python3 tools/validate_content.py
./tools/run_ci_local.sh
BRASSDEEP_EXPORT=1 ./tools/run_ci_local.sh
```

Headless tests: **84 passed**.

## Risks / limitations

- Placeholder geometric art (original, not polished)
- No quests/dialogue; town is functional hub only
- Controller remapping not yet a full settings editor
- Steamband parity remains partial (see feature matrix)
- Licensing: do not reuse Steamband data/prose in production content

## Next

See `NEXT_TASKS.md`.
