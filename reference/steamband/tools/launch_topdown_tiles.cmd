@echo off
setlocal

set "REPO_ROOT=%~dp0.."
set "BUILD_DIR=%REPO_ROOT%\build-rescue-sdl2\Debug"
set "GAME_EXE=%BUILD_DIR%\SteambandRedux.exe"
set "LOG_DIR=%BUILD_DIR%\lib\logs"
set "GAME_LOG=%LOG_DIR%\steamband.log"
set "WINDOW_LOG=%LOG_DIR%\window_debug.log"

echo SteambandRedux SDL2 top-down tile launcher
echo.
echo Build: %BUILD_DIR%
echo Game log: %GAME_LOG%
echo Window log: %WINDOW_LOG%
echo.

if not exist "%GAME_EXE%" (
    echo Missing %GAME_EXE%
    echo.
    echo Build the SDL2 Debug target first:
    echo   cmake --build build-rescue-sdl2 --config Debug
    echo.
    pause
    exit /b 1
)

echo Top-down controls:
echo   N = new game, O = open save, Enter/A = confirm
echo   Arrow keys/numpad/controller = legacy roguelike movement
echo   Escape/B = legacy cancel while the SDL tile window is focused
echo   Ctrl+F11 = hide/show the SDL2 top-down tile window
echo.
echo Before File New you will see a DEMO preview map (walls/floor/ore sample).
echo After starting a game, click the SDL tile window so keyboard/controller input works.
echo.
echo This launcher sets STEAMBAND_START_TOPDOWN=1 so the tile window opens automatically.
echo.

set "STEAMBAND_LOG_LEVEL=INFO"
set "STEAMBAND_START_TOPDOWN=1"
pushd "%BUILD_DIR%" >nul
start "" /wait "%GAME_EXE%"
set "GAME_EXIT=%ERRORLEVEL%"
popd >nul

echo.
echo Game exited with code %GAME_EXIT%.
echo Check logs above for top-down activation and tilesheet loading.
echo.
pause
exit /b %GAME_EXIT%
