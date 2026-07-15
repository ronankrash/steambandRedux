# C64 / Ultima Tile Acceptance Criteria

Last updated: 2026-07-08

Canonical atlas: `c64-ultima-v1` — 16×16 tiles, 144×64 BMP, 36 categories.

## Palette and pixels

- All tiles use colors from the Ultima V PC EGA palette (16 RGB values in `tools/generate_c64_ultima_tilesheet.py`).
- No bilinear scaling; renderer sets `SDL_HINT_RENDER_SCALE_QUALITY=nearest`.
- Display tile size is 16 or 32 pixels (integer multiples of source size).

## Projection

- Floor, grass, stone, walls, and shops are **orthogonal** (axis-aligned squares).
- Wall tiles read as top caps with dark south/east edges.
- Building tiles show roof triangle + door rectangle.

## Steamband categories

Gun shop, Machinist, Alchemy, Magic, Black Market, and Home each have distinct facade marks in the procedural atlas.

## Verification commands

```bash
python tools/generate_c64_ultima_tilesheet.py
python tools/test_c64_ultima_topdown_integration.py
python tools/validate_topdown_tileset.py lib/xtra/graf/topdown_c64_ultima.bmp --version c64-ultima-v1
```

## Agent sign-off checklist (Round 2)

- Graphics consultant: palette ≤16 colors, orthogonal grammar, integer scale
- Playtester: floor/wall/shops readable at 720p; steampunk shops distinct
- Engineer: `ctest` green; 144×64 loads; DENZI `topdown-v1` path still works
