#!/usr/bin/env python3
"""Validate checked-in topdown-v1 mapping JSON files."""

from __future__ import annotations

import sys

from topdown_mapping import validate_all_default_mappings


def main() -> int:
    errors = validate_all_default_mappings()
    if errors:
        print("FAIL: topdown mapping validation")
        for error in errors:
            print(f"  - {error}")
        return 1

    print("PASS: all tools/mappings/topdown-v1-*.json files are valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
