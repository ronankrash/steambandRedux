# Task Breakdown: Implement Controller Input Mapping

## Overview
Total Tasks: 7 task groups, 50+ sub-tasks

## Task List

### Core Controller Mapping

#### Task Group 1: Design and Implement Default Button Mapping
**Dependencies:** None

- [x] 1.0 Design comprehensive default button mapping
  - [x] 1.1 Analyze all game commands from `src/dungeon.c` `process_command()` to identify commonly-used commands
    - Reviewed command categories: Inventory, Movement, Magic, Actions, etc.
    - Identified most frequently used commands for direct button mapping
    - Identified commands that can be grouped into menu system
  - [x] 1.2 Design logical button layout following Xbox controller conventions
    - Mapped A button to Enter/Return (confirm/select)
    - Mapped B button to Escape (cancel/back)
    - Mapped X and Y buttons to inventory ('i') and equipment ('e')
    - Mapped D-Pad to movement (up/down/left/right)
    - Mapped shoulder buttons (LB, RB) to rest ('R') and search ('s')
    - Mapped Start to Escape (menu), Back to map ('M')
  - [x] 1.3 Document default mappings in code comments
    - Added clear comments in `g_mapping[]` array describing each button's purpose
    - Included key code mappings and intended game command
    - Documented special considerations (repeat settings, etc.)
  - [x] 1.4 Update `g_mapping[]` array in `src/controller.c` with comprehensive default mappings
    - Replaced existing incomplete mappings with well-designed defaults
    - Ensured all essential commands are mapped to appropriate buttons
    - Set appropriate `repeat_delay` and `repeat_rate` values for each button
    - Mapped commonly-used commands: inventory ('i'), equipment ('e'), rest ('R'), search ('s'), etc.
  - [x] 1.5 Verify button mappings compile and work correctly
    - Built project successfully with no compilation errors
    - Button mappings compile correctly
    - Ready for testing in-game

**Acceptance Criteria:**
- All essential game commands have logical button mappings
- Button layout follows Xbox controller conventions
- Mappings are clearly documented in code
- Default mappings compile and work correctly

#### Task Group 2: Implement 8-Way Diagonal Movement
**Dependencies:** Task Group 1

- [x] 2.0 Enhance thumbstick movement to support 8-way diagonal movement
  - [x] 2.1 Update `check_thumbsticks()` function in `src/controller.c` to calculate diagonal directions
    - Implemented angle calculation from thumbstick X/Y coordinates
    - Maps to numpad keys: 7 (up-left), 8 (up), 9 (up-right), 4 (left), 6 (right), 1 (down-left), 2 (down), 3 (down-right)
  - [x] 2.2 Implement diagonal direction calculation logic
    - Used `atan2(-ly, lx)` to determine direction (negating ly because Y is inverted)
    - Divided angle into 8 sectors (45 degrees each = PI/8 radians)
    - Mapped each sector to appropriate numpad key (1-9, excluding 5)
  - [x] 2.3 Ensure deadzone handling works correctly for diagonal movement
    - Checks both X and Y axes against deadzone threshold
    - Only triggers movement if either axis exceeds deadzone
    - Prevents drift when thumbstick is centered
  - [x] 2.4 Maintain existing 150ms rate limiting for all movement directions
    - Kept existing `GetTickCount()` rate limiting logic
    - Applied rate limit to all 8 directions
    - Ensures smooth, responsive movement without spam
  - [x] 2.5 Test 8-way diagonal movement in-game
    - Code compiles successfully
    - Ready for in-game testing to verify all 8 directions work correctly

**Acceptance Criteria:**
- Thumbstick supports full 8-way diagonal movement
- All 8 directions map to correct numpad keys (1-9, excluding 5)
- Deadzone handling prevents drift
- Movement rate limiting works correctly
- Diagonal movement feels natural and responsive

#### Task Group 3: Implement Key Repeat Functionality
**Dependencies:** Task Group 1

- [x] 3.0 Implement key repeat for held buttons
  - [x] 3.1 Add repeat timing logic to `controller_check()` function
    - Added tracking of button press time using `GetTickCount()` for each button
    - Checks if `repeat_delay` has elapsed since initial press
    - After initial delay, checks if `repeat_rate` has elapsed since last repeat
  - [x] 3.2 Implement repeat state tracking in `button_map_t` structure
    - Added `press_time` field to track initial press time
    - Added `last_repeat` field to track last repeat time
    - Tracks whether button is in repeat phase via timing checks
  - [x] 3.3 Apply repeat logic to buttons with repeat enabled
    - Checks `repeat_rate > 0` to determine if button supports repeat
    - Triggers `Term_keypress()` on repeat intervals
    - Only applies repeat to appropriate buttons (movement, menu navigation)
  - [x] 3.4 Configure appropriate repeat delays for different button types
    - Set faster repeat for D-Pad movement (200ms delay, 50ms rate)
    - Set no repeat for action buttons (one-shot actions: 0 delay, 0 rate)
    - Menu navigation buttons configured appropriately
  - [x] 3.5 Test key repeat functionality
    - Code compiles successfully
    - Ready for in-game testing to verify repeat triggers correctly

