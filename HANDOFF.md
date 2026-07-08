# SteambandRedux Handoff

Last updated: 2026-07-08
Branch: `cursor/c64-ultima-tileset-7ff6`
Remote status at update: branch is ahead of `origin/steambranch`; only local runtime score data may be dirty.

## Current Mission

Deliver a personally playable controller-first first-person Steamband build for Asus ROG Ally while preserving the legacy C game rules, items, maps, saves, and keyboard behavior. Steam/licensing/release cleanup is deliberately deferred until the first-person play loop is comfortable to play.

## Source Of Truth

- `HANDOFF.md`: current continuation state.
- `docs/BASELINE-AUDIT.md`: factual repo/build audit.
- `docs/BASELINE-VERIFICATION.md`: build/test commands and status.
- `docs/ARCHITECTURE-DECISION.md`: preserve legacy C engine as gameplay source of truth.
- `docs/PLAYTEST-CHECKLIST.md`: manual keyboard/controller/ROG Ally validation checklist.
- `docs/ROG-ALLY-FP-MANUAL-TEST.md`: focused first-person launch and smoke-test guide.
- `docs/ART-DIRECTION.md`: current no-asset art direction and future asset pipeline goals.
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
- `tools/launch_rog_ally_fp_smoke.cmd` launches the current SDL2 Debug first-person smoke build and prints the playtest controls/log paths.
- `tools/package_rog_ally_drop.ps1` assembles a lightweight ROG Ally/LAN drop folder with `SteambandRedux.exe`, SDL2 runtime DLL when present, and a full `lib/` tree beside the executable.
- `tools/launch_topdown_tiles.cmd` launches the SDL2 Debug build with `STEAMBAND_START_TOPDOWN=1` so the tile window opens automatically.
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
  - DENZI first-person wall textures are integrated: 8x 64x64 BMPs in `lib/xtra/graf/fp_walls_denzi/` auto-load in FP mode with feature-aware mapping (masonry, permanent, secret, rubble, magma, quartz, cave, timber). Procedural shading remains the fallback when BMPs are absent.
