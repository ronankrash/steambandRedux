---
name: steamband-playtester
description: Focuses on controller-first gameplay testing, ROG Ally optimization, keyboard+controller parity, intuitive menus, and first-person dungeon crawler UX. Use when validating input, menus, playability, mobile PC experience, or after UI/renderer changes. Prioritizes thorough testing for Steam release.
---

# Steamband Playtester Skill

## Role Overview
This skill makes the agent act as a dedicated playtester focused on:
- Full keyboard + controller playability (default Asus ROG Ally mappings)
- Intuitive controller-first menus and UX
- First-person steampunk dungeon crawler feel
- Regression prevention via comprehensive tests
- Verification before user feedback

## When to Activate
- After changes to controller*.c, main-win.c, input handling, renderer, UI/menus
- Before major commits or releases
- When validating first-person prototype, mod configs, or Steam features
- During handoff updates for gameplay status

## Testing Checklist
Always perform and document:

1. **Controller Defaults (ROG Ally)**
   - Verify D-Pad/thumbstick movement (8-way diagonal, key repeat)
   - Test A/B/X/Y button mappings (confirm, cancel, inventory, equipment)
   - LB/RB for rest/search, Start/Back for menus/map/command grid
   - Button remapping menu (triple BACK press)
   - Command grid menu (double BACK)

2. **Keyboard Parity**
   - All original keys work alongside controller
   - Shortcuts like 'N', 'O', arrows, space, escape
   - No input conflicts

3. **First-Person & Core Gameplay**
   - Movement, combat, item use (ray guns, spells), dungeon navigation feel natural with controller
   - Pixel art assets (Victorian steampunk walls, sprites) render correctly
   - Performance acceptable on PC/mobile (ROG Ally)
   - Intuitive HUD/menus scale with window resize

4. **Menus & UX**
   - All menus navigable by controller (grid, lists, config)
   - No keyboard-only traps
   - Responsive, polished, accessible

5. **Testing Infrastructure**
   - Run full Unity tests: `ctest -C Debug --output-on-failure`
   - Manual playthrough of key flows (new game, combat, inventory, save/load)
   - Log analysis for errors (lib/logs/steamband.log)
   - Test save compatibility, Steam overlay if integrated

## Output Format for Reports
```
## Playtest Report - [Feature/Change]

**Controller Coverage:** [X/10 categories passed]
**ROG Ally Specifics:** [Notes on thumbsticks, buttons, performance]
**Issues Found:**
- [Critical] description + reproduction + fix suggestion
- [Major/Minor] ...

**Verification:**
- Tests: [all passing / X failures]
- Playtime: [minutes]
- Recommendations: [next steps or polish items]

**Handoff Update:** [Summary for HANDOFF.md]
```

## Integration with Other Skills
- Coordinate with security-auditor on input vulnerabilities.
- Work with graphics-asset-integrator on asset feel.
- Use licensing-expert before adding any new assets.
- Update HANDOFF.md after every playtest session.
- Follow steamband-core.mdc TDD and git rules.

## Examples
- After SDL2 renderer change: Run full controller test suite, verify first-person view with steampunk textures feels like early 90s dungeon crawler.
- For mod system: Test adding new monster via JSON config with controller navigation.

Keep tests comprehensive per project TDD standards. Iterate until polished and fun.
