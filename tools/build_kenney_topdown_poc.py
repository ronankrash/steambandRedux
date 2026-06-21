#!/usr/bin/env python3
"""Build a topdown-v1 proof-of-concept atlas from Kenney's CC0 RPG pack."""

from __future__ import annotations

from pathlib import Path

from PIL import Image

import generate_topdown_tilesheet as base
from topdown_mapping import default_mapping_path, load_mapping, spritesheet_cell_map, spritesheet_cell_size, spritesheet_pitch


ROOT = Path(__file__).resolve().parents[1]
MAPPING = default_mapping_path("topdown-v1-kenney-cc0.json")
OUTPUT = ROOT / "lib/xtra/graf/topdown_poc_kenney.bmp"
TEMP = ROOT / "build-rescue-sdl2/kenney_base.bmp"


def paste_cell(
    atlas: Image.Image,
    source: Image.Image,
    tile_index: int,
    src_col: int,
    src_row: int,
    pitch: int,
    cell_size: int,
) -> None:
    src = source.crop((
        src_col * pitch,
        src_row * pitch,
        src_col * pitch + cell_size,
        src_row * pitch + cell_size,
    )).convert("RGBA")
    src = src.resize((base.TILE_SIZE, base.TILE_SIZE), Image.Resampling.NEAREST)
    if src.getbbox() is None:
        return
    dst = ((tile_index % base.COLUMNS) * base.TILE_SIZE, (tile_index // base.COLUMNS) * base.TILE_SIZE)
    atlas.paste(src, dst, src)


def main() -> int:
    mapping_data = load_mapping(MAPPING)
    source = ROOT / mapping_data.get("source_path", "")
    if not source.exists():
        raise FileNotFoundError(f"Missing source tileset: {source}")

    pitch = spritesheet_pitch(mapping_data)
    cell_size = spritesheet_cell_size(mapping_data)
    mapping = spritesheet_cell_map(mapping_data)

    TEMP.parent.mkdir(parents=True, exist_ok=True)
    base.generate(TEMP)
    atlas = Image.open(TEMP).convert("RGBA")
    sheet = Image.open(source).convert("RGBA")

    for tile_index, (src_col, src_row) in mapping.items():
        paste_cell(atlas, sheet, tile_index, src_col, src_row, pitch, cell_size)

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    atlas.convert("RGB").save(OUTPUT, "BMP")
    print(f"Generated {OUTPUT} from {source} using {MAPPING}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