- First-person view is not yet a finished gameplay mode. It is a live prototype/mirror of player position and cave data, not a polished replacement for the 2D UI.
- SDL2 top-down tile renderer now exists as a no-asset prototype:
  - Hidden by default.
  - Toggle with `Ctrl+F11`.
  - Can autostart via `STEAMBAND_START_TOPDOWN=1` or `tools\launch_topdown_tiles.cmd`.
  - `Ctrl+F11` exits back to the legacy 2D fallback; Escape is forwarded to the legacy game as cancel while top-down mode is focused.
  - It mirrors cave/player state through a tested tile-classification layer with safe out-of-bounds handling.
  - It loads the project-generated placeholder BMP at `lib/xtra/graf/sdl2_topdown_24.bmp` when available, or uses procedural pencil-like tile glyphs as fallback.
  - The placeholder atlas is reproducible via `python tools/generate_topdown_tilesheet.py`.
  - Compatible custom BMP experiments can set `STEAMBAND_TOPDOWN_TILESET` or drop `topdown_tileset.bmp` under `lib/user/` without changing code.
  - Local-only Ultima V PC tile tests are supported by `tools/convert_ultima5_tiles.py` and documented in `docs/LOCAL-ULTIMA5-TILE-TEST.md`; generated Ultima-derived assets remain ignored and must not be committed.
  - Current atlas contract: `topdown-v1`; 24x24 tiles, 9 columns x 4 rows, row-major categories: darkness, floor, wall, door, up stairs, down stairs, trap, glyph, shop, General Store shop, Clothing store shop, Gun shop, Machinist shop, Alchemy shop, Magic shop, Black Market shop, Home, rubble, ore vein, object, food/anodyne object, scroll/book object, potion/flask object, weapon/tool object, armor object, ray gun/launcher object, ammo object, money object, jewelry object, device/chest object, generic monster, automata, undead/demon, beast, humanoid, player.
  - Custom top-down BMPs are validated against the active atlas dimensions before texture creation; wrong-size sheets fall back to procedural tiles.
  - Successful tile loads log atlas dimensions and category count so probes/manual logs verify the expected sheet is active.
  - Shop features now map to individual shop facade tiles instead of one generic shop tile.
  - Special terrain tiles now distinguish glyphs, shops, rubble, and ore veins from generic floors/walls.
  - Object family tiles are renderer-only and derived from bounded `cave_o_idx -> o_list` lookups; invalid or unavailable object data falls back to terrain or the generic object tile.
  - Monster family tiles are renderer-only and derived from visible live monsters using bounded `cave_m_idx -> m_list -> r_info` lookups; unknown, unseen, or unavailable race data falls back to terrain or the generic monster tile.
  - Keyboard commands forward to the legacy input queue while the SDL2 tile window has focus.
  - SDL window title and bottom hint glyphs now distinguish top-down mode from first-person mode (`Ctrl+F11` exits, Escape cancels in-game, keyboard commands forward).
  - The SDL top-down title includes HP/SP/depth/status plus recent legacy/command feedback, so tile-first play has readable state text even without font assets.
  - The player tile now gets a bright focus/crosshair overlay in top-down mode so the current position is easier to spot at handheld scale.
  - If the SDL top-down window loses keyboard focus, a centered warning panel is drawn so input loss reads as a focus problem, not a game bug.
  - Default top-down atlas is now **C64/EGA orthogonal** at `lib/xtra/graf/topdown_c64_ultima.bmp` (`c64-ultima-v1`: native 16×16, 144×64 BMP, nearest-neighbor, integer 16/32px display). Generate with `python tools/generate_c64_ultima_tilesheet.py`; launch with `tools/launch_c64_ultima_tiles.cmd`.
  - Optional DENZI Ultima VI oblique atlas at `lib/xtra/graf/topdown_denzi.bmp` maps all 36 `topdown-v1` categories (24×24 upscaled). Launch with `tools\launch_denzi_tiles.cmd` for VI oblique mode.
  - Mapping spec: `tools/mappings/topdown-v1-denzi-cc-by-sa.json`; build with `python tools/build_denzi_topdown.py`; validate with `python tools/test_denzi_topdown_integration.py`.
  - Source art is vendored under `third_party/assets/denzi/` with `LICENSE.TXT` and `CREDIT.TXT`.
  - A legal proof-of-concept external atlas exists at `lib/xtra/graf/topdown_poc_puny_world.bmp`, generated from Shade's CC0 Puny World tileset and launched with `tools\launch_puny_world_tiles.cmd`.
  - A second legal proof-of-concept external atlas exists at `lib/xtra/graf/topdown_poc_kenney.bmp`, generated from Kenney's CC0 Roguelike/RPG pack and launched with `tools\launch_kenney_tiles.cmd`.
  - Asset matching is tracked by `docs/TOPDOWN-ASSET-COVERAGE.md`, generated with `python tools/report_topdown_asset_coverage.py`.
  - Current coverage is category-level: terrain is mostly covered, objects use broad item-family tiles, and monsters use broad family tiles.

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

