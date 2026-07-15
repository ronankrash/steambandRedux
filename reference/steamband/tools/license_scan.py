#!/usr/bin/env python3
"""Scan the repository for known Steamband/Angband licensing risk phrases."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


PATTERNS = {
    "educational_not_for_profit": r"educational,\s*research,\s*(?:\*\s*)?and not for profit",
    "not_for_profit": r"not for profit",
    "sell_or_market": r"sell or market",
    "commercial": r"commercial",
    "only_valid_license": r"only valid license",
    "gpl_reference": r"gnu general public license",
    "embedded_copyright": r"cptr copyright",
    "microsoft_sample": r"sample files",
}

DEFAULT_SUFFIXES = {
    ".c",
    ".h",
    ".txt",
    ".md",
    ".prf",
}

SKIP_DIRS = {
    ".git",
    ".cursor",
    "build",
    "build-rescue-nosdl",
    "build-rescue-sdl2",
    "third_party",
    "tools",
}

PROJECT_DOCS = {
    "ASSETS.md",
    "HANDOFF.md",
    "LICENSES.md",
    "README.md",
    "SECURITY-AUDIT.md",
}


def iter_files(root: Path, include_project_docs: bool):
    for path in root.rglob("*"):
        if not path.is_file():
            continue
        if any(part in SKIP_DIRS for part in path.parts):
            continue
        rel = path.relative_to(root)
        if not include_project_docs and (rel.parts[0] == "docs" or rel.name in PROJECT_DOCS):
            continue
        if path.suffix.lower() in DEFAULT_SUFFIXES:
            yield path


def scan(root: Path, include_project_docs: bool):
    matches: dict[str, list[Path]] = {name: [] for name in PATTERNS}
    compiled = {name: re.compile(pattern, re.IGNORECASE) for name, pattern in PATTERNS.items()}

    for path in iter_files(root, include_project_docs):
        try:
            text = path.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue

        for name, pattern in compiled.items():
            if pattern.search(text):
                matches[name].append(path)

    return matches


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=".", help="Repository root to scan")
    parser.add_argument("--details", action="store_true", help="Print matching file paths")
    parser.add_argument("--include-project-docs", action="store_true",
                        help="Include current project docs/manifests in results")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    matches = scan(root, args.include_project_docs)

    print(f"License risk scan root: {root}")
    for name, paths in matches.items():
        print(f"{name}: {len(paths)}")
        if args.details:
            for path in paths:
                print(f"  {path.relative_to(root)}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
