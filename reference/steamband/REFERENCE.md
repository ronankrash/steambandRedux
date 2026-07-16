# Steamband Reference Snapshot

This directory contains the historical Steamband / SteambandRedux C codebase and related assets.

**Status:** Design and behavioral reference only. It is not part of the Brassdeep production build.

## Licensing caution

- Original Angband/Steamband sources include not-for-profit distribution language. Do not assume commercial clearance.
- Third-party art under `third_party/assets/` has mixed licenses (CC0 and CC-BY-SA). See `ASSETS.md` and `LICENSES.md` in this folder.
- Do not paste or port C sources into the Godot project. Reimplement behavior from documented design intent.

## Useful entry points

- `src/` — legacy C engine
- `lib/edit/` — terrain, object, monster data tables
- `docs/` — prior architecture and playtest notes
- `HANDOFF.md` — last known rescue-branch state
