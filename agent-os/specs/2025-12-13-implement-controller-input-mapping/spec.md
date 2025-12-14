# Specification: Implement Controller Input Mapping

## Goal
Create a comprehensive, well-designed default button mapping system for Xbox 360 controllers that maps all commonly-used game commands to controller inputs, implements 8-way diagonal movement, key repeat functionality, and a grid menu system for accessing commands that don't fit on direct button mappings, with basic configuration support via both text config file and in-game menu.

## User Stories
- As a player, I want to play the game using only an Xbox 360 controller so that I can enjoy comfortable couch gaming without needing a keyboard
- As a player, I want to access all game commands via controller so that I don't need to switch between controller and keyboard during gameplay
- As a player, I want to customize my controller button mappings so that I can set up controls that feel natural to me

## Specific Requirements

**Comprehensive Default Button Mapping**
- Design a logical default mapping covering all essential game commands (movement, inventory, actions, menu navigation)
- Map primary actions to face buttons (A, B, X, Y) following console game conventions (A=confirm/select, B=cancel/back)
- Map movement to D-Pad and left thumbstick (8-way diagonal movement)
- Map secondary actions to shoulder buttons (LB, RB) and Start/Back buttons
- Organize mappings logically: most frequently used commands on easily accessible buttons
- Ensure mappings follow standard Xbox controller conventions where possible
- Document default mappings clearly in code comments and configuration file
- Support all commonly-used game commands identified in `src/dungeon.c` `process_command()`

**8-Way Diagonal Movement**
- Implement full 8-way diagonal movement for left thumbstick using numpad keys (1-9)
- Calculate movement direction based on thumbstick angle and magnitude
- Map diagonal directions: 7 (up-left), 8 (up), 9 (up-right), 4 (left), 6 (right), 1 (down-left), 2 (down), 3 (down-right)
- Use deadzone threshold (`XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE`) to prevent drift
- Maintain existing 150ms rate limiting to prevent movement spam
- Prioritize dominant axis when calculating diagonal direction (similar to existing `check_thumbsticks()` logic)
- Ensure smooth diagonal movement feels natural and responsive

**Key Repeat Functionality**
- Implement key repeat for buttons that are held down (useful for movement and repeated actions)
- Use `repeat_delay` (initial delay before repeat starts) and `repeat_rate` (delay between repeats) fields in `button_map_t` structure
- Apply repeat logic in `controller_check()` when button is held down (`current_pressed && g_mapping[i].pressed`)
- Use `GetTickCount()` pattern similar to existing thumbstick rate limiting
- Configure appropriate repeat delays for different button types (e.g., D-Pad movement may need faster repeat than action buttons)
- Only apply repeat to buttons where it makes sense (movement, menu navigation, not one-shot actions)

**Grid Menu System**
- Create grid menu system for accessing commands that don't fit on direct button mappings
- Organize commands into logical categories (Inventory, Actions, Movement, Magic, etc.) based on `src/main-ami.c` menu structure
- Display menu as grid layout navigable with D-Pad (up/down/left/right)
- Use A button to select/confirm menu item, B button to cancel/back
- Make menu context-sensitive (appears automatically when needed) with dedicated button as fallback
- Integrate menu selection with existing `Term_keypress()` system to inject key codes
- Ensure menu integrates seamlessly with existing game menu systems
- Support nested menus if needed for complex command organization

**Menu Navigation**
- Use D-Pad for menu navigation (up/down/left/right) in all game menus
- Use A button for confirm/select actions in menus
- Use B button for cancel/back actions in menus
- Ensure menu navigation works consistently across all game menus (inventory, equipment, spell selection, etc.)
- Support both grid menu system and existing game menus with same controller navigation
- Handle menu navigation state properly to prevent conflicts with gameplay controls

**Basic Configuration Support**
- Create text config file format (similar to `.prf` files) for storing button mappings
- Store config file in user directory (e.g., `lib/user/controller.prf` or similar)
- Support manual editing of config file for power users
- Create in-game menu for configuring button mappings accessible via controller
- Ensure in-game menu updates config file when mappings are changed
- Load button mappings from config file at startup (after `controller_init()`)
- Save button mappings to config file when changed via in-game menu
- Follow existing preference file patterns from `src/main-win.c` (`load_prefs()`, `save_prefs()`) and `src/files.c` (`process_pref_file()`)
- Support default mappings if config file doesn't exist or is invalid

**Analog Triggers Access**
- Ensure analog trigger state is accessible via XInput API (`g_state.Gamepad.bLeftTrigger`, `g_state.Gamepad.bRightTrigger`)
- Leave triggers unmapped in default configuration
- Structure code to allow easy future mapping of triggers without major refactoring
- Document trigger access in code comments for future implementation

**Context-Sensitive Menu Access**
- Implement context-sensitive menu that appears automatically when appropriate game contexts are detected
- Provide dedicated button (e.g., Back button or combination) as fallback for manual menu access
- Ensure menu doesn't interfere with normal gameplay when not needed
- Handle menu state transitions properly (show/hide, navigation, selection)

## Visual Design
No visual assets provided.

## Existing Code to Leverage

**Controller Input Handling (`src/controller.c`)**
- Build upon existing `button_map_t` structure with button, key_code, pressed state, and repeat timing fields
- Extend `controller_check()` function to handle key repeat logic and improved button mapping
- Reuse existing `Term_keypress()` integration pattern for injecting keypresses into game input system
- Maintain existing thumbstick rate limiting pattern (150ms delay via `GetTickCount()`)
- Keep existing controller state management (`g_state`, `g_connected`, `g_last_packet`)

**Command Processing (`src/dungeon.c`)**
- Reference `process_command()` switch statement to identify all available game commands and their key mappings
- Use command organization patterns (Inventory, Movement, Magic, Actions, etc.) for menu categorization
- Ensure controller mappings align with existing command structure

**Menu System Pattern (`src/main-ami.c`)**
- Adapt command menu structure from `src/main-ami.c` (lines 444-523) for organizing commands into logical categories
- Reference `NM_TITLE` and `NM_ITEM` structures as inspiration for grid menu organization
- Use command grouping patterns (Cmd1, Cmd2, Cmd3, etc.) for menu categories

**Preference File Handling (`src/main-win.c`, `src/files.c`)**
- Follow existing preference file loading/saving patterns from `load_prefs()` and `save_prefs()` functions
- Use `process_pref_file()` pattern from `src/files.c` for parsing config file
- Store config file in user directory following existing preference file conventions
- Support comment lines (starting with `#`) and blank lines in config file format

**Rate Limiting Pattern (`src/controller.c`)**
- Reuse `GetTickCount()` and static timer variable pattern for key repeat functionality
- Apply similar rate limiting logic used for thumbstick movement to button repeat

## Out of Scope
- Advanced configuration UI with visual button mapping editor (deferred to roadmap item #6)
- Steam controller support (mentioned but deferred for future implementation)
- Analog trigger mapping to game commands (deferred for future use, but triggers must be accessible)
- Configurable thumbstick rate limiting (keeping existing 150ms fixed rate)
- Radial/pie menu or list menu formats (using grid menu only)
- Multiple controller support (sticking with single controller, port 0)
- Controller vibration/rumble feedback
- Custom button combinations or macros (single button mappings only)
- Context-sensitive menu detection for all possible game states (basic implementation only)
- Menu system for non-controller-specific game menus (focusing on controller command menu)

