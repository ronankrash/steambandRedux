# Task Breakdown: Complete CMake Build System Migration

## Overview
Total Tasks: 5 task groups, 20+ sub-tasks

## Task List

### Build System Configuration

#### Task Group 1: CMake Configuration and Compiler Setup
**Dependencies:** None

- [x] 1.0 Configure CMake build system
  - [x] 1.1 Verify CMakeLists.txt structure and Windows platform detection
    - Check that `if(WIN32)` block correctly detects Windows platform
    - Verify `CMAKE_C_STANDARD 99` is set
    - Confirm Windows platform macros are defined (`WINDOWS`, `WIN32`, `_WIN32_WINNT=0x0501`)
  - [x] 1.2 Add `_CRT_SECURE_NO_WARNINGS` definition to suppress MSVC C4996 warnings
    - Add `add_definitions(-D_CRT_SECURE_NO_WARNINGS)` for MSVC compiler
    - Apply only to Windows/MSVC builds, not other platforms
  - [x] 1.3 Configure Debug and Release build types
    - Ensure CMake defaults are used (Debug: symbols, no optimization; Release: optimizations)
    - Verify `CMAKE_BUILD_TYPE` can be set to Debug or Release
    - Test that both configurations can be generated
  - [x] 1.4 Verify library linking configuration
    - Confirm all Windows system libraries are listed (kernel32, user32, gdi32, winspool, comdlg32, advapi32, shell32, ole32, oleaut32, uuid, odbc32, odbccp32, winmm)
    - Verify XInput library (`xinput`) is included
    - Check that libraries are linked to both main executable and test executable

**Acceptance Criteria:**
- CMakeLists.txt correctly detects Windows platform
- CRT security warnings are suppressed for MSVC builds
- Both Debug and Release configurations can be generated
- All required libraries are properly linked

### Source File Management

#### Task Group 2: Source File List Verification and Cleanup
**Dependencies:** Task Group 1

