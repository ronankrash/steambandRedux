@echo off
setlocal

set "REPO_ROOT=%~dp0.."
set "BUILD_DIR=%REPO_ROOT%\build-rescue-sdl2\Debug"
set "GAME_EXE=%BUILD_DIR%\SteambandRedux.exe"
set "C64_TILESET=%REPO_ROOT%\lib\xtra\graf\topdown_c64_ultima.bmp"

if not exist "%GAME_EXE%" (
    echo Missing %GAME_EXE%
    echo Build the SDL2 Debug target first:
    echo   cmake --build build-rescue-sdl2 --config Debug
    pause
    exit /b 1
)

if not exist "%C64_TILESET%" (
    echo Missing %C64_TILESET%
    echo Regenerate it with:
    echo   python tools\generate_c64_ultima_tilesheet.py
    pause
    exit /b 1
)

echo Launching SteambandRedux with C64/EGA orthogonal Ultima-style top-down tiles.
echo.
echo File -^> New, then focus the SDL window.
echo Ctrl+F11 toggles top-down mode at native 16x16 integer scale.
echo Ctrl+F12 toggles first-person mode.
echo.
set "STEAMBAND_LOG_LEVEL=INFO"
set "STEAMBAND_START_TOPDOWN=1"
set "STEAMBAND_TOPDOWN_TILESET=%C64_TILESET%"
pushd "%BUILD_DIR%" >nul
start "" /wait "%GAME_EXE%"
set "GAME_EXIT=%ERRORLEVEL%"
popd >nul
exit /b %GAME_EXIT%
