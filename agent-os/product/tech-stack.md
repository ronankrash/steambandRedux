# Tech Stack

This document defines the technical stack for SteambandRedux. This serves as a reference for all team members and helps maintain consistency across the project.

## Language & Runtime

- **Language:** C (C99 standard)
- **Legacy Compatibility:** Maintaining compatibility with existing 1990s-era Angband codebase
- **Platform:** Windows (Windows 10/11, targeting Windows SDK 10.0+)

## Build System & Compilation

- **Build System:** CMake (minimum version 3.10)
- **Primary Compiler:** Microsoft Visual C++ (MSVC) via Visual Studio 2022
- **C Standard:** C99 (`CMAKE_C_STANDARD 99`)
- **Build Configuration:** Debug and Release configurations supported
- **Platform Macros:** `WINDOWS`, `WIN32`, `_WIN32_WINNT=0x0501`

## Core Libraries & APIs

### Windows Platform APIs
- **Windows API:** Native Windows functionality (kernel32, user32, gdi32, winspool, comdlg32, advapi32, shell32, ole32, oleaut32, uuid, odbc32, odbccp32)
- **Windows Multimedia:** winmm (Windows Multimedia API)
- **XInput:** xinput (Xbox 360 controller support via Windows SDK)

### Third-Party SDKs
- **Steamworks SDK:** Steam platform integration (Steam API, achievements, overlay, cloud saves)
  - Version: Latest stable Steamworks SDK
  - Integration: Native C/C++ API

## Graphics & Display

- **Display System:** Windows GDI (Graphics Device Interface)
- **Window Management:** Native Windows windowing (`main-win.c`)
- **Graphics Format:** DIB (Device Independent Bitmap) support via `readdib.c`
- **Font Rendering:** Windows font system (`.fon` files)

## Input Systems

- **Keyboard Input:** Windows message loop (`WM_KEYDOWN`, `WM_CHAR`)
- **Controller Input:** XInput API for Xbox 360 controllers
- **Mouse Input:** Windows message handling (optional, for screensaver mode)

## File I/O & Storage

- **File System:** Windows file APIs (`CreateFile`, `ReadFile`, `WriteFile`)
- **Save Files:** Binary format with versioning
- **Configuration Files:** ASCII text preference files (`.prf` files)
- **Data Files:** Binary template files (`.raw` files) generated from ASCII templates

## Testing & Quality Assurance

- **Test Framework:** Custom lightweight unit testing framework (to be finalized - considering Unity, minunit, or custom)
- **Test Structure:** Separate test executable (`UnitTests` target in CMake)
- **Test Coverage:** Unit tests for logging, controller input, Steam integration, and core utilities
- **Build Integration:** Tests compiled alongside main application

## Logging & Debugging

- **Logging System:** Custom logging module (`logging.c`)
- **Log Levels:** DEBUG, INFO, WARNING, ERROR
- **Log Output:** File-based logging with rotation, debug console output
- **Debug Tools:** Visual Studio debugger, Windows debug console

## Development Tools

- **IDE:** Visual Studio 2022 (recommended), or any CMake-compatible IDE
- **Version Control:** Git
- **Code Analysis:** Visual Studio static analysis, compiler warnings
- **Documentation:** Inline code comments, markdown documentation files

## Platform-Specific Considerations

### Windows
- **Target Platform:** Windows 10/11 (64-bit)
- **Windows SDK:** 10.0.22621.0+ (as detected by CMake)
- **Entry Point:** `WinMain` (Windows application)
- **Window Class:** Custom window class (`AngbandWndProc`)
- **Resource Files:** Windows resource files (`.rc`) for icons and menus (if needed)

### Future Platform Support
- **Linux:** Not in initial scope, but codebase has `main-gcu.c` for potential future support
- **macOS:** Not in initial scope, but codebase has `main-mac.c` for potential future support

## Dependencies & External Libraries

### Required Libraries (Windows SDK)
- All Windows system libraries are linked via CMake (`target_link_libraries`)
- XInput library (part of Windows SDK, linked as `xinput`)

### External SDKs
- **Steamworks SDK:** 
  - Location: To be configured in CMake (typically `third_party/steamworks/`)
  - Required Components: Steam API, Steam Client API
  - Platform: Windows

## Build Artifacts

- **Executable:** `SteambandRedux.exe` (Windows application)
- **Test Executable:** `UnitTests.exe` (test runner)
- **Build Directory:** `build/` (CMake output directory)
- **Intermediate Files:** Object files (`.obj`) compiled by MSVC

## Code Organization

- **Source Directory:** `src/`
- **Header Files:** Co-located with source files in `src/`
- **Main Entry Point:** `src/main-win.c` (Windows platform)
- **Core Game Logic:** Platform-agnostic modules in `src/`
- **Platform Code:** `src/main-win.c` (Windows-specific implementation)
- **Test Files:** `src/tests/` directory

## Configuration & Preferences

- **User Preferences:** ASCII text files in `lib/user/` and `lib/pref/`
- **Game Data:** Binary files in `lib/data/` (generated from ASCII templates)
- **Help Files:** ASCII text files in `lib/help/`
- **Graphics Data:** Bitmap files (`.bmp`) and font files (`.fon`) in `lib/xtra/`

## Deployment & Distribution

- **Target Platform:** Steam (Steamworks SDK integration)
- **Distribution Format:** Windows executable (`.exe`)
- **Steam Integration:** Full Steamworks SDK features (achievements, overlay, cloud saves)
- **Build Configuration:** Release builds optimized for distribution

## Notes

- This is a legacy codebase being modernized, so some older patterns may remain
- The codebase maintains compatibility with existing save files and data formats
- CMake migration is in progress, replacing the old Borland C++ 4.5 build system
- Testing framework selection is pending final decision based on project needs

