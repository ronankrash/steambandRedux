#!/usr/bin/env python3
"""Generate the project-authored SDL2 top-down placeholder BMP atlas.

The output is intentionally simple 24-bit BMP data so the renderer can load it
with SDL_LoadBMP and no extra image dependencies. Art is original procedural
linework used as a permissive placeholder while final tiles are sourced.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path
from typing import Iterable

TILE_SIZE = 24
COLUMNS = 8
ROWS = 3
WIDTH = TILE_SIZE * COLUMNS
HEIGHT = TILE_SIZE * ROWS

CATEGORIES: list[tuple[str, tuple[int, int, int]]] = [
    ("darkness", (13, 12, 11)),
    ("floor", (50, 48, 42)),
    ("wall", (96, 88, 78)),
    ("door", (126, 82, 48)),
    ("up_stairs", (92, 132, 142)),
    ("down_stairs", (60, 102, 132)),
    ("trap", (138, 62, 116)),
    ("object", (188, 142, 66)),
    ("object_food", (116, 146, 72)),
    ("object_scroll", (180, 160, 116)),
    ("object_potion", (90, 136, 172)),
    ("object_weapon", (150, 138, 120)),
    ("object_armor", (116, 124, 132)),
    ("object_gun", (126, 102, 70)),
    ("object_ammo", (170, 130, 58)),
    ("object_money", (206, 164, 62)),
    ("object_jewelry", (152, 92, 166)),
    ("object_device", (94, 118, 126)),
    ("monster", (170, 56, 48)),
    ("monster_automata", (142, 122, 80)),
    ("monster_undead", (116, 84, 146)),
    ("monster_beast", (150, 88, 52)),
    ("monster_humanoid", (154, 92, 70)),
    ("player", (214, 190, 116)),
]


Color = tuple[int, int, int]
Pixels = list[list[Color]]


def clamp_channel(value: float) -> int:
    return max(0, min(255, int(value)))


def scale_color(color: Color, scale: float, offset: float = 0.0) -> Color:
    return tuple(clamp_channel(c * scale + offset) for c in color)  # type: ignore[return-value]


def put(pixels: Pixels, x: int, y: int, color: Color) -> None:
    if 0 <= x < WIDTH and 0 <= y < HEIGHT:
        pixels[y][x] = color


def rect(pixels: Pixels, x: int, y: int, width: int, height: int, color: Color) -> None:
    for yy in range(y, y + height):
        for xx in range(x, x + width):
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


def draw_border(pixels: Pixels, ox: int, oy: int, color: Color) -> None:
    for x in range(ox, ox + TILE_SIZE):
        put(pixels, x, oy, color)
        put(pixels, x, oy + TILE_SIZE - 1, color)
    for y in range(oy, oy + TILE_SIZE):
        put(pixels, ox, y, color)
        put(pixels, ox + TILE_SIZE - 1, y, color)


def draw_tile(pixels: Pixels, index: int, base: Color) -> None:
    ox = (index % COLUMNS) * TILE_SIZE
    oy = (index // COLUMNS) * TILE_SIZE
    dark = scale_color(base, 0.35)
    mid = scale_color(base, 1.18, 12.0)
    light = scale_color(base, 1.35, 20.0)

    rect(pixels, ox, oy, TILE_SIZE, TILE_SIZE, base)
    draw_border(pixels, ox, oy, dark)

    if index == 0:  # darkness
        for k in range(4, 22, 6):
            line(pixels, ox + k, oy + 3, ox + 3, oy + k, dark)
    elif index == 1:  # floor
        for x, y in ((5, 5), (17, 14), (9, 19)):
            put(pixels, ox + x, oy + y, mid)
    elif index == 2:  # wall
        line(pixels, ox + 2, oy + 8, ox + 21, oy + 8, dark)
        line(pixels, ox + 2, oy + 15, ox + 21, oy + 15, dark)
        line(pixels, ox + 11, oy + 2, ox + 11, oy + 21, dark)
    elif index == 3:  # door
        rect(pixels, ox + 6, oy + 3, 12, 18, mid)
        line(pixels, ox + 12, oy + 3, ox + 12, oy + 20, dark)
        rect(pixels, ox + 15, oy + 12, 2, 2, dark)
    elif index in (4, 5):  # stairs
        for k in range(5, 20, 4):
            line(pixels, ox + 5, oy + k, ox + 19, oy + k, dark)
        if index == 4:
            line(pixels, ox + 7, oy + 7, ox + 12, oy + 3, dark)
            line(pixels, ox + 12, oy + 3, ox + 17, oy + 7, dark)
        else:
            line(pixels, ox + 7, oy + 17, ox + 12, oy + 21, dark)
            line(pixels, ox + 12, oy + 21, ox + 17, oy + 17, dark)
    elif index == 6:  # trap
        line(pixels, ox + 5, oy + 5, ox + 18, oy + 18, dark)
        line(pixels, ox + 18, oy + 5, ox + 5, oy + 18, dark)
        line(pixels, ox + 12, oy + 3, ox + 12, oy + 21, mid)
    elif index == 7:  # object
        rect(pixels, ox + 7, oy + 8, 10, 9, mid)
        line(pixels, ox + 7, oy + 8, ox + 12, oy + 4, dark)
        line(pixels, ox + 17, oy + 8, ox + 12, oy + 4, dark)
    elif index == 8:  # food/anodyne
        rect(pixels, ox + 7, oy + 8, 10, 9, mid)
        line(pixels, ox + 8, oy + 8, ox + 12, oy + 4, dark)
        line(pixels, ox + 12, oy + 4, ox + 17, oy + 8, dark)
    elif index == 9:  # scroll/book
        rect(pixels, ox + 6, oy + 5, 12, 15, mid)
        line(pixels, ox + 8, oy + 9, ox + 16, oy + 9, dark)
        line(pixels, ox + 8, oy + 13, ox + 16, oy + 13, dark)
        line(pixels, ox + 8, oy + 17, ox + 13, oy + 17, dark)
    elif index == 10:  # potion/flask
        rect(pixels, ox + 10, oy + 5, 5, 4, light)
        rect(pixels, ox + 8, oy + 9, 9, 11, mid)
        line(pixels, ox + 8, oy + 9, ox + 12, oy + 4, dark)
        line(pixels, ox + 17, oy + 9, ox + 12, oy + 4, dark)
    elif index == 11:  # weapon/tool
        line(pixels, ox + 5, oy + 19, ox + 18, oy + 6, dark)
        line(pixels, ox + 8, oy + 20, ox + 20, oy + 8, mid)
        rect(pixels, ox + 4, oy + 18, 5, 3, dark)
    elif index == 12:  # armor
        rect(pixels, ox + 7, oy + 6, 10, 14, mid)
        line(pixels, ox + 7, oy + 6, ox + 12, oy + 3, dark)
        line(pixels, ox + 17, oy + 6, ox + 12, oy + 3, dark)
        line(pixels, ox + 9, oy + 11, ox + 15, oy + 11, dark)
    elif index == 13:  # gun/ray
        rect(pixels, ox + 5, oy + 11, 14, 4, mid)
        rect(pixels, ox + 15, oy + 7, 3, 4, dark)
        rect(pixels, ox + 7, oy + 15, 4, 5, dark)
        line(pixels, ox + 18, oy + 12, ox + 21, oy + 10, light)
    elif index == 14:  # ammo
        for x in (7, 11, 15):
            rect(pixels, ox + x, oy + 6, 3, 13, mid)
            put(pixels, ox + x + 1, oy + 4, light)
    elif index == 15:  # money
        rect(pixels, ox + 7, oy + 12, 10, 5, mid)
        rect(pixels, ox + 9, oy + 8, 8, 5, light)
        rect(pixels, ox + 11, oy + 5, 6, 4, mid)
    elif index == 16:  # jewelry
        line(pixels, ox + 7, oy + 12, ox + 12, oy + 7, dark)
        line(pixels, ox + 12, oy + 7, ox + 17, oy + 12, dark)
        line(pixels, ox + 7, oy + 12, ox + 12, oy + 18, dark)
        line(pixels, ox + 17, oy + 12, ox + 12, oy + 18, dark)
        rect(pixels, ox + 11, oy + 11, 3, 3, light)
    elif index == 17:  # device/chest
        rect(pixels, ox + 6, oy + 8, 12, 10, mid)
        line(pixels, ox + 6, oy + 12, ox + 18, oy + 12, dark)
        rect(pixels, ox + 11, oy + 12, 3, 4, dark)
        line(pixels, ox + 8, oy + 8, ox + 12, oy + 5, dark)
        line(pixels, ox + 16, oy + 8, ox + 12, oy + 5, dark)
    elif index == 18:  # generic monster
        line(pixels, ox + 4, oy + 12, ox + 12, oy + 4, dark)
        line(pixels, ox + 12, oy + 4, ox + 20, oy + 12, dark)
        line(pixels, ox + 4, oy + 12, ox + 12, oy + 20, dark)
        line(pixels, ox + 12, oy + 20, ox + 20, oy + 12, dark)
        rect(pixels, ox + 9, oy + 9, 2, 2, mid)
        rect(pixels, ox + 14, oy + 9, 2, 2, mid)
    elif index == 19:  # automata
        rect(pixels, ox + 6, oy + 6, 12, 12, mid)
        line(pixels, ox + 3, oy + 12, ox + 20, oy + 12, dark)
        line(pixels, ox + 12, oy + 3, ox + 12, oy + 20, dark)
        rect(pixels, ox + 10, oy + 10, 4, 4, light)
    elif index == 20:  # undead/demon
        line(pixels, ox + 7, oy + 8, ox + 12, oy + 4, dark)
        line(pixels, ox + 12, oy + 4, ox + 17, oy + 8, dark)
        rect(pixels, ox + 7, oy + 8, 11, 10, mid)
        rect(pixels, ox + 9, oy + 11, 2, 2, dark)
        rect(pixels, ox + 15, oy + 11, 2, 2, dark)
        line(pixels, ox + 10, oy + 19, ox + 16, oy + 19, dark)
    elif index == 21:  # beast
        rect(pixels, ox + 9, oy + 10, 7, 8, mid)
        rect(pixels, ox + 5, oy + 6, 4, 5, light)
        rect(pixels, ox + 10, oy + 4, 4, 5, light)
        rect(pixels, ox + 16, oy + 6, 4, 5, light)
        line(pixels, ox + 6, oy + 19, ox + 18, oy + 19, dark)
    elif index == 22:  # humanoid
        rect(pixels, ox + 10, oy + 4, 5, 5, light)
        rect(pixels, ox + 8, oy + 10, 9, 9, mid)
        line(pixels, ox + 5, oy + 20, ox + 20, oy + 20, dark)
        line(pixels, ox + 8, oy + 10, ox + 5, oy + 16, dark)
        line(pixels, ox + 17, oy + 10, ox + 20, oy + 16, dark)
    elif index == 23:  # player
        line(pixels, ox + 12, oy + 3, ox + 5, oy + 20, dark)
        line(pixels, ox + 12, oy + 3, ox + 19, oy + 20, dark)
        line(pixels, ox + 7, oy + 13, ox + 17, oy + 13, dark)
        rect(pixels, ox + 11, oy + 6, 3, 3, mid)


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
    pixels: Pixels = [[(0, 0, 0) for _ in range(WIDTH)] for __ in range(HEIGHT)]
    for index, (_, color) in enumerate(CATEGORIES):
        draw_tile(pixels, index, color)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(bmp_bytes(pixels))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        default="lib/xtra/graf/sdl2_topdown_24.bmp",
        help="Output BMP path relative to the repository root.",
    )
    parser.add_argument(
        "--print-contract",
        action="store_true",
        help="Print the atlas slot contract instead of generating the BMP.",
    )
    args = parser.parse_args()

    if args.print_contract:
        print(f"{TILE_SIZE}x{TILE_SIZE} BMP, {COLUMNS} columns x {ROWS} rows")
        for line_text in category_lines():
            print(line_text)
        return 0

    output = Path(args.output)
    generate(output)
    print(f"Generated {output} ({WIDTH}x{HEIGHT}, {len(CATEGORIES)} tiles)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
