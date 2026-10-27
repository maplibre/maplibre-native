#!/usr/bin/env python3
"""Generates the low-poly .glb models the plugin demo draws with the `gltf` layer.

The models are authored for the layer's conventions: one mesh unit is one meter, Y is up, and the front of a model
faces +Z so that `model-heading: 0` points it north. Every model is symmetric left to right. The `gltf` layer only
reads material base colors, so each part is a primitive with its own flat-colored material. The colors are
written as sRGB hex, the way they look, and stored as the linear values glTF specifies.

Usage: generate_models.py [output-directory]   (default: models/, next to this script)
"""

import json
import math
import struct
import sys
from pathlib import Path


class Mesh:
    """Triangles grouped by material color. Each face has its own vertices, so shading stays flat."""

    def __init__(self):
        self.parts = {}  # color -> (positions, normals, indices)

    def _part(self, color):
        return self.parts.setdefault(tuple(color), ([], [], []))

    def quad(self, color, a, b, c, d, normal):
        positions, normals, indices = self._part(color)
        base = len(positions)
        positions.extend([a, b, c, d])
        normals.extend([normal] * 4)
        indices.extend([base, base + 1, base + 2, base, base + 2, base + 3])

    def triangle(self, color, a, b, c, normal):
        positions, normals, indices = self._part(color)
        base = len(positions)
        positions.extend([a, b, c])
        normals.extend([normal] * 3)
        indices.extend([base, base + 1, base + 2])

    def box(self, color, center, size):
        cx, cy, cz = center
        hx, hy, hz = (s / 2 for s in size)
        x0, x1, y0, y1, z0, z1 = cx - hx, cx + hx, cy - hy, cy + hy, cz - hz, cz + hz
        self.quad(color, (x0, y1, z0), (x0, y1, z1), (x1, y1, z1), (x1, y1, z0), (0, 1, 0))
        self.quad(color, (x0, y0, z0), (x1, y0, z0), (x1, y0, z1), (x0, y0, z1), (0, -1, 0))
        self.quad(color, (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1), (0, 0, 1))
        self.quad(color, (x1, y0, z0), (x0, y0, z0), (x0, y1, z0), (x1, y1, z0), (0, 0, -1))
        self.quad(color, (x1, y0, z1), (x1, y0, z0), (x1, y1, z0), (x1, y1, z1), (1, 0, 0))
        self.quad(color, (x0, y0, z0), (x0, y0, z1), (x0, y1, z1), (x0, y1, z0), (-1, 0, 0))

    def cone(self, color, center, radius, height, sides=10):
        cx, y, cz = center
        tip = (cx, y + height, cz)
        slope = radius / height
        for i in range(sides):
            a0 = 2 * math.pi * i / sides
            a1 = 2 * math.pi * (i + 1) / sides
            p0 = (cx + radius * math.cos(a0), y, cz + radius * math.sin(a0))
            p1 = (cx + radius * math.cos(a1), y, cz + radius * math.sin(a1))
            mid = (a0 + a1) / 2
            n = (math.cos(mid), slope, math.sin(mid))
            length = math.sqrt(sum(v * v for v in n))
            self.triangle(color, p1, p0, tip, tuple(v / length for v in n))
            self.triangle(color, p0, p1, (cx, y, cz), (0, -1, 0))


def pad(data, filler):
    return data + filler * (-len(data) % 4)


