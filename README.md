# Brassdeep

Working title for an original single-player Windows steampunk roguelike.

**Engine:** Godot **4.7.1-stable** (compatibility renderer)  
**Simulation:** turn-based, grid-based  
**Presentation:** animated isometric (2:1)  
**Input:** controller-first (XInput-style), keyboard complete  

Steamband historical sources live under [`reference/steamband/`](reference/steamband/REFERENCE.md) and are **not** part of production builds.

## Bootstrap

```bash
chmod +x tools/run_ci_local.sh
./tools/run_ci_local.sh

# Same checks + Windows export ZIP under build/ (gitignored)
BRASSDEEP_EXPORT=1 ./tools/run_ci_local.sh

# Play (requires display)
./tools/bin/godot --path game
```

## Phase 2 playable loop

1. Create one of **36** race/class combinations (compare bonuses on pad).
2. Prepare in **Brassharbor** (merchants, healer, workbench/forge/alchemy, storage).
3. Descend a **5-level** expedition (foundry/mine themes, hazards, boss on depth 5).
4. Fight, loot, craft, spend skill points; extract back to town or die.
5. Save/load schema **v2** (v1 migrates when possible).

## Repository layout

| Path | Purpose |
|------|---------|
| `game/` | Godot project |
| `game/core/` | Deterministic simulation |
| `game/presentation/` | Scenes, UI, isometric view, input |
| `game/content/` | Data-driven definitions |
| `game/assets/` | Original placeholders |
| `tests/` | Test notes |
| `tools/` | Validation and CI helpers |
| `docs/` | Design and architecture |
| `reference/` | External historical references |
| `build/` | Ignored export output |

## Documentation

- `docs/PRODUCT_VISION.md`
- `docs/ARCHITECTURE.md`
- `docs/STEAMBAND_FEATURE_MATRIX.md`
- `docs/INPUT_AND_HANDHELD_UI.md`
- `docs/DECISIONS.md`
- `NEXT_TASKS.md`
- `HANDOFF.md`
- `AGENTS.md`
