#!/usr/bin/env python3
"""Build the integrated topdown-v1 atlas from DENZI's Ultima VI oblique tilesets."""

from __future__ import annotations

from pathlib import Path

from PIL import Image

import generate_topdown_tilesheet as base
from topdown_mapping import (
    default_mapping_path,
    entry_source_path,
    load_mapping,
    spritesheet_cell_size,
    spritesheet_pitch,
    spritesheet_sources,
)


ROOT = Path(__file__).resolve().parents[1]
MAPPING = default_mapping_path("topdown-v1-denzi-cc-by-sa.json")
OUTPUT = ROOT / "lib/xtra/graf/topdown_denzi.bmp"
TEMP = ROOT / "build-rescue-sdl2/denzi_base.bmp"


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


def paint_solid_black(atlas: Image.Image, tile_index: int) -> None:
    tile = Image.new("RGBA", (base.TILE_SIZE, base.TILE_SIZE), (0, 0, 0, 255))
    dst = ((tile_index % base.COLUMNS) * base.TILE_SIZE, (tile_index // base.COLUMNS) * base.TILE_SIZE)
    atlas.paste(tile, dst)


def load_sheet_cache(mapping_data: dict, root: Path) -> dict[str, Image.Image]:
    cache: dict[str, Image.Image] = {}
    for sheet_name, rel_path in spritesheet_sources(mapping_data).items():
        path = root / rel_path
        if not path.exists():
            raise FileNotFoundError(f"Missing source sheet '{sheet_name}': {path}")
        cache[sheet_name] = Image.open(path).convert("RGBA")
    return cache


def main() -> int:
    mapping_data = load_mapping(MAPPING)
    pitch = spritesheet_pitch(mapping_data)
    cell_size = spritesheet_cell_size(mapping_data)
    sheets = load_sheet_cache(mapping_data, ROOT)

    TEMP.parent.mkdir(parents=True, exist_ok=True)
    base.generate(TEMP)
    atlas = Image.open(TEMP).convert("RGBA")

    for entry in mapping_data["entries"]:
        tile_index = int(entry["index"])
        special = entry.get("special")
        if special == "solid_black":
            paint_solid_black(atlas, tile_index)
            continue

        rel_path = entry_source_path(mapping_data, entry)
        sheet_name = entry.get("sheet", mapping_data.get("default_sheet", "map"))
        source = sheets[str(sheet_name)]
        paste_cell(
            atlas,
            source,
            tile_index,
            int(entry["source_col"]),
            int(entry["source_row"]),
            pitch,
            cell_size,
        )

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    atlas.convert("RGB").save(OUTPUT, "BMP")
    print(f"Generated {OUTPUT} from DENZI sources using {MAPPING}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
