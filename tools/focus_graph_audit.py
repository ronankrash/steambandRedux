#!/usr/bin/env python3
"""Static audit: controller-reachable menu action graphs in main.gd modes."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAIN = ROOT / "game/presentation/scenes/main.gd"
ROUTER = ROOT / "game/presentation/input_router.gd"


def main() -> int:
    text = MAIN.read_text(encoding="utf-8")
    router = ROUTER.read_text(encoding="utf-8")
    errors: list[str] = []

    modes = set(re.findall(r"Mode\.([A-Z_]+)", text))
    required = {
        "TITLE",
        "CREATE",
        "INVENTORY",
        "CRAFT",
        "SKILLS",
        "MERCHANT",
        "STORAGE",
        "HEALER",
        "PAUSE",
        "DEAD",
        "CHARACTER",
        "PLAY",
        "RANGED",
    }
    missing_modes = sorted(required - modes)
    if missing_modes:
        errors.append(f"Missing mode references: {missing_modes}")

    for action in [
        "confirm",
        "cancel",
        "move",
        "inventory",
        "character",
        "pause",
        "quick_item",
        "ranged_mode",
        "interact",
        "inspect",
        "camera_peek",
        "cycle_next",
        "cycle_prev",
    ]:
        if f'"{action}"' not in router:
            errors.append(f"InputRouter missing action emit: {action}")

    if "menu_list.select(" not in text:
        errors.append("Menu rebuild does not select an item (focus trap risk)")
    if "ensure_current_is_visible" not in text:
        errors.append("Menu navigation missing ensure_current_is_visible")

    for banned in ["gui_get_hovered", "warp_mouse"]:
        if banned in text:
            errors.append(f"Potential mouse-only dependency in main.gd: {banned}")

    report = ROOT / "build/reports/focus_graph_audit.txt"
    report.parent.mkdir(parents=True, exist_ok=True)
    if errors:
        report.write_text("FAIL\n" + "\n".join(errors) + "\n", encoding="utf-8")
        print("FAIL: focus graph audit")
        for e in errors:
            print(" -", e)
        return 1
    report.write_text("PASS: controller focus graph audit\n", encoding="utf-8")
    print("PASS: focus graph audit")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
