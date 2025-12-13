# Task Breakdown: Implement Comprehensive Logging System

## Overview
Total Tasks: 6 task groups, 30+ sub-tasks

## Task List

### Core Logging Implementation

#### Task Group 1: Log Level System and Message Formatting
**Dependencies:** None

- [x] 1.0 Implement log level system and message formatting
  - [x] 1.1 Write 2-8 focused tests for log level filtering and message formatting
    - Test log level enum values (DEBUG, INFO, WARN, ERROR, FATAL)
    - Test log level filtering (messages below threshold are not logged)
    - Test message formatting with timestamps and level strings
    - Test printf-style formatting with variadic arguments
    - Test buffer overflow protection (4096 character limit)
  - [x] 1.2 Enhance logging.h header file
    - Verify log level enum is complete (LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL)
    - Verify convenience macros are defined (LOG_D, LOG_I, LOG_W, LOG_E, LOG_F)
    - Add function declarations for log level configuration
    - Add thread-safe function declarations if needed
  - [x] 1.3 Implement log level filtering logic
    - Add static variable to store minimum log level
    - Implement `log_set_level()` function to set minimum log level
    - Add early return in `log_msg()` if message level is below threshold
    - Default to INFO in Release builds, DEBUG in Debug builds (use `#ifdef _DEBUG`)
  - [x] 1.4 Enhance log message formatting
    - Verify timestamp formatting (`YYYY-MM-DD HH:MM:SS` format)
    - Verify log level string conversion (DEBUG, INFO, WARN, ERROR, FATAL)
    - Ensure printf-style formatting works with variadic arguments
    - Verify buffer size limit (4096 characters) prevents overflows
    - Format: `[timestamp] [LEVEL] formatted_message\n`
  - [x] 1.5 Ensure log level and formatting tests pass
    - Run ONLY the 2-8 tests written in 1.1
    - Verify log level filtering works correctly
    - Verify message formatting is correct
    - Do NOT run the entire test suite at this stage

**Acceptance Criteria:**
- The 2-8 tests written in 1.1 pass
- Log level filtering works correctly (messages below threshold are filtered)
- Message formatting includes timestamp and level string
- Printf-style formatting works with variadic arguments
- Buffer overflow protection prevents crashes

### File I/O and Path Management

#### Task Group 2: File-Based Logging Implementation
**Dependencies:** Task Group 1

- [x] 2.0 Implement file-based logging with path management
  - [x] 2.1 Write 2-8 focused tests for file logging
    - Test log file creation in `lib/logs/` directory
    - Test log messages are written to file correctly
    - Test directory creation if `lib/logs/` doesn't exist
    - Test path resolution using `path_parse()` pattern
    - Test file handle management (open/close)
  - [x] 2.2 Implement log file path resolution
    - Use `path_parse()` from `util.c` for path resolution
    - Construct log file path: `lib/logs/steamband.log` relative to executable
    - Follow existing path construction patterns from `main-win.c` (`init_stuff()`)
    - Handle path resolution errors gracefully
  - [x] 2.3 Implement directory creation
    - Check if `lib/logs/` directory exists before writing
    - Create directory if it doesn't exist (use Windows `CreateDirectory()` API)
    - Follow directory validation patterns from `main-win.c` (`validate_dir()`)
    - Handle directory creation errors (log error, continue without file logging)
  - [x] 2.4 Enhance log_init() function
    - Accept filename parameter (or NULL to disable file logging)
    - Use `my_fopen()` pattern from `files.c` for file opening with error checking
    - Open file in append mode ("a") or write mode ("w") as appropriate
    - Store file handle in static variable
    - Handle file open errors gracefully (log to console, continue without file)
  - [x] 2.5 Enhance log_msg() file output
    - Write formatted message to file using `fprintf()`
    - Use `fflush()` after each write to ensure messages are persisted
    - Follow file I/O error handling patterns from `files.c`
    - Handle write errors gracefully (log to console, continue)
  - [x] 2.6 Enhance log_close() function
    - Close file handle if open using `fclose()`
    - Follow `my_fclose()` pattern from `files.c`
    - Set file handle to NULL after closing
    - Handle close errors gracefully
  - [x] 2.7 Ensure file logging tests pass
    - Run ONLY the 2-8 tests written in 2.1
    - Verify log file is created correctly
    - Verify messages are written to file
    - Verify directory creation works
    - Do NOT run the entire test suite at this stage

**Acceptance Criteria:**
- The 2-8 tests written in 2.1 pass
- Log file is created in `lib/logs/steamband.log`
- Directory is created if it doesn't exist
- Messages are written to file correctly
- File I/O errors are handled gracefully

### Log Rotation

#### Task Group 3: Log File Rotation Implementation
**Dependencies:** Task Group 2

