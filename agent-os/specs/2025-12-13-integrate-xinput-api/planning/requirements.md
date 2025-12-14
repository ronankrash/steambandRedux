# Spec Requirements: Integrate XInput API

## Initial Description
Integrate XInput API — Add XInput library linking, implement controller detection, and create controller state polling mechanism in Windows message loop

## Requirements Discussion

### First Round Questions

**Q1:** I see there's already a `controller.c` and `controller.h` file with basic XInput implementation, and `controller_check()` is being called in the message loop. However, `controller_init()` doesn't appear to be called in `WinMain`. Should we add proper initialization in `WinMain` (similar to how logging is initialized), or is the current implementation sufficient?
**Answer:** Add proper initialization.

**Q2:** The XInput library is already linked in `CMakeLists.txt` (`xinput`). Should we verify the linking is correct and ensure it works with the Windows SDK version being used, or assume it's already working?
**Answer:** Verify the linking is correct.

**Q3:** For controller detection, I see the code checks `XInputGetState(0, &state)` for controller 0. Should we support multiple controllers (0-3), or is single controller support (controller 0) sufficient for this phase?
**Answer:** Single controller support is fine for now.

**Q4:** The current polling happens in `Term_xtra_win_clear()` which is called from the message loop. Should we ensure polling happens at a consistent rate (e.g., every frame or with a specific frequency), or is the current approach acceptable?
**Answer:** I don't know. Whatever is best practice.

**Q5:** I notice the controller code has basic button mappings and thumbstick handling. Should this spec focus only on the integration aspects (initialization, detection, polling), leaving button mapping implementation for the next roadmap item ("Implement Controller Input Mapping"), or should we enhance the existing mappings as part of this integration?
**Answer:** Let's leave button mapping implementation for the next roadmap item.

**Q6:** For error handling, should we log controller connection/disconnection events using the logging system, or is silent handling (just checking connection status) acceptable?
**Answer:** Let's use the logging system... with an option to silence it if desired.

**Q7:** Should we add any configuration options (e.g., enable/disable controller support, polling frequency) at this stage, or keep it simple and add configuration later?
**Answer:** Let's keep it simple and add configuration later.

### Existing Code to Reference

**Similar Features Identified:**

**Initialization Pattern:**
- **Feature:** Logging system initialization
- **Path:** `src/main-win.c` (lines 5082-5136)
- **Pattern:** Early initialization in `WinMain`, right after setting up temporary hooks (`plog_aux`, `quit_aux`, `core_aux`) and before `init_stuff()`. Uses `LOG_I()` for initialization messages. Follows pattern: check for errors, log initialization, set up system.

**Error Handling Pattern:**
- **Feature:** Windows API error handling
- **Path:** `src/logging.c`, `src/main-win.c`
- **Pattern:** Check return values from Windows API calls (e.g., `GetModuleFileName`, `CreateMutex`). Use logging macros (`LOG_I`, `LOG_W`, `LOG_E`, `LOG_F`) for different severity levels. Continue gracefully on non-fatal errors.

**Polling Mechanism:**
- **Feature:** Controller polling in message loop
- **Path:** `src/main-win.c` (lines 1903-1942)
- **Pattern:** `controller_check()` is called in `Term_xtra_win_event()` which is part of the message loop. Called both when waiting (`v=1`) and when checking (`v=0`). Already has rate limiting for thumbstick movement (150ms delay via `GetTickCount()`).

**Library Linking:**
- **Feature:** XInput library linking
- **Path:** `CMakeLists.txt` (line 74)
- **Pattern:** XInput is already linked as `xinput` in the `LIBS` variable. Should verify this works correctly with Windows SDK.

### Follow-up Questions

None required - all questions answered comprehensively.

1. I see there's already a `controller.c` and `controller.h` file with basic XInput implementation, and `controller_check()` is being called in the message loop. However, `controller_init()` doesn't appear to be called in `WinMain`. Should we add proper initialization in `WinMain` (similar to how logging is initialized), or is the current implementation sufficient?

