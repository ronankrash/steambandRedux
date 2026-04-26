#!/usr/bin/env python3
"""Build a topdown-v1 proof-of-concept atlas from Kenney's CC0 RPG pack."""

from __future__ import annotations

from pathlib import Path

from PIL import Image

import generate_topdown_tilesheet as base


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "third_party/assets/kenney_roguelike_rpg/Spritesheet/roguelikeSheet_transparent.png"
OUTPUT = ROOT / "lib/xtra/graf/topdown_poc_kenney.bmp"
TEMP = ROOT / "build-rescue-sdl2/kenney_base.bmp"
SOURCE_TILE = 16
SOURCE_PITCH = 17


def paste_cell(atlas: Image.Image, source: Image.Image, tile_index: int, src_col: int, src_row: int) -> None:
    src = source.crop((
        src_col * SOURCE_PITCH,
        src_row * SOURCE_PITCH,
        src_col * SOURCE_PITCH + SOURCE_TILE,
        src_row * SOURCE_PITCH + SOURCE_TILE,
    )).convert("RGBA")
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

    # Topdown-v1 target index -> Kenney spritesheet cell. The chosen cells are
    # legal CC0 POC stand-ins for old-school RPG terrain, buildings, loot, and
    # creature/icon silhouettes while final art direction is still evolving.
    mapping = {
        1: (42, 26),  # floor: green ground
        2: (24, 12),  # wall: grey block
        3: (42, 5),   # door
        4: (39, 9),   # up stairs/ladder-ish marker
        5: (40, 9),   # down stairs/ladder-ish marker
        6: (50, 28),  # trap/hazard marker
        7: (55, 1),   # glyph/sigil
        8: (32, 0),   # generic shop/building facade
        9: (33, 0),   # general store
        10: (34, 0),  # clothing
        11: (35, 0),  # gun shop
        12: (36, 0),  # machinist
        13: (37, 0),  # alchemy
        14: (38, 0),  # magic
        15: (39, 0),  # black market
        16: (40, 0),  # home
        17: (6, 4),   # rubble/rocks
        18: (42, 23), # ore/resource
        19: (42, 12), # generic object
        20: (44, 17), # food
        21: (44, 15), # scroll/book
        22: (45, 16), # potion/flask
        23: (38, 18), # weapon/tool
        24: (44, 8),  # armor/shield
        25: (39, 18), # gun/ranged
        26: (40, 18), # ammo
        27: (43, 13), # money/gold
        28: (45, 14), # jewelry
        29: (43, 20), # device/chest
        30: (49, 15), # generic monster
        31: (50, 15), # automata/construct
        32: (51, 15), # undead/demon
        33: (52, 15), # beast
        34: (53, 15), # humanoid
        35: (47, 15), # player
    }

    for tile_index, (src_col, src_row) in mapping.items():
        paste_cell(atlas, source, tile_index, src_col, src_row)

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    atlas.convert("RGB").save(OUTPUT, "BMP")
    print(f"Generated {OUTPUT} from {SOURCE}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
