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

Optional no-hardware first-person keyboard probe:

```powershell
powershell -ExecutionPolicy Bypass -File tools\probe_rog_ally_fp.ps1
```

Optional no-hardware top-down tile keyboard probe:

```powershell
powershell -ExecutionPolicy Bypass -File tools\probe_rog_ally_fp.ps1 -TopDown
```

The default probe launches the SDL2 Debug build, foregrounds the native Win32 window, injects `Ctrl+F12`, waits briefly, exits, and reports whether the new log segment proves renderer init, DDA startup, first-person activation, and clean shutdown. Add `-TopDown` to inject `Ctrl+F11` and assert SDL2 top-down tile activation instead. Add `-TryNewGame` for a separate best-effort title-screen `N` attempt; the visual birth prompts remain manual confirmation territory.

## First Minute Smoke

1. At the legacy prompt, press `N` for a new game or `O` to open an existing save.
2. Confirm the legacy 2D window still accepts keyboard input.
3. Press `Ctrl+F12` to show the SDL first-person prototype window.
4. On ROG Ally hardware, also test `L3 + R3` for first-person toggle.
5. Move in first-person mode:
   - Keyboard: `W`/Up forward, `S`/Down back, `A`/`D` strafe, Left/Right arrows turn.
   - Controller: D-pad/left stick move, right stick turns.
6. Confirm the first-person HUD updates HP/SP/depth/focus/status gauges and the SDL title bar shows exact HP/SP/depth/status text.
7. Double-press `Back` to open the command menu. Confirm the three-column menu stays inside the legacy window, every command has a short category prefix, and the selected command appears as a category/name first-person HUD/title overlay.
8. Walk near visible dungeon features and confirm color-coded first-person markers appear for monsters, objects, stairs, doors, and traps when present.
9. Press controller `B`, `Escape`, or `Ctrl+F12` to return to the 2D fallback when no controller menu is active.
10. Exit the game and review the logs.

## Top-Down Tile Smoke

Use this for the SDL2 custom 2D tile detour.

1. At the legacy prompt, press `N` for a new game or `O` to open an existing save.
2. Press `Ctrl+F11` to open the SDL2 top-down tile prototype.
3. Confirm the legacy Win32/GDI window still exists as the fallback source of truth.
4. Move with keyboard or controller and confirm the SDL2 tile view recenters on the player.
5. Confirm the loaded placeholder tiles or procedural fallback glyphs are distinct for walls, floors, doors, stairs, traps, objects, monsters, player, and darkness when those cells are present.
6. Press `Escape` or `Ctrl+F11` to return to the legacy 2D fallback.

To test a compatible custom tilesheet without replacing the repo placeholder, set `STEAMBAND_TOPDOWN_TILESET` to a 24x24 BMP atlas path before launch. The category order is documented in `ASSETS.md`.

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
- `Ctrl+F11`: toggle SDL2 top-down tile prototype from the keyboard.

## Menu Feel Checks

- Command menu: category prefixes should be readable at handheld scale, D-pad focus should stay obvious, and `B`/`Back` should close without sending a command.
- Config menu: highlighted mapping should be echoed near the top, `A` enters remap mode, `B`/`Back` cancels remap mode, and `B`/`Back` saves and closes from normal mode.

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

## LAN Drop Folder

From the development PC, build and test `build-rescue-sdl2`, then assemble a runnable folder for the Ally with:

```powershell
powershell -ExecutionPolicy Bypass -File tools\package_rog_ally_drop.ps1
```

The package contains `SteambandRedux.exe`, the SDL2 runtime DLL when present, and a full `lib\` tree beside the executable. You can pass `-DestinationPath \\ALLY-SHARE\SteambandRedux` or another local/network path to copy directly to the handheld.

For a tile-first playtest on the development PC, run:

```bat
tools\launch_topdown_tiles.cmd
```

The packaged Ally drop also includes `launch_topdown_tiles.cmd`, which sets `STEAMBAND_START_TOPDOWN=1` and opens the SDL2 tile window automatically.

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
