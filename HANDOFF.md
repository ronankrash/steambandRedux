# SteambandRedux Handoff

Last updated: 2026-04-25
Current phase: rescue baseline and process stabilization

## Ground Truth

Use this file plus `docs/BASELINE-AUDIT.md`, `docs/BASELINE-VERIFICATION.md`, `docs/REPO-HYGIENE.md`, and `docs/ARCHITECTURE-DECISION.md` as the current handoff set.
Use `docs/PLAYTEST-CHECKLIST.md` for repeatable manual validation and `docs/LICENSING-RISK-MANIFEST.md` for release-risk tracking.

Do not trust older claims unless they are backed by source code or repeatable commands.

## Current State

- The legacy Steamband/Angband-derived C code remains the canonical gameplay engine and data source.
- The current Windows UI path is still the legacy Term/GDI style interface.
- Controller work exists in `src/controller.c`, `src/controller_menu.c`, and `src/controller_config_menu.c`.
- Practical controller polling is XInput-based. SDL2 controller initialization is compiled only when SDL2 is available, and SDL controller events are not yet used by `controller_check()`.
- `src/renderer.c` contains a standalone SDL2 DDA raycaster prototype and is compiled only when SDL2 is available.
- `src/main-win.c` initializes, pulses, and shuts down the SDL renderer in SDL2 builds. The renderer mirrors player/cave state into the SDL prototype window during the Win32 event loop.
- The first-person renderer is now loop-integrated as a prototype, but not yet a finished playable feature.
- `agent-os/` still exists and is historical only unless a future PR explicitly migrates or removes it.
- `.gitignore`, `ASSETS.md`, and `LICENSES.md` now exist to support safer repo hygiene and asset/license tracking.

## Verification Status

- Clean CMake configure initially failed because SDL2 could not be found by CMake.
- The build now treats SDL2 as optional so the legacy 2D/XInput baseline can configure without SDL2.
- Non-SDL2 baseline verification passed:
  - `cmake -S . -B build-rescue-nosdl`
  - `cmake --build build-rescue-nosdl --config Debug`
  - `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure`
- SDL2 was installed locally with vcpkg at `C:/Users/bkars/vcpkg`.
- SDL2 renderer build verification passed:
  - `cmake -S . -B build-rescue-sdl2 -DCMAKE_TOOLCHAIN_FILE=C:/Users/bkars/vcpkg/scripts/buildsystems/vcpkg.cmake -DSTEAMBAND_ENABLE_SDL2=ON`
  - `cmake --build build-rescue-sdl2 --config Debug`
  - `ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure`
- Bounded SDL launch probe passed: `build-rescue-sdl2/Debug/SteambandRedux.exe` started and stayed alive for 3 seconds before test termination.
- Interactive keyboard/controller hardware smoke tests remain pending.
- See `docs/BASELINE-VERIFICATION.md` for required commands and pass criteria.

## Test Status

- `CMakeLists.txt` defines `UnitTests` and `UnityTestRunner`.
- `UnityTestRunner` is the primary Unity runner.
- `UnitTests` is an older logging-focused runner and does not execute the full Unity suite.
- Known gaps:
  - `renderer_render()` has only loop-level smoke coverage, not deterministic framebuffer or interaction coverage.
  - SDL2-specific smoke tests are ignored when SDL2 is disabled.
  - `test_util.c` is not part of the current CMake targets.
  - Controller timing and hardware paths require more tests and physical playtesting.

## Licensing Status

- Legacy source headers include educational/research/not-for-profit language.
- Do not assume paid Steam release compatibility until the base-game license story is clarified.
- No first-person visual/audio assets are approved yet.
- Future assets must be documented in `ASSETS.md` before integration.
- Third-party dependency and license tracking starts in `LICENSES.md`.

## Active Roles

- Team Lead: roadmap, PR sequencing, truthful handoff.
- Build/Release Engineer: CMake, SDL2 setup, CI, semver releases.
- Gameplay Preservation Engineer: original rules, items, saves, maps, keyboard parity.
- Security Engineer: legacy C hardening and save/file/input safety.
- Licensing/Assets Engineer: base license, third-party licenses, asset compliance.
- Renderer Engineer: SDL first-person view and integration.
- Controller/UI/UX Engineer: ROG Ally controls, menus, HUD, keyboard parity.
- Playtester: independent manual validation.
- Modding Engineer: future data-driven extension system.

## Next Actions

1. Run interactive keyboard/controller smoke tests on the current build.
2. Resolve or document the base license constraints before any Steam/commercial commitment.
3. Expand renderer tests beyond smoke coverage and tune event handling.
4. Add real SDL controller polling or document XInput-only controller scope.

## Handoff Rule

Update this file after significant changes. Claims must be factual, current, and tied to source paths or verification commands.
