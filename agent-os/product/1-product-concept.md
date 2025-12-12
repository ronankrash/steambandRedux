# SteambandRedux Product Concept

## Product Idea

Modernize SteambandRedux (an Angband roguelike variant) by adding Xbox 360 controller support and Steam integration, making it suitable for Steam release. This involves modernizing the build system, adding robust logging and testing infrastructure, and integrating modern gaming APIs.

## Key Features

### 1. Xbox 360 Controller Support
- Integrate XInput API for full Xbox 360 controller support
- Map controller buttons/triggers to game commands
- Support for movement, actions, and menu navigation via controller
- Configurable controller mappings

### 2. Steam Integration
- Integrate Steamworks SDK for Steam release
- Steam achievements support
- Steam overlay support
- Cloud saves (if applicable)
- Steam API initialization and callbacks

### 3. Robust Logging and Testing
- Implement comprehensive logging system with multiple log levels (DEBUG, INFO, WARNING, ERROR)
- File-based logging with rotation
- Debug console output
- Unit testing framework integration
- Test coverage for new features (controller input, logging, Steam integration)

## Target Users

### Primary Users
- **Gamers who prefer controller input**: Players who want to play roguelikes with a gamepad instead of keyboard
- **Steam users**: Gamers looking for classic roguelikes available on Steam platform
- **Existing Angband/Steamband players**: Current players who want modernized controls and Steam integration

### Use Cases
1. **Controller-first gameplay**: Player wants to play the entire game using only an Xbox 360 controller
2. **Steam integration**: Player wants achievements, cloud saves, and Steam overlay functionality
3. **Modern development**: Developers want logging and testing infrastructure for ongoing maintenance

## Tech Stack

### Core Technology
- **Language**: C (legacy codebase - maintaining compatibility)
- **Build System**: CMake (modernizing from Borland C++ 4.5)
- **Platform**: Windows (primary target)

### Libraries & APIs
- **XInput**: Xbox 360 controller support (Windows SDK)
- **Steamworks SDK**: Steam integration
- **Windows API**: Native Windows functionality (already in use)

### Development Tools
- **CMake**: Build system (minimum version 3.10)
- **Visual Studio**: Primary compiler (MSVC)
- **Testing Framework**: To be selected (considering Unity, minunit, or custom lightweight framework)

### Platform Considerations
- **Primary**: Windows (Windows 10/11)
- **Future consideration**: Linux and macOS support (not in initial scope)

## Project Status

### Current State
- Legacy C codebase (1990s-era Angband variant)
- Old build system (Borland C++ 4.5)
- No modern logging infrastructure
- No testing framework
- Windows-only platform code (`main-win.c`)

### Modernization Goals
- Migrate to CMake build system ✅ (in progress)
- Add comprehensive logging system
- Add unit testing framework
- Integrate XInput for controller support
- Integrate Steamworks SDK
- Prepare for Steam release

## Success Criteria

1. Game compiles successfully with modern build system
2. Xbox 360 controller fully functional for gameplay
3. Steam integration working (achievements, overlay)
4. Logging system operational with file output
5. Test suite in place for critical functionality
6. Ready for Steam release submission

