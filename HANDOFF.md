# SteambandRedux Handoff

Last updated: 2026-04-25
Branch: `steambranch`
Remote status at update: clean and aligned with `origin/steambranch`

## Current Mission

Modernize Steamband into a controller-first, Steam-ready, first-person steampunk dungeon crawler while preserving the legacy C game rules, items, maps, saves, and keyboard behavior.

## Source Of Truth

- `HANDOFF.md`: current continuation state.
- `docs/BASELINE-AUDIT.md`: factual repo/build audit.
- `docs/BASELINE-VERIFICATION.md`: build/test commands and status.
- `docs/ARCHITECTURE-DECISION.md`: preserve legacy C engine as gameplay source of truth.
- `docs/PLAYTEST-CHECKLIST.md`: manual keyboard/controller/ROG Ally validation checklist.
- `docs/LICENSING-RISK-MANIFEST.md`: known legacy licensing blockers.
- `tools/license_scan.py`: repeatable scan for restrictive legacy license phrases.

Do not trust older docs or prior agent claims unless backed by source or command output.

## What Is Working

- Non-SDL baseline configures/builds/tests:
  - `cmake -S . -B build-rescue-nosdl`
  - `cmake --build build-rescue-nosdl --config Debug`
  - `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure`
- SDL2 build configures/builds/tests with local vcpkg:
  - SDL2 installed at `C:/Users/bkars/vcpkg`
  - configure with `SDL2_DIR=C:/Users/bkars/vcpkg/installed/x64-windows/share/sdl2`
- `build/Debug` can be configured as the SDL2 build and copies `SDL2d.dll` beside Debug executables.
- Generated `build/` artifacts are no longer tracked by Git.
- `agent-os/` has been removed; current workflow is `.cursor/rules`, `.cursor/skills`, docs, and handoff.

## Current Gameplay/Renderer State

- Legacy Win32 Term/GDI display remains the main playable UI.
- Missing legacy `.FON` files no longer make the map unreadable: fallback glyphs translate special floor/wall slots into readable `.` and `#` with system fonts.
- SDL2 first-person renderer exists as a prototype:
  - Hidden by default.
  - Toggle with `Ctrl+F12`.
  - Controller toggle: `L3 + R3`.
  - Escape or `Ctrl+F12` in SDL window exits first-person mode.
  - Right stick turns the first-person camera.
  - SDL keyboard focus forwards basic commands into the legacy Angband input queue.
  - Renderer is pulsed from the Win32 loop and has bounded SDL event handling.
- First-person view is not yet a finished gameplay mode. It is a live prototype/mirror of player position and cave data, not a polished replacement for the 2D UI.

## Controller State

- Runtime input is still XInput-first.
- SDL `GameController` initialization exists but SDL controller polling is not the primary input path.
- Current mappings:
  - `A`: Enter/confirm.
  - `B`: Escape/cancel.
  - `X`: inventory (`i`).
  - `Y`: equipment (`e`).
  - D-pad: cardinal movement.
  - Left stick: 8-way movement.
  - Right stick: first-person camera turn while FP is active.
  - `LB`: rest (`R`).
  - `RB`: search (`s`).
  - `Back`: single map, double command menu, triple config menu.
  - Active controller menus can also be closed with `Back`.

## Verification Notes

- Automated non-SDL and SDL CTest paths were passing at the last full verification.
- Bounded launch probes have shown the executable starts and stays alive.
- Interactive keyboard/controller/ROG Ally smoke testing is still pending and must use `docs/PLAYTEST-CHECKLIST.md`.
- Default `build/Debug` can be locked if the game is running; close `SteambandRedux.exe` before rebuilding that tree.

## Licensing Status

This is the biggest release blocker.

- Many legacy source files contain educational/research/not-for-profit terms.
- `src/angband.h` includes no-sale/no-market language.
- `lib/help/general.txt` includes commercial-use restriction language.
- `lib/help/version.txt` includes legacy/GPL coexistence language.
- Do not claim Steam/commercial readiness until the license story is resolved.
- Use `python tools/license_scan.py --details` for the current manifest.

## Recommended Next Work

1. Run and record the manual playtest checklist on the current SDL2 `build/Debug` build.
2. Fix any user-visible controller/window/render issues found during manual testing.
3. Continue renderer hardening:
   - deterministic ray/strip tests,
   - event/focus policy polish,
   - visible HUD/mode hint,
   - texture/asset loading only after license-safe assets are approved.
4. Start licensing remediation:
   - full manifest from `tools/license_scan.py`,
   - legal/upstream policy decision,
   - align `LICENSES.md`, help text, and embedded copyright string.
5. Keep committing small, focused changes. Do not push unless explicitly requested.

## Team Roles To Continue

- Team Lead: keep handoff truthful, sequence PR-sized changes.
- Build/Release: CMake/vcpkg/CI, clean tree, semver only after verified working releases.
- Renderer: SDL first-person prototype, render tests, asset pipeline.
- Controller/UI/UX: ROG Ally controls, menu parity, focus/window behavior.
- Playtester: execute `docs/PLAYTEST-CHECKLIST.md`.
- Security: legacy C bounds/file/save/input review.
- Licensing/Assets: resolve base license and approve only documented permissive assets.
- Modding: future data-driven content, after baseline stability.
