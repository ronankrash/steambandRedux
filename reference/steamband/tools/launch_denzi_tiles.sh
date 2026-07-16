#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${STEAMBAND_BUILD_DIR:-$REPO_ROOT/build-rescue-sdl2/Debug}"
GAME_EXE="$BUILD_DIR/SteambandRedux.exe"
DENZI_TILESET="$REPO_ROOT/lib/xtra/graf/topdown_denzi.bmp"

if [[ ! -x "$GAME_EXE" && ! -f "$GAME_EXE" ]]; then
  echo "Missing $GAME_EXE" >&2
  echo "Build the SDL2 Debug target first:" >&2
  echo "  cmake --build build-rescue-sdl2 --config Debug" >&2
  exit 1
fi

if [[ ! -f "$DENZI_TILESET" ]]; then
  echo "Missing $DENZI_TILESET" >&2
  echo "Regenerate it with:" >&2
  echo "  python tools/build_denzi_topdown.py" >&2
  exit 1
fi

echo "Launching SteambandRedux with DENZI Ultima-style oblique top-down tiles."
echo
echo "Before starting a game you will see a DEMO preview map in the SDL window."
echo "For live dungeon tiles: File -> New, then focus the SDL window."
echo "Ctrl+F11 toggles top-down mode; Ctrl+F12 toggles first-person mode."
echo

export STEAMBAND_LOG_LEVEL=INFO
export STEAMBAND_START_TOPDOWN=1
export STEAMBAND_TOPDOWN_TILESET="$DENZI_TILESET"

cd "$BUILD_DIR"
exec "$GAME_EXE"
