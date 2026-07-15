#!/usr/bin/env python3
"""Convert a local, user-supplied Ultima V PC tiles.16 file to topdown-v1 BMP.

This tool is for private/local testing only. Do not commit the source file or
generated Ultima-derived atlas. Paths under local_assets/ and
lib/user/topdown_tileset.bmp are ignored by git.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

from PIL import Image

import generate_topdown_tilesheet as base
from topdown_mapping import default_mapping_path, load_mapping, ultima_index_map
from ultima5_lzw import find_tiles16_in_install, load_tiles16_bytes, tiles16_usage_hint


EGA_PALETTE = [
    (0x00, 0x00, 0x00), (0x00, 0x00, 0xAA), (0x00, 0xAA, 0x00), (0x00, 0xAA, 0xAA),
    (0xAA, 0x00, 0x00), (0xAA, 0x00, 0xAA), (0xAA, 0x55, 0x00), (0xAA, 0xAA, 0xAA),
    (0x55, 0x55, 0x55), (0x55, 0x55, 0xFF), (0x55, 0xFF, 0x55), (0x55, 0xFF, 0xFF),
    (0xFF, 0x55, 0x55), (0xFF, 0x55, 0xFF), (0xFF, 0xFF, 0x55), (0xFF, 0xFF, 0xFF),
]


def decode_tiles16_bytes(data: bytes) -> list[Image.Image]:
    tile_size = 16 * 8
    if len(data) < tile_size or len(data) % tile_size != 0:
        raise ValueError(f"tile data size {len(data)} is not a multiple of {tile_size} bytes")

    tiles: list[Image.Image] = []
    count = len(data) // tile_size
    for tile_index in range(count):
        tile = Image.new("RGBA", (16, 16))
        base_offset = tile_index * tile_size
        for y in range(16):
            row = data[base_offset + y * 8: base_offset + y * 8 + 8]
            for pair_index, byte in enumerate(row):
                hi = (byte >> 4) & 0x0F
                lo = byte & 0x0F
                tile.putpixel((pair_index * 2, y), EGA_PALETTE[hi] + (255,))
                tile.putpixel((pair_index * 2 + 1, y), EGA_PALETTE[lo] + (255,))
        tiles.append(tile)
    return tiles


def paste_tile(atlas: Image.Image, source: Image.Image, target_index: int) -> None:
    tile = source.resize((base.TILE_SIZE, base.TILE_SIZE), Image.Resampling.NEAREST)
    dst = ((target_index % base.COLUMNS) * base.TILE_SIZE, (target_index // base.COLUMNS) * base.TILE_SIZE)
    atlas.paste(tile, dst, tile)


def parse_mapping(text: str) -> dict[int, int]:
    mapping: dict[int, int] = {}
    if not text:
        return mapping
    for pair in text.split(","):
        left, right = pair.split(":", 1)
        mapping[int(left, 0)] = int(right, 0)
    return mapping


def resolve_tiles16_source(tiles16: str | None, install_dir: str | None) -> Path:
    if install_dir:
        install_path = Path(install_dir)
        if not install_path.is_dir():
            raise FileNotFoundError(f"install directory not found: {install_path}")
        found = find_tiles16_in_install(install_path)
        if found is None:
            raise FileNotFoundError(f"no TILES.16 found under {install_path}")
        return found

    if not tiles16:
        raise FileNotFoundError("provide --tiles16 or --install-dir")

    path = Path(tiles16)
    if not path.is_file():
        raise FileNotFoundError(str(path))
    return path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--tiles16",
        help="Path to TILES.16 from your Ultima V install (LZW-compressed or raw).",
    )
    parser.add_argument(
        "--install-dir",
        help="Ultima V install directory containing TILES.16 (alternative to --tiles16).",
    )
    parser.add_argument(
        "--output",
        default="lib/user/topdown_tileset.bmp",
        help="Output BMP path. Default is ignored by git and auto-loaded by the renderer.",
    )
    parser.add_argument(
        "--mapping",
        default="",
        help="Comma-separated topdown-index:ultima-tile-index overrides, e.g. 1:0x12,2:0x30.",
    )
    parser.add_argument(
        "--mapping-file",
        default=str(default_mapping_path("topdown-v1-ultima5-local.json")),
        help="JSON mapping spec for topdown-v1 slots. Default is the local Ultima V starter map.",
    )
    args = parser.parse_args()

    try:
        source_path = resolve_tiles16_source(args.tiles16, args.install_dir)
        raw_tiles = load_tiles16_bytes(source_path)
        tiles = decode_tiles16_bytes(raw_tiles)
    except FileNotFoundError as exc:
        print(f"FAIL: {exc}")
        print()
        print(tiles16_usage_hint())
        return 1
    except ValueError as exc:
        print(f"FAIL: {exc}")
        return 1

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)

    temp = output.with_suffix(".base.bmp")
    base.generate(temp)
    atlas = Image.open(temp).convert("RGBA")
    temp.unlink(missing_ok=True)

    mapping_path = Path(args.mapping_file)
    if mapping_path.exists():
        tile_mapping = ultima_index_map(load_mapping(mapping_path))
        print(f"Loaded mapping: {mapping_path}")
    else:
        tile_mapping = {
            0: 0x00, 1: 0x01, 2: 0x30, 3: 0x04, 4: 0x06, 5: 0x07,
            6: 0x10, 7: 0x03, 8: 0x08, 17: 0x31, 18: 0x32,
        }
        print(f"Mapping file missing ({mapping_path}); using built-in fallback sample.")
    tile_mapping.update(parse_mapping(args.mapping))

    for target_index, source_index in tile_mapping.items():
        if 0 <= target_index < len(base.CATEGORIES) and 0 <= source_index < len(tiles):
            paste_tile(atlas, tiles[source_index], target_index)

    atlas.convert("RGB").save(output, "BMP")
    print(f"Converted {source_path} -> {output}")
    print("Output is local-only; do not commit Ultima-derived assets.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
