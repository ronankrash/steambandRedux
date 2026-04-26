# Technical Notes

Last updated: 2026-04-25

## Current Stack

- Language: C, preserving the legacy Steamband/Angband engine.
- Build: CMake with Visual Studio/MSVC on Windows.
- Current UI: Win32 Term/GDI path in `src/main-win.c`.
- Controller: XInput-driven input in `src/controller.c`; SDL controller open is compiled only when SDL2 is available and is not used for polling yet.
- Renderer prototype: SDL2 DDA raycaster in `src/renderer.c`, compiled only when SDL2 is available and not yet driven by the game loop.
- Tests: Unity framework in `third_party/unity/` and `src/tests/`.
- Logging: `src/logging.c`.

## Dependency Notes

`CMakeLists.txt` can configure without SDL2 for the legacy 2D/XInput baseline:

```bash
cmake -S . -B build
```

To build the SDL2 renderer path, install SDL2 and configure with:

```bash
cmake -S . -B build-sdl2 -DCMAKE_TOOLCHAIN_FILE=C:/Users/bkars/vcpkg/scripts/buildsystems/vcpkg.cmake -DSTEAMBAND_ENABLE_SDL2=ON
```

SDL2 is installed locally via vcpkg at `C:/Users/bkars/vcpkg`. Both non-SDL2 and SDL2 Debug builds pass CTest locally.

## Test Targets

- `UnitTests`: older logging-focused runner.
- `UnityTestRunner`: primary Unity runner for infrastructure, logging, z-util, controller, and basic renderer smoke tests.

Known drift:

- SDL2-specific tests are ignored when SDL2 is disabled.
- `test_util.c` is not built.
- Renderer coverage does not exercise `renderer_render()`.

## Architecture Direction

The legacy C engine remains the source of truth for rules, items, maps, saves, and keyboard commands. See `docs/ARCHITECTURE-DECISION.md`.

First-person work should integrate incrementally with tests and a reversible fallback to the current 2D client.

## Security Notes

Legacy C still contains unsafe string and file handling patterns. Any touched code should be audited for bounds checks, null handling, save/pref validation, and clear error logging.
