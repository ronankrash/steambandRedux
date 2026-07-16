#!/usr/bin/env python3
"""Brassdeep Phase 2 content reachability analysis.

Builds a static reachability graph from game acquisition paths documented in
game/core/game_sim.gd (starting gear, merchants, drops, containers, expedition
loot, crafting) and fails CI when required content cannot be obtained in play.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
CONTENT = ROOT / "game" / "content"
REPORT_PATH = ROOT / "build" / "reports" / "content_reachability.json"

# Mirrors GameSim.theme_schedule
THEME_SCHEDULE = ["env_foundry", "env_mine", "env_foundry", "env_mine", "env_foundry"]

# Guaranteed for every new game (_give_starting_gear)
GUARANTEED_STARTERS = [
    "item_wrench",
    "item_pepperbox",
    "item_ball_ammo",
    "item_scrap",
    "item_machine_oil",
]

# Phase 2 minimums (tools/validate_content.py)
PHASE2_MINIMUMS = {
    "melee_weapons": 5,
    "firearms": 5,
    "ammo": 4,
    "armor": 10,
    "consumables": 6,
    "materials": 10,
}

RECIPE_CLOSURE_DEPTH = 12


def load_folder(name: str) -> dict[str, dict[str, Any]]:
    folder = CONTENT / name
    out: dict[str, dict[str, Any]] = {}
    if not folder.exists():
        raise SystemExit(f"Missing content folder: {folder}")
    for path in sorted(folder.glob("*.json")):
        data = json.loads(path.read_text(encoding="utf-8"))
        cid = str(data.get("id", ""))
        if not cid:
            raise SystemExit(f"Missing id in {path}")
        if cid in out:
            raise SystemExit(f"Duplicate id {cid} in {name}")
        out[cid] = data
    return out


def monster_matches_theme(monster: dict[str, Any], theme: str) -> bool:
    tags: list[str] = list(monster.get("theme_tags", []))
    theme_key = theme.replace("env_", "")
    return (
        not tags
        or "any" in tags
        or theme in tags
        or theme_key in tags
    )


def monster_spawn_reachable(monster: dict[str, Any], themes: list[str]) -> bool:
    if bool(monster.get("is_boss", False)) or monster.get("behavior") == "boss":
        return True  # depth 5 boss placement
    return any(monster_matches_theme(monster, theme) for theme in themes)


def monster_drop_id(monster: dict[str, Any]) -> str:
    drop = str(monster.get("drop_item_id", ""))
    if drop:
        return drop
    behavior = str(monster.get("behavior", ""))
    if behavior == "melee_pursuer":
        return "item_scrap"
    if behavior == "ranged_sentry":
        return "item_ball_ammo"
    return "item_scrap"


def collect_starting_items(
    races: dict[str, dict[str, Any]],
    classes: dict[str, dict[str, Any]],
) -> set[str]:
    found: set[str] = set(GUARANTEED_STARTERS)
    for race in races.values():
        for entry in race.get("starting_items", []):
            iid = str(entry.get("item_id", ""))
            if iid:
                found.add(iid)
    for klass in classes.values():
        for entry in klass.get("starting_items", []):
            iid = str(entry.get("item_id", ""))
            if iid:
                found.add(iid)
    return found


def collect_merchant_items(merchants: dict[str, dict[str, Any]]) -> set[str]:
    found: set[str] = set()
    for merchant in merchants.values():
        for entry in merchant.get("inventory", []):
            iid = str(entry.get("item_id", ""))
            if iid:
                found.add(iid)
    return found


def collect_skill_abilities(skills: dict[str, dict[str, Any]]) -> dict[str, str]:
    """Map ability_id -> parent skill_id."""
    out: dict[str, str] = {}
    for sid, skill in skills.items():
        for ability in skill.get("abilities", []):
            aid = str(ability.get("id", ""))
            if aid:
                out[aid] = sid
    return out


def skill_raisable(skill: dict[str, Any], skills: dict[str, dict[str, Any]]) -> bool:
    """All skills are raisable via SP when prerequisites (if any) can be met."""
    prereq = skill.get("prerequisite", {})
    if not isinstance(prereq, dict) or not prereq:
        return True
    need_id = str(prereq.get("skill_id", ""))
    need_rank = int(prereq.get("rank", 0))
    if need_id not in skills:
        return False
    need_skill = skills[need_id]
    max_rank = int(need_skill.get("max_rank", 5))
    return need_rank <= max_rank and skill_raisable(need_skill, skills)


def ability_reachable(
    ability: dict[str, Any],
    skill: dict[str, Any],
    skills: dict[str, dict[str, Any]],
    starting_skills: set[str],
) -> bool:
    sid = str(skill.get("id", ""))
    if sid in starting_skills:
        return True
    if not skill_raisable(skill, skills):
        return False
    rank_required = int(ability.get("rank_required", 99))
    max_rank = int(skill.get("max_rank", 5))
    return rank_required <= max_rank


def item_category_bucket(item: dict[str, Any]) -> str | None:
    slot = str(item.get("equip_slot", ""))
    if slot == "mainhand":
        return "melee_weapons"
    if slot == "ranged":
        return "firearms"
    if item.get("is_ammo") or slot == "ammo":
        return "ammo"
    if slot in {"head", "body", "hands", "feet"}:
        return "armor"
    if item.get("category") == "consumable":
        return "consumables"
    if item.get("category") == "material":
        return "materials"
    return None


def expand_item_reachability(
    items: dict[str, dict[str, Any]],
    recipes: dict[str, dict[str, Any]],
    seeds: set[str],
    monster_drops: set[str],
) -> tuple[set[str], dict[str, list[str]]]:
    """Fixed-point: starters, merchants, drops, expedition/container loot, recipes."""
    reachable = set(seeds)
    reachable.update(monster_drops)
    # Expedition floor loot (_spawn_loot)
    reachable.add("item_brass_plate")
    if "item_gears" in items:
        reachable.add("item_gears")
    # Containers always grant scrap (interact TILE_CONTAINER -> item_scrap)
    reachable.add("item_scrap")

    sources: dict[str, list[str]] = {iid: ["seed"] for iid in seeds}
    for iid in monster_drops:
        sources.setdefault(iid, []).append("monster_drop")
    sources.setdefault("item_brass_plate", []).append("dungeon_starter_loot")
    if "item_gears" in items:
        sources.setdefault("item_gears", []).append("expedition_depth2_loot")

    changed = True
    depth = 0
    while changed and depth < RECIPE_CLOSURE_DEPTH:
        changed = False
        depth += 1
        for recipe in recipes.values():
            result_id = str(recipe.get("result_id", ""))
            if not result_id or result_id in reachable:
                continue
            ingredients = recipe.get("ingredients", [])
            if not ingredients:
                continue
            if all(str(ing.get("item_id", "")) in reachable for ing in ingredients):
                reachable.add(result_id)
                sources.setdefault(result_id, []).append(f"recipe:{recipe.get('id', '?')}")
                changed = True

    return reachable, sources


def build_report() -> dict[str, Any]:
    races = load_folder("races")
    classes = load_folder("classes")
    skills = load_folder("skills")
    monsters = load_folder("monsters")
    items = load_folder("items")
    recipes = load_folder("recipes")
    affixes = load_folder("affixes")
    merchants = load_folder("merchants")
    environments = load_folder("environments")

    themes = list(dict.fromkeys(THEME_SCHEDULE))
    schedule_envs = set(THEME_SCHEDULE)

    starting_items = collect_starting_items(races, classes)
    merchant_items = collect_merchant_items(merchants)

    spawnable_monsters: set[str] = set()
    unreachable_monsters: list[str] = []
    for mid, mon in monsters.items():
        if monster_spawn_reachable(mon, themes):
            spawnable_monsters.add(mid)
        else:
            unreachable_monsters.append(mid)

    monster_drop_items: set[str] = set()
    for mid in spawnable_monsters:
        drop = monster_drop_id(monsters[mid])
        if drop in items:
            monster_drop_items.add(drop)

    reachable_items, item_sources = expand_item_reachability(
        items, recipes, starting_items | merchant_items, monster_drop_items
    )

    unreachable_items = sorted(iid for iid in items if iid not in reachable_items)

    # Skills: raisable via SP; also count class starting_skills as immediately active
    starting_skill_ids: set[str] = set()
    for klass in classes.values():
        starting_skill_ids.update(str(s) for s in klass.get("starting_skills", {}).keys())

    unreachable_skills = [
        sid for sid, skill in skills.items() if not skill_raisable(skill, skills)
    ]

    ability_map = collect_skill_abilities(skills)
    unreachable_abilities: list[str] = []
    for sid, skill in skills.items():
        for ability in skill.get("abilities", []):
            aid = str(ability.get("id", ""))
            if not aid:
                continue
            if not ability_reachable(ability, skill, skills, starting_skill_ids):
                unreachable_abilities.append(aid)

    # Affixes on reachable items
    reachable_affixes: set[str] = set()
    for iid in reachable_items:
        for aff in items[iid].get("allowed_affixes", []):
            reachable_affixes.add(str(aff))
    unreachable_affixes = sorted(aid for aid in affixes if aid not in reachable_affixes)

    unreachable_recipes = [
        rid
        for rid, recipe in recipes.items()
        if str(recipe.get("result_id", "")) not in reachable_items
    ]

    unreachable_environments = [
        eid for eid in environments if eid not in schedule_envs
    ]

    # Category coverage among reachable items
    category_counts: dict[str, int] = {k: 0 for k in PHASE2_MINIMUMS}
    for iid in reachable_items:
        bucket = item_category_bucket(items[iid])
        if bucket:
            category_counts[bucket] += 1

    category_shortfalls = {
        cat: {"required": need, "reachable": category_counts[cat]}
        for cat, need in PHASE2_MINIMUMS.items()
        if category_counts[cat] < need
    }

    required_ids: dict[str, list[str]] = {
        "races": sorted(races),
        "classes": sorted(classes),
        "skills": sorted(skills),
        "abilities": sorted(ability_map),
        "monsters": sorted(monsters),
        "recipes": sorted(recipes),
        "merchants": sorted(merchants),
        "environments": sorted(schedule_envs),
        "items": sorted(items),
        "affixes": sorted(affixes),
    }

    unreachable_by_kind: dict[str, list[str]] = {
        "races": [],  # always selectable at creation
        "classes": [],
        "skills": unreachable_skills,
        "abilities": sorted(unreachable_abilities),
        "monsters": sorted(unreachable_monsters),
        "recipes": sorted(unreachable_recipes),
        "merchants": [],
        "environments": unreachable_environments,
        "items": unreachable_items,
        "affixes": unreachable_affixes,
    }

    total_required = sum(len(v) for v in required_ids.values())
    total_unreachable = sum(len(v) for v in unreachable_by_kind.values())
    total_unreachable += len(category_shortfalls)
    reachable_count = total_required - sum(
        len(unreachable_by_kind[k]) for k in required_ids if k != "affixes"
    )
    # affixes counted in required_ids; include in unreachable tally
    total_unreachable += len(unreachable_by_kind["affixes"])
    pct = round(100.0 * (total_required - total_unreachable) / total_required, 2) if total_required else 100.0

    return {
        "summary": {
            "percentage_reachable": pct,
            "total_required_entities": total_required,
            "total_unreachable_entities": total_unreachable,
            "category_shortfalls": category_shortfalls,
            "pass": total_unreachable == 0,
        },
        "reachable": {
            "items_count": len(reachable_items),
            "spawnable_monsters": sorted(spawnable_monsters),
            "reachable_affixes": sorted(reachable_affixes),
            "themes": themes,
        },
        "unreachable": unreachable_by_kind,
        "item_sources_sample": {
            k: item_sources.get(k, ["unreachable"])
            for k in sorted(unreachable_items)[:20]
        },
        "details": {
            "starting_item_union": sorted(starting_items),
            "merchant_items": sorted(merchant_items),
            "monster_drop_items": sorted(monster_drop_items),
            "category_counts": category_counts,
        },
    }


def print_human_summary(report: dict[str, Any]) -> None:
    s = report["summary"]
    print(f"Content reachability: {s['percentage_reachable']}%")
    print(f"Required entities: {s['total_required_entities']}")
    print(f"Unreachable entities: {s['total_unreachable_entities']}")
    if s["category_shortfalls"]:
        print("Category shortfalls:")
        for cat, info in s["category_shortfalls"].items():
            print(f"  - {cat}: {info['reachable']}/{info['required']} reachable")
    unreachable = report["unreachable"]
    any_issue = False
    for kind, ids in unreachable.items():
        if ids:
            any_issue = True
            print(f"\nUnreachable {kind} ({len(ids)}):")
            for iid in ids:
                print(f"  - {iid}")
    if not any_issue and not s["category_shortfalls"]:
        print("\nAll required Phase 2 content is reachable.")
    elif s["pass"]:
        print("\nPASS (with category coverage)")
    else:
        print("\nFAIL: unreachable required content detected")


def main() -> int:
    report = build_report()
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {REPORT_PATH}")
    print_human_summary(report)
    return 0 if report["summary"]["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
