# Topdown-v1 Mapping Specs

The SDL2 top-down renderer uses a fixed `topdown-v1` atlas contract: 36 tiles,
24x24 pixels, 9 columns by 4 rows (216x96 BMP). Category order is defined in
`tools/generate_topdown_tilesheet.py` and documented in `ASSETS.md`.

Mapping JSON files describe how external art maps into those 36 slots without
hard-coding choices inside Python scripts.

## Files

| File | Purpose | Shippable |
|------|---------|-----------|
| `tools/mappings/topdown-v1-ultima5-local.json` | Local Ultima V `tiles.16` index map | No — proprietary reference only |
| `tools/mappings/topdown-v1-kenney-cc0.json` | Kenney Roguelike/RPG pack cells | Yes — CC0 |
| `tools/mappings/topdown-v1-puny-world-cc0.json` | Puny World overworld cells | Yes — CC0 |
| `tools/mappings/topdown-v1-denzi-cc-by-sa.json` | DENZI Ultima VI oblique map/items/monsters/character cells | Yes — CC-BY-SA 3.0 |

## JSON Schema

Common fields:

- `atlas_version`: must be `topdown-v1`
- `mapping_kind`: `ultima5_tile_index` or `spritesheet_cell`
- `source_name`: human-readable source label
- `license_note`: licensing guidance
- `entries`: array of slot mappings

Each entry includes:

- `index`: topdown-v1 slot (0–35)
- `category`: must match the contract name for that index
- source coordinates:
  - Ultima: `source_tile` (integer index into uncompressed `tiles.16`)
  - Spritesheet: `source_col`, `source_row` (grid cell in the source PNG)

Spritesheet mappings may also include:

- `source_pitch`: pixel stride between cells (Kenney uses 17, Puny World uses 16)
- `cell_size`: cropped cell size (usually 16)
- `source_sheets`: optional map of sheet names to source paths for multi-sheet packs (DENZI)
- `sheet`: per-entry sheet name when using `source_sheets`
- `special`: procedural atlas slot overrides such as `solid_black` for darkness

Partial mappings are allowed. Unmapped slots keep the procedural placeholder
from `generate_topdown_tilesheet.py`.

## Tools

```bash
# Validate all checked-in mapping JSON
python tools/validate_topdown_mappings.py

# Local Ultima V conversion (uses ultima5-local.json by default)
python tools/convert_ultima5_tiles.py --tiles16 local_assets/ultima5/tiles.16

# Regenerate CC0 POC atlases from mapping files
python tools/build_kenney_topdown_poc.py
python tools/build_puny_world_topdown_poc.py

# Regenerate integrated DENZI atlas
python tools/build_denzi_topdown.py
python tools/test_denzi_topdown_integration.py
```

## Refining Ultima Mappings

Ultima V stores tiles in a shiftable list; runtime swaps can change which pixel
art appears at a given dungeon code. Treat `topdown-v1-ultima5-local.json` as a
starting point, then:

1. Edit `source_tile` values in the JSON, or
2. Pass `--mapping index:tile` overrides on the converter command line.

Monster slots use indices `>= 256` in `tiles.16` per Ultima Codex dungeon
monster-tile notes.