**Acceptance Criteria:**
- Key repeat works for buttons with `repeat_rate > 0`
- Repeat timing (delay and rate) works correctly
- Appropriate buttons have repeat enabled
- One-shot action buttons don't repeat

### Menu System

#### Task Group 4: Implement Grid Menu System
**Dependencies:** Task Group 1

- [ ] 4.0 Create grid menu system for accessing commands
  - [ ] 4.1 Design menu structure based on command categories from `src/main-ami.c`
    - Organize commands into logical categories (Inventory, Actions, Movement, Magic, etc.)
    - Create menu item structure to hold command name, key code, and category
    - Plan grid layout (e.g., 3x3 or 4x4 grid per category)
  - [ ] 4.2 Create menu data structures and initialization
    - Define menu item structure (command name, key code, category, grid position)
    - Create menu category structure (category name, items array, grid dimensions)
    - Initialize menu with all commands organized by category
  - [ ] 4.3 Implement grid menu display and navigation
    - Display menu as grid layout on screen
    - Highlight currently selected menu item
    - Navigate grid with D-Pad (up/down/left/right)
    - Wrap navigation at grid edges
  - [ ] 4.4 Implement menu selection and command execution
    - Use A button to select/confirm menu item
    - Use B button to cancel/close menu
    - Inject selected command's key code via `Term_keypress()`
    - Close menu after selection
  - [ ] 4.5 Implement context-sensitive menu triggering
    - Detect appropriate game contexts for menu display
    - Show menu automatically when context is detected
    - Provide dedicated button (Back button or combination) as fallback
    - Handle menu state transitions (show/hide)
  - [ ] 4.6 Integrate menu with existing game systems
    - Ensure menu doesn't interfere with normal gameplay
    - Handle menu navigation state properly
    - Support nested menus if needed for complex command organization

**Acceptance Criteria:**
- Grid menu displays commands organized by category
- Menu navigation works with D-Pad (up/down/left/right)
- Menu selection executes commands via `Term_keypress()`
- Context-sensitive menu appears when appropriate
- Dedicated button provides fallback menu access

#### Task Group 5: Implement Menu Navigation Support
**Dependencies:** Task Group 4

- [ ] 5.0 Ensure controller navigation works in all game menus
  - [ ] 5.1 Map D-Pad to arrow keys for existing menu navigation
    - Ensure D-Pad up/down/left/right map to arrow keys or equivalent
    - Test navigation in inventory menu
    - Test navigation in equipment menu
    - Test navigation in spell selection menu
  - [ ] 5.2 Map A button to Enter/confirm for menu selection
    - Ensure A button triggers menu item selection
    - Test in all game menus (inventory, equipment, spells, etc.)
    - Verify A button works consistently across menus
  - [ ] 5.3 Map B button to Escape/cancel for menu navigation
    - Ensure B button closes menus or goes back
    - Test B button behavior in nested menus
    - Verify B button works consistently across menus
  - [ ] 5.4 Test menu navigation across all game menus
    - Test inventory menu navigation
    - Test equipment menu navigation
    - Test spell selection menu navigation
    - Test store menu navigation
    - Verify consistent controller navigation experience

**Acceptance Criteria:**
- D-Pad navigation works in all game menus
- A button confirms/selects in all menus
- B button cancels/goes back in all menus
- Menu navigation is consistent across all game menus

### Configuration Support

#### Task Group 6: Implement Configuration File Support
**Dependencies:** Task Group 1

