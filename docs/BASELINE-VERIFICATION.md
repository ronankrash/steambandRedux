# Baseline Verification

Last updated: 2026-04-25

## Required Commands

Baseline without SDL2:

```bash
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

Renderer build with SDL2:

```bash
cmake -S . -B build-sdl2 -DSDL2_DIR=<path-to-sdl2-cmake-config>
cmake --build build-sdl2 --config Debug
ctest --test-dir build-sdl2 -C Debug --output-on-failure
```

Direct Unity runner, after a successful Debug build:

```bash
build/Debug/UnityTestRunner.exe
```

Manual smoke test:

```bash
cd build/Debug
./SteambandRedux.exe
```

## Current Verification Result

- Clean non-SDL2 configure succeeds with `cmake -S . -B build-rescue-nosdl`.
- Debug build succeeds with `cmake --build build-rescue-nosdl --config Debug`.
- CTest succeeds with `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure`.
- The build emits legacy MSVC warnings, mostly narrowing/sign conversion warnings in older game code.
- Existing readable evidence shows `build/CMakeCache.txt` has `SDL2_DIR:PATH=SDL2_DIR-NOTFOUND`.
- SDL2 renderer build/test execution remains blocked until SDL2 is installed and discoverable.
- Manual game launch was not run in this pass.

Previous SDL2-required CMake failure, now addressed for baseline builds:

```text
CMake Error at CMakeLists.txt:8 (find_package):
  Could not find a package configuration file provided by "SDL2" with any of
  the following names:

    SDL2Config.cmake
    sdl2-config.cmake
```

## Baseline Pass Criteria

- CMake configure succeeds from a clean build directory. Status: passed for non-SDL2 baseline.
- `SteambandRedux` builds in Debug. Status: passed for non-SDL2 baseline.
- `UnityTestRunner` builds and passes. Status: passed through CTest.
- `ctest -C Debug --output-on-failure` passes. Status: passed.
- The game launches to the current 2D Windows terminal UI.
- Keyboard basics work: `N`, `O`, arrow/numpad movement when in game, Enter, Escape.
- Controller status is recorded with hardware present or explicitly marked untested.

## Known Test Gaps

- `renderer_render()` is not tested or called by the game loop.
- SDL controller hardware behavior is not covered by automated tests.
- BACK double/triple press timing has no automated coverage.
- `test_util.c` is not part of current CMake test targets.
- SDL2 renderer/controller smoke tests are ignored when SDL2 is disabled.
