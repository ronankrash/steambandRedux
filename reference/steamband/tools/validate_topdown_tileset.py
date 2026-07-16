#!/usr/bin/env python3
"""Validate a custom SDL2 top-down tileset BMP against the active contract."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

ATLAS_VERSION = "topdown-v1"
EXPECTED_WIDTH = 216
EXPECTED_HEIGHT = 96
EXPECTED_BPP = 24


def read_bmp_header(path: Path) -> tuple[int, int, int]:
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError("not a BMP file")

    dib_size = struct.unpack_from("<I", data, 14)[0]
    if dib_size < 40:
        raise ValueError("unsupported BMP DIB header")

    width = struct.unpack_from("<i", data, 18)[0]
    height = struct.unpack_from("<i", data, 22)[0]
    bpp = struct.unpack_from("<H", data, 28)[0]
    return width, abs(height), bpp


def validate(path: Path) -> list[str]:
    errors: list[str] = []
    try:
        width, height, bpp = read_bmp_header(path)
    except Exception as exc:
        return [str(exc)]

    if width != EXPECTED_WIDTH:
        errors.append(f"width {width}, expected {EXPECTED_WIDTH}")
    if height != EXPECTED_HEIGHT:
        errors.append(f"height {height}, expected {EXPECTED_HEIGHT}")
    if bpp != EXPECTED_BPP:
        errors.append(f"bits-per-pixel {bpp}, expected {EXPECTED_BPP}")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", help="Path to a candidate topdown-v1 BMP atlas.")
    args = parser.parse_args()

    path = Path(args.path)
    if not path.exists():
        print(f"FAIL: missing file: {path}")
        return 1

    errors = validate(path)
    if errors:
        print(f"FAIL: {path} is not compatible with {ATLAS_VERSION}")
        for error in errors:
            print(f"  - {error}")
        return 1

    print(f"PASS: {path} is compatible with {ATLAS_VERSION} ({EXPECTED_WIDTH}x{EXPECTED_HEIGHT}, {EXPECTED_BPP} bpp)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
