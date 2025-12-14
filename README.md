# SteambandRedux

A modernized roguelike game based on Steamband (an Angband variant), featuring Xbox 360 controller support and Steam platform integration.

## Overview

SteambandRedux is a modernization project that brings classic roguelike gameplay to modern gaming platforms. The project modernizes the legacy Steamband codebase (originally built with Borland C++ 4.5) by:

- **Adding Xbox 360 controller support** via XInput API
- **Integrating Steamworks SDK** for achievements, overlay, and cloud saves
- **Modernizing the build system** from Borland C++ to CMake
- **Adding comprehensive logging** for debugging and troubleshooting
- **Setting up unit testing infrastructure** for reliability

## Project Status

### ✅ Completed
- **CMake Build System Migration** - Successfully migrated from Borland C++ 4.5 to CMake, building on Windows with Visual Studio
- **Comprehensive Logging System** - Full logging infrastructure with DEBUG/INFO/WARNING/ERROR/FATAL levels, file output with rotation, and Windows debug console support

### 🚧 In Progress
- Unit Testing Framework setup
- XInput API integration

### 📋 Planned
- Controller input mapping and configuration
- Steamworks SDK integration
- Steam achievements and cloud saves
- Steam release preparation

See the [Product Roadmap](agent-os/product/roadmap.md) for detailed progress.

## Quick Start

### Prerequisites
- **Windows 10/11** (64-bit)
- **CMake** 3.10 or higher
- **Visual Studio 2022** (or compatible MSVC compiler)
- **Git**

### Building

```bash
# Clone the repository
git clone <repository-url>
cd steambandRedux

# Configure CMake
cmake -S . -B build

# Build (Debug configuration)
cmake --build build --config Debug

# Build (Release configuration)
cmake --build build --config Release
```

### Running

```bash
# Run the game
cd build/Debug
./SteambandRedux.exe

# Run unit tests (method 1: direct execution)
cd build/Debug
./UnityTestRunner.exe

# Run unit tests (method 2: via CMake ctest)
cd build
ctest -C Debug --output-on-failure
```

### Logging

Logs are written to `lib/logs/steamband.log` (relative to the executable). You can control the log level via environment variable:

```bash
# Set log level (DEBUG, INFO, WARN, ERROR, FATAL)
set STEAMBAND_LOG_LEVEL=DEBUG
./SteambandRedux.exe
```

## Project Structure

```
steambandRedux/
├── src/                    # Source code
│   ├── main-win.c         # Windows entry point and message loop
│   ├── logging.c          # Logging system implementation
│   ├── logging.h          # Logging system header
│   └── tests/             # Unit tests
│       ├── main_test.c    # Test runner
│       └── test_logging.c # Logging system tests
├── lib/                   # Game data directory
│   ├── logs/             # Log files (auto-created)
│   ├── data/             # Binary game data
│   ├── help/             # Help files
│   └── save/             # Save files
├── agent-os/             # Development process documentation
│   ├── product/          # Product planning documents
│   │   ├── mission.md           # Product mission and vision
│   │   ├── roadmap.md            # Development roadmap
│   │   └── tech-stack.md         # Technical stack documentation
│   ├── specs/            # Feature specifications
│   │   ├── 2025-12-11-complete-cmake-build-system-migration/
│   │   └── 2025-12-12-implement-comprehensive-logging-system/
│   └── commands/         # Development workflow commands
├── CMakeLists.txt        # CMake build configuration
└── README.md             # This file
```

## Development Process: Agent-OS

This project uses a structured development process called **Agent-OS** that organizes work into specifications, tasks, and verifications. This ensures systematic, well-documented development.

### Workflow Overview

1. **Product Planning** (`agent-os/commands/plan-product/`)
   - Define product concept, mission, roadmap, and tech stack
   - Documents: `agent-os/product/`

2. **Specification Creation** (`agent-os/commands/shape-spec/` and `write-spec/`)
   - Create detailed specifications for each feature
   - Each spec includes requirements, planning, and acceptance criteria
   - Specs are stored in `agent-os/specs/[date]-[feature-name]/`

3. **Task Breakdown** (`agent-os/commands/create-tasks/`)
   - Break specs into actionable tasks with dependencies
   - Tasks are documented in `agent-os/specs/[spec]/tasks.md`

4. **Implementation** (`agent-os/commands/implement-tasks/`)
   - Implement tasks systematically
   - Write tests alongside implementation
   - Update task checkboxes as work progresses

5. **Verification** (`agent-os/commands/implement-tasks/3-verify-implementation.md`)
   - Run full test suite
   - Verify all tasks are complete
   - Update roadmap
   - Create verification report

### Current Specifications

#### ✅ Complete CMake Build System Migration
**Spec:** `2025-12-11-complete-cmake-build-system-migration`  
**Status:** Complete  
**Summary:** Migrated from Borland C++ 4.5 to CMake, enabling modern compiler support and library integration.

**Key Achievements:**
- Created `CMakeLists.txt` with Windows/MSVC configuration
- Fixed compilation errors and warnings
- Verified Debug and Release builds
- Excluded problematic legacy files
- Created placeholders for future features

