# Baseline Audit

Last updated: 2026-04-25

## Scope

This audit records what is currently visible in the repository. It is intentionally conservative: claims from older handoffs, roadmap files, and prior agents are not treated as complete until they are backed by source code or repeatable commands.

## Repository State

- The workspace is dirty and includes source edits, documentation edits, generated build outputs, logs, save files, Cursor rules/skills, and a vendored Unity tree.
- `.gitignore` was missing before this rescue pass, so generated files and local runtime data were easy to stage accidentally.
- `agent-os/` still exists even though several docs previously claimed it had been removed. Treat it as historical reference only unless it is deliberately migrated or deleted in a focused PR.
- Current branch from `git status --short --branch`: `steambranch...origin/steambranch`.
- Current verified tag list contains `v0.2.3-alpha`; no `v0.2.0-sdl2-foundation` tag was found.
- Remotes are `origin` (`ronankrash/steambandRedux`) and `upstream` (`myshkin/steamband`). Do not push to `upstream`.

## Build And Dependencies

- `CMakeLists.txt` now treats SDL2 as optional for baseline builds. If SDL2 is not discoverable, the legacy 2D/XInput baseline configures without `src/renderer.c`.
- The existing `build/CMakeCache.txt` records `SDL2_DIR:PATH=SDL2_DIR-NOTFOUND`, so SDL2 renderer setup is not verified in the current cache.
- Visual Studio 2022 is referenced by the existing build cache.
- A clean configure attempt before the optional-SDL2 change failed because CMake could not find `SDL2Config.cmake` or `sdl2-config.cmake`.
- After the optional-SDL2 change, `cmake -S . -B build-rescue-nosdl`, `cmake --build build-rescue-nosdl --config Debug`, and `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure` all pass for the non-SDL2 baseline.

## Tests

- `CMakeLists.txt` defines two test executables: `UnitTests` and `UnityTestRunner`.
- `UnitTests` runs the older custom logging runner and does not execute renderer, z-util, or controller Unity tests.
- `UnityTestRunner` is the primary runner for Unity tests. It registers infrastructure, logging, z-util, controller, and SDL2 smoke tests.
- `test_renderer_basic` is included through `test_unity_infrastructure.c` and exercises only basic wall/OOB behavior plus a weak DDA smoke call.
- `test_sdl2_controller_init` is registered in `unity_test_runner.c` and is ignored when SDL2 is disabled at build time.
- `test_util.c` exists but is not part of the current CMake test targets.

## Renderer Status

- `src/renderer.c` contains a real SDL2 DDA-style raycaster prototype with a standalone SDL window, ceiling/floor fills, wall strips, and a fallback test map.
- `src/main-win.c` calls `renderer_init(get_renderer())` only when built with `STEAMBAND_HAS_SDL2`.
- No code outside `renderer.c` calls `renderer_render()`, `renderer_toggle_mode()`, or `renderer_shutdown()`.
- `g_use_2d_fallback` is internal to the renderer module and is not consumed by the Win32 terminal display path.
- The first-person renderer must not be described as a live game feature until it is driven by the game loop and controllable through keyboard/controller input.

## Controller Status

- XInput is the practical input path used by `controller_check()`.
- SDL2 `SDL_GameControllerOpen(0)` is attempted during initialization, but `controller_check()` does not read SDL controller events or axes.
- Left thumbstick maps to 8-way movement through virtual keypresses.
- Right thumbstick and triggers are currently unused.
- BACK double/triple press menu gestures exist but need automated timing tests and physical ROG Ally playtesting.

## Documentation Status

- `HANDOFF.md`, `README.md`, `docs/ROADMAP.md`, `docs/TECHNICAL.md`, and `SECURITY-AUDIT.md` previously overstated or blurred unverified work.
- The current documentation set should be treated as the source of truth only after this rescue pass.

## Licensing Status

- The legacy source contains Angband-era file headers allowing copying/distribution for educational, research, and not-for-profit purposes.
- That wording is a potential blocker for Steam distribution and any future paid release until clarified by legal review, upstream license history, or a documented relicensing path.
- Unity is vendored under MIT in `third_party/unity/LICENSE.txt`.
- SDL2 is expected to be permissive, but its license text must be carried in release packaging when SDL2 is distributed.
- No third-party art/audio assets are currently documented for the first-person view.

## Open Risks

- SDL2 renderer build/test status remains blocked until SDL2 is installed and discoverable by CMake.
- Manual gameplay launch and controller hardware checks are still pending.
- The first-person prototype is disconnected from the actual game loop.
- The license story is not ready for commercial release decisions.
- Legacy C still contains many unsafe string formatting/copying patterns that require phased hardening with tests.
