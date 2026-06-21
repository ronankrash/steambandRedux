# Local Ultima V Tile Test

This workflow is for private local testing only. Do not commit Ultima V art,
game files, converted BMPs, screenshots containing proprietary tiles, or copied
game data.

The repository ignores:

- `local_assets/`
- `lib/user/topdown_tileset.bmp`

## Mapping Spec

Full starter mapping for all 36 `topdown-v1` slots lives at:

- `tools/mappings/topdown-v1-ultima5-local.json`

That file is documentation plus converter input only. It does not contain
proprietary pixels. Edit it or pass `--mapping` overrides to refine tile picks
per install.

Shippable CC0 parallels for release POC builds:

- `tools/mappings/topdown-v1-kenney-cc0.json`
- `tools/mappings/topdown-v1-puny-world-cc0.json`

Validate mapping JSON with:

```bash
python tools/validate_topdown_mappings.py
```

## Expected Input

The converter accepts `TILES.16` directly from your Ultima V PC install. That file
is **LZW-compressed** on disk; the tools decompress it automatically. You can also
use a pre-decompressed raw dump (`65536` bytes).

Place a copy somewhere ignored by git, for example:

```text
local_assets/ultima5/tiles.16
```

Or point at your install folder or game file:

```bat
python tools/convert_ultima5_tiles.py --install-dir "C:\GOG Galaxy\Games\Ultima V"
python tools/convert_ultima5_tiles.py --tiles16 "C:\GOG Galaxy\Games\Ultima V\TILES.16"
```

To extract only the raw bytes:

```bash
python tools/extract_ultima5_tiles.py --install-dir "C:\GOG Galaxy\Games\Ultima V"
```

According to the Ultima Codex format notes, decompressed `tiles.16` contains 512
16x16 tiles, with 16-color rows stored as 8 bytes per row.

## Convert

Place your local file somewhere ignored, for example:

```text
local_assets/ultima5/tiles.16
```

Then run:

```bash
python tools/convert_ultima5_tiles.py --tiles16 local_assets/ultima5/tiles.16
python tools/validate_topdown_tileset.py lib/user/topdown_tileset.bmp
```

By default the converter loads `tools/mappings/topdown-v1-ultima5-local.json`.
Override with `--mapping-file path/to/custom.json` or one-off tweaks with
`--mapping 25:0x26`.

The default output `lib/user/topdown_tileset.bmp` is auto-loaded before the
checked-in placeholder atlas.

## Launch

```bat
tools\launch_topdown_tiles.cmd
```

The SDL2 top-down renderer will use `lib\user\topdown_tileset.bmp` if present.

## Refine Mapping

The default conversion only maps a small conservative sample of early Ultima V
tile IDs into `topdown-v1`. You can refine the mapping without editing code:

```bash
python tools/convert_ultima5_tiles.py --tiles16 local_assets/ultima5/tiles.16 --mapping 1:0x01,2:0x30,3:0x04
```

Mapping entries are `topdown-v1-tile-index:ultima-v-tile-index`.
