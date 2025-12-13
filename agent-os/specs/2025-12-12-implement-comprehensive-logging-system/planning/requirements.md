# Spec Requirements: Implement Comprehensive Logging System

## Initial Description

Implement Comprehensive Logging System — Create logging module with DEBUG/INFO/WARNING/ERROR levels, file output with rotation, and debug console output for development troubleshooting

## Requirements Discussion

### First Round Questions

**Q1:** I assume size-based rotation (e.g., rotate when log file reaches 10MB), keep last 5 rotated files (e.g., `steamband.log`, `steamband.log.1`, `steamband.log.2`, etc.), and use a simple naming scheme. Is that correct, or do you prefer time-based rotation (daily/weekly) or a different size/count?

**Answer:** Sounds good. (Size-based rotation at 10MB, keep last 5 files, simple naming scheme)

**Q2:** I'm thinking logs should go in `lib/logs/` relative to the executable (alongside `lib/data/`, `lib/file/`, etc.). Should we use that location, or should logs go in a user-specific directory (e.g., `%APPDATA%/SteambandRedux/logs/`)?

**Answer:** Your assumption of `lib/logs/` sounds good.

**Q3:** I'm assuming console output (via `OutputDebugString` on Windows) should always be enabled, with an option to disable it via a compile-time flag or runtime setting. Is that correct, or should it only be enabled in Debug builds?

**Answer:** That's correct. (Always enabled with option to disable)

**Q4:** I'm assuming a runtime minimum log level (e.g., INFO in Release, DEBUG in Debug builds), configurable via a preference file or environment variable. Is that correct, or should it be compile-time only?

**Answer:** Correct. (Runtime minimum log level, configurable)

**Q5:** I see `plog_hook`, `quit_hook`, and `core_hook` in platform-specific files, and `msg_print`/`message` functions for in-game messages. Should the logging system integrate with these hooks (e.g., log when `core_hook` is called), or remain separate from in-game message display?

**Answer:** Integrate. (Should integrate with existing error handling hooks)

**Q6:** I'm assuming we should log initialization steps, file I/O operations, errors/warnings, controller input events (for debugging), Steam API calls, and critical game state changes. Is that correct, or should we focus on specific areas?

**Answer:** Your assumption is correct. (Log initialization, file I/O, errors/warnings, controller input, Steam API calls, critical game state changes)

**Q7:** I'm assuming logging should be synchronous (blocking) for simplicity, with optional async logging as a future enhancement. Is that correct, or should we implement async logging from the start?

**Answer:** We can keep it blocking/simple for now, with option to go async later.

**Q8:** I'm assuming the game is single-threaded, so thread safety isn't required. Is that correct, or should we make it thread-safe for future multi-threading?

**Answer:** Can we make it thread-safe? Not sure, but thinking it might help later with the steam-integration and controller support? If not significant performance impact, single thread is fine.

### Existing Code to Reference

**Similar Features Identified:**

Based on codebase analysis, the following patterns were found:

- **File I/O Functions**: `src/util.c` contains `fd_open()`, `fd_close()`, `fd_read()`, `fd_write()`, `fd_make()`, `fd_move()` functions that handle file operations. These use `path_parse()` for path resolution and standard POSIX/Windows file APIs.

- **File Opening Pattern**: `src/files.c` contains `my_fopen()` and `my_fclose()` wrapper functions that handle file opening/closing with error checking. The `process_pref_file()` function demonstrates reading configuration files line-by-line.

- **Windows Preference Files**: `src/main-win.c` contains `load_prefs()` and `save_prefs()` functions that use Windows INI file APIs (`GetPrivateProfileString`, `WritePrivateProfileString`) for reading/writing preferences.

- **Error Handling Hooks**: Platform-specific files contain error handling hooks:
  - `plog_hook()` - for user messages
  - `quit_hook()` - for quit messages
  - `core_hook()` - for fatal errors (crashes)
  These are found in platform-specific main files (e.g., `src/main-acn.c`).

- **Message Display**: `src/util.c` contains `msg_print()`, `msg_format()`, `message()`, `message_format()` functions for displaying in-game messages to the player.

**Components to potentially reuse:**
- File path resolution patterns from `util.c` (`path_parse()`)
- File opening/closing error handling patterns from `files.c` (`my_fopen()`, `my_fclose()`)
- Configuration file reading patterns from `files.c` (`process_pref_file()`)

