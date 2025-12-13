# Spec Requirements: Set Up Unit Testing Framework

## Initial Description

Set Up Unit Testing Framework — Integrate testing framework (Unity/minunit), create test infrastructure, and write initial tests for logging system and core utilities

## Requirements Discussion

### First Round Questions

**Q1:** Framework Selection: I see there's already a basic custom test framework (`src/tests/test_framework.h`) with `ASSERT`, `RUN_TEST`, and `TEST_REPORT` macros that's working well for the logging tests. Should we:
- **Option A**: Replace it with Unity (more features, but requires integration)
- **Option B**: Replace it with minunit (minimal, single-header framework)
- **Option C**: Enhance the existing custom framework with additional features (fixtures, setup/teardown, better error reporting)
- **Option D**: Keep the existing framework but formalize and document it as the project standard?

**Answer:** Option A - Replace with Unity framework

**Q2:** Test Infrastructure Features: What additional features should the testing framework support? I'm assuming we need:
- Setup/teardown functions for test fixtures
- Better error reporting (file/line numbers, assertion messages)
- Test grouping/organization (test suites)
- Mocking support (or is that out of scope for now)?
- Should we include any of these, or keep it minimal?

**Answer:** Whatever is best practice for a game of this type

**Q3:** Core Utilities Testing: The roadmap mentions "write initial tests for logging system and core utilities". For "core utilities", I'm assuming we should test:
- `src/util.c` (utility functions like `path_parse`, string manipulation)
- `src/z-util.c` (z-util functions)
- `src/files.c` (file I/O utilities)
- Are there other specific utility modules you'd like tested, or should we focus on these?

**Answer:** Not sure, let's just focus on these for now -- unless you notice something later to add

**Q4:** CMake Integration: I see there's already a `UnitTests` target in `CMakeLists.txt`. Should we:
- Enhance it to automatically discover test files?
- Add a `ctest` integration for running tests via CMake?
- Keep it simple with manual test registration (current approach)?
- Add test coverage reporting (gcov/lcov) or is that future work?

**Answer:** Whatever is best practice and ease of maintenance

**Q5:** Test Organization: How should tests be organized? I'm assuming:
- One test file per source module (e.g., `test_util.c` for `util.c`)
- Tests in `src/tests/` directory (current structure)
- Test runner in `src/tests/main_test.c` (current approach)
- Is this structure acceptable, or should we organize differently?

**Answer:** Seems acceptable to me unless you think something makes better sense. I trust you.

**Q6:** Logging System Tests: The logging system already has 11 comprehensive tests. Should we:
- Keep these tests as-is and consider them complete?
- Enhance them with the new framework features (if we upgrade)?
- Add additional edge case tests?

**Answer:** Whatever is best practice for updating an old game's code-base

**Q7:** Documentation: Should we create:
- A testing guide/documentation for developers?
- Examples of how to write tests?
- Integration with the main README?

**Answer:** Yes to all

**Q8:** Build Integration: Should tests:
- Run automatically as part of the build process (via CMake)?
- Be runnable separately via `UnitTests.exe` (current approach)?
- Both?

**Answer:** Whatever is best practice. I don't have an opinion other than maybe once software is stable, we readdress due to performance impacts?

### Existing Code to Reference

**Similar Features Identified:**
- **Existing Test Framework**: `src/tests/test_framework.h` - Basic custom framework with `ASSERT`, `RUN_TEST`, and `TEST_REPORT` macros
- **Test Runner**: `src/tests/main_test.c` - Current test runner implementation
- **Logging Tests**: `src/tests/test_logging.c` - 11 comprehensive tests for logging system (good reference for test patterns)
- **CMake Test Target**: `CMakeLists.txt` already has `UnitTests` executable target configured

**Codebase Analysis Findings:**
- **util.c**: Contains functions like `path_parse()`, `fd_make()`, `fd_open()`, `fd_lock()`, `user_name()`, `user_home()`, message handling functions
- **z-util.c**: Contains `streq()`, `prefix()`, `suffix()`, `my_strcpy()`, `plog()`, `quit()`, `core()` functions
- **files.c**: Contains `my_fopen()`, `my_fclose()`, and various file I/O operations
- Current test structure follows pattern: one test file per module (`test_logging.c`), tests return 1 on success, 0 on failure
- Tests use `ASSERT()` macro for assertions, `RUN_TEST()` for execution, `TEST_REPORT()` for summary

### Follow-up Questions

None required - user provided clear guidance and trusts best practices.

## Visual Assets

### Files Provided:
No visual files found (verified via bash check)

### Visual Insights:
No visual assets provided.

## Requirements Summary

### Functional Requirements

