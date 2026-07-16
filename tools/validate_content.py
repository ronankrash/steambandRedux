#!/usr/bin/env python3
"""Validate Brassdeep content JSON for CI (Phase 2)."""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONTENT = ROOT / "game/content"
VALID_SLOTS = {
    "mainhand",
    "offhand",
    "ranged",
    "ammo",
    "head",
    "body",
    "hands",
    "feet",
    "accessory",
    "none",
    "",
}
VALID_ABILITIES = {
    "melee",
    "ranged",
    "poison_spit",
    "steam_burst",
    "fear",
    "repair",
    "summon",
    "suppress",
    "aoe",
}
VALID_STATIONS = {"workbench", "forge", "alchemy"}


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
    skills = load_folder("skills")
    monsters = load_folder("monsters")
    items = load_folder("items")
    recipes = load_folder("recipes")
    affixes = load_folder("affixes")
    merchants = load_folder("merchants")
    environments = load_folder("environments")

    if len(races) < 6:
        errors.append(f"Need at least 6 races, found {len(races)}")
    if len(classes) < 6:
        errors.append(f"Need at least 6 classes, found {len(classes)}")
    if len(skills) < 8:
        errors.append(f"Need at least 8 skills, found {len(skills)}")
    if len(monsters) < 12:
        errors.append(f"Need at least 12 monsters, found {len(monsters)}")
    if len(recipes) < 20:
        errors.append(f"Need at least 20 recipes, found {len(recipes)}")
    if len(affixes) < 12:
        errors.append(f"Need at least 12 affixes, found {len(affixes)}")
    if len(merchants) < 3:
        errors.append(f"Need at least 3 merchants, found {len(merchants)}")
    if len(environments) < 2:
        errors.append(f"Need at least 2 environments, found {len(environments)}")

    ability_count = sum(len(s.get("abilities", [])) for s in skills.values())
    if ability_count < 12:
        errors.append(f"Need at least 12 skill abilities, found {ability_count}")

    melee = sum(1 for i in items.values() if i.get("equip_slot") == "mainhand")
    firearms = sum(1 for i in items.values() if i.get("equip_slot") == "ranged")
    ammo = sum(1 for i in items.values() if i.get("is_ammo") or i.get("equip_slot") == "ammo")
    armor = sum(1 for i in items.values() if i.get("equip_slot") in {"head", "body", "hands", "feet"})
    consumables = sum(1 for i in items.values() if i.get("category") == "consumable")
    materials = sum(1 for i in items.values() if i.get("category") == "material")
    if melee < 5:
        errors.append(f"Need 5+ melee weapons, found {melee}")
    if firearms < 5:
        errors.append(f"Need 5+ firearms, found {firearms}")
    if ammo < 4:
        errors.append(f"Need 4+ ammo types, found {ammo}")
    if armor < 10:
        errors.append(f"Need 10+ armor pieces, found {armor}")
    if consumables < 6:
        errors.append(f"Need 6+ consumables, found {consumables}")
    if materials < 10:
        errors.append(f"Need 10+ materials, found {materials}")

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
        station = recipe.get("station", "workbench")
        if station not in VALID_STATIONS:
            errors.append(f"Recipe {rid} invalid station {station}")
        for ing in recipe.get("ingredients", []):
            iid = ing.get("item_id")
            if iid not in items:
                errors.append(f"Recipe {rid} invalid ingredient {iid}")
        result = recipe.get("result_id")
        if result not in items:
            errors.append(f"Recipe {rid} invalid result {result}")
        sid = recipe.get("skill_required")
        if sid and sid not in skills:
            errors.append(f"Recipe {rid} missing skill {sid}")

    boss_count = 0
    for mid, mon in monsters.items():
        for ability in mon.get("abilities", []):
            if ability not in VALID_ABILITIES:
                errors.append(f"Monster {mid} invalid ability {ability}")
        drop = mon.get("drop_item_id")
        if drop and drop not in items:
            errors.append(f"Monster {mid} invalid drop {drop}")
        if mon.get("is_boss") or mon.get("behavior") == "boss":
            boss_count += 1
    if boss_count < 1:
        errors.append("Need at least one boss")

    for mid, mer in merchants.items():
        for entry in mer.get("inventory", []):
            iid = entry.get("item_id")
            if iid not in items:
                errors.append(f"Merchant {mid} sells missing item {iid}")

    for rid, race in races.items():
        for entry in race.get("starting_items", []):
            iid = entry.get("item_id")
            if iid not in items:
                errors.append(f"Race {rid} starting item missing {iid}")
    for cid, cls in classes.items():
        for entry in cls.get("starting_items", []):
            iid = entry.get("item_id")
            if iid not in items:
                errors.append(f"Class {cid} starting item missing {iid}")
        for sid in cls.get("starting_skills", {}):
            if sid not in skills:
                errors.append(f"Class {cid} starting skill missing {sid}")

    if errors:
        print("FAIL: content validation")
        for err in errors:
            print(f"  - {err}")
        return 1

    print(
        f"PASS: content ok ({len(races)} races, {len(classes)} classes, "
        f"{len(skills)} skills/{ability_count} abilities, {len(monsters)} monsters, "
        f"{len(items)} items, {len(recipes)} recipes, {len(merchants)} merchants, "
        f"{len(environments)} environments)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
