@echo off
setlocal

set "REPO_ROOT=%~dp0.."
set "BUILD_DIR=%REPO_ROOT%\build-rescue-sdl2\Debug"
set "GAME_EXE=%BUILD_DIR%\SteambandRedux.exe"
set "POC_TILESET=%REPO_ROOT%\lib\xtra\graf\topdown_poc_puny_world.bmp"

if not exist "%GAME_EXE%" (
    echo Missing %GAME_EXE%
    echo Build the SDL2 Debug target first:
    echo   cmake --build build-rescue-sdl2 --config Debug
    pause
    exit /b 1
)

if not exist "%POC_TILESET%" (
    echo Missing %POC_TILESET%
    echo Regenerate it with:
    echo   python tools\build_puny_world_topdown_poc.py
    pause
    exit /b 1
)

echo Launching SteambandRedux with CC0 Puny World top-down proof-of-concept tiles.
echo.
set "STEAMBAND_LOG_LEVEL=INFO"
set "STEAMBAND_START_TOPDOWN=1"
set "STEAMBAND_TOPDOWN_TILESET=%POC_TILESET%"
pushd "%BUILD_DIR%" >nul
start "" /wait "%GAME_EXE%"
set "GAME_EXIT=%ERRORLEVEL%"
popd >nul
exit /b %GAME_EXIT%
