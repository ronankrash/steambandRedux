# SteambandRedux

SteambandRedux is a rescue and modernization project for Steamband 0.2.2, an Angband-derived steampunk roguelike.

The product goal is a controller-first, first-person steampunk dungeon crawler for modern Windows handheld PCs, especially the Asus ROG Ally. The engineering goal is to preserve the original game rules, items, map generation, saves, and keyboard controls while adding modern rendering, input, testing, and release practices.

## Current Status

This repository is in rescue-baseline mode.

Verified from source inspection:

- The legacy C codebase remains the gameplay source of truth.
- CMake, logging, Unity tests, XInput controller code, controller menus, and a basic SDL2 renderer prototype exist.
- `src/renderer.c` contains a DDA-style SDL2 raycaster prototype.
- SDL2 builds initialize, pulse, and shut down the renderer from the Win32 event loop, but first-person mode remains a prototype rather than a finished playable feature.
- SDL2 is optional for the baseline build and installed locally via vcpkg for renderer builds.
- Licensing needs review before Steam distribution or any paid release because legacy source headers include not-for-profit language.

See:

- `HANDOFF.md`
- `docs/BASELINE-AUDIT.md`
- `docs/BASELINE-VERIFICATION.md`
- `docs/LICENSING-RISK-MANIFEST.md`
- `docs/PLAYTEST-CHECKLIST.md`
- `docs/ROADMAP.md`
- `docs/TECHNICAL.md`
- `docs/ARCHITECTURE-DECISION.md`

## Build Requirements

- Windows 10/11
- Visual Studio 2022 or compatible MSVC toolchain
- CMake 3.10 or newer
- SDL2 development package discoverable by CMake for first-person renderer builds
- Git

Baseline configure/build without SDL2:

```bash
cmake -S . -B build
cmake --build build --config Debug
```

SDL2 renderer configure/build:

```bash
cmake -S . -B build -DSDL2_DIR=C:/Users/bkars/vcpkg/installed/x64-windows/share/sdl2 -DSTEAMBAND_ENABLE_SDL2=ON
cmake --build build --config Debug
```

SDL2 is currently installed locally via vcpkg at `C:/Users/bkars/vcpkg`. The build copies the SDL2 runtime DLL next to each Debug executable automatically.

## Running

After a successful Debug build:

```bash
cd build/Debug
./SteambandRedux.exe
```

For a quick Asus ROG Ally / first-person smoke test from the repository root:

```bat
tools\launch_rog_ally_fp_smoke.cmd
```

The legacy Windows terminal/GDI UI remains available. Toggle the SDL first-person
view with `Ctrl+F12` or `L3 + R3`; in the SDL window use `W`/Up forward,
`S`/Down back, `A`/`D` strafe, Left/Right arrows or right stick to turn, and
`Escape` to return to the 2D UI.

The SDL2 build also includes a no-asset top-down tile prototype for the custom
2D tileset detour. Toggle it with `Ctrl+F11`; it mirrors the legacy cave state
through a project-generated placeholder BMP or procedural pencil-like fallback
glyphs. Compatible local tilesheet experiments can either set
`STEAMBAND_TOPDOWN_TILESET` or place a `topdown_tileset.bmp` file under
`lib/user/`. Bundled external art still requires documentation in `ASSETS.md`.

Custom top-down sheets must target `topdown-v1`: a 216x96 BMP with 24x24 cells,
9 columns, and 4 rows. Wrong-size sheets are rejected and the renderer falls
back safely to the project placeholder or procedural tiles.

Regenerate the checked-in placeholder atlas with:

```bash
python tools/generate_topdown_tilesheet.py
```

Review current coverage against legacy terrain, object, and monster data with:

```bash
python tools/report_topdown_asset_coverage.py
```

To launch directly into the SDL2 top-down tile view from the SDL2 Debug build:

```bat
tools\launch_topdown_tiles.cmd
```

