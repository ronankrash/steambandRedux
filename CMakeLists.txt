cmake_minimum_required(VERSION 3.10)
project(SteambandRedux C)

set(CMAKE_C_STANDARD 99)

# Define macros for Windows build
if(WIN32)
    add_definitions(-DWINDOWS -DWIN32 -D_WIN32_WINNT=0x0501)
    # Suppress MSVC C4996 warnings for unsafe string functions (legacy code)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Include directories
include_directories(src)
include_directories(third_party/unity/src)

# Source files
# Note: xxxload1.c is excluded - it references missing artifact constants and is legacy code
set(SOURCES
    src/birth.c
    src/cave.c
    src/classpower.c
    src/cmd-attk.c
    src/cmd-book.c
    src/controller.c
    src/controller_menu.c
    src/controller_config_menu.c
    src/cmd-item.c
    src/cmd-know.c
    src/cmd-misc.c
    src/cmd-util.c
    src/dungeon.c
    src/files.c
    src/generate.c
    src/init1.c
    src/init2.c
    src/level.c
    src/load2.c
    src/logging.c
    src/melee1.c
    src/melee2.c
    src/monster1.c
    src/monster2.c
    src/mspells1.c
    src/mutation.c
    src/object1.c
    src/object2.c
    src/pet.c
    src/readdib.c
    src/save.c
    src/spells1.c
    src/spells2.c
    src/spells3.c
    src/steam_integration.c
    src/store.c
    src/tables.c
    src/util.c
    src/variable.c
    src/wizard1.c
    src/wizard2.c
    src/xtra1.c
    src/xtra2.c
    src/xxxrandart.c
    src/z-form.c
    src/z-rand.c
    src/z-term.c
    src/z-util.c
    src/z-virt.c
)

if(WIN32)
    list(APPEND SOURCES src/main-win.c)
    # Resource file would go here if we had it: src/angband.rc
    
    # Libraries needed for Windows
    set(LIBS kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32 winmm xinput)
else()
    # Logic for other platforms (e.g. ncurses)
    list(APPEND SOURCES src/main-gcu.c src/main.c)
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIR})
    set(LIBS ${CURSES_LIBRARIES})
endif()

add_executable(SteambandRedux WIN32 ${SOURCES})
target_link_libraries(SteambandRedux ${LIBS})

# Unity Testing Framework
set(UNITY_SOURCES
    third_party/unity/src/unity.c
)

# Tests
add_executable(UnitTests
    src/tests/main_test.c
    src/tests/test_logging.c
    src/tests/test_unity_infrastructure.c
    src/tests/unity_integration.c
    src/logging.c
    ${UNITY_SOURCES}
)
target_link_libraries(UnitTests ${LIBS})

# Unity Test Runner (for Unity framework tests)
# Note: test_util.c is excluded for now due to complex dependencies on game state
# Focus on testing z-util.c functions which are more isolated
add_executable(UnityTestRunner
    src/tests/unity_test_runner.c
    src/tests/test_unity_infrastructure.c
    src/tests/test_logging_unity.c
    src/tests/test_z_util.c
    src/tests/unity_integration.c
    src/logging.c
    src/z-util.c
    ${UNITY_SOURCES}
)
target_link_libraries(UnityTestRunner ${LIBS})

# Enable CTest
enable_testing()
add_test(NAME UnitTests COMMAND UnitTests)
add_test(NAME UnityTestRunner COMMAND UnityTestRunner)


