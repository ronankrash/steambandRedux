# Raw Feature Idea

## Feature Description

Complete CMake Build System Migration — Fix all compilation errors, ensure successful build on Windows with Visual Studio, and verify all source files compile correctly with modern toolchain.

## Source

From product roadmap item #1.

## Effort Estimate

Medium (M) - 1 week

## Notes

This is the foundational feature that must be completed before other features can proceed. The codebase is currently being migrated from Borland C++ 4.5 to CMake with Visual Studio. There are compilation errors that need to be resolved, particularly related to missing artifact constants in `xxxload1.c` (which has been removed from the build) and other legacy code issues.

