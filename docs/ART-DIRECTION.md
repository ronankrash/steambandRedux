# Art Direction Checklist

Last updated: 2026-04-26

## Presentation Target

Aim for a mid-to-late 90s first-person dungeon crawler read: chunky 4:3 composition, low-resolution wall texture rhythm, warm gaslight, oxidized metal, soot-dark stone, and clear UI gauges that work on a handheld screen. References such as later Wizardry-era crawlers, Lands of Lore-era readability, and Victorian/steampunk adventure art are style references only.

## Current No-Asset Renderer Goals

- Preserve the legacy 2D client as the fallback while the SDL2 first-person window remains a prototype.
- Keep texture loading disabled until `ASSETS.md` records verified commercial-permissive sources.
- Use deterministic procedural wall variation for now: masonry rows, plate seams, brass rivet highlights, distance fog, side shading, and a dark vignette.
- Use the SDL2 top-down tile view as the first long-term 2D art pipeline slice: procedural tile fills, dark pencil-like outlines, simple hatching/glyph marks, and no external art until licenses are approved.
- Keep the palette grounded in soot stone, aged brass, leather brown, dark brick, cool ceiling haze, and warm horizon gaslight.
- Favor readable silhouette and orientation over realism; walls should communicate dungeon depth even without approved textures.

## Renderer Palette Targets

- Near wall masonry: warm gray-brown and dark brick, with brass or copper highlights used sparingly.
- Far walls: fog toward near-black brown, never bright saturated colors.
- Floors: darker and cooler than the horizon so wall bottoms remain readable.
- Ceiling and horizon: subtle gaslight glow, not a flat skybox.
- UI panels: translucent black/brown backing, brass accents, green/amber/red state cues, and compact glyph-like controls.

## Texture Pipeline Goals

- Base wall set: stone brick, soot brick, brass/pipe wall, timber/metal plate wall, ore or machinery accent wall.
- Floor and ceiling set: stone floor, metal grating, stained concrete, low-contrast ceiling haze or rafters.
- Sprite set: monsters, objects, traps, projectiles, stairs/doors, and steampunk dungeon dressing.
- Top-down tile set: 16x16 or 24x24 orthogonal floor, darkness, walls, doors, stairs, traps, objects, player, and broad creature-family tiles before decorative variants.
- Font/UI set: readable bitmap-style UI at 720p, 1080p, and handheld scale.
- All assets must have durable source, author, license, and commercial-use notes in `ASSETS.md` before being loaded.

## Top-Down Tile Direction

Ultima V tile sheets and Balor of the Evil Eye are style references only. The desired in-game result is a lightweight early-90s top-down read with muted fills, sketch-like outlines, sparse hatching, and strong icon silhouettes. Do not copy, trace, recolor, or bundle tiles from those games or screenshots.

The current SDL2 top-down mode has a small project-generated placeholder BMP at `lib/xtra/graf/sdl2_topdown_24.bmp` plus a procedural fallback if the BMP is missing. It classifies live cave cells into stable renderer categories before drawing, so a compatible custom sheet can be swapped without changing gameplay rules.

Initial custom tilesheets should use this atlas contract:

- BMP format loaded through SDL2 `SDL_LoadBMP`; no `SDL2_image` dependency yet.
- 24x24 source tiles, 7 columns by 2 rows.
- Row-major category order: darkness, floor, wall, door, up stairs, down stairs, trap, object, generic monster, automata, undead/demon, beast, humanoid, player.
- Runtime override for experiments: set `STEAMBAND_TOPDOWN_TILESET` to a compatible BMP path.
- Project placeholder regeneration: run `python tools/generate_topdown_tilesheet.py`. Use `--print-contract` to print the current slot order.

## Handheld UX Goals

- First-person HUD must be readable at 720p without relying on tiny text.
- Controller hints should stay short and visual; long help belongs in the legacy help/menu flow.
- Camera-relative movement and right-stick turn should be visually obvious through wall motion and depth cues.
- Debug overlays remain off by default so the prototype feels like a dungeon view, not a test harness.

## Candidate Asset Search Checklist

No external art candidates are approved or integrated yet. When sourcing begins, prefer CC0/Public Domain/MIT packs with separate wall/floor/sprite sheets, a limited 16/32-bit palette, and a source page that can be archived or cited durably in `ASSETS.md`.
