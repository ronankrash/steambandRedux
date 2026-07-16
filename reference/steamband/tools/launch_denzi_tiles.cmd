@echo off
setlocal

set "REPO_ROOT=%~dp0.."
set "BUILD_DIR=%REPO_ROOT%\build-rescue-sdl2\Debug"
set "GAME_EXE=%BUILD_DIR%\SteambandRedux.exe"
set "DENZI_TILESET=%REPO_ROOT%\lib\xtra\graf\topdown_denzi.bmp"

if not exist "%GAME_EXE%" (
    echo Missing %GAME_EXE%
    echo Build the SDL2 Debug target first:
    echo   cmake --build build-rescue-sdl2 --config Debug
    pause
    exit /b 1
)

if not exist "%DENZI_TILESET%" (
    echo Missing %DENZI_TILESET%
    echo Regenerate it with:
    echo   python tools\build_denzi_topdown.py
    pause
    exit /b 1
)

echo Launching SteambandRedux with DENZI Ultima-style oblique top-down tiles.
echo.
echo Before starting a game you will see a DEMO preview map in the SDL window.
echo For live dungeon tiles: File -^> New (or controller N), then click the SDL window.
echo Arrow keys / controller move once a character exists.
echo Ctrl+F11 toggles top-down mode; Ctrl+F12 toggles first-person mode.
echo Ctrl+F12 first-person mode uses DENZI wall textures when fp_walls_denzi is present.
echo.
set "STEAMBAND_LOG_LEVEL=INFO"
set "STEAMBAND_START_TOPDOWN=1"
set "STEAMBAND_TOPDOWN_TILESET=%DENZI_TILESET%"
pushd "%BUILD_DIR%" >nul
start "" /wait "%GAME_EXE%"
set "GAME_EXIT=%ERRORLEVEL%"
popd >nul
exit /b %GAME_EXIT%
