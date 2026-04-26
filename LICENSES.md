# License Inventory

Last updated: 2026-04-25

This file is a working inventory, not legal advice.

## Base Game

- Component: Steamband 0.2.2 / Angband-derived source and data
- Source: `readme.txt`, source file headers
- Current status: Requires clarification before Steam distribution or paid release
- Notes: Many legacy source files contain Angband-era language allowing copying and distribution for educational, research, and not-for-profit purposes. Do not assume commercial permission until reviewed.

## New Project Contributions

- Component: SteambandRedux modernization code and documentation
- Intended policy: Keep new project-authored work open source and compatible with the base game license
- Status: Pending a top-level license decision after base-game license review

## Third-Party Dependencies

- Component: Unity Test Framework
- Path: `third_party/unity/`
- License: MIT
- License file: `third_party/unity/LICENSE.txt`

- Component: SDL2
- Path: External dependency, not currently vendored
- License: zlib-style permissive license expected
- Requirement: Include SDL2 license text in packaged releases if SDL2 binaries are distributed

- Component: Steamworks SDK
- Path: Not committed
- License: Valve Steamworks SDK terms
- Requirement: Do not commit SDK files unless the license permits it. Use local SDK discovery and Steam Partner documentation.

## Asset Policy

Future visual/audio assets must be CC0, Public Domain, MIT, or otherwise clearly commercial-permissive. No NC, ND, unclear freeware, ripped game art, or AI-generated assets without documented commercial rights.