The launcher targets `build-rescue-sdl2\Debug\SteambandRedux.exe`, prints the
current controls, and points to the SDL build logs. See
`docs\ROG-ALLY-FP-MANUAL-TEST.md` for the focused manual test guide.

To assemble a lightweight LAN drop folder for the Asus ROG Ally after building:

```powershell
powershell -ExecutionPolicy Bypass -File tools\package_rog_ally_drop.ps1
```

Pass `-DestinationPath \\ALLY-SHARE\SteambandRedux` or another destination to
copy the executable, SDL2 runtime DLL, and full `lib\` data tree directly there.

## Testing

After a successful build:

```bash
ctest --test-dir build -C Debug --output-on-failure
```

Primary Unity runner:

```bash
build/Debug/UnityTestRunner.exe
```

Current known test drift:

- `UnitTests` is an older logging-focused runner.
- `UnityTestRunner` is the primary Unity runner.
- SDL2-specific smoke tests are ignored when SDL2 is disabled.
- `test_util.c` is present but not built.
- Renderer tests do not cover live rendering or game-loop integration.

Latest baseline check: both the non-SDL2 Debug build/CTest (`build-rescue-nosdl`) and SDL2 Debug build/CTest (`build-rescue-sdl2`) pass locally. Manual gameplay/controller smoke testing is still pending.

## Controls

Keyboard support must remain compatible with original Steamband/Angband commands.

Current controller implementation is primarily XInput:

- A: Enter/confirm
- B: Escape/cancel
- X: inventory (`i`)
- Y: equipment (`e`)
- D-pad: cardinal movement
- Left stick: 8-way numpad-style movement
- Start: Escape
- Back: map after the gesture window, double Back opens the command menu, triple Back opens button configuration
- LB: rest (`R`)
- RB: search (`s`)

SDL2 first-person prototype:

- `Ctrl+F12`: show/hide the SDL first-person prototype window.
- `L3 + R3`: controller shortcut to show/hide the SDL first-person prototype window.
- `W`/Up: move forward relative to the camera.
- `S`/Down: move backward relative to the camera.
- `A`/`D`: strafe left/right relative to the camera.
- Left/Right arrows or right stick: turn camera while first-person mode is active.
- `Escape` or `Ctrl+F12` in the SDL window exits first-person mode.
- Other keyboard commands in the SDL window are forwarded to the legacy game input queue.

SDL2 top-down tile prototype:

- `Ctrl+F11`: show/hide the SDL2 tile window.
- `Escape` while the SDL window is focused exits back to the legacy 2D UI.
- Keyboard commands in the SDL tile window are forwarded to the legacy game input queue.

## Repository Process

This project uses Cursor rules and skills plus a handoff document for agent-assisted work:

- `.cursor/rules/steamband-core.mdc`
- `.cursor/rules/git-workflow.mdc`
- `.cursor/rules/security-legacy-c.mdc`
- `.cursor/rules/asset-licensing.mdc`
- `.cursor/rules/controller-rog-ally.mdc`
- `.cursor/skills/`
- `HANDOFF.md`

Development rules:

- Use feature branches and small conventional commits.
- Do not commit generated build outputs, logs, local saves, or dependency caches.
- Add tests for meaningful behavior changes.
- Update `HANDOFF.md` after significant work.
- Keep docs factual and tied to source or command output.

See `docs/REPO-HYGIENE.md`.

## Licensing

The original Steamband/Angband-derived code and data retain their original notices. See `readme.txt` and source file headers.

Important: legacy source headers include educational/research/not-for-profit language. Steam distribution and any paid release require license clarification before proceeding.

Third-party and project license tracking starts in `LICENSES.md`.

Future art/audio assets must be documented in `ASSETS.md` before use. Only CC0, Public Domain, MIT, or clearly commercial-permissive assets are acceptable.

## Original Credits

Steamband 0.2.2 by Courtney C. Campbell, based on Moria, Umoria, and Angband.

See `readme.txt` for the original project readme and credits.
