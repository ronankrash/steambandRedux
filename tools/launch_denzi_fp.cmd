@echo off
setlocal

set "REPO_ROOT=%~dp0.."
set "BUILD_DIR=%REPO_ROOT%\build-rescue-sdl2\Debug"
set "GAME_EXE=%BUILD_DIR%\SteambandRedux.exe"
set "DENZI_TILESET=%REPO_ROOT%\lib\xtra\graf\topdown_denzi.bmp"
set "DENZI_FP_WALLS=%REPO_ROOT%\lib\xtra\graf\fp_walls_denzi"

if not exist "%GAME_EXE%" (
    echo Missing %GAME_EXE%
    echo Build the SDL2 Debug target first:
    echo   cmake --build build-rescue-sdl2 --config Debug
    pause
    exit /b 1
)

if not exist "%DENZI_FP_WALLS%\wall_00_masonry.bmp" (
    echo Missing DENZI first-person wall textures.
    echo Regenerate them with:
    echo   python tools\build_denzi_fp_walls.py
    pause
    exit /b 1
)

echo Launching SteambandRedux with DENZI first-person wall textures.
echo.
echo File -^> New, then focus the SDL window.
echo Ctrl+F12 or L3+R3 toggles first-person mode with textured walls.
echo Ctrl+F11 toggles top-down oblique tiles when desired.
echo.
set "STEAMBAND_LOG_LEVEL=INFO"
set "STEAMBAND_FP_WALL_TEXTURES=%DENZI_FP_WALLS%"
set "STEAMBAND_TOPDOWN_TILESET=%DENZI_TILESET%"
pushd "%BUILD_DIR%" >nul
start "" /wait "%GAME_EXE%"
set "GAME_EXIT=%ERRORLEVEL%"
popd >nul
exit /b %GAME_EXIT%
