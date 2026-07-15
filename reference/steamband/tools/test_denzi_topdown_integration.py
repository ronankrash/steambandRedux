#!/usr/bin/env python3
"""Integration checks for the DENZI topdown-v1 atlas pipeline."""

from __future__ import annotations

import struct
import subprocess
import sys
from pathlib import Path

from topdown_mapping import default_mapping_path, load_mapping, validate_mapping


ROOT = Path(__file__).resolve().parents[1]
MAPPING = default_mapping_path("topdown-v1-denzi-cc-by-sa.json")
OUTPUT = ROOT / "lib/xtra/graf/topdown_denzi.bmp"
EXPECTED_WIDTH = 216
EXPECTED_HEIGHT = 96
EXPECTED_BPP = 24
EXPECTED_ENTRIES = 36


def read_bmp_header(path: Path) -> tuple[int, int, int]:
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError("not a BMP file")
    width = struct.unpack_from("<i", data, 18)[0]
    height = struct.unpack_from("<i", data, 22)[0]
    bpp = struct.unpack_from("<H", data, 28)[0]
    return width, abs(height), bpp


def main() -> int:
    data = load_mapping(MAPPING)
    errors = validate_mapping(data, MAPPING)
    if errors:
        print("FAIL: DENZI mapping validation")
        for error in errors:
            print(f"  - {error}")
        return 1

    entries = data.get("entries", [])
    if len(entries) != EXPECTED_ENTRIES:
        print(f"FAIL: expected {EXPECTED_ENTRIES} mapped categories, found {len(entries)}")
        return 1

    build = subprocess.run(
        [sys.executable, str(ROOT / "tools/build_denzi_topdown.py")],
        cwd=ROOT,
        check=False,
        capture_output=True,
        text=True,
    )
    if build.returncode != 0:
        print("FAIL: build_denzi_topdown.py")
        print(build.stdout)
        print(build.stderr)
        return 1

    if not OUTPUT.exists():
        print(f"FAIL: missing generated atlas: {OUTPUT}")
        return 1

    width, height, bpp = read_bmp_header(OUTPUT)
    if width != EXPECTED_WIDTH or height != EXPECTED_HEIGHT or bpp != EXPECTED_BPP:
        print(
            f"FAIL: atlas dimensions {width}x{height}@{bpp}bpp; "
            f"expected {EXPECTED_WIDTH}x{EXPECTED_HEIGHT}@{EXPECTED_BPP}bpp"
        )
        return 1

    print("PASS: DENZI topdown-v1 mapping, build, and atlas dimensions")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