**Framework Integration:**
- Replace existing custom test framework (`test_framework.h`) with Unity testing framework
- Unity framework provides: better assertions, test fixtures, setup/teardown, improved error reporting, test suites
- Maintain compatibility with existing test structure where possible
- Ensure smooth migration path for existing logging tests

**Test Infrastructure:**
- Implement Unity framework integration following C game development best practices
- Support for test fixtures (setup/teardown functions)
- Enhanced error reporting with file/line numbers and detailed assertion messages
- Test suite organization for grouping related tests
- Mocking support evaluated but may be deferred if not immediately needed

**Core Utilities Testing:**
- Write initial tests for `src/util.c` functions:
  - `path_parse()` - Path resolution and parsing
  - `fd_make()`, `fd_open()`, `fd_lock()` - File descriptor operations
  - `user_name()`, `user_home()` - User/system functions
  - Message handling functions (if testable)
- Write initial tests for `src/z-util.c` functions:
  - `streq()`, `prefix()`, `suffix()` - String comparison utilities
  - `my_strcpy()` - Safe string copy function
  - Error handling functions (`plog()`, `quit()`, `core()`) - May require special handling
- Write initial tests for `src/files.c` functions:
  - `my_fopen()`, `my_fclose()` - File I/O wrappers
  - Other file operations as appropriate
- Additional utility modules may be added later if identified during implementation

**CMake Integration:**
- Enhance `UnitTests` target in `CMakeLists.txt` following best practices
- Consider automatic test discovery (if Unity supports it) or maintain manual registration for simplicity
- Add `ctest` integration for running tests via CMake (`ctest` command)
- Test coverage reporting (gcov/lcov) deferred to future work
- Balance between automation and ease of maintenance

**Test Organization:**
- Maintain current structure: one test file per source module (`test_util.c`, `test_z_util.c`, `test_files.c`)
- Tests located in `src/tests/` directory
- Test runner in `src/tests/main_test.c` (or Unity's test runner)
- Follow existing naming conventions (`test_*.c`)

**Logging System Tests:**
- Migrate existing 11 logging tests to Unity framework
- Enhance tests with Unity's improved assertion capabilities
- Add additional edge case tests following best practices for legacy codebase modernization
- Ensure all existing test coverage is preserved

**Documentation:**
- Create testing guide/documentation for developers
- Include examples of how to write tests using Unity framework
- Document test organization and conventions
- Integrate testing documentation into main README.md
- Provide migration guide from old framework to Unity

**Build Integration:**
- Tests runnable separately via `UnitTests.exe` (current approach maintained)
- Consider optional automatic test execution during build (configurable)
- Performance impact to be evaluated when software is stable
- Balance between immediate feedback and build performance

### Reusability Opportunities

- **Existing Test Patterns**: Reference `test_logging.c` for test structure and patterns
- **Test Framework Macros**: Current `ASSERT`, `RUN_TEST`, `TEST_REPORT` patterns can inform Unity test structure
- **CMake Configuration**: Existing `UnitTests` target provides foundation for enhancement
- **File I/O Patterns**: Existing `my_fopen()`/`my_fclose()` wrappers in `files.c` can be used for test file operations

### Scope Boundaries

**In Scope:**
- Unity framework integration and setup
- Migration of existing logging tests to Unity
- Initial test suite for `util.c`, `z-util.c`, `files.c`
- CMake integration with `ctest` support
- Test infrastructure documentation
- Test examples and developer guide

**Out of Scope:**
- Test coverage reporting (gcov/lcov) - deferred to future work
- Comprehensive mocking framework - evaluated but may be deferred
- Performance testing framework - separate concern
- Integration testing framework - separate concern
- Automatic test discovery if Unity doesn't support it well

### Technical Considerations

**Unity Framework:**
- Unity is a lightweight C testing framework by ThrowTheSwitch
- Header-only or minimal source files
- Well-suited for C projects and embedded systems
- Integrates well with CMake
- Provides assertions, test fixtures, and test runners

**Integration Points:**
- CMake build system (`CMakeLists.txt`)
- Existing test structure (`src/tests/`)
- Current test patterns (`test_logging.c` as reference)
- Windows/MSVC compiler compatibility

**Legacy Codebase Considerations:**
- Some functions may have dependencies on global state
- Error handling functions (`quit()`, `core()`) may require special test handling
- File I/O functions may need temporary file management in tests
- Platform-specific code may need conditional compilation in tests

**Best Practices for C Game Testing:**
- Use Arrange-Act-Assert (AAA) pattern
- Keep tests short, simple, and fast
- Test public interfaces and critical paths
- Isolate units under test where possible
- Use descriptive test names
- Handle test fixtures and cleanup properly

