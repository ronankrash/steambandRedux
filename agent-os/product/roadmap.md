# Product Roadmap (Updated for First-Person Steampunk Conversion)

**Vision:** Transform 2D ASCII roguelike into polished first-person steampunk dungeon crawler using SDL2, CC0 pixel art (Victorian aesthetic), comprehensive TDD, agent team orchestration, moddable JSON configs, full controller support (ROG Ally default), and Steam integration. All changes follow security audits, permissive licensing, frequent git commits with semver tags. See .cursor/rules/, .cursor/skills/, HANDOFF.md, SECURITY-AUDIT.md.

## Completed Foundation
1. [x] CMake Build, Logging, Unit Testing (Unity), XInput/Controller Mapping & Config (with tests) - Modern infrastructure complete.
2. [x] Agent Team Setup - Cursor rules (.cursor/rules/steamband-*.mdc), project skills (.cursor/skills/steamband-*/), HANDOFF.md template, git workflow (fork-only, frequent small commits, semver), licensing/security baseline (SECURITY-AUDIT.md).

## Current Phase: Graphics & Architecture Overhaul (SDL2 First-Person)
3. [ ] Integrate SDL2 - Update CMakeLists.txt, replace GDI/term rendering with SDL2 for windowing, textures, input. Optimize for ROG Ally. (Test-driven).
4. [ ] First-Person Renderer Prototype - Implement raycasting or 2.5D sprite-based view using existing cave/map data ([cave.c](src/cave.c), [dungeon.c](src/dungeon.c)). Integrate CC0 steampunk/Victorian pixel assets (walls, floors, sprites). Playtester validation.
5. [ ] Asset Pipeline & Licensing - Source/document permissive assets per asset-licensing.mdc. Create ASSETS.md. Graphics skill coordinates.

## Subsequent Phases (Agent Team Iterates)
6. [ ] UI/UX Polish - Intuitive controller-first menus leveraging [controller_menu.c](src/controller_menu.c). Polished HUD for first-person view.
7. [ ] Modding & Extensibility - JSON data-driven content for items/monsters/levels. Hooks for NPC AI and memory systems. Easy mod loading.
8. [ ] Full Steam Integration - Complete [steam_integration.c](src/steam_integration.c), achievements, cloud saves, overlay. Tests for Steam features.
9. [ ] Security Hardening & Optimizations - Address legacy issues from SECURITY-AUDIT.md (string funcs, file I/O). Performance for production.
10. [ ] Comprehensive Testing & Playtesting - Expand Unity tests. Dedicated playtester validates all controller flows, first-person feel, mod examples.
11. [ ] Polish, Documentation, Releases - Frequent semver tags (v0.1.0 for prototype), Steam store assets, user docs. Seek feedback on viable product.

## Notes
- **TDD Mandatory:** All functionality has best-practice tests. Use existing test framework + new ones for renderer/mod system.
- **Agent Independence:** Roles use dedicated skills/rules. Update HANDOFF.md after milestones. Iterate until polished playable game.
- **Licensing:** All assets CC0/permissive. Open source everything suitable for Steam free release.
- **Dependencies:** SDL2 before renderer; licensing before assets; tests before commits.
- Updated per user requirements for first-person conversion, controller focus, moddability, agent team, and production readiness.

See [mission.md](agent-os/product/mission.md) for updated pitch and [HANDOFF.md](../HANDOFF.md) for current state.

