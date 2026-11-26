#!/usr/bin/env python3
"""
Generate corrupt-gzip-metadata.pmtiles fixture.

This fixture is a minimal PMTiles v3 archive with:
  - internal_compression = COMPRESSION_GZIP (0x2)
  - json_metadata_bytes > 0, starting with gzip magic bytes (0x1f 0x8b) but otherwise corrupt

Used by test PMTilesFileSource.CorruptGzipMetadata to verify that a decompression
failure on the JSON metadata block is converted to an error response instead of
propagating as an exception out of the PMTiles file source thread.
"""

import gzip
import os
import struct


def write_varint(value):
    result = bytearray()
    while value >= 0x80:
        result.append((value & 0x7f) | 0x80)
        value >>= 7
    result.append(value)
    return bytes(result)


# Root directory: zero entries, gzip-compressed because internal_compression = GZIP.
# It is valid, so the failure under test is isolated to the metadata block.
DIR_DATA = gzip.compress(write_varint(0))

# Metadata: starts with gzip magic bytes (0x1f 0x8b) but the rest is garbage, so
# util::decompress() throws.
METADATA = b'\x1f\x8b' + b'\xde\xad\xbe\xef' * 4  # 18 bytes of corrupt "gzip"

root_dir_offset = 127
root_dir_bytes = len(DIR_DATA)
json_meta_offset = root_dir_offset + root_dir_bytes
json_meta_bytes = len(METADATA)
leaf_dirs_offset = json_meta_offset + json_meta_bytes
leaf_dirs_bytes = 0
tile_data_offset = leaf_dirs_offset
tile_data_bytes = 0

hdr = b'PMTiles'
hdr += struct.pack('<B', 3)
hdr += struct.pack('<Q', root_dir_offset)
hdr += struct.pack('<Q', root_dir_bytes)
hdr += struct.pack('<Q', json_meta_offset)  # json_metadata_offset
hdr += struct.pack('<Q', json_meta_bytes)   # json_metadata_bytes > 0
hdr += struct.pack('<Q', leaf_dirs_offset)  # leaf_dirs_offset
hdr += struct.pack('<Q', leaf_dirs_bytes)   # leaf_dirs_bytes = 0
hdr += struct.pack('<Q', tile_data_offset)
hdr += struct.pack('<Q', tile_data_bytes)
hdr += struct.pack('<Q', 0)                 # addressed_tiles_count
hdr += struct.pack('<Q', 0)                 # tile_entries_count
hdr += struct.pack('<Q', 0)                 # tile_contents_count
hdr += struct.pack('<B', 0)                 # clustered = false
hdr += struct.pack('<B', 2)                 # internal_compression = GZIP
hdr += struct.pack('<B', 1)                 # tile_compression = NONE
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

out = hdr + DIR_DATA + METADATA

script_dir = os.path.dirname(os.path.abspath(__file__))
path = os.path.join(script_dir, 'corrupt-gzip-metadata.pmtiles')

with open(path, 'wb') as f:
    f.write(out)

print(f"Written {len(out)} bytes to {path}")
print("  internal_compression = GZIP (0x02) — metadata starts with gzip magic but is corrupt")
