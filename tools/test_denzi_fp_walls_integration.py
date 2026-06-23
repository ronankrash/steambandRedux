#!/usr/bin/env python3
"""Integration checks for DENZI first-person wall texture BMPs."""

from __future__ import annotations

import json
import struct
import subprocess
import sys
from pathlib import Path

from topdown_mapping import load_mapping


ROOT = Path(__file__).resolve().parents[1]
MAPPING = ROOT / "tools/mappings/fp-walls-v1-denzi-cc-by-sa.json"
OUTPUT_DIR = ROOT / "lib/xtra/graf/fp_walls_denzi"
EXPECTED_SIZE = 64
EXPECTED_COUNT = 8


def validate_fp_wall_mapping(data: dict) -> list[str]:
    errors: list[str] = []
    if data.get("atlas_version") != "fp-walls-v1":
        errors.append("atlas_version must be 'fp-walls-v1'")
    entries = data.get("entries")
    if not isinstance(entries, list) or len(entries) != EXPECTED_COUNT:
        errors.append(f"entries must contain {EXPECTED_COUNT} textures")
    return errors


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
    errors = validate_fp_wall_mapping(data)
    if errors:
        print("FAIL: DENZI FP wall mapping validation")
        for error in errors:
            print(f"  - {error}")
        return 1

    build = subprocess.run(
        [sys.executable, str(ROOT / "tools/build_denzi_fp_walls.py")],
        cwd=ROOT,
        check=False,
        capture_output=True,
        text=True,
    )
    if build.returncode != 0:
        print("FAIL: build_denzi_fp_walls.py")
        print(build.stdout)
        print(build.stderr)
        return 1

    manifest_path = OUTPUT_DIR / "manifest.json"
    if not manifest_path.exists():
        print(f"FAIL: missing manifest: {manifest_path}")
        return 1

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    textures = manifest.get("textures", [])
    if len(textures) != EXPECTED_COUNT:
        print(f"FAIL: expected {EXPECTED_COUNT} wall textures, found {len(textures)}")
        return 1

    for entry in textures:
        path = ROOT / str(entry["path"])
        if not path.exists():
            print(f"FAIL: missing wall texture: {path}")
            return 1
        width, height, bpp = read_bmp_header(path)
        if width != EXPECTED_SIZE or height != EXPECTED_SIZE or bpp != 24:
            print(f"FAIL: {path} is {width}x{height}@{bpp}bpp; expected {EXPECTED_SIZE}x{EXPECTED_SIZE}@24bpp")
            return 1

    print("PASS: DENZI first-person wall texture mapping, build, and BMP dimensions")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
