# Architecture Decision Records

## ADR-001: Godot 4.7.1 + GDScript

- **Decision:** Pin Godot 4.7.1-stable; typed GDScript; compatibility renderer.
- **Why:** Stable toolchain, headless CI, broad Windows GPU coverage.
- **Status:** Accepted

## ADR-002: Simulation / presentation / content split

- **Decision:** Keep `game/core` free of Node/sprite dependencies.
- **Why:** Deterministic tests and save safety.
- **Status:** Accepted

## ADR-003: Steamband as reference only

- **Decision:** Relocate legacy tree to `reference/steamband/`; no C ports.
- **Why:** Licensing uncertainty + clean architecture.
- **Status:** Accepted

## ADR-004: JSON content + schema-versioned saves

- **Decision:** Content JSON with string IDs; saves declare `schema_version`.
- **Why:** Validatable data and graceful migration failures.
- **Status:** Accepted (schema now v2)

## ADR-005: Turn-based sim, animated presentation

- **Decision:** Game state advances on committed turns; `AnimSequencer` visualizes afterward and locks input via `awaiting_visual`.
- **Why:** Avoid real-time combat; presentation cannot alter combat results.
- **Status:** Accepted

## ADR-006: Procedural geometric placeholders

- **Decision:** Original drawn polygons/circles for tiles/actors; no third-party art yet.
- **Why:** Unblock playable slice without licensing risk.
- **Status:** Accepted

## ADR-007: Progression model (Phase 2)

- **Decision:** Traditional save-and-reload roguelike with a persistent hub. Character XP/skills/inventory persist across expeditions via save. Death ends the character (permadeath for that operative); no separate meta-unlock tree.
- **Why:** Matches classic Steamband/Angband expectations while supporting town prep between runs.
- **Status:** Accepted

## ADR-008: Save schema v2

- **Decision:** Increment to schema 2 for hub depth, merchants, storage, skills, settings. Migrate schema 1 when possible; reject unknown versions with a clear message.
- **Why:** Phase 2 state does not fit schema 1 safely without explicit fields.
- **Status:** Accepted