- [x] 3.0 Implement log file rotation
  - [x] 3.1 Write 2-8 focused tests for log rotation
    - Test rotation triggers when file reaches 10MB
    - Test rotation creates correctly named files (`steamband.log.1`, `steamband.log.2`, etc.)
    - Test rotation keeps only last 5 files (oldest files are deleted)
    - Test rotation preserves log file handle
    - Test rotation works correctly with multiple rotations
  - [x] 3.2 Implement file size checking
    - Check log file size before each write (use `ftell()` and `fseek()` or `GetFileSize()` on Windows)
    - Compare file size against 10MB threshold (10 * 1024 * 1024 bytes)
    - Trigger rotation when size exceeds threshold
  - [x] 3.3 Implement rotation file renaming
    - Rename existing rotated files: `steamband.log.N` -> `steamband.log.N+1`
    - Start from highest number (5) and work backwards to 1
    - Delete `steamband.log.5` if it exists (oldest file)
    - Rename `steamband.log` -> `steamband.log.1`
    - Use Windows `MoveFile()` API or standard `rename()` function
  - [x] 3.4 Implement rotation file cleanup
    - Delete oldest file (`steamband.log.5`) if it exists before rotation
    - Ensure only last 5 files are kept
    - Handle file deletion errors gracefully (log warning, continue)
  - [x] 3.5 Integrate rotation into log_msg()
    - Check file size before writing message
    - Call rotation function if size exceeds threshold
    - Reopen log file after rotation
    - Ensure rotation doesn't interrupt logging (messages aren't lost)
  - [x] 3.6 Ensure log rotation tests pass
    - Run ONLY the 2-8 tests written in 3.1
    - Verify rotation triggers at 10MB threshold
    - Verify file naming is correct
    - Verify only 5 files are kept
    - Do NOT run the entire test suite at this stage

**Acceptance Criteria:**
- The 2-8 tests written in 3.1 pass
- Rotation triggers when file reaches 10MB
- Rotated files are named correctly (`steamband.log.1` through `steamband.log.5`)
- Only last 5 files are kept (oldest files deleted)
- Rotation doesn't interrupt logging

### Thread Safety

#### Task Group 4: Thread-Safe Logging Implementation
**Dependencies:** Task Groups 1-3

- [x] 4.0 Implement thread-safe logging
  - [x] 4.1 Write 2-8 focused tests for thread safety
    - Test concurrent log writes from multiple threads
    - Test log messages are not interleaved or corrupted
    - Test mutex protects shared state correctly
    - Test mutex initialization and cleanup
  - [x] 4.2 Implement Windows mutex creation
    - Create mutex using `CreateMutex()` API in `log_init()`
    - Use named mutex or static mutex handle
    - Store mutex handle in static variable
    - Handle mutex creation errors (log error, continue without thread safety)
  - [x] 4.3 Protect shared logging state with mutex
    - Acquire mutex using `WaitForSingleObject()` before accessing shared state
    - Protect file handle, rotation state, and log level access
    - Release mutex using `ReleaseMutex()` after critical section
    - Keep critical sections small to minimize contention
  - [x] 4.4 Integrate mutex into log_msg()
    - Acquire mutex at start of `log_msg()`
    - Protect all file I/O operations
    - Protect rotation operations
    - Release mutex before returning
    - Handle mutex errors gracefully (log warning, continue)
  - [x] 4.5 Clean up mutex in log_close()
    - Release mutex handle using `CloseHandle()` in `log_close()`
    - Ensure mutex is released even if errors occur
    - Set mutex handle to NULL after closing
  - [x] 4.6 Ensure thread safety tests pass
    - Run ONLY the 2-8 tests written in 4.1
    - Verify concurrent writes don't corrupt log messages
    - Verify mutex protects shared state correctly
    - Do NOT run the entire test suite at this stage

**Acceptance Criteria:**
- The 2-8 tests written in 4.1 pass
- Concurrent log writes from multiple threads work correctly
- Log messages are not interleaved or corrupted
- Mutex protects shared logging state
- Mutex is properly initialized and cleaned up

### Configuration and Integration

#### Task Group 5: Configuration and Error Hook Integration
**Dependencies:** Task Groups 1-4

