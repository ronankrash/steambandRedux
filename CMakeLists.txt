cmake_minimum_required(VERSION 3.10)
project(SteambandRedux C)

set(CMAKE_C_STANDARD 99)

# Define macros for Windows build
if(WIN32)
    add_definitions(-DWINDOWS -DWIN32 -D_WIN32_WINNT=0x0501)
endif()

# Include directories
include_directories(src)

# Source files
set(SOURCES
    src/birth.c
    src/cave.c
    src/classpower.c
    src/cmd-attk.c
    src/cmd-book.c
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
    src/store.c
    src/tables.c
    src/util.c
    src/variable.c
    src/wizard1.c
    src/wizard2.c
    src/xtra1.c
    src/xtra2.c
    src/xxxload1.c
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
    set(LIBS kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32 winmm)
else()
    # Logic for other platforms (e.g. ncurses)
    list(APPEND SOURCES src/main-gcu.c src/main.c)
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIR})
    set(LIBS ${CURSES_LIBRARIES})
endif()

add_executable(SteambandRedux WIN32 ${SOURCES})
target_link_libraries(SteambandRedux ${LIBS})


