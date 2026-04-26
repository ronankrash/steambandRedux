# Playtest Checklist

Last updated: 2026-04-25

Use this checklist for current 2D baseline smoke tests and near-term SDL2 first-person prototype tests.

## Preflight Record

```text
Build tested:
Hardware:
Display mode:
Power profile:
Controller mode:
Save file used:
Tester:
Date:
```

Confirm both automated test paths before manual play:

```bash
ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure
ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure
```

## Launch And Keyboard Parity

Pass if:

- Game launches to the legacy prompt.
- `N` starts new-game flow.
- `O` opens load flow.
- `Escape` cancels/backtracks.
- `Enter` confirms selections.
- Arrow keys and numpad movement work in-game.
- Original commands still work: `i`, `e`, `w`, `t`, `d`, `o`, `c`, `s`, `R`, `l`, `f`, `m`, `p`, `<`, `>`, `M`, `C`, `?`.

Fail if:

- Any required action becomes controller-only.
- Keyboard input is lost after controller use.
- Window focus moves so keys no longer reach the game.

## Default Controller Mapping

Test with an Asus ROG Ally or Xbox-compatible XInput controller.

Pass if:

- `A` sends Enter/confirm.
- `B` sends Escape/cancel.
- `X` opens inventory with `i`.
- `Y` opens equipment with `e`.
- D-pad moves cardinally as numpad `8/2/4/6`.
- Holding D-pad repeats movement at a usable rate.
- Left stick provides 8-way movement: `7/8/9/4/6/1/2/3`.
- `Start` behaves as Escape.
- `Back` opens map via `M` unless a menu gesture consumes it.
- `LB` rests via `R`.
- `RB` searches via `s`.

Record:

```text
D-pad repeat: PASS/FAIL, notes:
Left stick deadzone: PASS/FAIL, notes:
Diagonal reliability: PASS/FAIL, notes:
Accidental inputs: PASS/FAIL, notes:
```

## Controller Command Menu

Expected behavior: double `Back` opens the command grid.

Pass if:

- Double `Back` within roughly 500 ms opens `Controller Command Menu`.
- D-pad navigates the grid.
- `A` selects a command and closes the menu.
- `B` cancels and closes the menu.
- Selected commands match keyboard behavior.

High-risk item: double/triple `Back` gesture overlap may be hard to perform reliably. Record failures as UX bugs, not tester error.

## Controller Config Menu

Expected behavior: triple `Back` opens button configuration.

Pass if:

- Triple `Back` within roughly 500 ms opens `Controller Button Configuration`.
- D-pad up/down navigates mappings.
- `A` enters remap mode.
- Pressing another mapped button updates the target mapping.
- `B` saves/exits in normal config mode.
- `lib/user/controller.prf` is created or updated.
- Restarting the game reloads saved mappings.

Record whether `B` cancels remap or becomes a remap source.

## Save/Load And Core Gameplay

Pass if:

- New character creation can be completed by keyboard.
- Same flow is navigable enough by controller for confirm/cancel and common choices.
- Movement, bump combat, pickup/use, inventory, equipment, rest, search, map, stairs, save, and load work.
- Alternating keyboard and controller does not crash or trap input.

## ROG Ally Risks

Record explicitly:

- Text readability at 720p and 1080p.
- Focus remains on the intended game window.
- Ally controls are detected as controller 0.
- Sleep/resume preserves input.
- External controller disconnect/reconnect behavior.
- Left-stick drift/deadzone.
- Idle CPU/heat in menus.
- Double/triple `Back` discoverability.
- Right stick behavior. Current expected result: first-person camera turning only after implemented.

## First-Person Prototype

Use for SDL2 builds.

Pass if:

- Legacy 2D UI still launches and remains available.
- SDL renderer window appears only in SDL2 builds.
- Renderer syncs to real player position after movement, stairs, load, and new level.
- Keyboard movement still matches original command behavior.
- Controller left stick/D-pad movement works without breaking roguelike turns.
- Right stick look/turn works after implementation.
- `Escape`/`B` exits menus or mode predictably.
- Window resize preserves readable UI.
- SDL window close does not crash the main game.
- Shutdown/relaunch does not leave orphan windows or locked input.

## Recording Format

```text
ID:
Area:
Build: nosdl/sdl2
Input: keyboard/controller/both
Steps:
Expected:
Actual:
Result: PASS/FAIL/BLOCKED/NOT TESTED
Severity: Critical/Major/Minor
ROG Ally notes:
Log excerpt/path:
Screenshot/video:
Follow-up:
```

Summary:

```text
Controller Coverage: __ / __ passed
Keyboard Parity: PASS/FAIL
ROG Ally Hardware: PASS/FAIL/BLOCKED
First-Person Prototype: PASS/FAIL/NOT LIVE
Critical Issues:
Major Issues:
Minor Issues:
Residual Risks:
Tester Recommendation: ship baseline / fix before next milestone / blocked
```
