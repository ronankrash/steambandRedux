#!/usr/bin/env python3
"""Validate Brassdeep content JSON for CI."""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONTENT = ROOT / "game/content"
VALID_SLOTS = {"weapon", "armor", "offhand", "none", ""}
VALID_ABILITIES = {"melee", "ranged", "poison_spit", "steam_burst"}


def load_folder(name: str) -> dict[str, dict]:
    folder = CONTENT / name
    out: dict[str, dict] = {}
    if not folder.exists():
        raise SystemExit(f"Missing content folder: {folder}")
    for path in sorted(folder.glob("*.json")):
        data = json.loads(path.read_text(encoding="utf-8"))
        cid = data.get("id")
        if not cid:
            raise SystemExit(f"Missing id in {path}")
        if cid in out:
            raise SystemExit(f"Duplicate id {cid} in {name}")
        out[cid] = data
    return out


def main() -> int:
    errors: list[str] = []
    races = load_folder("races")
    classes = load_folder("classes")
    monsters = load_folder("monsters")
    items = load_folder("items")
    recipes = load_folder("recipes")
    affixes = load_folder("affixes")

    if len(races) < 2:
        errors.append("Need at least 2 races")
    if len(classes) < 2:
        errors.append("Need at least 2 classes")
    if len(monsters) < 3:
        errors.append("Need at least 3 monsters")
    if len(recipes) < 3:
        errors.append("Need at least 3 recipes")

    for iid, item in items.items():
        slot = item.get("equip_slot", "")
        if slot not in VALID_SLOTS:
            errors.append(f"Item {iid} invalid equip_slot {slot!r}")
        for aff in item.get("allowed_affixes", []):
            if aff not in affixes:
                errors.append(f"Item {iid} references missing affix {aff}")
        for field in ("name", "description"):
            if field not in item:
                errors.append(f"Item {iid} missing {field}")

    for rid, recipe in recipes.items():
        for ing in recipe.get("ingredients", []):
            iid = ing.get("item_id")
            if iid not in items:
                errors.append(f"Recipe {rid} invalid ingredient {iid}")
        result = recipe.get("result_id")
        if result not in items:
            errors.append(f"Recipe {rid} invalid result {result}")

    for mid, mon in monsters.items():
        for ability in mon.get("abilities", []):
            if ability not in VALID_ABILITIES:
                errors.append(f"Monster {mid} invalid ability {ability}")
        drop = mon.get("drop_item_id")
        if drop and drop not in items:
            errors.append(f"Monster {mid} invalid drop {drop}")

    if errors:
        print("FAIL: content validation")
        for err in errors:
            print(f"  - {err}")
        return 1

    print(
        f"PASS: content ok ({len(races)} races, {len(classes)} classes, "
        f"{len(monsters)} monsters, {len(items)} items, {len(recipes)} recipes)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