- [x] 5.0 Implement configuration and error hook integration
  - [x] 5.1 Write 2-8 focused tests for configuration and hooks
    - Test log level configuration from preference file
    - Test environment variable override for log level
    - Test error hook integration (plog_hook, quit_hook, core_hook)
    - Test logging initialization in WinMain()
  - [x] 5.2 Implement log level configuration reading
    - Read log level from preference file using `GetPrivateProfileString()` pattern from `main-win.c`
    - Support environment variable override (`STEAMBAND_LOG_LEVEL`)
    - Parse log level string (DEBUG, INFO, WARN, ERROR, FATAL)
    - Set log level using `log_set_level()` function
    - Default to INFO in Release, DEBUG in Debug builds
  - [x] 5.3 Integrate logging initialization into WinMain()
    - Call `log_init()` early in `WinMain()` before other initialization
    - Construct log file path relative to executable location
    - Handle initialization errors gracefully (log to console, continue)
    - Ensure logging is available for all subsequent initialization steps
  - [x] 5.4 Integrate with plog_hook()
    - Modify `plog_hook()` implementation in platform-specific main file
    - Call `LOG_I()` to log user messages at INFO level
    - Preserve existing `plog_hook()` functionality
    - Ensure hook integration doesn't break existing behavior
  - [x] 5.5 Integrate with quit_hook()
    - Modify `quit_hook()` implementation in platform-specific main file
    - Call `LOG_I()` to log quit messages at INFO level
    - Preserve existing `quit_hook()` functionality
    - Ensure hook integration doesn't break existing behavior
  - [x] 5.6 Integrate with core_hook()
    - Modify `core_hook()` implementation in platform-specific main file
    - Call `LOG_F()` to log fatal errors at FATAL level before crash
    - Ensure fatal error is logged before application exits
    - Preserve existing `core_hook()` functionality (panic save, etc.)
  - [x] 5.7 Implement logging cleanup on exit
    - Call `log_close()` in application exit path
    - Ensure log file is flushed and closed properly
    - Handle cleanup errors gracefully
  - [x] 5.8 Ensure configuration and hook integration tests pass
    - Run ONLY the 2-8 tests written in 5.1
    - Verify log level configuration works
    - Verify error hooks log correctly
    - Verify initialization works in WinMain()
    - Do NOT run the entire test suite at this stage

**Acceptance Criteria:**
- The 2-8 tests written in 5.1 pass
- Log level configuration works from preference file and environment variable
- Error hooks (plog_hook, quit_hook, core_hook) log correctly
- Logging is initialized early in WinMain()
- Logging cleanup works on application exit

### Testing and Verification

#### Task Group 6: Test Review & Gap Analysis
**Dependencies:** Task Groups 1-5

- [x] 6.0 Review existing tests and fill critical gaps only
  - [x] 6.1 Review tests from Task Groups 1-5
    - Review the 2-8 tests written by core-logging-engineer (Task 1.1)
    - Review the 2-8 tests written by file-io-engineer (Task 2.1)
    - Review the 2-8 tests written by rotation-engineer (Task 3.1)
    - Review the 2-8 tests written by thread-safety-engineer (Task 4.1)
    - Review the 2-8 tests written by integration-engineer (Task 5.1)
    - Total existing tests: approximately 10-40 tests
  - [x] 6.2 Analyze test coverage gaps for THIS feature only
    - Identify critical logging workflows that lack test coverage
    - Focus ONLY on gaps related to this spec's logging requirements
    - Do NOT assess entire application test coverage
    - Prioritize end-to-end logging workflows over unit test gaps
  - [x] 6.3 Write up to 10 additional strategic tests maximum
    - Add maximum of 10 new tests to fill identified critical gaps
    - Focus on integration points (logging + error hooks, logging + file I/O)
    - Test end-to-end workflows (initialization -> logging -> rotation -> cleanup)
    - Do NOT write comprehensive coverage for all scenarios
    - Skip edge cases, performance tests unless business-critical
  - [x] 6.4 Run feature-specific tests only
    - Run ONLY tests related to this spec's logging feature (tests from 1.1, 2.1, 3.1, 4.1, 5.1, and 6.3)
    - Expected total: approximately 20-50 tests maximum
    - Do NOT run the entire application test suite
    - Verify critical logging workflows pass

**Acceptance Criteria:**
- All feature-specific tests pass (approximately 20-50 tests total)
- Critical logging workflows for this feature are covered
- No more than 10 additional tests added when filling in testing gaps
- Testing focused exclusively on this spec's logging requirements

## Execution Order

Recommended implementation sequence:
1. Core Logging Implementation (Task Group 1) - Foundation for all other features
2. File I/O and Path Management (Task Group 2) - Enables file-based logging
3. Log Rotation (Task Group 3) - Builds on file logging
4. Thread Safety (Task Group 4) - Adds thread safety to existing logging
5. Configuration and Integration (Task Group 5) - Integrates logging into application
6. Test Review & Gap Analysis (Task Group 6) - Final verification and gap filling

## Notes

- This is a logging infrastructure implementation, not a game feature
- Focus is on creating robust, production-ready logging system
- Placeholder `logging.c` and `logging.h` files exist and should be enhanced, not replaced
- Thread safety is important for future Steam integration and controller support
- Error hook integration preserves existing functionality while adding logging
- Testing should focus on critical workflows, not exhaustive edge case coverage

