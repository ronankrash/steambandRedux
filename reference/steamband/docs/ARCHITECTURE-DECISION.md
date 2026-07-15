# Architecture Decision: Preserve The Legacy Game Engine

Status: Accepted for rescue baseline
Date: 2026-04-25

## Context

SteambandRedux is a modernization of Steamband, an Angband-derived roguelike with a large body of existing rules, items, monsters, saves, map generation, and keyboard command behavior. The desired product is a first-person steampunk dungeon crawler with ROG Ally-friendly controls, Steam readiness, and moddability.

The current repository has a partially working legacy Windows terminal/GDI path, controller additions, tests, and a disconnected SDL2 raycaster prototype.

## Decision

Preserve the existing C codebase as the canonical gameplay engine and data source during the rescue phase.

Do not remove original gameplay functionality, keyboard mappings, item behavior, combat rules, map generation, save/load behavior, or text-mode affordances unless a future PR intentionally replaces them with tested equivalent behavior.

Near-term modernization should layer a verified renderer/input path onto the existing engine. A clean-slate or modern-language client remains possible, but it must treat the legacy code as the reference implementation until parity tests exist.

## Options Considered

### Continue In C With SDL2

Pros:
- Lowest risk to existing rules and data.
- Reuses current build, tests, and source layout.
- Makes it easier to compare behavior before/after renderer changes.

Cons:
- Legacy C security and maintainability work remains significant.
- SDL integration must be carefully separated from the Win32 terminal path.

### Build A Separate Modern Client

Pros:
- Cleaner UI, renderer, packaging, and controller architecture.
- Easier future modding and Steam platform integration.

Cons:
- High risk of losing subtle game rules.
- Requires an engine API or data extraction layer before it can be trustworthy.
- Needs broad parity tests before replacing the current game.

## Consequences

- First priority is a verified baseline, not a rewrite.
- Renderer work must be integrated behind clear seams and tested against real player/cave state.
- Any future modern client should begin as a consumer of exported game state or data, not as a replacement for rules.
- Modding should start with documented data formats and tests, then expand gradually.

## Guardrails

- Keep original keyboard controls working.
- Keep controller support additive.
- Require tests for preserved behavior before refactors.
- Document licensing constraints before Steam or paid distribution decisions.
