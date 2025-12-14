# Product Roadmap

1. [x] Complete CMake Build System Migration — Fix all compilation errors, ensure successful build on Windows with Visual Studio, and verify all source files compile correctly with modern toolchain `M`
2. [x] Implement Comprehensive Logging System — Create logging module with DEBUG/INFO/WARNING/ERROR levels, file output with rotation, and debug console output for development troubleshooting `M`
3. [x] Set Up Unit Testing Framework — Integrate testing framework (Unity/minunit), create test infrastructure, and write initial tests for logging system and core utilities `M`
4. [x] Integrate XInput API — Add XInput library linking, implement controller detection, and create controller state polling mechanism in Windows message loop `M`
5. [ ] Implement Controller Input Mapping — Map Xbox 360 controller buttons/triggers to game commands (movement, actions, menu navigation) and integrate with existing input system `L`
6. [ ] Add Controller Configuration — Create in-game controller mapping configuration allowing players to customize button assignments and save preferences `M`
7. [ ] Integrate Steamworks SDK — Add Steamworks SDK to build system, implement Steam API initialization in WinMain, and set up Steam callback handling `M`
8. [ ] Implement Steam Overlay Support — Enable Steam overlay functionality, ensure overlay works during gameplay, and handle overlay activation/deactivation events `S`
9. [ ] Create Steam Achievements System — Define achievement list, implement achievement unlocking logic, and integrate with game milestones and accomplishments `M`
10. [ ] Add Steam Cloud Saves — Implement Steam Cloud save synchronization, handle save file upload/download, and provide fallback to local saves `M`
11. [ ] Write Tests for Controller Support — Create unit tests for controller input mapping, button detection, and input translation to game commands `S`
12. [ ] Write Tests for Steam Integration — Create unit tests for Steam API initialization, achievement unlocking, and cloud save operations `S`
13. [ ] Polish and Documentation — Create user documentation for controller setup, Steam features, and prepare Steam store page materials `M`
14. [ ] Steam Release Preparation — Final testing, Steam build configuration, store assets preparation, and submission to Steam platform `L`

> Notes
> - Order items by technical dependencies and product architecture
> - Each item should represent an end-to-end (frontend + backend) functional and testable feature
> - Foundation features (build system, logging, testing) must be completed before core features
> - Controller support and Steam integration can be developed in parallel after foundation is complete
> - Advanced features (configurable controls, achievements, cloud saves) build upon core features