**Backend logic to reference:**
- Error handling hook integration patterns from platform-specific main files
- File I/O error handling and path resolution from `util.c` and `files.c`

### Follow-up Questions

None required - all questions answered comprehensively.

## Visual Assets

### Files Provided:
No visual files found (bash check confirmed no files in visuals folder)

### Visual Insights:
No visual assets provided. This is expected for a logging system implementation, which is primarily a backend/infrastructure feature.

## Requirements Summary

### Functional Requirements

- **Log Levels**: Support DEBUG, INFO, WARNING, ERROR, and FATAL log levels
- **File Output**: Write logs to files in `lib/logs/` directory with rotation support
- **Log Rotation**: Size-based rotation at 10MB, keep last 5 rotated files with naming scheme (`steamband.log`, `steamband.log.1`, `steamband.log.2`, etc.)
- **Console Output**: Always output to Windows debug console via `OutputDebugString()`, with option to disable via compile-time flag or runtime setting
- **Log Level Filtering**: Runtime minimum log level (INFO in Release, DEBUG in Debug builds), configurable via preference file or environment variable
- **Integration**: Integrate with existing error handling hooks (`plog_hook`, `quit_hook`, `core_hook`) to log errors and fatal conditions
- **Logging Scope**: Log initialization steps, file I/O operations, errors/warnings, controller input events (for debugging), Steam API calls, and critical game state changes
- **Performance**: Synchronous/blocking logging for simplicity, with architecture allowing async logging as future enhancement
- **Thread Safety**: Implement thread-safe logging using mutexes (minimal performance impact, enables future multi-threading support for Steam integration and controller support)

### Reusability Opportunities

- **File I/O Patterns**: Reuse `path_parse()` from `util.c` for log file path resolution
- **File Operations**: Follow `my_fopen()`/`my_fclose()` patterns from `files.c` for file opening/closing with error handling
- **Configuration Reading**: Use similar patterns to `process_pref_file()` in `files.c` for reading log level configuration from preference files
- **Error Handling**: Integrate with existing error hooks (`plog_hook`, `quit_hook`, `core_hook`) found in platform-specific main files
- **Windows APIs**: Consider Windows-specific file APIs similar to `load_prefs()`/`save_prefs()` in `main-win.c` if needed for Windows-specific log file handling

### Scope Boundaries

**In Scope:**
- Complete logging module implementation with all log levels
- File-based logging with rotation (size-based, 10MB threshold, 5 files)
- Windows debug console output (`OutputDebugString`)
- Runtime log level filtering (configurable)
- Integration with existing error handling hooks
- Thread-safe implementation using mutexes
- Logging for initialization, file I/O, errors, controller input, Steam API calls, and game state changes
- Synchronous/blocking logging implementation

**Out of Scope:**
- Async logging (architecture should allow for future enhancement, but not implemented initially)
- Log file compression (future enhancement)
- Remote logging or network logging (future enhancement)
- Log file encryption (future enhancement)
- GUI log viewer (future enhancement)
- Log file analysis tools (future enhancement)

### Technical Considerations

- **Integration Points**: 
  - Must integrate with existing error handling hooks (`plog_hook`, `quit_hook`, `core_hook`) in platform-specific files
  - Should integrate with file I/O operations throughout the codebase
  - Must work with Steam API integration (future)
  - Must work with controller input system (future)

- **Existing System Constraints**:
  - Must work with Windows platform (primary target)
  - Must use C99 standard (as per tech stack)
  - Must follow existing file I/O patterns (`my_fopen`, `fd_open`, etc.)
  - Must respect `lib/` directory structure for game data

- **Technology Preferences**:
  - Use standard C file I/O (`fopen`, `fclose`, `fprintf`, `fflush`)
  - Use Windows mutex APIs (`CreateMutex`, `WaitForSingleObject`, `ReleaseMutex`) for thread safety
  - Use `OutputDebugString()` for Windows debug console output
  - Follow existing code patterns for file path resolution

- **Similar Code Patterns to Follow**:
  - File I/O error handling from `util.c` and `files.c`
  - Path resolution from `util.c` (`path_parse()`)
  - Configuration file reading from `files.c` (`process_pref_file()`)
  - Error handling hook integration from platform-specific main files

- **Thread Safety Implementation**:
  - Use Windows mutex (`CreateMutex`, `WaitForSingleObject`, `ReleaseMutex`) to protect shared logging state
  - Minimal performance impact (mutex contention is low for logging operations)
  - Enables future multi-threading support for Steam callbacks and controller polling

