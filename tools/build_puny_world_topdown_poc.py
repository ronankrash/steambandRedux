#!/usr/bin/env python3
"""Build a topdown-v1 proof-of-concept atlas from the CC0 Puny World tileset."""

from __future__ import annotations

from pathlib import Path

from PIL import Image

import generate_topdown_tilesheet as base


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "third_party/assets/puny_world/punyworld-overworld-tileset.png"
OUTPUT = ROOT / "lib/xtra/graf/topdown_poc_puny_world.bmp"
TEMP = ROOT / "build-rescue-sdl2/puny_world_base.bmp"


def paste_cell(atlas: Image.Image, source: Image.Image, tile_index: int, src_col: int, src_row: int) -> None:
    src = source.crop((src_col * 16, src_row * 16, src_col * 16 + 16, src_row * 16 + 16)).convert("RGBA")
    src = src.resize((base.TILE_SIZE, base.TILE_SIZE), Image.Resampling.NEAREST)
    if src.getbbox() is None:
        return
    dst = ((tile_index % base.COLUMNS) * base.TILE_SIZE, (tile_index // base.COLUMNS) * base.TILE_SIZE)
    atlas.paste(src, dst, src)


def main() -> int:
    if not SOURCE.exists():
        raise FileNotFoundError(f"Missing source tileset: {SOURCE}")

    TEMP.parent.mkdir(parents=True, exist_ok=True)
    base.generate(TEMP)
    atlas = Image.open(TEMP).convert("RGBA")
    source = Image.open(SOURCE).convert("RGBA")

    # Top-down v1 tile indices. Cells were chosen from Puny World's terrain,
    # bridge, building, and resource-node areas to create a legal Ultima-like POC.
    mapping = {
        1: (0, 0),    # floor: grass
        2: (0, 2),    # wall: stone/rock edge
        3: (8, 28),   # door: building doorway
        4: (0, 20),   # up stairs: bridge/plank cue
        5: (1, 20),   # down stairs: bridge/plank cue variant
        8: (1, 26),   # generic shop: brown building
        9: (2, 26),   # general store
        10: (5, 26),  # clothing
        11: (8, 26),  # gun shop
        12: (11, 26), # machinist
        13: (1, 32),  # alchemy
        14: (4, 32),  # magic
        15: (13, 32), # black market / red facade
        16: (7, 28),  # home
        17: (4, 2),   # rubble: resource/rocks
        18: (5, 2),   # ore: resource node
    }

    for tile_index, (src_col, src_row) in mapping.items():
        paste_cell(atlas, source, tile_index, src_col, src_row)

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    atlas.convert("RGB").save(OUTPUT, "BMP")
    print(f"Generated {OUTPUT} from {SOURCE}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
