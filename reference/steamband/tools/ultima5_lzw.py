"""Ultima V PC LZW helpers for TILES.16 and related graphics files.

Format notes (Ultima Codex):
- Compressed files: uint32 little-endian uncompressed length, then LZW stream.
- TILES.16 uncompressed payload is 65536 bytes (512 x 16x16 tiles, 8 bytes/row).
- LZW uses CLEAR=0x100, END=0x101, 9-bit start, LSB-first bit packing.
"""

from __future__ import annotations

import struct
from pathlib import Path

CLEAR_CODE = 0x100
END_CODE = 0x101
MAX_CODEWORD_BITS = 12
EXPECTED_TILES16_RAW_SIZE = 65536
TILE_ROW_BYTES = 8
TILE_PIXEL_ROWS = 16
TILE_RAW_BYTES = TILE_ROW_BYTES * TILE_PIXEL_ROWS


def is_valid_lzw_file(file_bytes: bytes) -> bool:
    if len(file_bytes) < 6:
        return False
    if file_bytes[3] != 0:
        return False
    if file_bytes[4] != 0 or (file_bytes[5] & 1) != 1:
        return False
    return True


def get_uncompressed_size(file_bytes: bytes) -> int:
    return struct.unpack_from("<I", file_bytes, 0)[0]


def _read_codeword(source: bytes, bits_read: int, codeword_size: int) -> tuple[int, int]:
    byte_index = bits_read // 8
    b0 = source[byte_index] if byte_index < len(source) else 0
    b1 = source[byte_index + 1] if byte_index + 1 < len(source) else 0
    b2 = source[byte_index + 2] if byte_index + 2 < len(source) else 0
    codeword = ((b2 << 16) | (b1 << 8) | b0) >> (bits_read % 8)
    mask = {9: 0x1FF, 10: 0x3FF, 11: 0x7FF, 12: 0xFFF}.get(codeword_size)
    if mask is None:
        raise ValueError(f"unsupported codeword size {codeword_size}")
    codeword &= mask
    return codeword, bits_read + codeword_size


def decompress_lzw(source: bytes) -> bytes:
    """Decompress Ultima V LZW payload (without the 4-byte length header)."""
    dict_roots: list[int] = [0] * 10000
    dict_codes: list[int] = [0] * 10000
    dict_contains = 0x102
    stack: list[int] = []

    def dict_init() -> None:
        nonlocal dict_contains
        dict_contains = 0x102

    def dict_add(root: int, codeword: int) -> None:
        nonlocal dict_contains
        dict_roots[dict_contains] = root
        dict_codes[dict_contains] = codeword
        dict_contains += 1

    def get_string(codeword: int) -> int:
        stack.clear()
        current = codeword
        while current > 0xFF:
            root = dict_roots[current]
            current = dict_codes[current]
            stack.append(root)
        stack.append(current)
        return stack[-1]

    destination = bytearray()
    bits_read = 0
    codeword_size = 9
    next_free_codeword = 0x102
    dictionary_size = 0x200
    previous_word = 0
    end_marker_reached = False

    while not end_marker_reached:
        codeword, bits_read = _read_codeword(source, bits_read, codeword_size)

        if codeword == CLEAR_CODE:
            codeword_size = 9
            next_free_codeword = 0x102
            dictionary_size = 0x200
            dict_init()
            codeword, bits_read = _read_codeword(source, bits_read, codeword_size)
            destination.append(codeword & 0xFF)
            previous_word = codeword
            continue

        if codeword == END_CODE:
            end_marker_reached = True
            continue

        if codeword < next_free_codeword:
            get_string(codeword)
            first_char = stack[-1]
            while stack:
                destination.append(stack.pop() & 0xFF)
            dict_add(first_char, previous_word)
            next_free_codeword += 1
            if next_free_codeword >= dictionary_size and codeword_size < MAX_CODEWORD_BITS:
                codeword_size += 1
                dictionary_size *= 2
        else:
            get_string(previous_word)
            first_char = stack[-1]
            while stack:
                destination.append(stack.pop() & 0xFF)
            destination.append(first_char & 0xFF)
            if codeword != next_free_codeword:
                raise ValueError("invalid LZW stream: codeword ahead of dictionary")
            dict_add(first_char, previous_word)
            next_free_codeword += 1
            if next_free_codeword >= dictionary_size and codeword_size < MAX_CODEWORD_BITS:
                codeword_size += 1
                dictionary_size *= 2

        previous_word = codeword

    return bytes(destination)


def decompress_lzw_file(file_bytes: bytes) -> bytes:
    if not is_valid_lzw_file(file_bytes):
        raise ValueError("file does not look like Ultima V LZW data")
    expected = get_uncompressed_size(file_bytes)
    payload = decompress_lzw(file_bytes[4:])
    if len(payload) < expected:
        raise ValueError(f"LZW output {len(payload)} bytes, expected at least {expected}")
    return payload[:expected]


def is_raw_tiles16(file_bytes: bytes) -> bool:
    return len(file_bytes) == EXPECTED_TILES16_RAW_SIZE and len(file_bytes) % TILE_RAW_BYTES == 0


def load_tiles16_bytes(path: Path) -> bytes:
    data = path.read_bytes()
    if is_raw_tiles16(data):
        return data
    if is_valid_lzw_file(data):
        raw = decompress_lzw_file(data)
        if not is_raw_tiles16(raw):
            raise ValueError(
                f"decompressed {path} to {len(raw)} bytes; expected {EXPECTED_TILES16_RAW_SIZE}"
            )
        return raw
    raise ValueError(
        f"{path} is neither raw Ultima V tiles.16 ({EXPECTED_TILES16_RAW_SIZE} bytes) "
        f"nor LZW-compressed TILES.16 (uint32 length + stream)."
    )


def find_tiles16_in_install(install_dir: Path) -> Path | None:
    candidates = [
        install_dir / "TILES.16",
        install_dir / "tiles.16",
    ]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    return None


def tiles16_usage_hint() -> str:
    return (
        "Obtain TILES.16 from your own Ultima V PC install (GOG/physical/disc copy).\n"
        "Copy it locally, for example:\n"
        "  local_assets/ultima5/tiles.16\n"
        "Or pass the game folder or file directly:\n"
        "  python tools/convert_ultima5_tiles.py --install-dir \"C:\\Games\\Ultima5\"\n"
        "  python tools/convert_ultima5_tiles.py --tiles16 \"C:\\Games\\Ultima5\\TILES.16\"\n"
        "The converter accepts both LZW-compressed game files and pre-decompressed raw dumps."
    )