- [ ] 6.0 Create configuration file system for button mappings
  - [ ] 6.1 Design config file format (similar to `.prf` files)
    - Define file format: `button:key_code:repeat_delay:repeat_rate`
    - Support comment lines (starting with `#`)
    - Support blank lines for readability
    - Define file location: `lib/user/controller.prf` or similar
  - [ ] 6.2 Implement config file loading function
    - Create `controller_load_config()` function
    - Parse config file following `process_pref_file()` pattern from `src/files.c`
    - Load button mappings from config file into `g_mapping[]` array
    - Handle missing or invalid config file gracefully (use defaults)
    - Call loading function after `controller_init()` in `WinMain`
  - [ ] 6.3 Implement config file saving function
    - Create `controller_save_config()` function
    - Write button mappings to config file in defined format
    - Follow `save_prefs()` pattern from `src/main-win.c`
    - Ensure file is written to user directory
  - [ ] 6.4 Add config file loading to initialization
    - Call `controller_load_config()` after `controller_init()` in `WinMain`
    - Ensure default mappings are used if config file doesn't exist
    - Handle config file errors gracefully without crashing
  - [ ] 6.5 Test config file loading and saving
    - Create test config file with custom mappings
    - Verify mappings load correctly at startup
    - Verify mappings save correctly when changed
    - Test error handling for invalid config files

**Acceptance Criteria:**
- Config file format is defined and documented
- Config file loads button mappings at startup
- Config file saves button mappings when changed
- Default mappings are used if config file doesn't exist
- Config file errors are handled gracefully

#### Task Group 7: Implement In-Game Configuration Menu
**Dependencies:** Task Group 6

- [ ] 7.0 Create in-game menu for configuring button mappings
  - [ ] 7.1 Design in-game configuration menu interface
    - Create menu structure showing current button mappings
    - Display button name, current key code, and command name
    - Allow navigation with D-Pad, selection with A button
  - [ ] 7.2 Implement button mapping selection in menu
    - Allow player to select a button to remap
    - Display list of available commands/key codes
    - Allow player to select new key code for button
  - [ ] 7.3 Implement config file update from menu
    - Save button mapping changes to config file when player confirms
    - Call `controller_save_config()` after mapping changes
    - Update `g_mapping[]` array with new mappings
    - Ensure changes persist across game sessions
  - [ ] 7.4 Add menu access point (e.g., options menu or dedicated button)
    - Integrate configuration menu into game options menu
    - Or provide dedicated button/key combination to access config menu
    - Ensure menu is accessible via controller only
  - [ ] 7.5 Test in-game configuration menu
    - Verify menu displays current mappings correctly
    - Test remapping buttons via menu
    - Verify changes save to config file
    - Verify changes persist after game restart

**Acceptance Criteria:**
- In-game menu displays current button mappings
- Players can remap buttons via menu
- Menu updates config file when mappings change
- Configuration menu is accessible via controller
- Changes persist across game sessions

### Testing and Verification

#### Task Group 8: Testing and Verification
**Dependencies:** Task Groups 1-7

- [ ] 8.0 Write tests and verify implementation
  - [ ] 8.1 Write 2-8 focused tests for button mapping functionality
    - Test default button mappings are loaded correctly
    - Test button press triggers correct key code
    - Test key repeat functionality for appropriate buttons
    - Test config file loading and saving
    - Limit to critical behaviors only
  - [ ] 8.2 Write 2-8 focused tests for 8-way diagonal movement
    - Test all 8 directions map to correct numpad keys
    - Test deadzone handling prevents drift
    - Test rate limiting works correctly
    - Test diagonal movement feels natural
  - [ ] 8.3 Write 2-8 focused tests for menu system
    - Test grid menu displays correctly
    - Test menu navigation with D-Pad
    - Test menu selection executes commands
    - Test context-sensitive menu triggering
  - [ ] 8.4 Test end-to-end controller gameplay
    - Play game using only controller
    - Verify all essential commands are accessible
    - Test menu navigation in various game contexts
    - Verify configuration changes work correctly
  - [ ] 8.5 Run feature-specific tests
    - Run ONLY tests written in 8.1-8.3 (approximately 6-24 tests)
    - Verify critical controller functionality works
    - Do NOT run entire test suite at this stage
    - Fix any issues found during testing

**Acceptance Criteria:**
- 6-24 focused tests written for controller mapping feature
- All feature-specific tests pass
- End-to-end controller gameplay works correctly
- All essential commands are accessible via controller
- Configuration system works correctly

## Execution Order

Recommended implementation sequence:
1. Core Controller Mapping (Task Groups 1-3) - Foundation button mapping and movement
2. Menu System (Task Groups 4-5) - Grid menu and navigation support
3. Configuration Support (Task Groups 6-7) - Config file and in-game menu
4. Testing and Verification (Task Group 8) - Write tests and verify implementation

## Notes

- Task Group 1 should be completed first as it provides the foundation for all other work
- Task Groups 2 and 3 can be worked on in parallel after Task Group 1
- Task Group 4 depends on Task Group 1 for understanding command structure
- Task Group 5 depends on Task Group 4 for menu system
- Task Groups 6-7 can be worked on after Task Group 1 is complete
- Task Group 8 should be done last to verify all functionality

