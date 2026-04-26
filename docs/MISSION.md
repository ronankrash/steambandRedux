# Product Mission

**Note:** This is the active mission summary. Historical `agent-os/` material still exists in the repository but is not the source of truth.

## Pitch

SteambandRedux is a modernization of Steamband that helps players enjoy classic steampunk roguelike gameplay with modern controls, testing, and presentation. The target experience is a first-person steampunk dungeon crawler for Windows handheld PCs, while preserving the original game rules, items, maps, saves, and keyboard commands.

## Users

[Full personas from original: The Couch Gamer, The Steam Collector, The Classic Gaming Enthusiast - see original for details or summarized in README.]

## The Problem

Legacy roguelikes often lack modern controller support, release tooling, and approachable presentation. This project addresses that with CMake, tests, controller-first UX, a future integrated SDL renderer, license-safe assets, and a long-term modding path.

## Differentiators (First-Person Steampunk Vision)

- First-Person Steampunk Dungeon Crawler with Controller & Mod Support
- Will use SDL2 for an immersive view and CC0/permissive steampunk/Victorian pixel art.
- Complete keyboard + controller support with ROG Ally defaults, intuitive menus, playtester validation.
- JSON configuration for monsters, items, levels, future NPC AI.
- Rigorous TDD, security audits, open source compliance.
- Cursor rules (.cursor/rules/), skills (.cursor/skills/), HANDOFF.md for agent-assisted iteration toward Steam release.

## Key Features

- Xbox 360 / SDL controller support
- Steam Platform Integration (planned, license-dependent)
- Modern CMake build
- Comprehensive logging and Unity testing
- First-person raycasting renderer prototype (not yet live in gameplay)
- Moddability via JSON

See docs/ROADMAP.md for current status and HANDOFF.md for latest handoff.

Original detailed content preserved in this migration for reference.
