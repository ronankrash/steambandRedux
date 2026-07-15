# Brassdeep Handoff

## Current state

- Legacy Steamband tree relocated to `reference/steamband/` (reference only; licensing uncertain for reuse).
- New Godot **4.7.1-stable** project under `game/` with simulation / presentation / content layers.
- Vertical slice playable: create → explore → combat → loot → craft → save/load → death/restart.
- Headless tests + `tools/validate_content.py` + GitHub Actions Windows export workflow.

## Verified

- Content validation passes.
- Headless simulation tests cover RNG, turns, melee, LOS, stacking, equipment, affixes, crafting, skill points, save schema.

## Risks

- Steamband reference materials are **not** cleared for commercial reuse; Brassdeep content is original.
- Presentation art is geometric placeholders (readable, not polished).
- Visual animation lock is a short timer, not full tweens yet.
- Windows export requires Godot export templates (installed in CI).

## Next

See `NEXT_TASKS.md`.
