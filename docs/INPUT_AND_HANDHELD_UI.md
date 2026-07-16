# Input and Handheld UI

Target devices: Windows handheld PCs (Asus ROG Ally class) at 1280×720 and 1920×1080. Minimum readable UI font ~14–18px (adjustable in pause menu).

## Controller mapping

| Input | Action |
|-------|--------|
| Left stick / D-pad | 8-way move / menu navigate |
| A | Confirm / interact / contextual melee |
| B | Cancel / close |
| X | Use quick item |
| Y | Inventory / equipment |
| LB / RB | Cycle ranged targets |
| LT | Inspect tile context |
| RT | Ranged targeting mode |
| Start (Menu) | Pause / save / anim+text settings |
| Back (View) | Character sheet / skills entry |
| Right stick | Camera peek |

Mouse is never required for normal play. Keyboard remains complete (WASD/arrows, Space, F, Q, I, C, Esc).

## Prompt policy

`GameServices.last_input_device` tracks keyboard vs controller. HUD prompts switch labels accordingly.

## Layout constraints

- Status bar + equipment HUD top, prompt bar bottom, combat log docked right.
- Centered menu panels with large selectable rows; focus always restored on open.
- Pause menu exposes animation speed (normal/fast) and text size.
- Critical flows (create, inventory, skills, craft, merchant) are pad-navigable without hover.
