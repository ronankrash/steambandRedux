#!/usr/bin/env python3
"""Integration checks for the c64-ultima-v1 procedural tile atlas."""

from __future__ import annotations

import struct
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "lib/xtra/graf/topdown_c64_ultima.bmp"
GENERATOR = ROOT / "tools/generate_c64_ultima_tilesheet.py"
EXPECTED_WIDTH = 144
EXPECTED_HEIGHT = 64
EXPECTED_BPP = 24
EXPECTED_TILE = 16
EXPECTED_CATEGORIES = 36


def read_bmp_header(path: Path) -> tuple[int, int, int]:
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError("not a BMP file")
    width = struct.unpack_from("<i", data, 18)[0]
    height = struct.unpack_from("<i", data, 22)[0]
    bpp = struct.unpack_from("<H", data, 28)[0]
    return width, abs(height), bpp


def main() -> int:
    build = subprocess.run(
        [sys.executable, str(GENERATOR)],
        cwd=ROOT,
        check=False,
        capture_output=True,
        text=True,
    )
    if build.returncode != 0:
        print("FAIL: generate_c64_ultima_tilesheet.py")
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

    contract = subprocess.run(
        [sys.executable, str(GENERATOR), "--print-contract"],
        cwd=ROOT,
        check=False,
        capture_output=True,
        text=True,
    )
    if contract.returncode != 0 or "c64-ultima-v1" not in contract.stdout:
        print("FAIL: generator contract missing c64-ultima-v1")
        return 1

    if f"{EXPECTED_TILE}x{EXPECTED_TILE}" not in contract.stdout:
        print("FAIL: generator contract missing 16x16 tile size")
        return 1

    print("PASS: c64-ultima-v1 procedural atlas generation and BMP dimensions")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
