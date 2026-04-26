# Product Roadmap

Last updated: 2026-04-25

## Vision

Restore Steamband as a modern, controller-first, first-person steampunk dungeon crawler for Windows handheld PCs such as the Asus ROG Ally, while preserving the original game rules, items, map generation, saves, and keyboard controls.

## Phase 0: Rescue Baseline

Status: mostly complete; dirty rescue-slice edits still need review/commit

- Make repository state truthful and reviewable.
- Separate generated files, logs, saves, and source edits.
- Verify SDL2 dependency setup and clean build/test commands.
- Clarify base-game licensing constraints before Steam or paid release planning.
- Establish concise team rules, skills, handoff, and PR workflow.

Exit criteria:

- Clean configure/build/test result is documented.
- README, handoff, technical docs, license inventory, and asset inventory are current.
- Dirty tree is classified so future commits can be small and focused.

## Phase 1: Playable Legacy Baseline

Status: automated baseline verified; manual keyboard/controller playtest pending

- Confirm the current 2D Windows client launches.
- Verify keyboard basics and original command behavior.
- Verify controller basics with XInput hardware.
- Fix tests that are present but not registered.
- Add regression tests around critical preserved gameplay behavior.

## Phase 2: Renderer Integration Prototype

Status: live prototype behind toggle; not a finished gameplay mode

- Drive `renderer_render()` from the actual game loop.
- Add mode switching without breaking the legacy 2D display.
- Sync player position and facing from real gameplay state.
- Add keyboard/controller movement/look paths for first-person mode.
- Keep original keyboard mappings available.
- Add renderer tests for DDA math, bounds behavior, and state sync.

## Phase 3: Controller-First UI And ROG Ally UX

Status: partially implemented; hardware playtest pending

- Map right stick to look/turn in first-person mode.
- Rework command groupings for inventory/equipment/character, map/journal, combat, and utility actions.
- Validate BACK double/triple press alternatives on hardware.
- Ensure every menu has controller and keyboard parity.

## Phase 4: Assets And Visual Direction

Status: blocked on asset approval; renderer texture slots intentionally empty

- Source only CC0, Public Domain, MIT, or clearly commercial-permissive assets.
- Document all art/audio in `ASSETS.md`.
- Build a texture/sprite pipeline suitable for the SDL renderer.
- Match Victorian steampunk references without copying proprietary games.

## Phase 5: Modding

Status: future

- Design data-driven content loading for monsters, items, levels, and presentation metadata.
- Preserve base game behavior as defaults.
- Add tests for parsing, validation, and compatibility.
- Document mod licensing expectations.

## Phase 6: Steam Readiness

Status: future and license-dependent

- Resolve base-game license concerns.
- Add packaging, release notes, dependency license bundling, and semver tags.
- Integrate Steamworks only through licensed local SDK setup.
- Validate saves, controller support, overlay behavior, and release builds.
