#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${STEAMBAND_BUILD_DIR:-$REPO_ROOT/build-rescue-sdl2/Debug}"
GAME_EXE="$BUILD_DIR/SteambandRedux.exe"
C64_TILESET="$REPO_ROOT/lib/xtra/graf/topdown_c64_ultima.bmp"

if [[ ! -x "$GAME_EXE" && ! -f "$GAME_EXE" ]]; then
  echo "Missing $GAME_EXE" >&2
  exit 1
fi

if [[ ! -f "$C64_TILESET" ]]; then
  echo "Missing $C64_TILESET" >&2
  echo "Regenerate: python tools/generate_c64_ultima_tilesheet.py" >&2
  exit 1
fi

export STEAMBAND_LOG_LEVEL=INFO
export STEAMBAND_START_TOPDOWN=1
export STEAMBAND_TOPDOWN_TILESET="$C64_TILESET"

cd "$BUILD_DIR"
exec "$GAME_EXE"
