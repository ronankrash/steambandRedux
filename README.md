# Brassdeep

Working title for an original single-player Windows steampunk roguelike.

**Engine:** Godot **4.7.1-stable** (compatibility renderer)  
**Simulation:** turn-based, grid-based  
**Presentation:** animated isometric (2:1)  
**Input:** controller-first (XInput-style), keyboard complete  

Steamband historical sources live under [`reference/steamband/`](reference/steamband/REFERENCE.md) and are **not** part of production builds.

## Bootstrap

```bash
# Validate content + run headless simulation tests
chmod +x tools/run_ci_local.sh
./tools/run_ci_local.sh

# Same checks + Windows export ZIP under build/ (gitignored)
BRASSDEEP_EXPORT=1 ./tools/run_ci_local.sh

# Play (requires display)
./tools/bin/godot --path game
```

CI installs Godot 4.7.1 and export templates, validates content, runs tests, and uploads `Brassdeep-windows.zip`.

## Repository layout

| Path | Purpose |
|------|---------|
| `game/` | Godot project |
| `game/core/` | Deterministic simulation |
| `game/presentation/` | Scenes, UI, isometric view, input |
| `game/content/` | Data-driven definitions |
| `game/assets/` | Original placeholders |
| `tests/` | Extra test notes / mirrors |
| `tools/` | Validation and CI helpers |
| `docs/` | Design and architecture |
| `reference/` | External historical references |
| `build/` | Ignored export output |

## Vertical slice (implemented)

- Character creation: Human / Automaton × Adventurer / Engineer
- Procedural industrial-ruin dungeon
- Turn scheduler with enemy acts after player
- Bump melee, pepperbox firearm + ammo, LOS, poison status, combat log
- Three enemy behaviors: pursuer, ranged sentry, poison spitter
- Inventory, equipment, stacking ammo, one affix system hook
- Workbench crafting with three recipes
- Doors, containers, lever, destructible boiler, fog-of-war
- Save/load schema v1, death summary, restart

## Documentation

- `docs/PRODUCT_VISION.md`
- `docs/ARCHITECTURE.md`
- `docs/STEAMBAND_FEATURE_MATRIX.md`
- `docs/INPUT_AND_HANDHELD_UI.md`
- `docs/DECISIONS.md`
- `NEXT_TASKS.md`
- `AGENTS.md`
