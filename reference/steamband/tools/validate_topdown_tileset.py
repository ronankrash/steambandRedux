#!/usr/bin/env python3
"""Validate a custom SDL2 top-down tileset BMP against the active contract."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

ATLAS_VERSIONS = {
    "topdown-v1": {"width": 216, "height": 96, "tile": 24},
    "c64-ultima-v1": {"width": 144, "height": 64, "tile": 16},
}
DEFAULT_VERSION = "topdown-v1"
EXPECTED_WIDTH = ATLAS_VERSIONS[DEFAULT_VERSION]["width"]
EXPECTED_HEIGHT = ATLAS_VERSIONS[DEFAULT_VERSION]["height"]
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


def validate(path: Path, version: str = DEFAULT_VERSION) -> list[str]:
    errors: list[str] = []
    spec = ATLAS_VERSIONS.get(version)
    if not spec:
        return [f"unknown atlas version {version!r}"]
    try:
        width, height, bpp = read_bmp_header(path)
    except Exception as exc:
        return [str(exc)]

    if width != spec["width"]:
        errors.append(f"width {width}, expected {spec['width']}")
    if height != spec["height"]:
        errors.append(f"height {height}, expected {spec['height']}")
    if bpp != EXPECTED_BPP:
        errors.append(f"bits-per-pixel {bpp}, expected {EXPECTED_BPP}")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", help="Path to a candidate topdown BMP atlas.")
    parser.add_argument(
        "--version",
        default=DEFAULT_VERSION,
        choices=sorted(ATLAS_VERSIONS),
        help="Atlas contract version to validate against.",
    )
    args = parser.parse_args()

    path = Path(args.path)
    if not path.exists():
        print(f"FAIL: missing file: {path}")
        return 1

    errors = validate(path, args.version)
    if errors:
        print(f"FAIL: {path} is not compatible with {args.version}")
        for error in errors:
            print(f"  - {error}")
        return 1

    spec = ATLAS_VERSIONS[args.version]
    print(
        f"PASS: {path} is compatible with {args.version} "
        f"({spec['width']}x{spec['height']}, {EXPECTED_BPP} bpp)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
