# ROG Ally First-Person Manual Test

Last updated: 2026-04-25

Use this guide to launch the current SDL2 first-person prototype on an Asus ROG Ally or another XInput-compatible Windows handheld. This is a local smoke harness for playtesting only; first-person mode is still a prototype layered beside the legacy Win32 game UI.

## Launch

From the repository root:

```bat
tools\launch_rog_ally_fp_smoke.cmd
```

The launcher uses the verified local SDL2 Debug build:

```text
build-rescue-sdl2\Debug\SteambandRedux.exe
```

It expects `SDL2d.dll` next to the executable and prints the log paths before launch. If the executable is missing, build it with:

```bash
cmake -S . -B build-rescue-sdl2 -DCMAKE_TOOLCHAIN_FILE=C:/Users/bkars/vcpkg/scripts/buildsystems/vcpkg.cmake -DSTEAMBAND_ENABLE_SDL2=ON
cmake --build build-rescue-sdl2 --config Debug
```

Optional automated check before manual play:

```bash
ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure
```

## First Minute Smoke

1. At the legacy prompt, press `N` for a new game or `O` to open an existing save.
2. Confirm the legacy 2D window still accepts keyboard input.
3. Press `Ctrl+F12` to show the SDL first-person prototype window.
4. On ROG Ally hardware, also test `L3 + R3` for first-person toggle.
5. Move in first-person mode:
   - Keyboard: `W`/Up forward, `S`/Down back, `A`/`D` strafe, Left/Right arrows turn.
   - Controller: D-pad/left stick move, right stick turns.
6. Confirm the first-person HUD updates HP/SP/depth/focus/status gauges and the SDL title bar shows exact HP/SP/depth/status text.
7. Double-press `Back` to open the command menu and confirm the selected command appears as a short first-person HUD/title overlay.
8. Press `Escape` or `Ctrl+F12` while the SDL window is focused to return to the 2D fallback.
9. Exit the game and review the logs.

## Controller Controls

- `A`: Enter/confirm.
- `B`: Escape/cancel.
- `X`: inventory (`i`).
- `Y`: equipment (`e`).
- D-pad: cardinal movement in 2D; camera-relative movement in first-person mode.
- Left stick: 8-way movement in 2D; camera-relative movement in first-person mode.
- Right stick: turns the first-person camera while first-person mode is active.
- `Start`: Escape.
- `Back`: single press opens map after the gesture window, double press opens the command menu, triple press opens button configuration.
- `LB`: rest (`R`).
- `RB`: search (`s`).
- `L3 + R3`: toggle first-person prototype.

## Logs To Check

Primary game log:

```text
build-rescue-sdl2\Debug\lib\logs\steamband.log
```

Window/debug log:

```text
build-rescue-sdl2\Debug\lib\logs\window_debug.log
```

Useful log evidence:

```text
Controller init line:
Renderer init line:
DDA startup line:
First-person activation line:
First-person exit/fallback line:
```

Known acceptable notes in the current local logs:

- Missing `8X13.FON` warnings are expected; the game falls back to larger system fixed-pitch fonts.
- If no controller is attached, the controller line reports no controller detected.
- Some controllers may report an SDL mapping warning and fall back to XInput.

## Record Results

```text
Build:
Hardware:
Display mode:
Power profile:
Controller mode:
Save file used:
Result: PASS/FAIL/BLOCKED
Notes:
Log excerpt/path:
```

Follow the broader checklist in `docs\PLAYTEST-CHECKLIST.md` after this quick smoke passes.
