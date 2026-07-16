# Brassdeep Agent Guide

Pinned engine: **Godot 4.7.1-stable** (`tools/bin/godot`).

## Mission

Build an original steampunk turn-based isometric roguelike. Steamband under `reference/steamband/` is design reference only — do not paste C code into `game/`.

## Architecture

1. **Simulation** (`game/core/`) — deterministic, presentation-free.
2. **Presentation** (`game/presentation/`) — Godot scenes, isometric view, input, UI.
3. **Content** (`game/content/`) — JSON definitions with stable string IDs.

## Commands

```bash
python tools/validate_content.py
chmod +x tools/run_ci_local.sh && tools/run_ci_local.sh
# optional Windows export locally:
BRASSDEEP_EXPORT=1 tools/run_ci_local.sh
```

## Rules

- Prefer small commits and failing tests over silent stubs.
- Keep simulation independent of sprites/UI.
- Update `docs/STEAMBAND_FEATURE_MATRIX.md` and `NEXT_TASKS.md` when parity status changes.
- Do not mark incomplete systems finished.
- Controller usability and deterministic simulation outrank visual polish.
