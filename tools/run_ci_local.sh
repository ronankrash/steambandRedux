#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GODOT_VERSION="4.7.1-stable"
GODOT_BIN="${GODOT_BIN:-$ROOT/tools/bin/godot}"
TEMPLATE_DIR="${HOME}/.local/share/godot/export_templates/4.7.1.stable"
REPORTS="$ROOT/build/reports"
mkdir -p "$ROOT/tools/bin" "$ROOT/build" "$REPORTS"

if [[ ! -x "$GODOT_BIN" ]]; then
  echo "Downloading Godot $GODOT_VERSION..."
  tmp="$(mktemp -d)"
  curl -fsSL -o "$tmp/godot.zip" \
    "https://github.com/godotengine/godot-builds/releases/download/${GODOT_VERSION}/Godot_v${GODOT_VERSION}_linux.x86_64.zip"
  unzip -o "$tmp/godot.zip" -d "$tmp"
  mv "$tmp"/Godot_v*_linux.x86_64 "$GODOT_BIN"
  chmod +x "$GODOT_BIN"
fi

echo "== Content validation =="
python3 "$ROOT/tools/validate_content.py"

echo "== Content reachability =="
python3 "$ROOT/tools/content_reachability.py"

echo "== Focus graph audit =="
python3 "$ROOT/tools/focus_graph_audit.py"

echo "== Godot import =="
"$GODOT_BIN" --headless --path "$ROOT/game" --import
"$GODOT_BIN" --headless --path "$ROOT/game" --quit-after 1

echo "== Unit / simulation tests =="
"$GODOT_BIN" --headless --path "$ROOT/game" --script res://tests/run_headless_tests.gd

echo "== Stability tests =="
"$GODOT_BIN" --headless --path "$ROOT/game" --script res://tests/run_stability_tests.gd

echo "== Dungeon seed connectivity (500) =="
"$GODOT_BIN" --headless --path "$ROOT/game" --script res://tests/run_dungeon_seeds.gd

echo "== Presentation E2E (headless scene) =="
set +e
"$GODOT_BIN" --headless --path "$ROOT/game" --script res://tests/run_presentation_e2e.gd
E2E_CODE=$?
set -e
if [[ "$E2E_CODE" -ne 0 ]]; then
  echo "Presentation E2E failed with $E2E_CODE" >&2
  exit "$E2E_CODE"
fi

if [[ "${BRASSDEEP_EXPORT:-0}" == "1" ]]; then
  echo "== Windows export =="
  if [[ ! -d "$TEMPLATE_DIR" ]]; then
    echo "Downloading export templates..."
    tmp="$(mktemp -d)"
    curl -fsSL -o "$tmp/templates.tpz" \
      "https://github.com/godotengine/godot-builds/releases/download/${GODOT_VERSION}/Godot_v${GODOT_VERSION}_export_templates.tpz"
    mkdir -p "$TEMPLATE_DIR"
    unzip -o "$tmp/templates.tpz" -d "$tmp/templates"
    cp -a "$tmp/templates/templates/"* "$TEMPLATE_DIR/"
  fi
  mkdir -p "$ROOT/build"
  "$GODOT_BIN" --headless --path "$ROOT/game" --export-release "Windows Desktop" "$ROOT/build/Brassdeep.exe"
  test -f "$ROOT/build/Brassdeep.exe"
  test -f "$ROOT/build/Brassdeep.pck"
  (
    cd "$ROOT/build"
    rm -f Brassdeep-windows.zip
    zip -r Brassdeep-windows.zip Brassdeep.exe Brassdeep.pck
    if [[ -f Brassdeep.console.exe ]]; then
      zip -u Brassdeep-windows.zip Brassdeep.console.exe
    fi
  )
  echo "Artifact: $ROOT/build/Brassdeep-windows.zip"

  if [[ "${BRASSDEEP_WINE_SMOKE:-0}" == "1" ]]; then
    echo "== Windows Wine smoke (optional; often crashes on Wine 9) =="
    chmod +x "$ROOT/tools/run_windows_smoke.sh"
    BRASSDEEP_ALLOW_WINE_CRASH="${BRASSDEEP_ALLOW_WINE_CRASH:-1}" \
      "$ROOT/tools/run_windows_smoke.sh" "$ROOT/build/Brassdeep-windows.zip"
  else
    echo "== Skipping local Wine smoke (CI uses windows-latest). Set BRASSDEEP_WINE_SMOKE=1 to attempt. =="
  fi
fi

# Optional screenshot pass with virtual display (non-headless)
if [[ "${BRASSDEEP_SCREENSHOTS:-0}" == "1" ]]; then
  echo "== Screenshot capture =="
  mkdir -p "$REPORTS/screenshots"
  if command -v xvfb-run >/dev/null 2>&1; then
    xvfb-run -a -s "-screen 0 1280x720x24" \
      env BRASSDEEP_CAPTURE=1 \
      "$GODOT_BIN" --path "$ROOT/game" --resolution 1280x720 --script res://tests/run_presentation_e2e.gd \
      | tee "$REPORTS/screenshot_run.log"
    # Copy from Godot user:// if present
    USER_SHOTS="$HOME/.local/share/godot/app_userdata/Brassdeep/screenshots"
    if [[ -d "$USER_SHOTS" ]]; then
      cp -a "$USER_SHOTS/." "$REPORTS/screenshots/" || true
    fi
  else
    echo "xvfb-run not available; skipping screenshots" >&2
    exit 1
  fi
fi

echo "All local checks passed."
