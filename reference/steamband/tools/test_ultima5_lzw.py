#!/usr/bin/env python3
"""Validate Ultima V LZW helper behavior."""

from __future__ import annotations

import struct
import unittest

from ultima5_lzw import (
    CLEAR_CODE,
    END_CODE,
    EXPECTED_TILES16_RAW_SIZE,
    decompress_lzw,
    is_raw_tiles16,
    is_valid_lzw_file,
)


class Ultima5LzwTests(unittest.TestCase):
    def test_raw_tiles16_size(self) -> None:
        self.assertTrue(is_raw_tiles16(bytes(EXPECTED_TILES16_RAW_SIZE)))

    def test_invalid_lzw_header(self) -> None:
        self.assertFalse(is_valid_lzw_file(b"\x00\x00\x00\x00"))

    def test_round_trip_simple_payload(self) -> None:
        # Build a minimal valid stream: CLEAR, byte 0x42, END
        # Pack codewords LSB-first at 9 bits: 0x100, 0x42, 0x101
        stream = bytearray()
        remainder = 0
        shift = 0
        for code in (CLEAR_CODE, 0x42, END_CODE):
            temp = (code << shift) + remainder
            shift += 9
            while shift >= 8:
                stream.append(temp & 0xFF)
                temp >>= 8
                shift -= 8
            remainder = temp & 0xFF
        if shift > 0:
            stream.append(remainder)

        payload = decompress_lzw(bytes(stream))
        self.assertEqual(payload, b"\x42")


if __name__ == "__main__":
    unittest.main()
