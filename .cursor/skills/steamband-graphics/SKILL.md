---
name: steamband-graphics
description: Handles integration of CC0 steampunk/Victorian pixel art assets for first-person dungeon crawler. Sources appropriate tiles/sprites matching early 90s aesthetic (Celtic Tales, Eye of Balor influence). Works with SDL2 renderer, ensures licensing compliance. Use for asset pipeline, texture loading, first-person view implementation.
---

# Steamband Graphics Integrator

## Asset Strategy
- **Style**: Pixel art resembling early 90s (Victorian steampunk: brass, gears, leather, wood, gaslight, pipes). Walls, floors, sprites for monsters (mechanical beasts, airship crew, etc.), items, UI.
- **Sources**: Strictly CC0/Public Domain from OpenGameArt, itch.io. Document all in ASSETS.md.
- **First-Person**: Wall textures for raycasting/2.5D, billboard sprites for objects/NPCs, animated elements where possible.

## Workflow
1. Source/verify licenses (use licensing skill).
2. Integrate via SDL2 textures in new renderer (replace GDI/term).
3. Optimize loading/performance for Steam/PC.
4. Support mod overrides for custom assets.
5. Test with playtester for visual feel and controller navigation.

## Technical
- Update CMake for SDL2_image if needed.
- Preserve cave/map data from existing cave.c/generate.c for procedural dungeons.
- Raycasting or sprite stacking for 3D-like view.
- Scalable UI for resizable windows.

Coordinate with playtester for UX, licensing-expert for compliance, modder for extensibility. Follow all core rules. Verify with tests and screenshots where possible.
