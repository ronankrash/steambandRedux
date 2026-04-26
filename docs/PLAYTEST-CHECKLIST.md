# Playtest Checklist

Last updated: 2026-04-25

Use this checklist for current 2D baseline smoke tests and near-term SDL2 first-person prototype tests.

## Fast ROG Ally First-Person Smoke

Use this when the goal is simply to launch the current SDL2 prototype and verify the handheld controls quickly.

For the shortest local run, use the smoke launcher from the repository root:

```bat
tools\launch_rog_ally_fp_smoke.cmd
```

See `docs\ROG-ALLY-FP-MANUAL-TEST.md` for the focused ROG Ally launch guide, controls, and log review notes.

```bash
cmake -S . -B build-rescue-sdl2 -DCMAKE_TOOLCHAIN_FILE=C:/Users/bkars/vcpkg/scripts/buildsystems/vcpkg.cmake -DSTEAMBAND_ENABLE_SDL2=ON
cmake --build build-rescue-sdl2 --config Debug
ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure
build-rescue-sdl2/Debug/SteambandRedux.exe
```

Quick pass:

- At the legacy prompt, press `N` for a new game or `O` to open a save.
- Confirm the launch screen shows the ROG Ally control hints, including `A`/`B`/`X`/`Y`, `Back=Map/Menu/Config`, and the first-person toggle hint.
- Toggle first-person mode with `Ctrl+F12`; on Ally/controller hardware also test `L3 + R3`.
- In first-person mode, use D-pad or left stick for roguelike movement and right stick for camera turn.
- Press `Escape` or `Ctrl+F12` while the SDL window is focused to return to the 2D fallback.
- Check `build-rescue-sdl2/Debug/lib/logs/steamband.log` for controller init, renderer init, DDA startup, and first-person activation lines.

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
- Launch-screen control hints are readable at 720p and 1080p and do not obscure the `New`/`Open` prompt.
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
- The launch hints advertise controls that are unavailable in the current build.

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
- Single `Back` opens map via `M` after the double/triple gesture window expires.
- `LB` rests via `R`.
- `RB` searches via `s`.
- The launch screen summarizes these defaults before a new game or save is opened.

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

- Double `Back` within roughly 500 ms opens `Controller Command Menu` after the triple-press window expires.
- D-pad navigates the grid.
- `A` selects a command and closes the menu.
- `B` or `Back` cancels and closes the menu.
- Selected commands match keyboard behavior.

High-risk item: double/triple `Back` gesture overlap may still be hard to perform reliably. Record failures as UX bugs, not tester error.

## Controller Config Menu

Expected behavior: triple `Back` opens button configuration.

Pass if:

- Triple `Back` within roughly 500 ms opens `Controller Button Configuration`.
- D-pad up/down navigates mappings.
- `A` enters remap mode.
- Pressing another mapped button updates the target mapping.
- The `A` press used to enter remap mode is ignored until released.
- `B` or `Back` saves/exits in normal config mode.
- `B` or `Back` cancels remap mode after the entry press is released.
- `lib/user/controller.prf` is created or updated.
- Restarting the game reloads saved mappings.

Fail if `B` becomes a remap source instead of cancelling remap mode.

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
- Right stick behavior. Current expected result: first-person camera turns only while first-person mode is active.

## First-Person Prototype

Use for SDL2 builds.

Pass if:

- Legacy 2D UI still launches and remains available.
- `Ctrl+F12` and `L3 + R3` show/hide the SDL renderer window in SDL2 builds.
- Renderer syncs to real player position after movement, stairs, load, and new level.
- Focused SDL keyboard uses first-person movement: `W`/Up move forward relative to the camera, `S`/Down move backward, `A`/`D` strafe, and Left/Right arrows turn.
- Non-movement keyboard commands such as inventory/equipment/stairs still forward to the legacy command queue where not reserved for first-person movement.
- Keyboard commands still work while the SDL first-person window has focus.
- Controller left stick/D-pad movement is camera-relative while first-person mode is active and still consumes normal roguelike turns.
- Right stick turns the first-person camera while the SDL window is active.
- First-person HUD gauges update for HP, SP, dungeon depth, SDL focus, status ailments, and recent-message activity.
- SDL window title includes exact HP/SP/depth/status text for readable fallback feedback.
- Double `Back` command-menu selection is visible through the first-person HUD/title as a short command overlay message.
- `Escape` and `Ctrl+F12` exit first-person mode predictably when the SDL window has focus.
- Window resize preserves readable UI.
- Window resize is clamped to a playable viewport and does not distort or crash the raycaster.
- Atmospheric ceiling/floor gradients, distance fog, side shading, and wall color variation are visible without unapproved art assets.
- Debug minimap is hidden by default so the first-person view feels immersive rather than like a test overlay.
- SDL window close does not crash the main game.
- Shutdown/relaunch does not leave orphan windows or locked input.

Record log evidence from `build-rescue-sdl2/Debug/lib/logs/steamband.log` when available:

```text
Controller init line:
Renderer init line:
DDA startup line:
First-person activation line:
First-person exit/fallback line:
```

Without physical ROG Ally hardware, validate:

- SDL2 and non-SDL builds/tests pass.
- SDL2 launch stays alive long enough to reach the legacy prompt.
- `Ctrl+F12` activates first-person mode and logs activation.
- `Escape`/`Ctrl+F12` exit behavior is checked with a focused SDL window.
- Log output records whether SDL controller mapping is available or falls back to XInput.

Requires ROG Ally or XInput hardware:

- `L3 + R3` first-person toggle.
- Right-stick camera turn feel, deadzone, drift, and turn speed.
- D-pad/left-stick camera-relative movement reliability during live roguelike turns.
- Confirm forward/back/strafe directions match the visible camera direction after rotating.
- 720p/1080p readability, sleep/resume, and thermal/idle behavior.

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
