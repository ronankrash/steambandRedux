@echo off
setlocal

set "REPO_ROOT=%~dp0.."
set "BUILD_DIR=%REPO_ROOT%\build-rescue-sdl2\Debug"
set "GAME_EXE=%BUILD_DIR%\SteambandRedux.exe"
set "SDL_DLL=%BUILD_DIR%\SDL2d.dll"
set "LOG_DIR=%BUILD_DIR%\lib\logs"
set "GAME_LOG=%LOG_DIR%\steamband.log"
set "WINDOW_LOG=%LOG_DIR%\window_debug.log"

echo SteambandRedux ROG Ally first-person smoke launcher
echo.
echo Build: %BUILD_DIR%
echo Game log: %GAME_LOG%
echo Window log: %WINDOW_LOG%
echo.

if not exist "%GAME_EXE%" (
    echo Missing %GAME_EXE%
    echo.
    echo Build the SDL2 Debug target first:
    echo   cmake -S . -B build-rescue-sdl2 -DCMAKE_TOOLCHAIN_FILE=C:/Users/bkars/vcpkg/scripts/buildsystems/vcpkg.cmake -DSTEAMBAND_ENABLE_SDL2=ON
    echo   cmake --build build-rescue-sdl2 --config Debug
    echo.
    pause
    exit /b 1
)

if not exist "%SDL_DLL%" (
    echo Warning: %SDL_DLL% was not found.
    echo The SDL2 first-person prototype may fail to start until the SDL runtime is copied next to the executable.
    echo.
)

echo Quick controls:
echo   N = new game, O = open save, Enter/A = confirm, Esc/B = cancel
echo   Ctrl+F12 or L3+R3 = toggle first-person prototype
echo   Ctrl+F11 = toggle SDL2 top-down tile prototype
echo   FP keyboard: W/Up forward, S/Down back, A/D strafe, Left/Right turn
echo   FP controller: D-pad/left stick move, right stick turns
echo   Back: single map, double command menu, triple config menu
echo.
echo After exit, review:
echo   %GAME_LOG%
echo   %WINDOW_LOG%
echo.

set "STEAMBAND_LOG_LEVEL=INFO"
pushd "%BUILD_DIR%" >nul
start "" /wait "%GAME_EXE%"
set "GAME_EXIT=%ERRORLEVEL%"
popd >nul

echo.
echo Game exited with code %GAME_EXIT%.
echo Check logs above for controller init, renderer init, DDA startup, and first-person activation.
echo.
pause
exit /b %GAME_EXIT%
