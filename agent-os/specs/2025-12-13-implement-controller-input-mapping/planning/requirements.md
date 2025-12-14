# Spec Requirements: Implement Controller Input Mapping

## Initial Description
Implement Controller Input Mapping — Map Xbox 360 controller buttons/triggers to game commands (movement, actions, menu navigation) and integrate with existing input system

## Requirements Discussion

### First Round Questions

**Q1:** I see there's already a basic button mapping structure in `controller.c` with some mappings (A, B, X, Y, D-Pad, Start, Back, Shoulders), but the mappings seem incomplete and some comments suggest uncertainty (e.g., "A -> Take off? No, usually Confirm/Enter/Fire"). Should we define a comprehensive, well-thought-out default mapping that covers all essential game commands (movement, inventory, actions, menu navigation), or should we refine the existing mappings incrementally?
**Answer:** Let's define a comprehensive well-thought-out default mapping.

**Q2:** For movement, I see the thumbstick handling currently only supports 4-way movement (up/down/left/right). Should we implement full 8-way diagonal movement (using numpad keys 1-9) for the left thumbstick, or is 4-way movement sufficient for this phase?
**Answer:** If we are able to support 8-way diagonal movement, let's do that.

**Q3:** The Xbox 360 controller has analog triggers (left and right). Should we map these to any game commands (e.g., running, casting spells, using items), or leave them unmapped for now?
**Answer:** Let's leave them unmapped for now, but usable for the future if needed.

**Q4:** For menu navigation, I assume the D-Pad should handle menu navigation (up/down/left/right), and A button should confirm/select, B button should cancel/back, similar to standard console game conventions. Is that correct, or should we use different buttons for menu navigation?
**Answer:** Those suggestions sound good.

**Q5:** I notice the current button mapping has `repeat_delay` and `repeat_rate` fields but they're not fully implemented (only D-Pad has values set). Should we implement key repeat functionality for buttons that are held down (useful for movement or repeated actions), or should buttons only trigger on press/release events?
**Answer:** Let's support key repeat functionality.

**Q6:** The current thumbstick movement has a 150ms rate limit to prevent spam. Should we make this configurable or adjust the timing, or is the current approach acceptable?
**Answer:** Let's leave as is for now.

**Q7:** For scope boundaries, I'm assuming this spec focuses on implementing a complete, well-designed default button mapping. The roadmap shows "Add Controller Configuration" (item #6) as a separate feature for allowing players to customize mappings. Should we keep configurability out of scope for this spec, or include basic configuration support?
**Answer:** Let's include basic configuration support. I also want to support custom community steam controllers (though I understand this may not be in scope here).

**Q8:** Are there any game commands or actions that should be explicitly excluded from controller mapping, or should we aim to map all commonly-used commands?
**Answer:** I want to map all commonly-used commands. If we don't have enough keys to map everything, we should implement some sort of menu system where we can group logical commands, and navigate through them via controller. There may be some menus in place already ... I know some versions of this game had helpful navigation for android phones for example... but I don't have an example of this to provide currently.

### Existing Code to Reference

**Similar Features Identified:**

**Menu System Pattern:**
- **Feature:** Command menu system organized by categories
- **Path:** `src/main-ami.c` (lines 444-523)
- **Pattern:** Commands are organized into categories (Cmd1, Cmd2, Cmd3, etc.) with menu items that map to key codes. Uses `NM_TITLE` and `NM_ITEM` structures. Commands are grouped logically (Inventory, Actions, Movement, Magic, etc.). This pattern could be adapted for a controller command menu system.

**Input Handling Pattern:**
- **Feature:** Controller input polling and button-to-key translation
- **Path:** `src/controller.c` (lines 127-189)
- **Pattern:** `controller_check()` polls controller state, detects button press/release events, and translates buttons to key codes via `Term_keypress()`. Has basic button mapping structure with `button_map_t` struct containing button, key_code, pressed state, and repeat timing fields.

**Command Processing Pattern:**
- **Feature:** Game command processing via switch statement
- **Path:** `src/dungeon.c` (lines 1183-1777)
- **Pattern:** `process_command()` uses a large switch statement to map character key codes to game commands (`do_cmd_*` functions). Commands are organized into logical groups (Inventory, Movement, Magic, Actions, etc.). This shows all available game commands and their key mappings.

