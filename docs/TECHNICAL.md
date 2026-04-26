# Technical Notes

Last updated: 2026-04-25

## Current Stack

- Language: C, preserving the legacy Steamband/Angband engine.
- Build: CMake with Visual Studio/MSVC on Windows.
- Current UI: Win32 Term/GDI path in `src/main-win.c`.
- Controller: XInput-driven input in `src/controller.c`; SDL controller open is present but not used for polling.
- Renderer prototype: SDL2 DDA raycaster in `src/renderer.c`, not yet driven by the game loop.
- Tests: Unity framework in `third_party/unity/` and `src/tests/`.
- Logging: `src/logging.c`.

## Dependency Notes

`CMakeLists.txt` currently requires SDL2:

```bash
cmake -S . -B build -DSDL2_DIR=<path-to-sdl2-cmake-config>
```

The existing build cache records `SDL2_DIR:PATH=SDL2_DIR-NOTFOUND`, so local SDL2 setup must be fixed before build claims are made.

## Test Targets

- `UnitTests`: older logging-focused runner.
- `UnityTestRunner`: primary Unity runner for infrastructure, logging, z-util, controller, and basic renderer smoke tests.

Known drift:

- `test_sdl2_controller_init()` is not registered.
- `test_util.c` is not built.
- Renderer coverage does not exercise `renderer_render()`.

## Architecture Direction

The legacy C engine remains the source of truth for rules, items, maps, saves, and keyboard commands. See `docs/ARCHITECTURE-DECISION.md`.

First-person work should integrate incrementally with tests and a reversible fallback to the current 2D client.

## Security Notes

Legacy C still contains unsafe string and file handling patterns. Any touched code should be audited for bounds checks, null handling, save/pref validation, and clear error logging.
