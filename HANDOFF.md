# SteambandRedux Handoff

Last updated: 2026-04-26
Branch: `steambranch`
Remote status at update: branch is ahead of `origin/steambranch`; working tree has uncommitted rescue-slice edits.

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
- `run-rog-ally-fp.bat` launches the current `build/Debug/SteambandRedux.exe` and prints the first-person playtest controls.
- `tools/probe_rog_ally_fp.ps1` performs a no-hardware native-window keyboard probe against the SDL2 Debug build and verifies renderer init, DDA startup, first-person activation, and shutdown from the new log segment.
- Generated `build/` artifacts are no longer tracked by Git.
- `agent-os/` has been removed; current workflow is `.cursor/rules`, `.cursor/skills`, docs, and handoff.

## Current Gameplay/Renderer State

- Legacy Win32 Term/GDI display remains the main playable UI.
- The startup prompt now prints compact controller/first-person control hints before `New`/`Open`, so ROG Ally defaults and FP entry are discoverable immediately.
- Missing legacy `.FON` files no longer make the map unreadable: fallback glyphs translate special floor/wall slots into readable `.` and `#` with system fonts.
- SDL2 first-person renderer exists as a prototype:
  - Hidden by default.
  - Toggle with `Ctrl+F12`.
  - Controller toggle: `L3 + R3`.
  - Escape, controller `B`, or `Ctrl+F12` exits first-person mode when no controller menu is active.
  - Right stick turns the first-person camera.
  - SDL keyboard focus forwards commands into the legacy Angband input queue.
  - In the SDL window, `W`/Up move forward relative to the camera, `S`/Down move backward, `A`/`D` strafe, and Left/Right arrows turn the camera.
  - Renderer is pulsed from the Win32 loop and has bounded SDL event handling.
  - Renderer ray/strip math now has deterministic Unity coverage via pure trace helpers.
  - SDL key forwarding is gated on first-person mode plus SDL keyboard focus.
  - In-window HUD hint blocks show mode/focus state plus compact move/turn glyphs without adding font or art assets.
  - In-window first-person HUD now mirrors legacy state with asset-free HP/SP bars, depth gauge, focus gauge, status pips, and recent-message activity.
  - SDL window title now exposes readable first-person state: HP/SP, depth, status labels, focus hint, and the latest legacy message when available.
  - Controller command-menu selection publishes category/name overlay feedback into the first-person HUD/title, so double-Back command focus is visible from the SDL view.
  - Controller command menu now uses a three-column 80-column-safe layout with short category prefixes and redraw clearing to avoid stale/overlapping rows.
  - First-person view renders asset-free world markers for visible monsters, objects, stairs, doors, and traps so actionable dungeon state is no longer limited to the 2D map.
  - Wall strips use deterministic feature-aware colors plus distance/side shading; debug minimap is hidden by default for immersion.
  - SDL resize events clamp renderer dimensions to a safe readable viewport range.
  - Texture slots remain empty unless assets are explicitly approved in `ASSETS.md`.
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
  - D-pad/left stick become camera-relative while first-person mode is active.
  - Right stick: first-person camera turn while FP is active.
  - `LB`: rest (`R`).
  - `RB`: search (`s`).
  - `Back`: single map after the double/triple gesture window, double command menu, triple config menu.
  - Active controller menus can also be closed with `Back`.
- Controller fixes in this branch prevent double/triple `Back` from leaking the single-map command, and prevent the `A` press used to enter config remap mode from becoming the remap source.
- Config menu redraws now clear the old prompt area, echoes the highlighted mapping near the top, and labels `B`/`Back` behavior as cancel-remap or save-and-close depending on mode.
- Startup logs now print the current ROG Ally defaults and Back/L3+R3 gestures so a handheld playtester can confirm the active mapping from `lib/logs/steamband.log`.
- The controller layer exposes short playability hint strings with Unity coverage, keeping the visible startup hints synchronized with tested defaults.

## Verification Notes

