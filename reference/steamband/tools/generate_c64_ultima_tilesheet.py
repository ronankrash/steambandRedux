#!/usr/bin/env python3
"""Generate a native 16x16 C64/EGA orthogonal tile atlas for SteambandRedux.

Art is project-authored procedural pixel work using the Ultima V PC EGA palette.
No third-party tiles are traced or bundled. Output is a 144x64 24-bit BMP for the
c64-ultima-v1 SDL2 top-down contract.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path
from typing import Iterable

TILE_SIZE = 16
COLUMNS = 9
ROWS = 4
ATLAS_VERSION = "c64-ultima-v1"
WIDTH = TILE_SIZE * COLUMNS
HEIGHT = TILE_SIZE * ROWS

# Ultima V PC / C64-adjacent EGA palette (same as tools/convert_ultima5_tiles.py).
EGA: list[tuple[int, int, int]] = [
    (0x00, 0x00, 0x00), (0x00, 0x00, 0xAA), (0x00, 0xAA, 0x00), (0x00, 0xAA, 0xAA),
    (0xAA, 0x00, 0x00), (0xAA, 0x00, 0xAA), (0xAA, 0x55, 0x00), (0xAA, 0xAA, 0xAA),
    (0x55, 0x55, 0x55), (0x55, 0x55, 0xFF), (0x55, 0xFF, 0x55), (0x55, 0xFF, 0xFF),
    (0xFF, 0x55, 0x55), (0xFF, 0x55, 0xFF), (0xFF, 0xFF, 0x55), (0xFF, 0xFF, 0xFF),
]

BLK, DBL, DGR, CYN, DRD, MAG, BRN, LGR, DGY, LBL, LGN, LCY, LRD, LMG, YEL, WHT = EGA

CATEGORIES: list[tuple[str, tuple[int, int, int]]] = [
    ("darkness", BLK),
    ("floor", LGR),
    ("wall", DGY),
    ("door", BRN),
    ("up_stairs", LGR),
    ("down_stairs", LGR),
    ("trap", DRD),
    ("glyph", LMG),
    ("shop", BRN),
    ("shop_general", BRN),
    ("shop_clothing", LGR),
    ("shop_gun", BRN),
    ("shop_machinist", DGY),
    ("shop_alchemy", LBL),
    ("shop_magic", MAG),
    ("shop_black_market", DGY),
    ("shop_home", YEL),
    ("rubble", DGY),
    ("ore", BRN),
    ("object", YEL),
    ("object_food", LGN),
    ("object_scroll", WHT),
    ("object_potion", LBL),
    ("object_weapon", LGR),
    ("object_armor", LGR),
    ("object_gun", BRN),
    ("object_ammo", YEL),
    ("object_money", YEL),
    ("object_jewelry", LMG),
    ("object_device", BRN),
    ("monster", LRD),
    ("monster_automata", DGY),
    ("monster_undead", LMG),
    ("monster_beast", BRN),
    ("monster_humanoid", LRD),
    ("player", YEL),
]

Color = tuple[int, int, int]
Pixels = list[list[Color]]


def put(pixels: Pixels, x: int, y: int, color: Color) -> None:
    if 0 <= x < WIDTH and 0 <= y < HEIGHT:
        pixels[y][x] = color


def rect(pixels: Pixels, x: int, y: int, w: int, h: int, color: Color) -> None:
    for yy in range(y, y + h):
        for xx in range(x, x + w):
            put(pixels, xx, yy, color)


def line(pixels: Pixels, x0: int, y0: int, x1: int, y1: int, color: Color) -> None:
    dx = abs(x1 - x0)
    sx = 1 if x0 < x1 else -1
    dy = -abs(y1 - y0)
    sy = 1 if y0 < y1 else -1
    err = dx + dy
    while True:
        put(pixels, x0, y0, color)
        if x0 == x1 and y0 == y1:
            break
        e2 = 2 * err
        if e2 >= dy:
            err += dy
            x0 += sx
        if e2 <= dx:
            err += dx
            y0 += sy


def draw_grass(pixels: Pixels, ox: int, oy: int) -> None:
    rect(pixels, ox, oy, 16, 16, DGR)
    for x, y in ((2, 3), (6, 8), (11, 4), (13, 11), (4, 13), (9, 6)):
        put(pixels, ox + x, oy + y, LGN)
    for x, y in ((8, 2), (3, 10), (14, 7)):
        put(pixels, ox + x, oy + y, BRN)


def draw_stone_floor(pixels: Pixels, ox: int, oy: int) -> None:
    for y in range(16):
        for x in range(16):
            put(pixels, ox + x, oy + y, LGR if (x + y) % 2 == 0 else DGY)


def draw_wall_cap(pixels: Pixels, ox: int, oy: int) -> None:
    rect(pixels, ox, oy, 16, 16, DGY)
    rect(pixels, ox + 1, oy + 1, 14, 10, DGY)
    line(pixels, ox, oy + 15, ox + 15, oy + 15, BLK)
    line(pixels, ox + 15, oy, ox + 15, oy + 15, BLK)
    line(pixels, ox, oy, ox + 15, oy, LGR)
    line(pixels, ox, oy, ox, oy + 15, BLK)


def draw_door(pixels: Pixels, ox: int, oy: int) -> None:
    draw_stone_floor(pixels, ox, oy)
    rect(pixels, ox + 5, oy + 3, 6, 11, BRN)
    line(pixels, ox + 5, oy + 3, ox + 10, oy + 3, BLK)
    line(pixels, ox + 5, oy + 13, ox + 10, oy + 13, BLK)
    put(pixels, ox + 9, oy + 9, YEL)


def draw_stairs(pixels: Pixels, ox: int, oy: int, up: bool) -> None:
    draw_stone_floor(pixels, ox, oy)
    for row in range(4):
        y = oy + 4 + row * 3
        line(pixels, ox + 3, y, ox + 12, y, BLK)
    if up:
        line(pixels, ox + 5, oy + 12, ox + 8, oy + 5, WHT)
        line(pixels, ox + 8, oy + 5, ox + 11, oy + 12, WHT)
    else:
        line(pixels, ox + 5, oy + 5, ox + 8, oy + 12, WHT)
        line(pixels, ox + 8, oy + 12, ox + 11, oy + 5, WHT)


def draw_trap(pixels: Pixels, ox: int, oy: int) -> None:
    draw_stone_floor(pixels, ox, oy)
    line(pixels, ox + 3, oy + 3, ox + 12, oy + 12, DRD)
    line(pixels, ox + 12, oy + 3, ox + 3, oy + 12, DRD)
    put(pixels, ox + 7, oy + 7, LRD)
    put(pixels, ox + 8, oy + 7, LRD)


def draw_glyph(pixels: Pixels, ox: int, oy: int) -> None:
    draw_stone_floor(pixels, ox, oy)
    line(pixels, ox + 8, oy + 2, ox + 13, oy + 8, LMG)
    line(pixels, ox + 13, oy + 8, ox + 8, oy + 14, LMG)
    line(pixels, ox + 8, oy + 14, ox + 3, oy + 8, LMG)
    line(pixels, ox + 3, oy + 8, ox + 8, oy + 2, LMG)
    put(pixels, ox + 8, oy + 8, WHT)


def draw_shop(pixels: Pixels, ox: int, oy: int, accent: Color, mark: str) -> None:
    draw_stone_floor(pixels, ox, oy)
    rect(pixels, ox + 2, oy + 7, 12, 7, accent)
    line(pixels, ox + 2, oy + 7, ox + 8, oy + 3, BLK)
    line(pixels, ox + 8, oy + 3, ox + 14, oy + 7, BLK)
    rect(pixels, ox + 6, oy + 9, 4, 5, BLK)
    if mark == "gun":
        rect(pixels, ox + 10, oy + 4, 4, 2, DGY)
        put(pixels, ox + 13, oy + 4, YEL)
    elif mark == "gear":
        put(pixels, ox + 11, oy + 4, LGR)
        put(pixels, ox + 12, oy + 5, LGR)
        put(pixels, ox + 10, oy + 5, LGR)
    elif mark == "flask":
        put(pixels, ox + 11, oy + 4, LBL)
        put(pixels, ox + 11, oy + 5, LCY)
    elif mark == "star":
        put(pixels, ox + 11, oy + 4, LMG)
        put(pixels, ox + 10, oy + 5, LMG)
        put(pixels, ox + 12, oy + 5, LMG)
    elif mark == "skull":
        put(pixels, ox + 10, oy + 4, WHT)
        put(pixels, ox + 12, oy + 4, WHT)
    elif mark == "home":
        put(pixels, ox + 11, oy + 4, YEL)
    elif mark == "cloth":
        put(pixels, ox + 11, oy + 4, WHT)
        put(pixels, ox + 10, oy + 5, LBL)
    else:
        put(pixels, ox + 11, oy + 4, YEL)


def draw_rubble(pixels: Pixels, ox: int, oy: int) -> None:
    draw_stone_floor(pixels, ox, oy)
    rect(pixels, ox + 2, oy + 10, 4, 3, DGY)
    rect(pixels, ox + 7, oy + 8, 5, 4, LGR)
    rect(pixels, ox + 11, oy + 11, 3, 3, DGY)


def draw_ore(pixels: Pixels, ox: int, oy: int, magma: bool) -> None:
    draw_stone_floor(pixels, ox, oy)
    c1, c2 = (DRD, YEL) if magma else (LBL, WHT)
    line(pixels, ox + 2, oy + 13, ox + 13, oy + 3, c1)
    line(pixels, ox + 4, oy + 14, ox + 14, oy + 4, c2)
    put(pixels, ox + 8, oy + 8, c2)


def draw_object_icon(pixels: Pixels, ox: int, oy: int, kind: str) -> None:
    draw_stone_floor(pixels, ox, oy)
    if kind == "food":
        rect(pixels, ox + 5, oy + 6, 6, 5, LGN)
    elif kind == "scroll":
        rect(pixels, ox + 5, oy + 4, 6, 9, WHT)
        line(pixels, ox + 6, oy + 6, ox + 10, oy + 6, BLK)
    elif kind == "potion":
        rect(pixels, ox + 7, oy + 5, 3, 2, WHT)
        rect(pixels, ox + 6, oy + 7, 5, 6, LBL)
    elif kind == "weapon":
        line(pixels, ox + 4, oy + 12, ox + 12, oy + 4, LGR)
        put(pixels, ox + 4, oy + 12, BRN)
    elif kind == "armor":
        rect(pixels, ox + 5, oy + 5, 6, 8, LGR)
        line(pixels, ox + 5, oy + 5, ox + 8, oy + 3, WHT)
        line(pixels, ox + 11, oy + 5, ox + 8, oy + 3, WHT)
    elif kind == "gun":
        rect(pixels, ox + 4, oy + 7, 9, 2, BRN)
        rect(pixels, ox + 11, oy + 6, 2, 2, DGY)
    elif kind == "ammo":
        for x in (6, 8, 10):
            put(pixels, ox + x, oy + 6, YEL)
            put(pixels, ox + x, oy + 8, YEL)
    elif kind == "money":
        put(pixels, ox + 7, oy + 7, YEL)
        put(pixels, ox + 8, oy + 7, YEL)
        put(pixels, ox + 7, oy + 8, YEL)
    elif kind == "jewelry":
        put(pixels, ox + 8, oy + 7, LMG)
        line(pixels, ox + 6, oy + 9, ox + 10, oy + 9, LMG)
    elif kind == "device":
        rect(pixels, ox + 5, oy + 6, 6, 5, BRN)
        line(pixels, ox + 5, oy + 8, ox + 10, oy + 8, BLK)
    else:
        rect(pixels, ox + 6, oy + 6, 4, 4, YEL)


def draw_monster(pixels: Pixels, ox: int, oy: int, kind: str) -> None:
    draw_stone_floor(pixels, ox, oy)
    if kind == "automata":
        rect(pixels, ox + 4, oy + 4, 8, 8, DGY)
        put(pixels, ox + 6, oy + 7, LRD)
        put(pixels, ox + 9, oy + 7, LRD)
    elif kind == "undead":
        rect(pixels, ox + 5, oy + 4, 6, 8, LGR)
        put(pixels, ox + 6, oy + 6, BLK)
        put(pixels, ox + 9, oy + 6, BLK)
    elif kind == "beast":
        put(pixels, ox + 5, oy + 8, BRN)
        put(pixels, ox + 7, oy + 7, BRN)
        put(pixels, ox + 9, oy + 8, BRN)
        put(pixels, ox + 7, oy + 5, LRD)
    elif kind == "humanoid":
        put(pixels, ox + 7, oy + 4, LRD)
        put(pixels, ox + 6, oy + 6, LRD)
        put(pixels, ox + 7, oy + 6, LRD)
        put(pixels, ox + 8, oy + 6, LRD)
        line(pixels, ox + 5, oy + 12, ox + 10, oy + 12, BLK)
    else:
        put(pixels, ox + 6, oy + 6, LRD)
        put(pixels, ox + 9, oy + 6, LRD)
        put(pixels, ox + 7, oy + 9, LRD)


def draw_player(pixels: Pixels, ox: int, oy: int) -> None:
    draw_stone_floor(pixels, ox, oy)
    put(pixels, ox + 7, oy + 3, YEL)
    put(pixels, ox + 6, oy + 4, YEL)
    put(pixels, ox + 7, oy + 4, YEL)
    put(pixels, ox + 8, oy + 4, YEL)
    rect(pixels, ox + 6, oy + 5, 3, 5, LBL)
    line(pixels, ox + 5, oy + 11, ox + 10, oy + 11, BLK)


def draw_tile(pixels: Pixels, index: int) -> None:
    ox = (index % COLUMNS) * TILE_SIZE
    oy = (index // COLUMNS) * TILE_SIZE

    if index == 0:
        rect(pixels, ox, oy, 16, 16, BLK)
    elif index == 1:
        draw_grass(pixels, ox, oy)
    elif index == 2:
        draw_wall_cap(pixels, ox, oy)
    elif index == 3:
        draw_door(pixels, ox, oy)
    elif index == 4:
        draw_stairs(pixels, ox, oy, up=True)
    elif index == 5:
        draw_stairs(pixels, ox, oy, up=False)
    elif index == 6:
        draw_trap(pixels, ox, oy)
    elif index == 7:
        draw_glyph(pixels, ox, oy)
    elif index == 8:
        draw_shop(pixels, ox, oy, BRN, "generic")
    elif index == 9:
        draw_shop(pixels, ox, oy, BRN, "generic")
    elif index == 10:
        draw_shop(pixels, ox, oy, LGR, "cloth")
    elif index == 11:
        draw_shop(pixels, ox, oy, BRN, "gun")
    elif index == 12:
        draw_shop(pixels, ox, oy, DGY, "gear")
    elif index == 13:
        draw_shop(pixels, ox, oy, LBL, "flask")
    elif index == 14:
        draw_shop(pixels, ox, oy, MAG, "star")
    elif index == 15:
        draw_shop(pixels, ox, oy, DGY, "skull")
    elif index == 16:
        draw_shop(pixels, ox, oy, YEL, "home")
    elif index == 17:
        draw_rubble(pixels, ox, oy)
    elif index == 18:
        draw_ore(pixels, ox, oy, magma=False)
    elif index == 19:
        draw_object_icon(pixels, ox, oy, "generic")
    elif index == 20:
        draw_object_icon(pixels, ox, oy, "food")
    elif index == 21:
        draw_object_icon(pixels, ox, oy, "scroll")
    elif index == 22:
        draw_object_icon(pixels, ox, oy, "potion")
    elif index == 23:
        draw_object_icon(pixels, ox, oy, "weapon")
    elif index == 24:
        draw_object_icon(pixels, ox, oy, "armor")
    elif index == 25:
        draw_object_icon(pixels, ox, oy, "gun")
    elif index == 26:
        draw_object_icon(pixels, ox, oy, "ammo")
    elif index == 27:
        draw_object_icon(pixels, ox, oy, "money")
    elif index == 28:
        draw_object_icon(pixels, ox, oy, "jewelry")
    elif index == 29:
        draw_object_icon(pixels, ox, oy, "device")
    elif index == 30:
        draw_monster(pixels, ox, oy, "generic")
    elif index == 31:
        draw_monster(pixels, ox, oy, "automata")
    elif index == 32:
        draw_monster(pixels, ox, oy, "undead")
    elif index == 33:
        draw_monster(pixels, ox, oy, "beast")
    elif index == 34:
        draw_monster(pixels, ox, oy, "humanoid")
    elif index == 35:
        draw_player(pixels, ox, oy)


def bmp_bytes(pixels: Pixels) -> bytes:
    row_size = ((WIDTH * 3 + 3) // 4) * 4
    pixel_data = bytearray()
    for y in range(HEIGHT - 1, -1, -1):
        row = bytearray()
        for x in range(WIDTH):
            r, g, b = pixels[y][x]
            row += bytes((b, g, r))
        row += b"\0" * (row_size - len(row))
        pixel_data += row
    file_size = 54 + len(pixel_data)
    header = b"BM" + struct.pack("<IHHI", file_size, 0, 0, 54)
    info = struct.pack("<IIIHHIIIIII", 40, WIDTH, HEIGHT, 1, 24, 0,
                       len(pixel_data), 2835, 2835, 0, 0)
    return header + info + pixel_data


def category_lines() -> Iterable[str]:
    for index, (name, _) in enumerate(CATEGORIES):
        yield f"{index:02d}: {name}"


def generate(output: Path) -> None:
    pixels: Pixels = [[BLK for _ in range(WIDTH)] for _ in range(HEIGHT)]
    for index in range(len(CATEGORIES)):
        draw_tile(pixels, index)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(bmp_bytes(pixels))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        default="lib/xtra/graf/topdown_c64_ultima.bmp",
        help="Output BMP path relative to the repository root.",
    )
    parser.add_argument("--print-contract", action="store_true")
    args = parser.parse_args()

    if args.print_contract:
        print(f"{ATLAS_VERSION}: {TILE_SIZE}x{TILE_SIZE} BMP, {COLUMNS} columns x {ROWS} rows")
        print(f"dimensions: {WIDTH}x{HEIGHT}")
        for line_text in category_lines():
            print(line_text)
        return 0

    output = Path(args.output)
    generate(output)
    print(f"Generated {output} ({WIDTH}x{HEIGHT}, {len(CATEGORIES)} tiles, {ATLAS_VERSION})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
