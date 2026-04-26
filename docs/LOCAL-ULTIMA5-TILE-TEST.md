# Local Ultima V Tile Test

This workflow is for private local testing only. Do not commit Ultima V art,
game files, converted BMPs, screenshots containing proprietary tiles, or copied
game data.

The repository ignores:

- `local_assets/`
- `lib/user/topdown_tileset.bmp`

## Expected Input

The converter expects an already-uncompressed PC `tiles.16` file from your own
Ultima V install or extraction workflow. According to the Ultima Codex format
notes, `tiles.16` contains 512 16x16 tiles, with 16-color rows stored as 8 bytes
per row.

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
