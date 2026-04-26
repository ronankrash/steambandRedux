# SteambandRedux Handoff

Last updated: 2026-04-25
Current phase: rescue baseline and process stabilization

## Ground Truth

Use this file plus `docs/BASELINE-AUDIT.md`, `docs/BASELINE-VERIFICATION.md`, `docs/REPO-HYGIENE.md`, and `docs/ARCHITECTURE-DECISION.md` as the current handoff set.

Do not trust older claims unless they are backed by source code or repeatable commands.

## Current State

- The legacy Steamband/Angband-derived C code remains the canonical gameplay engine and data source.
- The current Windows UI path is still the legacy Term/GDI style interface.
- Controller work exists in `src/controller.c`, `src/controller_menu.c`, and `src/controller_config_menu.c`.
- Practical controller polling is XInput-based. SDL2 `SDL_GameControllerOpen()` is attempted, but SDL controller events are not yet used by `controller_check()`.
- `src/renderer.c` contains a standalone SDL2 DDA raycaster prototype.
- `src/main-win.c` initializes the renderer but does not call `renderer_render()`, `renderer_toggle_mode()`, or `renderer_shutdown()`.
- The first-person renderer is not yet a live playable game feature.
- `agent-os/` still exists and is historical only unless a future PR explicitly migrates or removes it.
- `.gitignore`, `ASSETS.md`, and `LICENSES.md` now exist to support safer repo hygiene and asset/license tracking.

## Verification Status

- Clean CMake configure was attempted with `cmake -S . -B build-rescue-verify`.
- Configure failed because SDL2 could not be found by CMake.
- Existing `build/CMakeCache.txt` shows `SDL2_DIR:PATH=SDL2_DIR-NOTFOUND`.
- SDL2 is required by `CMakeLists.txt`, so future agents must install/configure SDL2 before claiming build success.
- See `docs/BASELINE-VERIFICATION.md` for required commands and pass criteria.

## Test Status

- `CMakeLists.txt` defines `UnitTests` and `UnityTestRunner`.
- `UnityTestRunner` is the primary Unity runner.
- `UnitTests` is an older logging-focused runner and does not execute the full Unity suite.
- Known gaps:
  - `renderer_render()` has no automated coverage.
  - `test_sdl2_controller_init()` is not registered in the Unity runner.
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

1. Verify SDL2 setup and clean CMake configure/build from a fresh build directory.
2. Run `ctest -C Debug --output-on-failure` after a successful build.
3. Launch the game and record keyboard/controller smoke-test results.
4. Fix runner drift: register or remove dead tests and make README test counts match actual runner output.
5. Decide how to handle `agent-os/` in a focused cleanup PR.
6. Resolve or document the base license constraints before any Steam/commercial commitment.
7. Only then integrate the renderer into the real game loop with tests.

## Handoff Rule

Update this file after significant changes. Claims must be factual, current, and tied to source paths or verification commands.
