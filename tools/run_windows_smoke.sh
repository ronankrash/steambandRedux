#!/usr/bin/env bash
# Extract Windows ZIP and launch under Wine+Xvfb with --smoke-test.
# NOTE: Godot 4.7.1 Windows binaries currently crash immediately under Wine 9.0
# in this Linux environment (null deref before engine init; stock Godot win64
# editor fails the same way). CI uses windows-latest for the authoritative smoke.
# Set BRASSDEEP_ALLOW_WINE_CRASH=1 to treat that known crash as a soft skip locally.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ZIP="${1:-$ROOT/build/Brassdeep-windows.zip}"
OUT="${2:-$ROOT/build/windows_smoke}"
LOG_DIR="$ROOT/build/reports"
mkdir -p "$LOG_DIR" "$OUT"

if [[ ! -f "$ZIP" ]]; then
  echo "Missing Windows ZIP: $ZIP" >&2
  exit 1
fi

rm -rf "$OUT"
mkdir -p "$OUT"
unzip -qo "$ZIP" -d "$OUT"

EXE="$(find "$OUT" -maxdepth 2 \( -iname 'Brassdeep.console.exe' -o -iname 'Brassdeep.exe' \) | head -n1)"
PCK="$(find "$OUT" -maxdepth 2 -iname 'Brassdeep.pck' | head -n1)"
if [[ -z "$EXE" || -z "$PCK" ]]; then
  echo "Missing Brassdeep executable or Brassdeep.pck in $OUT" >&2
  find "$OUT" -maxdepth 2 -type f | head
  exit 1
fi

# Prefer console wrapper when present for stdout capture.
CONSOLE="$(find "$OUT" -maxdepth 2 -iname 'Brassdeep.console.exe' | head -n1 || true)"
if [[ -n "$CONSOLE" ]]; then
  EXE="$CONSOLE"
fi

echo "Found EXE=$EXE PCK=$PCK"

WINE_BIN=""
if [[ -x /usr/lib/wine/wine64 ]]; then
  WINE_BIN=/usr/lib/wine/wine64
elif command -v wine64 >/dev/null 2>&1; then
  WINE_BIN="$(command -v wine64)"
elif command -v wine >/dev/null 2>&1; then
  WINE_BIN="$(command -v wine)"
else
  echo "wine not installed" >&2
  exit 1
fi

export WINEPREFIX="${WINEPREFIX:-$ROOT/build/wineprefix}"
export WINEDLLOVERRIDES="mscoree,mshtml="
mkdir -p "$WINEPREFIX"

LAUNCH_LOG="$LOG_DIR/windows_smoke_launch.log"
: >"$LAUNCH_LOG"

run_smoke() {
  cd "$(dirname "$EXE")"
  "$WINE_BIN" "$(basename "$EXE")" -- --smoke-test --instant-anims
}

set +e
if command -v xvfb-run >/dev/null 2>&1; then
  xvfb-run -a -s "-screen 0 1280x720x24" bash -c "cd \"$(dirname "$EXE\")\" && \"$WINE_BIN\" \"$(basename "$EXE")\" -- --smoke-test --instant-anims" \
    2>&1 | tee -a "$LAUNCH_LOG"
  CODE=${PIPESTATUS[0]}
else
  echo "xvfb-run missing; attempting wine without virtual display" | tee -a "$LAUNCH_LOG"
  run_smoke 2>&1 | tee -a "$LAUNCH_LOG"
  CODE=${PIPESTATUS[0]}
fi
set -e

if rg -n "AUTOMATION_RESULT: PASS" "$LAUNCH_LOG" >/dev/null 2>&1; then
  if rg -n "SCRIPT ERROR|FATAL|Failed to load|Cannot open|AUTOMATION_RESULT: FAIL" "$LAUNCH_LOG" >/dev/null 2>&1; then
    echo "Fatal patterns detected in launch log" >&2
    exit 1
  fi
  echo "Windows smoke launch OK"
  exit 0
fi

if rg -n "Unhandled page fault|Application could not be started" "$LAUNCH_LOG" >/dev/null 2>&1; then
  echo "Wine crashed before Godot initialized (known issue: Godot 4.7.1 + Wine 9)." | tee -a "$LAUNCH_LOG"
  echo "Authoritative smoke runs on GitHub windows-latest." | tee -a "$LAUNCH_LOG"
  if [[ "${BRASSDEEP_ALLOW_WINE_CRASH:-0}" == "1" ]]; then
    echo "BRASSDEEP_ALLOW_WINE_CRASH=1 — soft-skipping local Wine smoke." | tee -a "$LAUNCH_LOG"
    exit 0
  fi
fi

echo "Smoke test did not report AUTOMATION_RESULT: PASS" >&2
echo "Exit code was $CODE" | tee -a "$LAUNCH_LOG"
exit 1
