# ROG Ally Controller Playtest — Brassdeep Phase 2

**Status:** Manual procedure only. This document does **not** claim the ROG Ally test has been passed. Run on actual Windows handheld hardware (ASUS ROG Ally or equivalent) at **1280×720**.

## Installation

1. Download the CI artifact `Brassdeep-windows.zip` (or build via `BRASSDEEP_EXPORT=1 ./tools/run_ci_local.sh`).
2. Extract to a writable folder (e.g. `C:\Games\Brassdeep\`).
3. Confirm `Brassdeep.exe` and `Brassdeep.pck` sit in the same directory.
4. Optional: use `Brassdeep.console.exe` when capturing stdout logs.

## Launch

- Double-click `Brassdeep.exe`, or from PowerShell:
  ```powershell
  cd C:\Games\Brassdeep
  .\Brassdeep.exe
  ```
- Set display mode to **1280×720** windowed or fullscreen in Armoury Crate / Windows display settings for the session.
- Confirm the title screen shows controller prompts after any stick/button input (`controller` appears in the status bar).

## Expected controller mapping (default)

| Action | Typical Ally / Xbox mapping |
|--------|-----------------------------|
| Move / menu navigate | Left stick / D-pad |
| Confirm / interact | A |
| Cancel / back | B |
| Quick item | X |
| Inventory | Y |
| Ranged targeting | RT (right trigger) |
| Character sheet | View / Select |
| Pause / save | Menu / Start |
| Cycle ranged targets | LB / RB |

No required action may depend on mouse hover, mouse wheel, pointer position, or keyboard text entry.

## 20-minute critical-path playthrough

1. **Title** — New Game.
2. **Create** — Pick any race/class; note seed; Begin in Brassharbor.
3. **Hub panels** — Open/close Inventory, Character, Skills, Pause. Confirm focus never disappears.
4. **Merchant** — Walk to a trader tile; buy one item; sell one item.
5. **Storage** — Deposit one item; withdraw it.
6. **Craft** — Visit workbench, forge, and alchemy; craft one available recipe at each (gather/buy mats if needed).
7. **Equip** — Equip a melee weapon, body armor, and ammo for a firearm if available.
8. **Skills** — Spend at least one skill point.
9. **Expedition** — Take the stairs; confirm depth 1 loads.
10. **Combat** — Melee a foe; enter ranged mode; fire; use a consumable.
11. **Items** — Drop an item; pick it up.
12. **Depth** — Find stairs down; descend once.
13. **Save/Load** — Pause → Save → Title → Load; verify depth, gold, equipment.
14. **Death or extract** — Either die intentionally (note Fallen summary) or reach extract/boss if time allows; restart or return to title.

## Menu-navigation checks

- Every major panel: Inventory, Character, Skills, Craft, Merchant, Storage, Pause, Death.
- Confirm A activates the highlighted row; B closes to play (or title from create/death as labeled).
- Scroll long inventories with D-pad only; ensure the selection bar remains visible.

## Performance observations to record

| Metric | Observation |
|--------|-------------|
| Title → town load time | |
| Expedition generation hitch | |
| Sustained FPS at 1280×720 | |
| Input latency (stick → move) | |
| Thermal / fan during 20 min | |
| Battery drain (optional) | |

## Save-file location

Godot user data on Windows (typical):

`%APPDATA%\Godot\app_userdata\Brassdeep\`

Look for save JSON written by `SaveSystem` (schema v2).

## How to capture logs

1. Launch `Brassdeep.console.exe` from PowerShell and tee output:
   ```powershell
   .\Brassdeep.console.exe 2>&1 | Tee-Object -FilePath brassdeep_playtest.log
   ```
2. Or set environment / redirect as preferred; attach the log to defect reports.
3. Screenshots: Win+PrtScn or Xbox Game Bar (Win+G).

## Defect-report template

```
### Title
Short summary

### Device
ROG Ally (model/BIOS) | Windows build | 1280×720 windowed/fullscreen

### Build
ZIP/SHA or CI run URL | seed | race | class

### Steps
1.
2.
3.

### Expected
…

### Actual
…

### Severity
Blocker / Major / Minor / Polish

### Attachments
Log path, screenshots, save file
```

## Sign-off

| Check | Tester | Date | Pass? |
|-------|--------|------|-------|
| Critical path 20 min | | | |
| Controller-only menus | | | |
| Save/load integrity | | | |
| No focus traps | | | |

**Do not mark this table Pass unless executed on real compatible Windows hardware.**
