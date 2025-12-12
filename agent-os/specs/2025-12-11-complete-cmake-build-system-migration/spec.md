# Specification: Complete CMake Build System Migration

## Goal
Fix all compilation errors, ensure successful build on Windows with Visual Studio, and verify the executable can start and load game data files from the `lib/` directory.

## User Stories
- As a developer, I want the project to build successfully with CMake and Visual Studio so that I can compile and test the game
- As a developer, I want the executable to start and load game data correctly so that I can verify the build system migration is complete

## Specific Requirements

**Fix Compilation Errors**
- Resolve all compilation errors preventing successful build with MSVC
- Exclude broken legacy files (like `xxxload1.c`) that reference missing constants
- Ensure all core source files compile without errors
- Suppress MSVC C4996 warnings (unsafe string functions) using `_CRT_SECURE_NO_WARNINGS` definition

**Configure CMake Build System**
- Support Debug and Release build configurations via CMake `CMAKE_BUILD_TYPE`
- Use CMake defaults for compiler optimizations (Debug: symbols, no optimization; Release: optimizations)
- Maintain existing CMakeLists.txt structure with Windows platform detection
- Ensure proper Windows platform macros (`WINDOWS`, `WIN32`, `_WIN32_WINNT=0x0501`)

**Create Placeholder Files**
- Create minimal placeholder files for planned features: `src/controller.c`, `src/logging.c`, `src/steam_integration.c`
- Create test infrastructure placeholders: `src/tests/main_test.c`, `src/tests/test_logging.c`
- Placeholder files should compile but contain minimal stub implementations

**Verify Executable Functionality**
- Ensure executable can start without crashing
- Verify executable can locate and load game data files from `lib/` directory structure
- Confirm `init_file_paths()` and `init_stuff()` functions execute successfully
- Validate that required subdirectories (`lib/data`, `lib/file`, `lib/help`, etc.) are accessible

**Reference Existing Build Configuration**
- Use `src/Makefile.bcc` as reference for determining essential Windows source files
- Compare current CMakeLists.txt source list against Makefile.bcc to ensure completeness
- Include Steamband-specific files (`level.c`, `xxxrandart.c`) identified in current build

**Library Configuration**
- Link required Windows system libraries (kernel32, user32, gdi32, winspool, comdlg32, advapi32, shell32, ole32, oleaut32, uuid, odbc32, odbccp32, winmm)
- Include XInput library (`xinput`) for future controller support
- Configure test executable (`UnitTests`) with appropriate library dependencies

**Build Verification**
- Generate Visual Studio solution files successfully with CMake
- Build both Debug and Release configurations without errors
- Verify executable is created in expected output directory (`build/`)

## Visual Design
No visual assets provided for this specification.

## Existing Code to Leverage

**Makefile.bcc (Borland C++ 5.5 Win32)**
- Reference build configuration showing complete list of Windows source files
- Shows essential files: `main-win.c`, `readdib.c`, core game modules (`birth.c`, `cave.c`, `cmd-*.c`, etc.)
- Demonstrates Windows-specific compiler flags and defines (`-D_WIN32_WINNT=0x0400`, `-DWINVER=0x0400`)
- Provides pattern for resource file handling (`angband.rc`)

**Current CMakeLists.txt**
- Already contains correct file structure and Windows platform detection
- Includes proper C99 standard configuration (`CMAKE_C_STANDARD 99`)
- Has Windows library linking configured correctly
- Contains test target structure ready for expansion

**main-win.c init_stuff() Function**
- Demonstrates how game locates `lib/` directory relative to executable
- Shows path construction pattern: extracts executable path, appends `lib\`, validates directories
- Calls `init_file_paths()` to set up all ANGBAND_DIR_* variables
- Validates required subdirectories and files (e.g., `lib/file/news.txt`)

**variable.c Directory Path Variables**
- Defines all `ANGBAND_DIR_*` constants used throughout codebase
- Shows expected directory structure: `ANGBAND_DIR_DATA`, `ANGBAND_DIR_FILE`, `ANGBAND_DIR_HELP`, etc.
- These variables are initialized by `init_file_paths()` and must be accessible for game to function

## Out of Scope
- Fixing code warnings beyond suppressing CRT security warnings (warnings will be addressed in future work)
- Implementing actual functionality in placeholder files (`controller.c`, `logging.c`, `steam_integration.c` - these are separate roadmap items)
- Creating Windows resource file (`angband.rc`) for icons/menus - can be added later if needed
- Restoring broken legacy files (`xxxload1.c`) - excluded for now, can be revisited later
- Code modernization beyond build system (separate effort, not part of build migration)
- Adding new features or modifying game logic (only build system changes)
- Cross-platform support beyond Windows (Linux/macOS support deferred)
- Steamworks SDK integration (separate roadmap item)
- Controller input implementation (separate roadmap item)
- Comprehensive test suite implementation (only placeholder test infrastructure)

