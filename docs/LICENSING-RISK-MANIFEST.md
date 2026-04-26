# Licensing Risk Manifest

Last updated: 2026-04-25

This manifest is a working engineering record, not legal advice.

## Executive Summary

The main release risk is the project’s inherited license stack. The codebase contains several overlapping historical notices:

- Educational/research/not-for-profit source headers.
- Ancient Angband/Koeneke no-sale/no-market language in `src/angband.h`.
- Player-facing help text saying Campbell source may not be used commercially.
- Version/help text describing GPL availability for some contributions while stating the old Moria/Angband license remains the valid license until all parts are GPL.

Do not claim Steam/commercial readiness until this is resolved.

## Highest-Risk Evidence

### Standard Legacy Header

Representative files:

- `src/angband.h`
- `src/config.h`
- `src/defines.h`
- `src/externs.h`
- `src/types.h`
- `src/files.c`
- `src/save.c`
- `src/load2.c`
- `src/cave.c`
- `src/dungeon.c`
- `src/main-win.c`
- `src/z-*.c`
- `src/z-*.h`

Risk language:

```text
This software may be copied and distributed for educational, research,
and not for profit purposes...
```

### No-Sale / No-Market Language

File: `src/angband.h`

Risk language:

```text
No one who-so-ever may sell or market this software in any form without
the expressed written consent of the author Robert Alan Koeneke.
```

### Embedded Runtime Notice

File: `src/variable.c`

Risk: the compiled binary includes a copyright string with educational/research/not-for-profit language.

### Player-Facing Commercial Restriction

File: `lib/help/general.txt`

Risk: help text states Campbell source may not be used commercially.

### GPL Coexistence Text

File: `lib/help/version.txt`

Risk: describes GPL availability for some contributions but states the original Moria/Angband license remains the valid license until all parts are GPL.

### Microsoft Sample Code

Files:

- `src/readdib.c`
- `src/readdib.h`

Risk: distinct Microsoft sample-file terms must be preserved and reviewed for packaged release.

## Current Third-Party Dependencies

- Unity Test Framework: vendored in `third_party/unity/`, MIT license in `third_party/unity/LICENSE.txt`.
- SDL2: installed locally via vcpkg at `C:/Users/bkars/vcpkg`, version observed `sdl2:x64-windows@2.32.10`, Zlib license reported by vcpkg.
- Steamworks SDK: not committed. Must remain external unless Valve terms allow inclusion.

## Remediation Plan

1. Create a full manifest of source/help files containing each risky phrase.
2. Decide whether this fork can remain under legacy terms, migrate to a modern Angband license where valid, or needs explicit permission/legal review.
3. Align `LICENSES.md`, root license docs, `lib/help/general.txt`, `lib/help/version.txt`, and `src/variable.c` with the approved policy.
4. Add SPDX-style headers for Redux-authored files only after the base license strategy is decided.
5. Include third-party license texts in packaged releases.

## Agent Guardrails

- Do not add paid-release, commercial-use, or Steam-ready claims until the base license is resolved.
- Do not remove historical notices casually.
- Do not copy proprietary reference assets.
- Do not commit Steamworks SDK files.
- Do not add assets without updating `ASSETS.md`.

## Search Patterns

```bash
rg -n "This software may be copied and distributed for educational" --glob "*.{c,h}"
rg -n "not for profit" -i --glob "*.{c,h,txt,md}"
rg -n "sell or market" -i --glob "*.{c,h,txt,md}"
rg -n "commercial" -i --glob "lib/help/*.txt"
rg -n "GNU GENERAL PUBLIC LICENSE|only valid license" -i --glob "lib/help/version.txt"
rg -n "cptr copyright" --glob "src/variable.c"
rg --files-without-match "This software may be copied" -g "*.c" src
rg -n "Microsoft Corp|Sample Files" -i --glob "src/readdib.*"
rg --files -g "LICENSE*"
rg -n "SDL2|Steamworks|vcpkg" -i LICENSES.md README.md CMakeLists.txt
```
