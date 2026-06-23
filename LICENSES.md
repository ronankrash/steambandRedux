# License Inventory

Last updated: 2026-04-25

This file is a working inventory, not legal advice.

For repeatable risk inventory, run:

```bash
python tools/license_scan.py --details
```

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
- Version observed: `2.6.0` in `third_party/unity/library.json`

- Component: Microsoft DIB sample code
- Paths: `src/readdib.c`, `src/readdib.h`
- Source: Historical Angband Windows bitmap-loading code
- License/status: `src/readdib.c` includes Microsoft sample-file terms allowing royalty-free use, modification, reproduction, and distribution of Sample Files and modified versions, with no Microsoft warranty obligation or liability. Preserve the notice and review packaging requirements before release.

- Component: SDL2
- Path: External dependency installed locally via vcpkg at `C:/Users/bkars/vcpkg`
- Version observed: `sdl2:x64-windows@2.32.10`
- License: Zlib (reported by vcpkg)
- Requirement: Include SDL2 license text in packaged releases if SDL2 binaries are distributed

- Component: Steamworks SDK
- Path: Not committed
- License: Valve Steamworks SDK terms
- Requirement: Do not commit SDK files unless the license permits it. Use local SDK discovery and Steam Partner documentation.

## Asset Policy

Future visual/audio assets must be CC0, Public Domain, MIT, or otherwise clearly commercial-permissive. No NC, ND, unclear freeware, ripped game art, or AI-generated assets without documented commercial rights.

## Integrated Third-Party Art

- Component: DENZI 16x16 Ultima VI oblique tilesets
- Paths: `third_party/assets/denzi/`, generated atlas `lib/xtra/graf/topdown_denzi.bmp`, first-person wall BMPs `lib/xtra/graf/fp_walls_denzi/`
- Source: https://opengameart.org/content/denzis-16x16-oblique-tilesets
- Author: DENZI (denzi.diary@gmail.com)
- License: CC-BY-SA 3.0
- License file: `third_party/assets/denzi/LICENSE.TXT`
- Attribution: required; see `ASSETS.md`
- Share-alike: generated atlas and mapping JSON are derivative works under CC-BY-SA 3.0
- Commercial use: permitted with attribution and share-alike compliance
