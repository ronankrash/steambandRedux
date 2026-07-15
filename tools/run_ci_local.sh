#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GODOT_VERSION="4.7.1-stable"
GODOT_BIN="${GODOT_BIN:-$ROOT/tools/bin/godot}"
TEMPLATE_DIR="${HOME}/.local/share/godot/export_templates/4.7.1.stable"

mkdir -p "$ROOT/tools/bin" "$ROOT/build"

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

echo "== Godot import =="
"$GODOT_BIN" --headless --path "$ROOT/game" --import || true
"$GODOT_BIN" --headless --path "$ROOT/game" --quit-after 1

echo "== Headless tests =="
set +e
"$GODOT_BIN" --headless --path "$ROOT/game" --script res://tests/run_headless_tests.gd
TEST_EXIT=$?
set -e
if [[ "$TEST_EXIT" -ne 0 ]]; then
  echo "Headless tests failed with exit $TEST_EXIT" >&2
  exit "$TEST_EXIT"
fi

if [[ "${BRASSDEEP_EXPORT:-0}" == "1" ]]; then
  echo "== Windows export =="
  if [[ ! -f "$ROOT/game/export_presets.cfg" ]]; then
    echo "Missing export_presets.cfg" >&2
    exit 1
  fi
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
  (
    cd "$ROOT/build"
    zip -r Brassdeep-windows.zip Brassdeep.exe Brassdeep.pck 2>/dev/null \
      || zip -r Brassdeep-windows.zip . -i 'Brassdeep*'
  )
  echo "Artifact: $ROOT/build/Brassdeep-windows.zip"
fi

echo "All local checks passed."
