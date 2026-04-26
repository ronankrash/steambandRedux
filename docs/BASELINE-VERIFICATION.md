# Baseline Verification

Last updated: 2026-04-25

## Required Commands

From a Windows developer environment with Visual Studio 2022 and SDL2 installed:

```bash
cmake -S . -B build -DSDL2_DIR=<path-to-sdl2-cmake-config>
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
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

- Clean CMake configure was attempted with `cmake -S . -B build-rescue-verify`.
- Configure failed because SDL2 could not be found by CMake.
- Existing readable evidence shows `build/CMakeCache.txt` has `SDL2_DIR:PATH=SDL2_DIR-NOTFOUND`.
- Because SDL2 is required by `CMakeLists.txt`, build and test execution are blocked until SDL2 is installed and discoverable.

Observed CMake failure:

```text
CMake Error at CMakeLists.txt:8 (find_package):
  Could not find a package configuration file provided by "SDL2" with any of
  the following names:

    SDL2Config.cmake
    sdl2-config.cmake
```

## Pass Criteria

- CMake configure succeeds from a clean build directory.
- `SteambandRedux` builds in Debug.
- `UnityTestRunner` builds and passes.
- `ctest -C Debug --output-on-failure` passes.
- The game launches to the current 2D Windows terminal UI.
- Keyboard basics work: `N`, `O`, arrow/numpad movement when in game, Enter, Escape.
- Controller status is recorded with hardware present or explicitly marked untested.

## Known Test Gaps

- `renderer_render()` is not tested or called by the game loop.
- SDL controller hardware behavior is not covered by automated tests.
- BACK double/triple press timing has no automated coverage.
- `test_util.c` is not part of current CMake test targets.
- `test_sdl2_controller_init()` exists but is not registered in `UnityTestRunner`.
