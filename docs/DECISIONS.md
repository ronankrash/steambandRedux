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
- **Status:** Accepted

## ADR-005: Turn-based sim, animated presentation later

- **Decision:** Game state advances on committed turns; visuals may tween afterward.
- **Why:** Avoid real-time combat complexity in milestone 1.
- **Status:** Accepted

## ADR-006: Procedural geometric placeholders

- **Decision:** Original drawn polygons/circles for tiles/actors; no third-party art yet.
- **Why:** Unblock playable slice without licensing risk.
- **Status:** Accepted
