#!/usr/bin/env python3
"""Report how current SDL2 top-down tiles cover legacy game data.

This is intentionally a lightweight audit tool, not a game-data generator. It
parses the text edit files and maps entries to the current renderer tile
contract so art gaps are visible before sourcing or drawing more assets.
"""

from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
TERRAIN = ROOT / "lib/edit/terrain.txt"
OBJECTS = ROOT / "lib/edit/object.txt"
MONSTERS = ROOT / "lib/edit/monster.txt"
DEFAULT_OUTPUT = ROOT / "docs/TOPDOWN-ASSET-COVERAGE.md"


@dataclass
class Entry:
    idx: int
    name: str
    glyph: str = ""
    color: str = ""
    info: str = ""
    flags: set[str] = field(default_factory=set)


def parse_edit_file(path: Path) -> list[Entry]:
    entries: list[Entry] = []
    current: Entry | None = None

    for raw in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or ":" not in line:
            continue

        tag, rest = line.split(":", 1)
        if tag == "N":
            parts = rest.split(":", 1)
            if len(parts) != 2:
                continue
            current = Entry(idx=int(parts[0]), name=parts[1])
            entries.append(current)
        elif current and tag == "G":
            parts = rest.split(":", 1)
            current.glyph = parts[0]
            current.color = parts[1] if len(parts) > 1 else ""
        elif current and tag == "I":
            current.info = rest
        elif current and tag == "F":
            flags = [f.strip() for f in rest.replace("|", " ").split()]
            current.flags.update(flag for flag in flags if flag)

    return entries


def terrain_tile(entry: Entry) -> str:
    idx = entry.idx
    if idx == 0:
        return "darkness"
    if idx in (1, 2):
        return "floor"
    if idx == 3:
        return "glyph"
    if idx in (4, 5) or 0x20 <= idx <= 0x2F:
        return "door"
    if idx == 6:
        return "up stairs"
    if idx == 7:
        return "down stairs"
    if 0x10 <= idx <= 0x1F:
        return "trap"
    if 0x08 <= idx <= 0x0F:
        return "shop"
    if idx == 0x31:
        return "rubble"
    if 0x32 <= idx <= 0x37:
        return "ore"
    if idx >= 0x30:
        return "wall"
    return "floor"


def object_group(entry: Entry) -> str:
    if entry.idx == 0:
        return "pile"
    tval = entry.info.split(":", 1)[0] if entry.info else "unknown"
    by_glyph = {
        ",": "food/anodyne",
        "?": "scroll/book",
        "!": "potion/flask",
        "=": "ring",
        '"': "amulet",
        "]": "armor",
        "[": "armor",
        ")": "weapon/tool",
        "/": "weapon/tool",
        "|": "weapon/tool",
        "\\": "weapon/tool",
        "}": "launcher/ray gun",
        "{": "ammo",
        "$": "money",
        "~": "device/light",
    }
    return by_glyph.get(entry.glyph, f"tval {tval}")


def monster_family(entry: Entry) -> str:
    flags = entry.flags
    if "AUTOMATA" in flags:
        return "automata"
    if "UNDEAD" in flags or "DEMON" in flags:
        return "undead/demon"
    if flags.intersection({"ANIMAL", "DRAGON", "ALIEN", "BEASTMAN", "TROLL", "GIANT"}):
        return "beast"
    if entry.glyph in {"p", "h", "t"}:
        return "humanoid"
    if entry.idx == 0:
        return "player"
    return "generic monster"


def examples(entries: list[Entry], key_func, limit: int = 4) -> dict[str, list[str]]:
    out: dict[str, list[str]] = {}
    for entry in entries:
        key = key_func(entry)
        out.setdefault(key, [])
        if len(out[key]) < limit:
            out[key].append(f"{entry.idx}: {entry.name}")
    return out


def bullet_counts(counter: Counter[str], sample_map: dict[str, list[str]]) -> str:
    lines = []
    for key, count in sorted(counter.items()):
        samples = "; ".join(sample_map.get(key, []))
        lines.append(f"- `{key}`: {count} entries. Examples: {samples}")
    return "\n".join(lines)


def render_report() -> str:
    terrain = parse_edit_file(TERRAIN)
    objects = parse_edit_file(OBJECTS)
    monsters = parse_edit_file(MONSTERS)

    terrain_counts = Counter(terrain_tile(e) for e in terrain)
    object_counts = Counter(object_group(e) for e in objects)
    monster_counts = Counter(monster_family(e) for e in monsters)

    return f"""# SDL2 Top-Down Asset Coverage

Last generated: by `python tools/report_topdown_asset_coverage.py`

This report maps the current SDL2 top-down tile contract to legacy game data in
`lib/edit/terrain.txt`, `lib/edit/object.txt`, and `lib/edit/monster.txt`.

## Current Atlas

The checked-in placeholder atlas is `lib/xtra/graf/sdl2_topdown_24.bmp`.
It is project-generated and currently provides the `topdown-v1` contract with
36 broad tiles:

- darkness
- floor
- wall
- door
- up stairs
- down stairs
- trap
- glyph
- shop
- General Store shop
- Clothing store shop
- Gun shop
- Machinist shop
- Alchemy shop
- Magic shop
- Black Market shop
- Home
- rubble
- ore vein
- object
- food/anodyne object
- scroll/book object
- potion/flask object
- weapon/tool object
- armor object
- ray gun/launcher object
- ammo object
- money object
- jewelry object
- device/chest object
- generic monster
- automata
- undead/demon
- beast
- humanoid
- player

## Terrain Coverage

Terrain is matched to current broad tiles, including shops, glyphs, rubble, and
ore veins. Store types and individual trap kinds still share broad family tiles.

{bullet_counts(terrain_counts, examples(terrain, terrain_tile))}

## Object Coverage

Objects are now matched to broad item-family tiles in the live renderer when
bounded `cave_o_idx -> o_list` data is available. Unknown or invalid object data
falls back to terrain or the generic object tile. The groups below are the
coverage buckets used to guide future art polish.

{bullet_counts(object_counts, examples(objects, object_group))}

## Monster Coverage

Monsters are partially matched. The renderer uses live `r_info` metadata to map
visible monsters into broad family tiles; unknown or unmapped races use the
generic monster tile.

{bullet_counts(monster_counts, examples(monsters, monster_family))}

## Gaps To Close Before A Real Tileset Claim

- Split shop/town facades by store type after the base shop tile is visually proven.
- Split object-family placeholders into higher-quality final art for food,
  scroll/books, potions, weapons/tools, armor, ray guns/launchers, ammo, money,
  devices/lights, rings, and amulets.
- Consider separate monster tiles for dragon, alien, animal, brute, undead,
  demon, automata, townsfolk/humanoid, and generic oddities instead of the
  current broad buckets.
- Add tile contract versioning before changing atlas geometry again.
- Keep all external art out of the repo until `ASSETS.md` records source,
  author, exact license, and commercial-use status.
"""


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT), help="Markdown report path.")
    args = parser.parse_args()

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(render_report(), encoding="utf-8")
    print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
