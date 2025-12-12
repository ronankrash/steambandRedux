# Spec Requirements: Complete CMake Build System Migration

## Initial Description

Complete CMake Build System Migration — Fix all compilation errors, ensure successful build on Windows with Visual Studio, and verify all source files compile correctly with modern toolchain.

## Requirements Discussion

### First Round Questions

**Q1:** I assume we need to fix all compilation errors (not just warnings) to get a successful build. Should we treat MSVC warnings (like C4996 for unsafe string functions) as errors, or only fix actual compilation failures?

**Answer:** I am not sure. What do you suggest?

**Suggestion Provided:** Fix compilation errors first. For MSVC C4996 warnings (unsafe string functions), suppress them for now with `#define _CRT_SECURE_NO_WARNINGS` in a common header or CMake, since this is legacy code. We can address warnings later.

**Approved:** Yes, user approved this approach.

**Q2:** I'm thinking we should support both Debug and Release configurations in CMake. Should we configure compiler optimizations differently for each, or use CMake defaults?

**Answer:** I am not sure. What do you suggest?

**Suggestion Provided:** Use CMake defaults. Configure Debug with symbols and no optimization, Release with optimizations. CMake handles this via `CMAKE_BUILD_TYPE`.

**Approved:** Yes, user approved this approach.

**Q3:** I see `xxxload1.c` was removed due to missing artifact constants. Should we exclude other legacy files that reference removed features, or attempt to fix/restore the missing definitions?

**Answer:** I don't know the code base enough to answer. Suggestions?

**Suggestion Provided:** Exclude broken legacy files (like `xxxload1.c`) for now. Focus on getting a clean build with the core files. We can revisit legacy support later if needed.

**Approved:** Yes, user approved this approach.

**Q4:** I notice `CMakeLists.txt` references `controller.c`, `logging.c`, `steam_integration.c`, and test files that may not exist yet. Should we include only existing source files for now, or create placeholder files for planned features?

**Answer:** let's create place holder files for planned features

**Q5:** I see a comment about `src/angband.rc` resource file. Do we need to locate/create this resource file for icons/menus, or can we proceed without it for now?

**Answer:** I don't know honestly

**Suggestion Provided:** Proceed without `angband.rc` for now. The game should run without it (icons/menus are optional). We can add it later if needed.

**Approved:** Yes, user approved this approach.

**Q6:** I'm assuming we should verify the build produces a working executable that can at least start (even if game functionality isn't complete). Should we also verify it can load game data files from the `lib/` directory, or just ensure it compiles and links successfully?

**Answer:** let's be thorough and assure it can load from lib/ directory

**Q7:** For compiler warnings about deprecated functions (like `strcpy`, `sprintf`), should we suppress them with `_CRT_SECURE_NO_WARNINGS`, or leave them as warnings for future cleanup?

**Answer:** Not sure -- suggestions?

**Suggestion Provided:** Suppress CRT security warnings for now using `_CRT_SECURE_NO_WARNINGS` to unblock the build.

**Approved:** Yes, user approved this approach.

**Q8:** Are there any other source files in `src/` that should be included in the build but aren't currently listed in `CMakeLists.txt`?

**Answer:** I don't know. I do know that I was able to get it to run and compile in a docker container a few months ago.

**Follow-up Needed:** Requested information about Docker build configuration.

### Existing Code to Reference

**Similar Features Identified:**
- **Makefile.bcc (Borland C++ 5.5 Win32)**: Reference build configuration showing core Windows source files
  - Path: `src/Makefile.bcc`
  - Files to reference: Shows complete list of Windows build source files (main-win.c, readdib.c, birth.c, cave.c, cmd-*.c, etc.)
  - Note: This is the most modern Borland build file and shows what files are essential for Windows build
- **Makefile.win (Borland C++ 4.5)**: Older Windows build configuration
  - Path: `src/Makefile.win`
  - Note: References `itsybits.c` and `load1.c` which may not be needed
- **Current CMakeLists.txt**: Already includes most correct files
  - Path: `CMakeLists.txt`
  - Note: Comparison shows current list is mostly correct, includes `level.c` and `xxxrandart.c` which may be Steamband-specific additions

### Follow-up Questions

**Follow-up 1:** You mentioned you got it to compile in a Docker container a few months ago. Do you have:
- The Dockerfile or build commands used?
- Notes on which files were included?
- Any build configuration that worked?

This would help confirm the exact file list and any special build settings needed.

**Answer:** Unfortunately I don't have the old docker config -- it was on a different computer I no longer have. The suggestions otherwise mentioned look good to me!

## Visual Assets

### Files Provided:
No visual assets provided.

### Visual Insights:
- No visual assets to analyze
- User mentioned screenshots exist online but none provided yet

## Requirements Summary

### Functional Requirements
- Fix all compilation errors to achieve successful build with Visual Studio
- Suppress MSVC C4996 warnings (unsafe string functions) using `_CRT_SECURE_NO_WARNINGS`
- Support both Debug and Release build configurations via CMake
- Create placeholder files for planned features (`controller.c`, `logging.c`, `steam_integration.c`, test files)
- Exclude broken legacy files (like `xxxload1.c`) from build
- Verify executable can start and load game data files from `lib/` directory
- Ensure all core source files compile correctly with modern toolchain (MSVC/Visual Studio 2022)

### Reusability Opportunities
- **Makefile.bcc**: Reference for determining essential Windows source files
- **Current CMakeLists.txt**: Already has correct file structure, needs error fixes
- **Existing build configurations**: Multiple makefiles show file organization patterns

### Scope Boundaries
**In Scope:**
- Fixing compilation errors in existing source files
- Configuring CMake for Debug and Release builds
- Creating minimal placeholder files for planned features
- Suppressing CRT security warnings to unblock build
- Verifying executable can start and load data from `lib/` directory
- Ensuring successful build with Visual Studio 2022

**Out of Scope:**
- Fixing code warnings (only suppressing for now)
- Implementing functionality in placeholder files (those are separate roadmap items)
- Creating resource file (`angband.rc`) - can be added later
- Restoring broken legacy files (`xxxload1.c`) - excluded for now
- Code modernization beyond build system (separate effort)

### Technical Considerations
- **Build System**: CMake 3.10+ with Visual Studio 2022 generator
- **Compiler**: MSVC (Microsoft Visual C++)
- **C Standard**: C99 (`CMAKE_C_STANDARD 99`)
- **Platform Macros**: `WINDOWS`, `WIN32`, `_WIN32_WINNT=0x0501`
- **Warning Suppression**: `_CRT_SECURE_NO_WARNINGS` to handle legacy unsafe string functions
- **Reference Build**: Makefile.bcc shows working Windows build file list
- **Docker Build Info**: User had working Docker build previously but no longer has access to that configuration
- **File Comparison**: Current CMakeLists.txt mostly matches Makefile.bcc, includes `level.c` and `xxxrandart.c` which may be Steamband-specific
- **Library Dependencies**: Windows system libraries (kernel32, user32, gdi32, etc.) and xinput already configured
- **Test Target**: UnitTests executable already configured in CMakeLists.txt

