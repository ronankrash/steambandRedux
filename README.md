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
cmake -S . -B build-sdl2 -DCMAKE_TOOLCHAIN_FILE=C:/Users/bkars/vcpkg/scripts/buildsystems/vcpkg.cmake -DSTEAMBAND_ENABLE_SDL2=ON
cmake --build build-sdl2 --config Debug
```

SDL2 is currently installed locally via vcpkg at `C:/Users/bkars/vcpkg`.

## Running

After a successful Debug build:

```bash
cd build/Debug
./SteambandRedux.exe
```

The current player-facing display is still the legacy Windows terminal/GDI UI.

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
- Back: map/menu gestures
- LB: rest (`R`)
- RB: search (`s`)

Right stick look/turn is planned for first-person mode but is not implemented.

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
