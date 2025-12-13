# Specification: Implement Comprehensive Logging System

## Goal
Create a comprehensive logging module with DEBUG/INFO/WARNING/ERROR/FATAL levels, file output with rotation, Windows debug console output, and integration with existing error handling hooks to enable development troubleshooting and production diagnostics.

## User Stories
- As a developer, I want detailed logging with multiple log levels so that I can debug issues during development and troubleshoot problems in production
- As a developer, I want log file rotation so that log files don't grow unbounded and consume excessive disk space
- As a developer, I want logging integrated with existing error handling hooks so that errors and fatal conditions are automatically logged

## Specific Requirements

**Log Level System**
- Support five log levels: DEBUG, INFO, WARNING, ERROR, FATAL
- Implement runtime log level filtering (minimum level configurable)
- Default to INFO level in Release builds, DEBUG level in Debug builds
- Allow configuration via preference file or environment variable
- Provide convenience macros: `LOG_D()`, `LOG_I()`, `LOG_W()`, `LOG_E()`, `LOG_F()`

**File-Based Logging**
- Write logs to `lib/logs/steamband.log` relative to executable location
- Use `path_parse()` from `util.c` for path resolution following existing patterns
- Implement size-based log rotation at 10MB threshold
- Keep last 5 rotated files with naming scheme: `steamband.log`, `steamband.log.1`, `steamband.log.2`, etc.
- Rotate by renaming existing files and creating new log file
- Ensure `lib/logs/` directory exists before writing (create if needed)
- Use standard C file I/O (`fopen`, `fprintf`, `fflush`, `fclose`) following `my_fopen()`/`my_fclose()` patterns

**Windows Debug Console Output**
- Always output log messages to Windows debug console via `OutputDebugString()`
- Format messages consistently with file output: `[timestamp] [LEVEL] message`
- Provide compile-time flag or runtime setting to disable console output if needed
- Use Windows-specific conditional compilation (`#ifdef WINDOWS`)

**Log Message Formatting**
- Include timestamp in format: `YYYY-MM-DD HH:MM:SS`
- Include log level string: `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`
- Support printf-style formatting with variadic arguments
- Limit message buffer size to 4096 characters to prevent buffer overflows
- Format: `[timestamp] [LEVEL] formatted_message\n`

**Thread Safety**
- Implement thread-safe logging using Windows mutex APIs (`CreateMutex`, `WaitForSingleObject`, `ReleaseMutex`)
- Protect shared logging state (file handle, rotation state, log level) with mutex
- Minimize mutex contention by keeping critical sections small
- Enable future multi-threading support for Steam callbacks and controller polling

**Error Handling Hook Integration**
- Integrate with `plog_hook()` to log user messages at INFO level
- Integrate with `quit_hook()` to log quit messages at INFO level
- Integrate with `core_hook()` to log fatal errors at FATAL level before crash
- Hook integration should be in platform-specific main files (e.g., `main-win.c`)
- Preserve existing hook functionality while adding logging

**Logging Scope**
- Log initialization steps (file path setup, directory validation, module initialization)
- Log file I/O operations (file opens, closes, read/write errors)
- Log errors and warnings throughout codebase
- Log controller input events at DEBUG level (for debugging controller support)
- Log Steam API calls at INFO/DEBUG level (for debugging Steam integration)
- Log critical game state changes (level transitions, save/load operations)

**Configuration and Initialization**
- Initialize logging system early in `WinMain()` before other initialization
- Read log level configuration from preference file using `process_pref_file()` patterns
- Support environment variable override for log level (e.g., `STEAMBAND_LOG_LEVEL=DEBUG`)
- Allow disabling file logging by passing NULL to `log_init()`
- Ensure proper cleanup with `log_close()` on application exit

**Performance Considerations**
- Implement synchronous/blocking logging for simplicity
- Use `fflush()` after each log write to ensure messages are persisted
- Design architecture to allow async logging as future enhancement
- Minimize overhead of log level checks (early return if level below threshold)

## Visual Design
No visual assets provided for this specification.

## Existing Code to Leverage

**logging.c and logging.h (Placeholder Files)**
- Current placeholder implementation provides basic structure with `log_init()`, `log_close()`, `log_msg()` functions
- Existing log level enum (`LOG_DEBUG`, `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`, `LOG_FATAL`) can be extended
- Convenience macros (`LOG_D`, `LOG_I`, `LOG_W`, `LOG_E`, `LOG_F`) already defined
- Windows `OutputDebugString()` integration already present
- Base implementation provides timestamp formatting and message formatting patterns

**util.c File I/O Functions**
- `path_parse()` function handles path resolution and should be used for log file paths
- `fd_open()`, `fd_close()` demonstrate file operation error handling patterns
- File locking patterns (`fd_lock()`) show how to handle file operations safely
- Error handling patterns (return codes, error checking) should be followed

**files.c File Operations**
- `my_fopen()` and `my_fclose()` wrapper functions demonstrate proper file opening/closing with error checking
- `process_pref_file()` shows how to read configuration files line-by-line
- Error handling patterns for file operations should be replicated
- File path construction and validation patterns should be followed

**main-win.c Error Handling Hooks**
- `plog_hook()`, `quit_hook()`, `core_hook()` functions exist in platform-specific files
- Hook registration pattern shows how to integrate logging with existing error handling
- Windows-specific initialization patterns in `WinMain()` should be followed for log initialization
- Directory validation patterns (`validate_dir()`) can be reused for `lib/logs/` directory

**main-win.c Preference File Handling**
- `load_prefs()` and `save_prefs()` functions use Windows INI file APIs for configuration
- Pattern for reading configuration values can be adapted for log level configuration
- `GetPrivateProfileString()` and `GetPrivateProfileInt()` patterns can be used for log settings

**util.c Message Display Functions**
- `msg_print()`, `msg_format()`, `message()`, `message_format()` show message formatting patterns
- Variadic argument handling patterns should be followed for `log_msg()` function
- Buffer size management (1024-byte buffers) patterns can inform log message buffer sizing

## Out of Scope
- Async logging implementation (architecture should allow for future enhancement, but synchronous logging only)
- Log file compression (future enhancement)
- Remote logging or network logging (future enhancement)
- Log file encryption (future enhancement)
- GUI log viewer application (future enhancement)
- Log file analysis tools or parsers (future enhancement)
- Logging to multiple files simultaneously (single log file only)
- Custom log formatters or plugins (fixed format only)
- Logging to syslog or Windows Event Log (file and debug console only)
- Performance profiling or metrics logging (separate feature)
- Logging configuration UI (preference file and environment variable only)