2. The XInput library is already linked in `CMakeLists.txt` (`xinput`). Should we verify the linking is correct and ensure it works with the Windows SDK version being used, or assume it's already working?

3. For controller detection, I see the code checks `XInputGetState(0, &state)` for controller 0. Should we support multiple controllers (0-3), or is single controller support (controller 0) sufficient for this phase?

4. The current polling happens in `Term_xtra_win_clear()` which is called from the message loop. Should we ensure polling happens at a consistent rate (e.g., every frame or with a specific frequency), or is the current approach acceptable?

5. I notice the controller code has basic button mappings and thumbstick handling. Should this spec focus only on the integration aspects (initialization, detection, polling), leaving button mapping implementation for the next roadmap item ("Implement Controller Input Mapping"), or should we enhance the existing mappings as part of this integration?

6. For error handling, should we log controller connection/disconnection events using the logging system, or is silent handling (just checking connection status) acceptable?

7. Should we add any configuration options (e.g., enable/disable controller support, polling frequency) at this stage, or keep it simple and add configuration later?

**Existing Code Reuse:**
Are there existing features in your codebase with similar patterns we should reference? For example:
- Similar initialization patterns (like logging initialization in WinMain)
- Input handling patterns from keyboard/mouse code
- Polling mechanisms in the message loop
- Error handling patterns for Windows APIs

Please provide file/folder paths or names of these features if they exist.

**Visual Assets Request:**
Do you have any design mockups, wireframes, or screenshots that could help guide the development?

If yes, please place them in: `agent-os/specs/2025-12-13-integrate-xinput-api/planning/visuals/`

Use descriptive file names like:
- controller-integration-diagram.png
- message-loop-flow.png
- xinput-polling-timing.png

## Visual Assets

### Files Provided:
No visual files found.

### Visual Insights:
No visual assets provided.

## Requirements Summary

### Functional Requirements
- **Add proper controller initialization** in `WinMain` following the logging initialization pattern (early initialization, before `init_stuff()`)
- **Verify XInput library linking** in CMakeLists.txt works correctly with Windows SDK
- **Implement single controller support** (controller 0 only) for this phase
- **Ensure proper polling mechanism** in message loop (already exists in `Term_xtra_win_event()`, verify best practices)
- **Add logging for controller events** (connection/disconnection) with option to silence if desired
- **Keep implementation simple** - no configuration options at this stage (deferred to future roadmap items)
- **Focus on integration only** - button mapping implementation deferred to next roadmap item

### Reusability Opportunities
- **Initialization pattern:** Follow logging system initialization pattern in `WinMain` (lines 5082-5136)
- **Error handling:** Use existing logging macros (`LOG_I`, `LOG_W`, `LOG_E`) for controller events
- **Polling mechanism:** Current `controller_check()` calls in `Term_xtra_win_event()` are appropriate
- **Library linking:** Verify existing XInput linking in CMakeLists.txt

### Scope Boundaries
**In Scope:**
- Add `controller_init()` call in `WinMain` following initialization pattern
- Verify XInput library linking works correctly
- Ensure controller detection works (single controller, controller 0)
- Add logging for controller connection/disconnection events
- Verify polling mechanism follows best practices
- Add option to silence controller logging if desired

**Out of Scope:**
- Button mapping implementation (deferred to next roadmap item)
- Multiple controller support (single controller only for now)
- Configuration options (keep simple, add later)
- Button mapping enhancements

### Technical Considerations
- **Initialization location:** In `WinMain`, after logging initialization, before `init_stuff()` (around line 5136)
- **Logging integration:** Use existing logging system with `LOG_I()` for info, `LOG_W()` for warnings
- **Silence option:** Add a flag or environment variable to disable controller logging (similar to log level)
- **Polling frequency:** Current implementation calls `controller_check()` every time `Term_xtra_win_event()` is called, which is reasonable. Verify this doesn't cause performance issues.
- **Library verification:** Check that `xinput` library links correctly and XInput API functions are available
- **Error handling:** Check `XInputGetState()` return values and log appropriately
- **Best practices:** Follow Windows API error handling patterns used elsewhere in codebase