**Rate Limiting Pattern:**
- **Feature:** Thumbstick movement rate limiting
- **Path:** `src/controller.c` (lines 84-122)
- **Pattern:** Uses `GetTickCount()` and static `last_move` variable to rate-limit thumbstick movement to 150ms intervals, preventing input spam.

### Visual Assets

**Files Provided:**
No visual assets provided.

**Visual Insights:**
No visual assets to analyze.

## Requirements Summary

### Functional Requirements
- **Comprehensive Default Button Mapping**: Create a well-designed default mapping that covers all essential game commands (movement, inventory, actions, menu navigation)
- **8-Way Diagonal Movement**: Implement full 8-way diagonal movement for left thumbstick using numpad keys (1-9)
- **Analog Triggers**: Leave analog triggers unmapped but ensure they're accessible for future use
- **Menu Navigation**: D-Pad for menu navigation (up/down/left/right), A button for confirm/select, B button for cancel/back
- **Key Repeat Functionality**: Implement key repeat for buttons that are held down (useful for movement and repeated actions)
- **Command Menu System**: Implement a grid menu system to group logical commands and navigate through them via controller (D-Pad navigation, A to select)
- **Basic Configuration Support**: Include both text config file (similar to `.prf` files) and in-game menu for button mappings. In-game menu updates config file. Supports both casual users (in-game menu) and power users (text file editing)
- **Menu Access**: Context-sensitive menu that appears when needed, with dedicated button as fallback for manual access
- **All Common Commands**: Map all commonly-used game commands to controller inputs

### Reusability Opportunities
- **Menu System**: Reference `src/main-ami.c` command menu structure for organizing commands into logical categories
- **Input Handling**: Build upon existing `controller_check()` and button mapping structure in `src/controller.c`
- **Command Processing**: Reference `src/dungeon.c` `process_command()` to understand all available game commands
- **Rate Limiting**: Reuse existing thumbstick rate limiting pattern (150ms) for button repeat functionality

### Scope Boundaries
**In Scope:**
- Comprehensive default button mapping for Xbox 360 controller
- 8-way diagonal movement support
- Key repeat functionality for held buttons
- Basic configuration support for button mappings
- Command menu system for accessing commands that don't fit on direct button mappings
- Integration with existing input system (`Term_keypress()`)

**Out of Scope:**
- Advanced configuration UI (deferred to roadmap item #6)
- Steam controller support (mentioned but may be deferred)
- Analog trigger mapping (deferred for future use)
- Configurable thumbstick rate limiting (deferred)
- Radial/pie menu or list menu formats (using grid menu only)

### Follow-up Questions

**Follow-up 1:** For the command menu system (when there aren't enough buttons), what format should it use? Radial/pie menu, list menu (scroll up/down), grid menu (navigate with D-Pad), or other?
**Answer:** For now let's go with grid menu.

**Follow-up 2:** For basic configuration support, what format should we use? Text config file (similar to `.prf` files), in-game menu (accessed via controller), or both?
**Answer:** Both -- I want simplicity for the casual user, but more configuration support for the power users as well. The in-game menu should update config file.

**Follow-up 3:** Should the command menu be accessible via a dedicated button (e.g., holding a button opens the menu), or should it be context-sensitive (e.g., appears automatically when needed)?
**Answer:** Context-sensitive... perhaps with a dedicated button in case context-sensitive isn't working for some reason?

### Technical Considerations
- **Integration Points**: Must integrate with existing `Term_keypress()` function for injecting keypresses into game input system
- **Existing Structure**: Build upon `button_map_t` structure and `controller_check()` function in `src/controller.c`
- **Menu System**: Create grid menu system for command selection, adapting existing menu patterns from `src/main-ami.c` for organization
- **Command Organization**: Reference command categories from `src/dungeon.c` and `src/main-ami.c` to organize commands logically into grid menu
- **Rate Limiting**: Use `GetTickCount()` pattern similar to existing thumbstick rate limiting
- **Analog Triggers**: Ensure trigger state is accessible via XInput API for future mapping, but don't implement mapping in this spec
- **Configuration Format**: Text config file (similar to `.prf` files) that can be edited manually and updated by in-game menu
- **Menu Access**: Context-sensitive menu that appears when needed, with dedicated button as fallback for manual access

