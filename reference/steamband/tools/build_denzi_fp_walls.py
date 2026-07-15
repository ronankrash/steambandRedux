#!/usr/bin/env python3
"""Build first-person 64x64 wall texture BMPs from DENZI oblique map tiles."""

from __future__ import annotations

import json
import struct
from pathlib import Path

from PIL import Image

from topdown_mapping import default_mapping_path, load_mapping, spritesheet_cell_size, spritesheet_pitch


ROOT = Path(__file__).resolve().parents[1]
MAPPING = default_mapping_path("fp-walls-v1-denzi-cc-by-sa.json")
TEX_SIZE = 64


def write_bmp24(path: Path, pixels: list[list[tuple[int, int, int]]]) -> None:
    width = len(pixels[0])
    height = len(pixels)
    row_stride = ((width * 3 + 3) // 4) * 4
    pixel_bytes = bytearray()
    for row in reversed(pixels):
        row_data = bytearray()
        for r, g, b in row:
            row_data.extend((b, g, r))
        row_data.extend(b"\x00" * (row_stride - width * 3))
        pixel_bytes.extend(row_data)

    header_size = 14 + 40
    file_size = header_size + len(pixel_bytes)
    with path.open("wb") as handle:
        handle.write(b"BM")
        handle.write(struct.pack("<I", file_size))
        handle.write(struct.pack("<HH", 0, 0))
        handle.write(struct.pack("<I", header_size))
        handle.write(struct.pack("<I", 40))
        handle.write(struct.pack("<ii", width, height))
        handle.write(struct.pack("<H", 1))
        handle.write(struct.pack("<H", 24))
        handle.write(struct.pack("<I", 0))
        handle.write(struct.pack("<I", len(pixel_bytes)))
        handle.write(struct.pack("<iiii", 2835, 2835, 0, 0))
        handle.write(pixel_bytes)


def image_to_pixels(image: Image.Image) -> list[list[tuple[int, int, int]]]:
    rgb = image.convert("RGB")
    width, height = rgb.size
    data = list(rgb.getdata())
    rows: list[list[tuple[int, int, int]]] = []
    for y in range(height):
        start = y * width
        rows.append(list(data[start : start + width]))
    return rows


def main() -> int:
    mapping_data = load_mapping(MAPPING)
    source = ROOT / mapping_data["source_path"]
    if not source.exists():
        raise FileNotFoundError(f"Missing source tileset: {source}")

    pitch = spritesheet_pitch(mapping_data)
    cell_size = spritesheet_cell_size(mapping_data)
    output_size = int(mapping_data.get("output_size", TEX_SIZE))
    output_dir = ROOT / mapping_data.get("output_dir", "lib/xtra/graf/fp_walls_denzi")
    output_dir.mkdir(parents=True, exist_ok=True)

    sheet = Image.open(source).convert("RGBA")
    manifest: list[dict[str, object]] = []

    for entry in mapping_data["entries"]:
        index = int(entry["index"])
        name = str(entry.get("name", f"wall_{index:02d}"))
        src_col = int(entry["source_col"])
        src_row = int(entry["source_row"])
        cell = sheet.crop((
            src_col * pitch,
            src_row * pitch,
            src_col * pitch + cell_size,
            src_row * pitch + cell_size,
        ))
        cell = cell.resize((output_size, output_size), Image.Resampling.NEAREST)
        output_path = output_dir / f"wall_{index:02d}_{name}.bmp"
        write_bmp24(output_path, image_to_pixels(cell))
        manifest.append({
            "index": index,
            "name": name,
            "path": str(output_path.relative_to(ROOT)).replace("\\", "/"),
            "source_col": src_col,
            "source_row": src_row,
        })
        print(f"Generated {output_path}")

    manifest_path = output_dir / "manifest.json"
    manifest_path.write_text(json.dumps({
        "version": mapping_data.get("atlas_version", "fp-walls-v1"),
        "texture_size": output_size,
        "textures": manifest,
    }, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {manifest_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
