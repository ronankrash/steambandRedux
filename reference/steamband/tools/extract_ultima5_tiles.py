#!/usr/bin/env python3
"""Extract raw Ultima V TILES.16 bytes from a PC install file."""

from __future__ import annotations

import argparse
from pathlib import Path

from ultima5_lzw import (
    EXPECTED_TILES16_RAW_SIZE,
    find_tiles16_in_install,
    load_tiles16_bytes,
    tiles16_usage_hint,
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tiles16", help="Path to TILES.16 from your Ultima V install.")
    parser.add_argument(
        "--install-dir",
        help="Ultima V install directory containing TILES.16 (alternative to --tiles16).",
    )
    parser.add_argument(
        "--output",
        default="local_assets/ultima5/tiles.raw",
        help="Output path for decompressed raw tile bytes.",
    )
    args = parser.parse_args()

    source: Path | None = None
    if args.install_dir:
        install_dir = Path(args.install_dir)
        if not install_dir.is_dir():
            print(f"FAIL: install directory not found: {install_dir}")
            return 1
        source = find_tiles16_in_install(install_dir)
        if source is None:
            print(f"FAIL: no TILES.16 found under {install_dir}")
            print(tiles16_usage_hint())
            return 1
    elif args.tiles16:
        source = Path(args.tiles16)
    else:
        print("FAIL: provide --tiles16 or --install-dir")
        print(tiles16_usage_hint())
        return 1

    if not source.is_file():
        print(f"FAIL: file not found: {source}")
        print(tiles16_usage_hint())
        return 1

    try:
        raw = load_tiles16_bytes(source)
    except ValueError as exc:
        print(f"FAIL: {exc}")
        return 1

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(raw)
    print(f"Extracted {len(raw)} bytes from {source} -> {output}")
    if len(raw) != EXPECTED_TILES16_RAW_SIZE:
        print(f"WARNING: expected {EXPECTED_TILES16_RAW_SIZE} bytes")
    print("Local-only output; do not commit Ultima-derived assets.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