def glb(mesh):
    binary = bytearray()
    views, accessors, primitives, materials = [], [], [], []
    for color, (positions, normals, indices) in mesh.parts.items():
        lows = [min(p[i] for p in positions) for i in range(3)]
        highs = [max(p[i] for p in positions) for i in range(3)]
        accessor_ids = []
        for data, fmt, ctype, kind, target, bounds in (
            (positions, "fff", 5126, "VEC3", 34962, (lows, highs)),
            (normals, "fff", 5126, "VEC3", 34962, None),
            (indices, "H", 5123, "SCALAR", 34963, None),
        ):
            offset = len(binary)
            for item in data:
                binary += struct.pack("<" + fmt, *item) if isinstance(item, tuple) else struct.pack("<" + fmt, item)
            binary += b"\0" * (-len(binary) % 4)
            views.append({"buffer": 0, "byteOffset": offset, "byteLength": len(data) * struct.calcsize(fmt), "target": target})
            accessor = {"bufferView": len(views) - 1, "componentType": ctype, "count": len(data), "type": kind}
            if bounds:
                accessor["min"], accessor["max"] = bounds
            accessors.append(accessor)
            accessor_ids.append(len(accessors) - 1)
        materials.append({"pbrMetallicRoughness": {"baseColorFactor": list(color), "metallicFactor": 0, "roughnessFactor": 1}})
        primitives.append({
            "attributes": {"POSITION": accessor_ids[0], "NORMAL": accessor_ids[1]},
            "indices": accessor_ids[2],
            "material": len(materials) - 1,
            "mode": 4,
        })
    document = {
        "asset": {"version": "2.0", "generator": "plugins/demo/assets/generate_models.py"},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [{"mesh": 0}],
        "meshes": [{"primitives": primitives}],
        "materials": materials,
        "accessors": accessors,
        "bufferViews": views,
        "buffers": [{"byteLength": len(binary)}],
    }
    json_chunk = pad(json.dumps(document, separators=(",", ":")).encode(), b" ")
    bin_chunk = pad(bytes(binary), b"\0")
    total = 12 + 8 + len(json_chunk) + 8 + len(bin_chunk)
    return (
        struct.pack("<III", 0x46546C67, 2, total)
        + struct.pack("<II", len(json_chunk), 0x4E4F534A) + json_chunk
        + struct.pack("<II", len(bin_chunk), 0x004E4942) + bin_chunk
    )


def srgb_to_linear(channel):
    return channel / 12.92 if channel <= 0.04045 else ((channel + 0.055) / 1.055) ** 2.4


def rgb(hex_color, alpha=1.0):
    """A base color factor from a hex color written the way it looks (sRGB): glTF gives base colors in linear."""
    value = int(hex_color.lstrip("#"), 16)
    return tuple(round(srgb_to_linear(((value >> shift) & 255) / 255), 6) for shift in (16, 8, 0)) + (alpha,)


def car(body_color):
    """A 4.5 m long, 1.9 m wide hatchback."""
    mesh = Mesh()
    mesh.box(rgb(body_color), (0, 0.65, 0), (1.9, 0.7, 4.5))
    mesh.box(rgb("#2B3A4A"), (0, 1.25, -0.15), (1.7, 0.6, 2.5))
    mesh.box(rgb(body_color), (0, 1.57, -0.15), (1.7, 0.06, 2.1))
    for x in (-0.95, 0.95):
        for z in (-1.4, 1.4):
            mesh.box(rgb("#1B1B1D"), (x, 0.32, z), (0.25, 0.64, 0.64))
    for x in (-0.65, 0.65):
        mesh.box(rgb("#FFF3C4"), (x, 0.75, 2.26), (0.4, 0.18, 0.05))
        mesh.box(rgb("#C62828"), (x, 0.75, -2.26), (0.4, 0.18, 0.05))
    return mesh


def tree():
    """A 7 m tree: trunk and a two-tier cone canopy."""
    mesh = Mesh()
    mesh.box(rgb("#6B4A2B"), (0, 1.0, 0), (0.5, 2.0, 0.5))
    mesh.cone(rgb("#2E7D32"), (0, 1.6, 0), 2.2, 3.6)
    mesh.cone(rgb("#388E3C"), (0, 3.6, 0), 1.6, 3.4)
    return mesh


MODELS = {
    "car-red.glb": car("#D32F2F"),
    "car-blue.glb": car("#1976D2"),
    "car-yellow.glb": car("#F9A825"),
    "tree.glb": tree(),
}


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parent / "models"
    out.mkdir(parents=True, exist_ok=True)
    for name, mesh in MODELS.items():
        data = glb(mesh)
        (out / name).write_bytes(data)
        print(f"{out / name} ({len(data)} bytes)")


if __name__ == "__main__":
    main()