**Verification:** [Final Verification Report](agent-os/specs/2025-12-11-complete-cmake-build-system-migration/verifications/final-verification.md)

#### ✅ Implement Comprehensive Logging System
**Spec:** `2025-12-12-implement-comprehensive-logging-system`  
**Status:** Complete  
**Summary:** Implemented production-ready logging system with file output, rotation, thread safety, and error hook integration.

**Key Features:**
- Five log levels: DEBUG, INFO, WARN, ERROR, FATAL
- File-based logging with automatic directory creation
- Log rotation at 10MB (keeps 5 rotated files)
- Thread-safe operation using Windows mutexes
- Windows debug console output
- Integration with error handling hooks (`plog_hook`, `quit_hook`, `core_hook`)
- Environment variable configuration (`STEAMBAND_LOG_LEVEL`)
- 11 unit tests, all passing

**Verification:** [Final Verification Report](agent-os/specs/2025-12-12-implement-comprehensive-logging-system/verifications/final-verification.md)

### Upcoming Specifications

- **Set Up Unit Testing Framework** - Integrate testing framework and create test infrastructure
- **Integrate XInput API** - Add Xbox 360 controller detection and polling
- **Implement Controller Input Mapping** - Map controller buttons to game commands
- **Integrate Steamworks SDK** - Add Steam platform integration

See [Product Roadmap](agent-os/product/roadmap.md) for the complete list.

## Technical Details

### Technology Stack

- **Language:** C (C99 standard, maintaining legacy compatibility)
- **Build System:** CMake 3.10+
- **Compiler:** Microsoft Visual C++ (MSVC) via Visual Studio 2022
- **Platform:** Windows 10/11 (64-bit)
- **APIs:** Windows API, XInput (Xbox 360 controllers), Steamworks SDK

### Key Components

- **Logging System** (`src/logging.c`): Thread-safe logging with file output and rotation
- **Windows Entry Point** (`src/main-win.c`): Windows message loop and initialization
- **Build System** (`CMakeLists.txt`): CMake configuration for modern compilation

For detailed technical information, see [Tech Stack Documentation](agent-os/product/tech-stack.md).

## Game Information

### About Steamband

Steamband is a variant of Angband, a classic roguelike dungeon-crawling game. The original Steamband 0.2.2 was created by Courtney C. Campbell and is based on:

- **Moria** (1985) by Robert Alan Koeneke
- **Umoria** (1989) by James E. Wilson
- **Angband** (various versions) by multiple contributors

### Original Credits

```
Steamband 0.2.2 by Courtney C. Campbell
Based on Moria, Umoria, and Angband
Send comments, bug reports, and patches to: campbell@oook.cz
Visit the Angband Home Page at: http://www.thangorodrim.net/
```

## Contributing

This project is currently in active development. The codebase is being modernized systematically using the Agent-OS process. If you're interested in contributing:

1. Review the [Product Roadmap](agent-os/product/roadmap.md) to see what's planned
2. Check existing [Specifications](agent-os/specs/) to understand the development process
3. Review the [Tech Stack](agent-os/product/tech-stack.md) for technical requirements

## License

This project maintains the original Angband/Steamband license. See the original `readme.txt` file for license details.

## Resources

- **Product Mission:** [agent-os/product/mission.md](agent-os/product/mission.md)
- **Product Roadmap:** [agent-os/product/roadmap.md](agent-os/product/roadmap.md)
- **Tech Stack:** [agent-os/product/tech-stack.md](agent-os/product/tech-stack.md)
- **Original README:** [readme.txt](readme.txt)

## Testing

The project uses the **Unity** testing framework for unit tests. Tests are located in `src/tests/` and can be run via:

- **Direct execution**: `build/Debug/UnityTestRunner.exe`
- **CMake ctest**: `cd build && ctest -C Debug --output-on-failure`

### Test Structure

- **Unity Infrastructure Tests** (`test_unity_infrastructure.c`) - Verify Unity framework integration
- **Logging System Tests** (`test_logging_unity.c`) - 13 tests covering all logging functionality
- **Core Utilities Tests** (`test_z_util.c`) - 10 tests for string utilities and buffer overflow protection

### Current Test Coverage

- ✅ Unity framework integration (5 tests)
- ✅ Logging system (13 tests: levels, filtering, formatting, rotation, thread safety)
- ✅ z-util.c utilities (10 tests: streq, prefix, suffix, my_strcpy)
- ⏳ util.c utilities (tests written but deferred due to game state dependencies)
- ⏳ files.c utilities (deferred due to game state dependencies)

For detailed information on writing and running tests, see the [Testing Guide](agent-os/specs/2025-12-12-set-up-unit-testing-framework/documentation/testing-guide.md).

## Status Badges

- ✅ **CMake Build System** - Complete
- ✅ **Logging System** - Complete
- ✅ **Unit Testing Framework** - Complete
- 📋 **XInput Integration** - Planned
- 📋 **Steam Integration** - Planned

---

**Note:** This is a modernization project. The game itself is based on the classic Steamband/Angband codebase, and we're adding modern features while preserving the authentic gameplay experience.

