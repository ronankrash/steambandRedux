# AGENTS.md

## Cursor Cloud specific instructions

SteambandRedux is a **Windows-only** roguelike (an Angband/Steamband variant). Its
native build targets MSVC + the Win32 GUI API (`src/main-win.c`), XInput, and GDI.
There is no MSVC or real Windows on the Linux cloud VM, so the development workflow
here is to **cross-compile for Windows with MinGW-w64 and run the resulting `.exe`
files under Wine**.

### What the VM already has (installed in the snapshot)
- `mingw-w64` (provides `x86_64-w64-mingw32-gcc` and the Windows import libs:
  xinput, winmm, ole32, comdlg32, odbc32, etc.)
- `wine` / `wine64` + `xvfb` (to run the Windows executables; a desktop X server
  is available on `DISPLAY=:1`)
- `libncurses-dev` (only relevant to the unused non-Windows CMake branch)
- The startup update script runs `git submodule update --init --recursive`, which
  populates `third_party/unity` (the vendored Unity test framework, referenced as a
  git submodule via `.gitmodules`). The test targets will not configure without it.

### Build (cross-compile to Windows)
Always build out-of-source into `build-mingw/` using the provided toolchain file.
The checked-in `build/` directory holds stale MSVC project files from Windows — do
not reuse it on Linux.

```bash
cmake -S . -B build-mingw -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw64.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build-mingw -j4
```

Produces `build-mingw/SteambandRedux.exe` (GUI), `build-mingw/UnitTests.exe`, and
`build-mingw/UnityTestRunner.exe` (both console). A harmless `"STRICT" redefined`
warning from `main-win.c` is expected.

### Test
The toolchain file wires Wine in as the ctest cross-compiling emulator, so:

```bash
export WINEPREFIX=$HOME/.wine-steamband WINEDEBUG=-all
cd build-mingw && ctest --output-on-failure        # runs both suites via wine
```

Or run a suite directly: `wine build-mingw/UnityTestRunner.exe` (38 tests) /
`wine build-mingw/UnitTests.exe` (11 tests).

### Run the game (GUI, under Wine)
```bash
export WINEPREFIX=$HOME/.wine-steamband WINEDEBUG=-all DISPLAY=:1
cd build-mingw && wine SteambandRedux.exe
```
Start a new game with the `N` key (or the File → New menu), then proceed through the
Angband character creation prompts (gender/race/class/stats/name) to enter the Town.

### Non-obvious gotchas
- **Run from `build-mingw/`.** The game finds the game-data `lib/` directory by
  walking *up* from the executable's own path looking for `lib/apex` (so it resolves
  to the repo-root `/workspace/lib`). Separately, the logger writes to
  `lib/logs/steamband.log` **relative to the current working directory**, so logs
  land in `build-mingw/lib/logs/` — not the repo-root `lib/`.
- **MinGW SEH shim:** `main-win.c` uses MSVC `__try`/`__except`. GCC/MinGW does not
  support these, so a `#if !defined(_MSC_VER)` pass-through shim near the top of
  `main-win.c` maps them to plain blocks. The MSVC build is unaffected.
- **Fonts:** `lib/xtra/font/8X13.FON` is absent, so the game logs a warning and falls
  back to a system fixed-pitch font. Title text, the status panel, and messages
  render legibly, but under Wine the ASCII map glyphs can show as box/replacement
  characters. This is a pre-existing rendering quirk (see the repo's last commit
  message), not a build/setup problem — the engine, input, and commands all work.
- A Wine prefix at `$HOME/.wine-steamband` is initialized in the snapshot; recreate
  it with `WINEPREFIX=$HOME/.wine-steamband wineboot --init` if missing.
