#!/usr/bin/env python3
"""Generates the models the gltf render tests draw: unit boxes in a single flat color.

A box is one mesh unit on every side and stands on the ground (Y from 0 to 1), so `model-scale` is its size in meters.

Usage: generate_render_test_models.py [output-directory]   (default: ../render-tests/assets, next to this script)
"""

import json
import struct
import sys
from pathlib import Path

# (normal, four corners counter-clockwise seen from outside) for each face of the unit box.
FACES = [
    ((0, 1, 0), [(-0.5, 1, -0.5), (-0.5, 1, 0.5), (0.5, 1, 0.5), (0.5, 1, -0.5)]),
    ((0, -1, 0), [(-0.5, 0, -0.5), (0.5, 0, -0.5), (0.5, 0, 0.5), (-0.5, 0, 0.5)]),
    ((0, 0, 1), [(-0.5, 0, 0.5), (0.5, 0, 0.5), (0.5, 1, 0.5), (-0.5, 1, 0.5)]),
    ((0, 0, -1), [(0.5, 0, -0.5), (-0.5, 0, -0.5), (-0.5, 1, -0.5), (0.5, 1, -0.5)]),
    ((1, 0, 0), [(0.5, 0, 0.5), (0.5, 0, -0.5), (0.5, 1, -0.5), (0.5, 1, 0.5)]),
    ((-1, 0, 0), [(-0.5, 0, -0.5), (-0.5, 0, 0.5), (-0.5, 1, 0.5), (-0.5, 1, -0.5)]),
]

MODELS = {
    "box-red.glb": (0.85, 0.12, 0.1, 1.0),
    "box-blue.glb": (0.1, 0.3, 0.9, 1.0),
}


def box(color):
    positions, normals, indices = [], [], []
    for normal, corners in FACES:
        base = len(positions)
        positions += corners
        normals += [normal] * 4
        indices += [base, base + 1, base + 2, base, base + 2, base + 3]

    binary = bytearray()
    views, accessors = [], []
    for data, fmt, component_type, kind, target in (
        (positions, "<fff", 5126, "VEC3", 34962),
        (normals, "<fff", 5126, "VEC3", 34962),
        (indices, "<H", 5123, "SCALAR", 34963),
    ):
        offset = len(binary)
        for item in data:
            binary += struct.pack(fmt, *item) if isinstance(item, tuple) else struct.pack(fmt, item)
        binary += b"\0" * (-len(binary) % 4)
        views.append({"buffer": 0, "byteOffset": offset, "byteLength": len(data) * struct.calcsize(fmt), "target": target})
        accessor = {"bufferView": len(views) - 1, "componentType": component_type, "count": len(data), "type": kind}
        if kind == "VEC3" and data is positions:
            accessor["min"] = [min(p[i] for p in positions) for i in range(3)]
            accessor["max"] = [max(p[i] for p in positions) for i in range(3)]
        accessors.append(accessor)

    document = {
        "asset": {"version": "2.0", "generator": "plugins/gltf-layer/scripts/generate_render_test_models.py"},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [{"mesh": 0}],
        "meshes": [{"primitives": [{"attributes": {"POSITION": 0, "NORMAL": 1}, "indices": 2, "material": 0, "mode": 4}]}],
        "materials": [{"pbrMetallicRoughness": {"baseColorFactor": list(color), "metallicFactor": 0, "roughnessFactor": 1}}],
        "accessors": accessors,
        "bufferViews": views,
        "buffers": [{"byteLength": len(binary)}],
    }
    json_chunk = json.dumps(document, separators=(",", ":")).encode()
    json_chunk += b" " * (-len(json_chunk) % 4)
    total = 12 + 8 + len(json_chunk) + 8 + len(binary)
    return (
        struct.pack("<III", 0x46546C67, 2, total)
        + struct.pack("<II", len(json_chunk), 0x4E4F534A) + json_chunk
        + struct.pack("<II", len(binary), 0x004E4942) + bytes(binary)
    )


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent / "render-tests" / "assets"
    out.mkdir(parents=True, exist_ok=True)
    for name, color in MODELS.items():
        (out / name).write_bytes(box(color))
        print(out / name)


if __name__ == "__main__":
    main()