- Latest ROG Ally playtest-prep pass added a Unity contract test for default controller mappings and passed:
  - `cmake --build build-rescue-nosdl --config Debug`
  - `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure`
  - `cmake --build build-rescue-sdl2 --config Debug`
  - `ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure`
  - Bounded SDL launch probe: `build-rescue-sdl2/Debug/SteambandRedux.exe` stayed running for 3 seconds before forced test shutdown.
- Latest first-person play conversion added camera-relative SDL keyboard and controller movement transforms and passed:
  - `cmake --build build-rescue-sdl2 --config Debug`
  - `ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure`
  - `cmake --build build-rescue-nosdl --config Debug`
  - `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure`
  - Bounded SDL launch probe: `build-rescue-sdl2/Debug/SteambandRedux.exe` stayed running for 3 seconds before forced test shutdown.
  - Default SDL build also updated and passed: `cmake --build build --config Debug`, `ctest --test-dir build -C Debug --output-on-failure`, and a 3-second bounded launch probe for `build/Debug/SteambandRedux.exe`.
- Automated non-SDL and SDL CTest paths passed in this pass:
  - `cmake --build build-rescue-nosdl --config Debug`
  - `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure`
  - `ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure`
- Latest SDL renderer polish passed:
  - `cmake --build build-rescue-sdl2 --config Debug`
  - `ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure`
- Non-SDL regression after the renderer polish passed:
  - `cmake --build build-rescue-nosdl --config Debug`
  - `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure`
- Latest first-person discoverability slice added startup controller/FP hints and a Unity contract test for the hint strings. Verification passed:
  - `cmake --build build-rescue-nosdl --config Debug`
  - `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure`
  - `cmake --build build-rescue-sdl2 --config Debug`
  - `ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure`
  - Bounded SDL launch probe: `build-rescue-sdl2/Debug/SteambandRedux.exe` stayed running for 3 seconds before forced test shutdown.
- Latest first-person HUD/game-feedback slice added pure HUD snapshot helpers and asset-free SDL HUD/status/title feedback. Verification passed:
  - `cmake --build build-rescue-sdl2 --config Debug`
  - `ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure`
  - `cmake --build build-rescue-nosdl --config Debug`
  - `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure`
- Latest command feedback slice bridges controller command-menu selection into the first-person HUD/title and passed:
  - `cmake --build build --config Debug`
  - `ctest --test-dir build -C Debug --output-on-failure`
  - Bounded launch probe: `build/Debug/SteambandRedux.exe` stayed running for 3 seconds before forced test shutdown.
- Latest handheld menu UX slice tightened command/config menu layout, category feedback, redraw clearing, and docs. Verification passed:
  - `cmake --build build-rescue-nosdl --config Debug`
  - `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure`
  - `cmake --build build-rescue-sdl2 --config Debug`
  - `ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure`
  - Bounded SDL launch probe: `build-rescue-sdl2/Debug/SteambandRedux.exe` stayed running for 3 seconds before forced test shutdown.
- Bounded SDL launch probe passed: `build-rescue-sdl2/Debug/SteambandRedux.exe` stayed running for 3 seconds before forced test shutdown.
- Latest no-hardware ROG Ally playtest probe passed:
  - `cmake --build build-rescue-nosdl --config Debug`
  - `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure`
  - `cmake --build build-rescue-sdl2 --config Debug`
  - `ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure`
  - `powershell -ExecutionPolicy Bypass -File tools/probe_rog_ally_fp.ps1`
  - Direct `SendInput` probe activated first-person mode and logged renderer shutdown.
- Character creation could not be fully verified automatically in this pass: title-screen key injection is feasible, but the birth flow remains visual and needs manual confirmation or future UI-state readback.
- `tools/probe_rog_ally_fp.ps1 -TryNewGame` is intentionally a separate bounded title-screen injection probe and does not assert first-person activation in the same run.
- `python tools/license_scan.py --details` still reports inherited release blockers: 70 educational/not-for-profit files, 75 not-for-profit matches, 1 sell-or-market match, 1 commercial-use help match, 1 legacy/GPL coexistence match, 2 embedded copyright-string locations, and 1 Microsoft sample-file match.
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
   - manual SDL first-person focus/window smoke testing,
   - replace abstract HUD blocks with licensed UI text/art after asset approval,
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
