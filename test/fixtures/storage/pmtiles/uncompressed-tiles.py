#!/usr/bin/env python3
"""
Generate uncompressed-tiles.pmtiles fixture.

This fixture is a minimal PMTiles v3 archive with:
  - tile_compression = COMPRESSION_GZIP (0x2) — header claims gzip-compressed tiles
  - Actual tile data: raw PNG signature bytes (0x89 0x50...), NOT gzip-wrapped

Used by test PMTilesFileSource.UncompressedTile to verify that the implementation
correctly handles uncompressed tiles within an archive that claims compression.
"""

import struct


def write_varint(value):
    result = bytearray()
    while value >= 0x80:
        result.append((value & 0x7f) | 0x80)
        value >>= 7
    result.append(value)
    return bytes(result)


# Tile data: PNG signature bytes (0x89 0x50...) so is_compressed() returns false.
TILE_DATA = b'\x89PNG\r\n\x1a\n' + b'\x00' * 20  # 28 bytes

tile_length = len(TILE_DATA)
n_entries = 1

# Serialize directory: 1 entry, tile_id=0 (z=0,x=0,y=0 → hilbert id 0)
dir_data = write_varint(n_entries)
dir_data += write_varint(0)           # tile_id delta (0 - 0 = 0)
dir_data += write_varint(1)           # run_length = 1
dir_data += write_varint(tile_length) # length
dir_data += write_varint(0 + 1)       # offset = 0, encoded as offset+1 = 1 (first entry)

root_dir_offset = 127
root_dir_bytes = len(dir_data)
json_meta_offset = root_dir_offset + root_dir_bytes
json_meta_bytes = 0
leaf_dirs_offset = json_meta_offset
leaf_dirs_bytes = 0
tile_data_offset = json_meta_offset   # tile data immediately after directory
tile_data_bytes = tile_length

hdr = b'PMTiles'
hdr += struct.pack('<B', 3)                  # version 3 (offset 7)
hdr += struct.pack('<Q', root_dir_offset)    # offset 8
hdr += struct.pack('<Q', root_dir_bytes)     # offset 16
hdr += struct.pack('<Q', json_meta_offset)   # offset 24
hdr += struct.pack('<Q', json_meta_bytes)    # offset 32
hdr += struct.pack('<Q', leaf_dirs_offset)   # offset 40
hdr += struct.pack('<Q', leaf_dirs_bytes)    # offset 48
hdr += struct.pack('<Q', tile_data_offset)   # offset 56
hdr += struct.pack('<Q', tile_data_bytes)    # offset 64
hdr += struct.pack('<Q', 1)                  # addressed_tiles_count  (offset 72)
hdr += struct.pack('<Q', 1)                  # tile_entries_count     (offset 80)
hdr += struct.pack('<Q', 1)                  # tile_contents_count    (offset 88)
hdr += struct.pack('<B', 0)                  # clustered = false      (offset 96)
hdr += struct.pack('<B', 1)                  # internal_compression = NONE (offset 97)
hdr += struct.pack('<B', 2)                  # tile_compression = GZIP (offset 98) ← KEY
hdr += struct.pack('<B', 2)                  # tile_type = PNG        (offset 99)
hdr += struct.pack('<B', 0)                  # min_zoom               (offset 100)
hdr += struct.pack('<B', 0)                  # max_zoom               (offset 101)
hdr += struct.pack('<i', -1800000000)        # min_lon_e7             (offset 102)
hdr += struct.pack('<i', -900000000)         # min_lat_e7             (offset 106)
hdr += struct.pack('<i',  1800000000)        # max_lon_e7             (offset 110)
hdr += struct.pack('<i',  900000000)         # max_lat_e7             (offset 114)
hdr += struct.pack('<B', 0)                  # center_zoom            (offset 118)
hdr += struct.pack('<i', 0)                  # center_lon_e7          (offset 119)
hdr += struct.pack('<i', 0)                  # center_lat_e7          (offset 123)
                                             # total = 127            (offset 127)

assert len(hdr) == 127, f"Header is {len(hdr)} bytes, expected 127"

out = hdr + dir_data + TILE_DATA

# Write in the same directory as this script
import os
script_dir = os.path.dirname(os.path.abspath(__file__))
path = os.path.join(script_dir, 'uncompressed-tiles.pmtiles')

with open(path, 'wb') as f:
    f.write(out)

print(f"Written {len(out)} bytes to {path}")
print(f"  Header:     127 bytes  (offset 0)")
print(f"  Root dir:   {len(dir_data)} bytes   (offset {root_dir_offset})")
print(f"  Tile data:  {tile_length} bytes  (offset {tile_data_offset})")
print(f"  tile_compression field = 0x02 (GZIP) — but tiles are raw PNG bytes (not gzip)")
