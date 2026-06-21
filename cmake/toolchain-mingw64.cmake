# CMake toolchain file for cross-compiling the Windows build on Linux using MinGW-w64.
#
# Usage:
#   cmake -S . -B build-mingw -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw64.cmake
#   cmake --build build-mingw
#
# The resulting .exe files target 64-bit Windows and can be run under Wine.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# Run the produced Windows executables (e.g. via ctest) through Wine.
find_program(WINE_EXECUTABLE NAMES wine)
if(WINE_EXECUTABLE)
    set(CMAKE_CROSSCOMPILING_EMULATOR ${WINE_EXECUTABLE})
endif()

# Search for programs only in the host, and headers/libraries only in the target.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
