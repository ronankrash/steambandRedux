# Specification: Set Up Unit Testing Framework

## Goal
Integrate Unity testing framework to replace the existing custom test framework, create comprehensive test infrastructure following C game development best practices, and write initial tests for logging system and core utility modules.

## User Stories
- As a developer, I want a robust testing framework with better assertions and error reporting so that I can write reliable tests and quickly identify failures
- As a developer, I want test fixtures and setup/teardown functions so that I can properly isolate test units and manage test state
- As a developer, I want comprehensive tests for core utilities so that I can confidently refactor and modernize the legacy codebase

## Specific Requirements

**Unity Framework Integration**
- Replace existing `test_framework.h` with Unity testing framework (ThrowTheSwitch Unity)
- Integrate Unity as header-only or minimal source files compatible with Windows/MSVC
- Maintain compatibility with existing test structure and patterns where possible
- Ensure smooth migration path for existing 11 logging tests
- Support Unity's assertion macros (`TEST_ASSERT_EQUAL`, `TEST_ASSERT_TRUE`, etc.)
- Implement Unity test runner or adapt existing `main_test.c` to use Unity runner

**Test Infrastructure Setup**
- Implement test fixtures with setup/teardown functions following Arrange-Act-Assert (AAA) pattern
- Support test suite organization for grouping related tests (logging tests, utility tests, etc.)
- Enhanced error reporting with file/line numbers and detailed assertion messages
- Test isolation to prevent side effects between tests
- Temporary file management for file I/O tests (create/cleanup test files)

**CMake Integration**
- Enhance `UnitTests` target in `CMakeLists.txt` to include Unity framework files
- Add Unity source files to test executable build
- Integrate with CMake's `ctest` for running tests via `ctest` command
- Maintain manual test registration approach (simpler than auto-discovery for C)
- Ensure tests compile and run on Windows with MSVC compiler

**Logging System Test Migration**
- Migrate all 11 existing logging tests from custom framework to Unity
- Preserve all existing test coverage (log levels, filtering, formatting, file I/O, rotation, thread safety)
- Enhance tests with Unity's improved assertion capabilities
- Add additional edge case tests following best practices for legacy codebase modernization
- Ensure test file cleanup and proper isolation

**Core Utilities Testing - util.c**
- Write tests for `path_parse()` function (path resolution and parsing)
- Write tests for `fd_make()`, `fd_open()`, `fd_lock()` file descriptor operations
- Write tests for `user_name()`, `user_home()` user/system functions (may require platform-specific handling)
- Test message handling functions if they are testable without game state dependencies
- Use temporary files for file operation tests, clean up after each test

**Core Utilities Testing - z-util.c**
- Write tests for `streq()`, `prefix()`, `suffix()` string comparison utilities
- Write tests for `my_strcpy()` safe string copy function (test buffer overflow protection)
- Error handling functions (`plog()`, `quit()`, `core()`) may require special handling or mocking
- Test edge cases: NULL pointers, empty strings, buffer boundaries

**Core Utilities Testing - files.c**
- Write tests for `my_fopen()` and `my_fclose()` file I/O wrapper functions
- Test file opening success and failure cases
- Test file closing and error handling
- Use temporary files for testing, ensure proper cleanup
- Test error conditions (invalid paths, permission issues if testable)

**Test Organization**
- Maintain current structure: one test file per source module (`test_util.c`, `test_z_util.c`, `test_files.c`)
- Keep tests in `src/tests/` directory following existing pattern
- Update test runner (`main_test.c`) to use Unity test runner or Unity macros
- Follow naming convention: `test_*.c` for test files, `test_*()` for test functions
- Group related tests into test suites using Unity's suite organization

**Documentation**
- Create testing guide documenting how to write tests using Unity framework
- Include examples showing Unity assertion macros, test fixtures, and test organization
- Document test conventions and best practices for the project
- Update main `README.md` with testing section (how to run tests, test structure)
- Provide migration guide from old custom framework to Unity (for reference)

**Build Integration**
- Tests runnable separately via `UnitTests.exe` (maintain current approach)
- Optional: Add CMake option to run tests automatically during build (configurable, disabled by default)
- Performance impact evaluation deferred until software is stable
- Balance between immediate feedback and build performance

## Visual Design
No visual assets provided for this specification.

## Existing Code to Leverage

**test_framework.h (Current Custom Framework)**
- Existing `ASSERT`, `RUN_TEST`, `TEST_REPORT` macros provide pattern for test structure
- Test function signature pattern (returns int, 1 for success, 0 for failure) can inform Unity test structure
- Test runner pattern in `main_test.c` shows how tests are registered and executed
- Error reporting format (`FAILED: %s (%s:%d)`) shows desired error detail level

**test_logging.c (Existing Test Patterns)**
- Comprehensive test examples showing test structure and patterns
- File I/O test patterns (create test files, verify content, cleanup)
- Test isolation patterns (remove test files before/after tests)
- Platform-specific code handling (`#ifdef WINDOWS` for cleanup commands)
- Test data setup and teardown patterns

**CMakeLists.txt (Build Configuration)**
- Existing `UnitTests` executable target provides foundation
- Test source file inclusion pattern shows how to add test files
- Library linking pattern for test executable
- Windows/MSVC compatibility patterns already established

**files.c (File I/O Patterns)**
- `my_fopen()` and `my_fclose()` wrapper functions show error handling patterns
- File operation error checking patterns can be used in tests
- File path handling patterns inform test file path management

**util.c (Utility Function Patterns)**
- `path_parse()` function shows path resolution patterns for test file paths
- Error handling patterns (return codes) inform test assertion design
- File descriptor operations show how to test file I/O functions

## Out of Scope
- Test coverage reporting (gcov/lcov) - deferred to future work
- Comprehensive mocking framework - evaluated but deferred for now
- Performance testing framework - separate concern from unit testing
- Integration testing framework - separate concern
- Automatic test discovery - manual registration is simpler and more maintainable for C
- Test parallelization - sequential execution is sufficient for current needs
- Test result XML output - console output is sufficient
- GUI test runner - command-line execution only
- Test database or test history tracking - basic test execution only
- Code coverage visualization tools - deferred to future work