- [x] 2.0 Verify and update source file list
  - [x] 2.1 Compare CMakeLists.txt source list against Makefile.bcc reference
    - Read `src/Makefile.bcc` to get authoritative Windows source file list
    - Compare each file in Makefile.bcc against CMakeLists.txt SOURCES list
    - Identify any missing files that should be included
    - Identify any files in CMakeLists.txt that shouldn't be there
  - [x] 2.2 Exclude broken legacy files from build
    - Remove `xxxload1.c` from SOURCES list (or ensure it's not included)
    - Document why it's excluded (missing artifact constants)
    - Add comment in CMakeLists.txt explaining exclusion
  - [x] 2.3 Verify Steamband-specific files are included
    - Confirm `level.c` is in source list (Steamband-specific)
    - Confirm `xxxrandart.c` is in source list (Steamband-specific)
    - Ensure these files are present in the source directory
  - [x] 2.4 Verify Windows-specific files are included
    - Confirm `src/main-win.c` is included for Windows builds
    - Confirm `src/readdib.c` is included (Windows DIB support)
    - Ensure platform-specific files are only included for correct platform

**Acceptance Criteria:**
- Source file list matches Makefile.bcc reference (minus excluded legacy files)
- Broken legacy files are excluded from build
- Steamband-specific files are included
- Windows-specific files are correctly conditionally included

### Placeholder File Creation

#### Task Group 3: Create Placeholder Files for Planned Features
**Dependencies:** Task Group 2

- [x] 3.0 Create minimal placeholder files
  - [x] 3.1 Create `src/controller.c` placeholder
    - Create file with minimal stub implementation
    - Include necessary headers to compile without errors
    - Add comment indicating this is a placeholder for future XInput implementation
    - Ensure it compiles without warnings or errors
  - [x] 3.2 Create `src/logging.c` placeholder
    - Create file with minimal stub implementation
    - Include necessary headers to compile without errors
    - Add comment indicating this is a placeholder for future logging system
    - Ensure it compiles without warnings or errors
  - [x] 3.3 Create `src/steam_integration.c` placeholder
    - Create file with minimal stub implementation
    - Include necessary headers to compile without errors
    - Add comment indicating this is a placeholder for future Steamworks SDK integration
    - Ensure it compiles without warnings or errors
  - [x] 3.4 Create test infrastructure placeholders
    - Create `src/tests/` directory if it doesn't exist
    - Create `src/tests/main_test.c` with minimal test runner stub
    - Create `src/tests/test_logging.c` with minimal test stub
    - Ensure test files compile without errors
    - Verify UnitTests executable target builds successfully

**Acceptance Criteria:**
- All placeholder files exist and compile without errors
- Placeholder files contain minimal stub implementations
- Test infrastructure placeholders are created
- UnitTests executable builds successfully

### Compilation Fixes

#### Task Group 4: Fix Compilation Errors
**Dependencies:** Task Groups 1, 2, 3

- [x] 4.0 Resolve all compilation errors
  - [x] 4.1 Generate Visual Studio solution files
    - Run `cmake -S . -B build -G "Visual Studio 17 2022"` (or appropriate generator)
    - Verify solution files are generated successfully
    - Check for any CMake configuration errors
  - [x] 4.2 Attempt initial build and capture errors
    - Build Debug configuration: `cmake --build build --config Debug`
    - Capture all compilation errors
    - Document each error with file name and line number
  - [x] 4.3 Fix compilation errors systematically
    - Address errors one file at a time
    - Fix missing includes, undefined symbols, type mismatches
    - Ensure fixes don't break other files
    - Rebuild after each fix to verify progress
  - [x] 4.4 Verify Release build compiles
    - Build Release configuration: `cmake --build build --config Release`
    - Ensure no compilation errors in Release mode
    - Verify optimizations are applied correctly

**Acceptance Criteria:**
- Visual Studio solution files generate successfully
- Debug configuration builds without compilation errors
- Release configuration builds without compilation errors
- All source files compile correctly with MSVC

### Build Verification

#### Task Group 5: Verify Executable Functionality
**Dependencies:** Task Groups 1-4

- [x] 5.0 Verify executable can start and load game data
  - [x] 5.1 Verify executable is created
    - Check that `SteambandRedux.exe` exists in build output directory
    - Verify executable has correct architecture (x64 or x86 as configured)
    - Check file size is reasonable (not empty or corrupted)
  - [x] 5.2 Test executable startup
    - Run executable from build directory
    - Verify it starts without immediate crash
    - Check for any runtime errors or missing DLL errors
  - [x] 5.3 Verify lib/ directory detection
    - Ensure `lib/` directory exists relative to executable location
    - Verify executable can locate `lib/` directory (check `init_stuff()` execution)
    - Confirm path construction works correctly (executable path + `lib\`)
  - [x] 5.4 Verify data file loading
    - Check that `init_file_paths()` executes successfully
    - Verify required subdirectories are accessible: `lib/data`, `lib/file`, `lib/help`, `lib/info`, `lib/pref`, `lib/save`, `lib/user`, `lib/xtra`
    - Confirm `lib/file/news.txt` can be found and validated
    - Test that ANGBAND_DIR_* variables are initialized correctly
  - [x] 5.5 Document build verification results
    - Document successful build configuration
    - Note any warnings that remain (but don't block build)
    - Create brief summary of what was verified

**Acceptance Criteria:**
- Executable is created successfully in build directory
- Executable starts without crashing
- Executable can locate and access `lib/` directory
- Required subdirectories are accessible
- `init_file_paths()` and `init_stuff()` execute successfully
- Game data files can be loaded from `lib/` directory structure

## Execution Order

Recommended implementation sequence:
1. Build System Configuration (Task Group 1) - Foundation for everything else
2. Source File Management (Task Group 2) - Ensure correct files are included
3. Placeholder File Creation (Task Group 3) - Create missing files needed for build
4. Compilation Fixes (Task Group 4) - Fix errors to achieve successful build
5. Build Verification (Task Group 5) - Verify executable works correctly

## Notes

- This is a build system migration task, not a feature implementation task
- Focus is on getting a clean build, not on implementing game features
- Placeholder files should be minimal stubs that compile but don't implement functionality
- Testing is limited to build verification and executable startup, not game functionality testing
- Legacy files (`xxxload1.c`) are intentionally excluded and can be revisited later

