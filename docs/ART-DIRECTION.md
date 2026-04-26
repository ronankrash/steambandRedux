# Art Direction Checklist

Last updated: 2026-04-26

## Presentation Target

Aim for a mid-to-late 90s first-person dungeon crawler read: chunky 4:3 composition, low-resolution wall texture rhythm, warm gaslight, oxidized metal, soot-dark stone, and clear UI gauges that work on a handheld screen. References such as later Wizardry-era crawlers, Lands of Lore-era readability, and Victorian/steampunk adventure art are style references only.

## Current No-Asset Renderer Goals

- Preserve the legacy 2D client as the fallback while the SDL2 first-person window remains a prototype.
- Keep texture loading disabled until `ASSETS.md` records verified commercial-permissive sources.
- Use deterministic procedural wall variation for now: masonry rows, plate seams, brass rivet highlights, distance fog, side shading, and a dark vignette.
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
- Font/UI set: readable bitmap-style UI at 720p, 1080p, and handheld scale.
- All assets must have durable source, author, license, and commercial-use notes in `ASSETS.md` before being loaded.

## Handheld UX Goals

- First-person HUD must be readable at 720p without relying on tiny text.
- Controller hints should stay short and visual; long help belongs in the legacy help/menu flow.
- Camera-relative movement and right-stick turn should be visually obvious through wall motion and depth cues.
- Debug overlays remain off by default so the prototype feels like a dungeon view, not a test harness.

## Candidate Asset Search Checklist

No external art candidates are approved or integrated yet. When sourcing begins, prefer CC0/Public Domain/MIT packs with separate wall/floor/sprite sheets, a limited 16/32-bit palette, and a source page that can be archived or cited durably in `ASSETS.md`.
