# Input and Handheld UI

Target devices: Windows handheld PCs (Asus ROG Ally class) at 1280×720 and 1920×1080. Minimum readable UI font ~14–16px.

## Controller mapping (initial)

| Input | Action |
|-------|--------|
| Left stick / D-pad | 8-way move / menu navigate |
| A | Confirm / interact / contextual melee |
| B | Cancel / close |
| X | Use quick item (machine oil) |
| Y | Inventory |
| LB / RB | Cycle ranged targets |
| LT | Inspect mode |
| RT | Ranged targeting mode |
| Start (Menu) | Pause / system |
| Back (View) | Character sheet |
| Right stick | Reserved (camera inspect / fine target) |

Mouse is never required for normal play. Keyboard remains complete (WASD/arrows, Space, F, Q, I, C, Esc).

## Prompt policy

`GameServices.last_input_device` tracks keyboard vs controller. HUD prompts switch glyphs/labels accordingly.

## Layout constraints

- Status bar top, prompt bar bottom, combat log docked right.
- Menu panels centered with large selectable rows for thumb reach.
- Critical text must remain legible when the window is 1280×720 on a ~7" panel.
