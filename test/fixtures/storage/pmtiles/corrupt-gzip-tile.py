#!/usr/bin/env python3
"""
Generate corrupt-gzip-tile.pmtiles fixture.

This fixture is a minimal PMTiles v3 archive with:
  - tile_compression = COMPRESSION_GZIP (0x2)
  - Tile data starts with gzip magic bytes (0x1f 0x8b) but is otherwise corrupt

Used by test PMTilesFileSource.CorruptGzipTile to verify that decompression
failures are converted to error responses instead of propagating as exceptions.
"""

import struct


def write_varint(value):
    result = bytearray()
    while value >= 0x80:
        result.append((value & 0x7f) | 0x80)
        value >>= 7
    result.append(value)
    return bytes(result)


# Tile data: starts with gzip magic bytes (0x1f 0x8b) so is_compressed() returns True,
# but the rest is garbage so util::decompress() throws.
TILE_DATA = b'\x1f\x8b' + b'\xde\xad\xbe\xef' * 4   # 18 bytes of corrupt "gzip"

tile_length = len(TILE_DATA)
n_entries = 1

# Serialize directory: 1 entry, tile_id=0 (z=0,x=0,y=0 → hilbert id 0)
dir_data = write_varint(n_entries)
dir_data += write_varint(0)            # tile_id delta
dir_data += write_varint(1)            # run_length = 1
dir_data += write_varint(tile_length)  # length
dir_data += write_varint(0 + 1)        # offset encoded as offset+1

root_dir_offset = 127
root_dir_bytes = len(dir_data)
json_meta_offset = root_dir_offset + root_dir_bytes
json_meta_bytes = 0
leaf_dirs_offset = json_meta_offset
leaf_dirs_bytes = 0
tile_data_offset = json_meta_offset
tile_data_bytes = tile_length

hdr = b'PMTiles'
hdr += struct.pack('<B', 3)
hdr += struct.pack('<Q', root_dir_offset)
hdr += struct.pack('<Q', root_dir_bytes)
hdr += struct.pack('<Q', json_meta_offset)  # json_metadata_offset
hdr += struct.pack('<Q', json_meta_bytes)   # json_metadata_bytes = 0
hdr += struct.pack('<Q', leaf_dirs_offset)  # leaf_dirs_offset
hdr += struct.pack('<Q', leaf_dirs_bytes)   # leaf_dirs_bytes = 0
hdr += struct.pack('<Q', tile_data_offset)
hdr += struct.pack('<Q', tile_data_bytes)
hdr += struct.pack('<Q', 1)                 # addressed_tiles_count
hdr += struct.pack('<Q', 1)                 # tile_entries_count
hdr += struct.pack('<Q', 1)                 # tile_contents_count
hdr += struct.pack('<B', 0)                 # clustered = false
hdr += struct.pack('<B', 1)                 # internal_compression = NONE
hdr += struct.pack('<B', 2)                 # tile_compression = GZIP
hdr += struct.pack('<B', 2)                 # tile_type = PNG
hdr += struct.pack('<B', 0)                 # min_zoom
hdr += struct.pack('<B', 0)                 # max_zoom
hdr += struct.pack('<i', -1800000000)
hdr += struct.pack('<i', -900000000)
hdr += struct.pack('<i',  1800000000)
hdr += struct.pack('<i',  900000000)
hdr += struct.pack('<B', 0)
hdr += struct.pack('<i', 0)
hdr += struct.pack('<i', 0)

assert len(hdr) == 127, f"Header is {len(hdr)} bytes"

out = hdr + dir_data + TILE_DATA

# Write in the same directory as this script
import os
script_dir = os.path.dirname(os.path.abspath(__file__))
path = os.path.join(script_dir, 'corrupt-gzip-tile.pmtiles')

with open(path, 'wb') as f:
    f.write(out)

print(f"Written {len(out)} bytes to {path}")
print(f"  tile_compression = GZIP (0x02) — tile data starts with gzip magic but is corrupt")
