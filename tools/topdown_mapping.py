"""Load and validate topdown-v1 atlas mapping JSON files."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

import generate_topdown_tilesheet as base

ROOT = Path(__file__).resolve().parents[1]
MAPPINGS_DIR = ROOT / "tools/mappings"

EXPECTED_CATEGORY_COUNT = len(base.CATEGORIES)
EXPECTED_INDICES = set(range(EXPECTED_CATEGORY_COUNT))
CATEGORY_BY_INDEX = {index: name for index, (name, _) in enumerate(base.CATEGORIES)}
CATEGORY_TO_INDEX = {name: index for index, name in CATEGORY_BY_INDEX.items()}


class MappingError(ValueError):
    """Raised when a mapping file fails validation."""


def _require_str(mapping: dict[str, Any], key: str) -> str:
    value = mapping.get(key)
    if not isinstance(value, str) or not value:
        raise MappingError(f"missing or invalid string field '{key}'")
    return value


def load_mapping(path: Path) -> dict[str, Any]:
    data = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(data, dict):
        raise MappingError(f"{path}: root must be an object")
    return data


def validate_mapping(data: dict[str, Any], path: Path | None = None) -> list[str]:
    label = str(path) if path else "mapping"
    errors: list[str] = []

    atlas_version = data.get("atlas_version")
    if atlas_version != base.ATLAS_VERSION:
        errors.append(f"{label}: atlas_version must be {base.ATLAS_VERSION!r}")

    mapping_kind = data.get("mapping_kind")
    if mapping_kind not in {"ultima5_tile_index", "spritesheet_cell"}:
        errors.append(f"{label}: unsupported mapping_kind {mapping_kind!r}")

    entries = data.get("entries")
    if not isinstance(entries, list) or not entries:
        errors.append(f"{label}: entries must be a non-empty array")
        return errors

    seen_indices: set[int] = set()
    for entry in entries:
        if not isinstance(entry, dict):
            errors.append(f"{label}: each entry must be an object")
            continue

        index = entry.get("index")
        category = entry.get("category")
        if not isinstance(index, int):
            errors.append(f"{label}: entry category={category!r} missing integer index")
            continue
        if index not in EXPECTED_INDICES:
            errors.append(f"{label}: index {index} out of range 0..{EXPECTED_CATEGORY_COUNT - 1}")
        if index in seen_indices:
            errors.append(f"{label}: duplicate index {index}")
        seen_indices.add(index)

        if not isinstance(category, str) or category not in CATEGORY_TO_INDEX:
            errors.append(f"{label}: invalid category {category!r} at index {index}")
        elif CATEGORY_TO_INDEX[category] != index:
            errors.append(
                f"{label}: index {index} category {category!r} does not match contract "
                f"({CATEGORY_BY_INDEX[index]!r})"
            )

        if mapping_kind == "ultima5_tile_index":
            source = entry.get("source_tile")
            if not isinstance(source, int) or source < 0:
                errors.append(f"{label}: index {index} needs non-negative integer source_tile")
        elif mapping_kind == "spritesheet_cell":
            source_col = entry.get("source_col")
            source_row = entry.get("source_row")
            if not isinstance(source_col, int) or source_col < 0:
                errors.append(f"{label}: index {index} needs non-negative integer source_col")
            if not isinstance(source_row, int) or source_row < 0:
                errors.append(f"{label}: index {index} needs non-negative integer source_row")

    return errors


def ultima_index_map(data: dict[str, Any]) -> dict[int, int]:
    if data.get("mapping_kind") != "ultima5_tile_index":
        raise MappingError("mapping_kind must be ultima5_tile_index")
    errors = validate_mapping(data)
    if errors:
        raise MappingError("; ".join(errors))

    mapping: dict[int, int] = {}
    for entry in data["entries"]:
        mapping[int(entry["index"])] = int(entry["source_tile"])
    return mapping


def spritesheet_cell_map(data: dict[str, Any]) -> dict[int, tuple[int, int]]:
    if data.get("mapping_kind") != "spritesheet_cell":
        raise MappingError("mapping_kind must be spritesheet_cell")
    errors = validate_mapping(data)
    if errors:
        raise MappingError("; ".join(errors))

    mapping: dict[int, tuple[int, int]] = {}
    for entry in data["entries"]:
        mapping[int(entry["index"])] = (int(entry["source_col"]), int(entry["source_row"]))
    return mapping


def spritesheet_pitch(data: dict[str, Any]) -> int:
    pitch = data.get("source_pitch", 0)
    if not isinstance(pitch, int) or pitch <= 0:
        raise MappingError("spritesheet_cell mappings require positive integer source_pitch")
    return pitch


def spritesheet_cell_size(data: dict[str, Any]) -> int:
    size = data.get("cell_size", 16)
    if not isinstance(size, int) or size <= 0:
        raise MappingError("spritesheet_cell mappings require positive integer cell_size")
    return size


def default_mapping_path(name: str) -> Path:
    return MAPPINGS_DIR / name


def validate_all_default_mappings() -> list[str]:
    errors: list[str] = []
    for path in sorted(MAPPINGS_DIR.glob("topdown-v1-*.json")):
        data = load_mapping(path)
        errors.extend(validate_mapping(data, path))
    return errors
