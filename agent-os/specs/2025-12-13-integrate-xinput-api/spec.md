# Specification: Integrate XInput API

## Goal
Properly integrate XInput API for Xbox 360 controller support by adding initialization in WinMain, verifying library linking, ensuring robust controller detection and polling, and adding logging for connection events.

## User Stories
- As a developer, I want controller initialization to happen early in the application startup so that controller support is available throughout the game session
- As a player, I want the game to detect when I connect or disconnect my Xbox controller so that I know the controller is working properly
- As a developer, I want controller connection/disconnection events logged so that I can troubleshoot controller issues during development

## Specific Requirements

**Controller Initialization**
- Add `controller_init()` call in `WinMain` following the logging initialization pattern
- Place initialization after logging setup (around line 5136) and before `init_stuff()`
- Use `LOG_I()` to log successful initialization or controller connection status
- Handle initialization failures gracefully without crashing the application
- Follow the same error handling pattern used for logging initialization

**XInput Library Linking Verification**
- Verify that `xinput` library is correctly linked in `CMakeLists.txt` (already present in `LIBS` variable)
- Ensure XInput API functions (`XInputGetState`, `XINPUT_STATE`, etc.) are available and compile correctly
- Test that linking works with the Windows SDK version being used (10.0.22621.0+)
- Remove redundant `#pragma comment(lib, "xinput.lib")` from `controller.c` if CMake linking is sufficient
- Verify build succeeds and XInput symbols resolve correctly

**Controller Detection**
- Support single controller only (controller 0) for this phase
- Detect controller connection status during initialization via `XInputGetState(0, &state)`
- Log controller connection status at initialization (connected or not connected)
- Handle `ERROR_DEVICE_NOT_CONNECTED` error code appropriately
- Update connection status during polling in `controller_check()` when state changes

**Polling Mechanism**
- Ensure `controller_check()` is called in `Term_xtra_win_event()` message loop handler (already implemented)
- Verify polling frequency is appropriate and doesn't cause performance issues
- Maintain existing rate limiting for thumbstick movement (150ms delay via `GetTickCount()`)
- Ensure polling happens consistently in both waiting (`v=1`) and checking (`v=0`) modes
- Follow best practices for Windows message loop polling (current implementation appears correct)

**Controller Event Logging**
- Log controller connection events using `LOG_I()` when controller connects
- Log controller disconnection events using `LOG_W()` when controller disconnects
- Add option to silence controller logging via environment variable (e.g., `STEAMBAND_CONTROLLER_LOG=0`)
- Log only state changes (connection/disconnection) to avoid log spam
- Use existing logging macros (`LOG_I`, `LOG_W`) following established patterns

**Error Handling**
- Check return values from `XInputGetState()` calls and handle errors appropriately
- Log errors using appropriate log levels (`LOG_W` for warnings, `LOG_E` for errors)
- Continue gracefully when controller is not connected (non-fatal error)
- Follow Windows API error handling patterns used elsewhere in codebase
- Avoid logging errors repeatedly if controller remains disconnected

## Visual Design
No visual assets provided.

## Existing Code to Leverage

**controller.c and controller.h**
- Existing XInput implementation with `controller_init()` and `controller_check()` functions
- Controller state management with `XINPUT_STATE` structure and connection tracking
- Button mapping structure (to be enhanced in future roadmap item)
- Thumbstick handling with deadzone checking and rate limiting
- Use existing functions as foundation, enhance with proper initialization and logging

**Logging System Initialization Pattern**
- Follow initialization pattern from `src/main-win.c` (lines 5082-5136)
- Early initialization in `WinMain` after temporary hooks setup
- Use `LOG_I()` for initialization messages
- Check for errors and log appropriately
- Place controller initialization after logging initialization, before `init_stuff()`

**Windows API Error Handling Pattern**
- Follow error handling patterns from `src/logging.c` and `src/main-win.c`
- Check return values from Windows API calls
- Use logging macros (`LOG_I`, `LOG_W`, `LOG_E`) for different severity levels
- Continue gracefully on non-fatal errors
- Use `ERROR_SUCCESS` and `ERROR_DEVICE_NOT_CONNECTED` error codes appropriately

**Message Loop Polling Pattern**
- Existing polling in `Term_xtra_win_event()` (lines 1903-1942) is appropriate
- `controller_check()` is called in both waiting and checking modes
- Rate limiting already implemented for thumbstick movement
- Maintain current polling frequency and mechanism

**CMake Library Linking**
- XInput already linked in `CMakeLists.txt` as `xinput` in `LIBS` variable
- Verify linking works correctly and remove redundant pragma comments if needed
- Ensure compatibility with Windows SDK version

## Out of Scope
- Button mapping implementation and enhancements (deferred to next roadmap item "Implement Controller Input Mapping")
- Multiple controller support (controller 1-3) - single controller only for this phase
- Configuration options for controller settings (enable/disable, polling frequency) - keep simple for now
- Custom button mapping UI or configuration files
- Controller vibration/rumble support
- Controller battery level monitoring
- Advanced thumbstick deadzone configuration
- Controller input calibration or sensitivity settings
- Support for non-Xbox controllers via DirectInput or other APIs
- Controller hotplugging detection beyond basic connection status