- Latest DENZI oblique tileset integration passed Python pipeline checks on Linux:
  - `python tools/test_denzi_topdown_integration.py`
  - `python tools/test_denzi_fp_walls_integration.py`
  - `python tools/validate_topdown_mappings.py`
  - `python tools/validate_topdown_tileset.py lib/xtra/graf/topdown_denzi.bmp`
  - `python tools/report_topdown_asset_coverage.py`
  - Windows/SDL manual playtest still required: `tools/launch_denzi_fp.cmd` or `tools/launch_denzi_tiles.cmd`, then File -> New, verify FP walls and top-down readability.
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
- Latest SDL2 top-down tile detour slice passed:
  - `cmake --build build-rescue-sdl2 --config Debug`
  - `ctest --test-dir build-rescue-sdl2 -C Debug --output-on-failure`
  - `cmake --build build-rescue-nosdl --config Debug`
  - `ctest --test-dir build-rescue-nosdl -C Debug --output-on-failure`
  - `powershell -ExecutionPolicy Bypass -File tools/probe_rog_ally_fp.ps1`
  - `powershell -ExecutionPolicy Bypass -File tools/probe_rog_ally_fp.ps1 -TopDown`
  - `powershell -ExecutionPolicy Bypass -File tools/package_rog_ally_drop.ps1`
  - Packaged drop folder contains `lib/xtra/graf/sdl2_topdown_24.bmp`.
  - Unity coverage now includes top-down tile priority, safe bounds, viewport sizing, fallback colors, and independent top-down mode toggling.
  - Automated no-hardware probing verifies `Ctrl+F11` top-down activation, placeholder tilesheet load, and clean shutdown from logs.
  - Probe keyboard injection now posts F11/F12 directly to the native game window while Ctrl is held to avoid foreground timing misses.
  - Placeholder atlas regeneration passed with `python tools/generate_topdown_tilesheet.py` and matched the committed BMP.
  - Manual visual validation of tile readability on the ROG Ally is still pending.
- `python tools/license_scan.py --details` still reports inherited release blockers: 70 educational/not-for-profit files, 75 not-for-profit matches, 1 sell-or-market match, 1 commercial-use help match, 1 legacy/GPL coexistence match, 2 embedded copyright-string locations, and 1 Microsoft sample-file match.
- Interactive keyboard/controller/ROG Ally smoke testing is still pending and must use `docs/PLAYTEST-CHECKLIST.md`.
- Default `build/Debug` can be locked if the game is running; close `SteambandRedux.exe` before rebuilding that tree.

## Licensing Status

This is a later release blocker, not the current playability focus.

- Many legacy source files contain educational/research/not-for-profit terms.
- `src/angband.h` includes no-sale/no-market language.
- `lib/help/general.txt` includes commercial-use restriction language.
- `lib/help/version.txt` includes legacy/GPL coexistence language.
- Do not claim Steam/commercial readiness until the license story is resolved.
- Use `python tools/license_scan.py --details` for the current manifest.

## Recommended Next Work

1. Prioritize manual ROG Ally playability over licensing/Steam:
   - Run `tools\launch_rog_ally_fp_smoke.cmd`.
   - Follow `docs\ROG-ALLY-FP-MANUAL-TEST.md`.
   - Record concrete failures around birth, movement, camera, top-down tile readability, command menu, inventory/equipment, stairs, combat, save/load, focus, and sleep/resume.
2. Fix first-person usability bugs immediately and commit each verified slice:
   - controller turn/move feel,
   - command coverage from FP,
   - character creation/controller traps,
   - visibility of monsters/items/doors/stairs/traps,
   - HUD/readability at 720p and 1080p.
3. Keep using no-asset/procedural presentation until gameplay feels good:
   - follow `docs/ART-DIRECTION.md`,
   - keep texture loading disabled until assets are explicitly approved in `ASSETS.md`.
4. Defer licensing/Steam remediation until the user can comfortably play the game in first person.
5. Do not push unless explicitly requested.

## Team Roles To Continue

- Team Lead: keep handoff truthful, sequence PR-sized changes.
- Build/Release: CMake/vcpkg/CI, clean tree, semver only after verified working releases.
- Renderer: SDL first-person prototype, render tests, asset pipeline.
- Controller/UI/UX: ROG Ally controls, menu parity, focus/window behavior.
- Playtester: execute `docs/PLAYTEST-CHECKLIST.md`.
- Security: legacy C bounds/file/save/input review.
- Licensing/Assets: resolve base license and approve only documented permissive assets.
- Modding: future data-driven content, after baseline stability.
