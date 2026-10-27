#!/usr/bin/env python3
"""Prepares the Eiffel tower that the gltf render tests draw, from a CC0 model of the 3D Model Repository.

Source: https://conic.openstreetmap.de/model/4 (revision 3), "Eiffel Tower" by n42k. The repository gives it the
Creative Commons CC0 1.0 Universal Public Domain Dedication, and says it was taken from
https://www.blendswap.com/blends/view/67944.

The gltf layer draws triangles with the base color factor of their material and nothing else, and its meshes have
16-bit indices. The source is 10.7 MB with 34 textures and 166,000 vertices, so this script reduces it to what the layer
can draw:

* It keeps the tower and drops meshes 4, 6 and 8: the first-floor railing frame and the guard rail, 160,000 vertices.
* It drops the texture coordinates and the textures. The tower's frames are lattice textures with transparent gaps, so a
  textured material gets the average color of its texture's opaque pixels as its base color factor (COLORS).
* Each source material stays a primitive, and the units stay meters, with the base of the tower on the ground (Y = 0).

Usage: prepare_eiffel_tower.py [source.glb] [output.glb]
  source.glb   defaults to a download of the pinned revision, which is checked against SOURCE_SHA256
  output.glb   defaults to ../render-tests/assets/eiffel-tower.glb, next to this script
"""

import hashlib
import json
import struct
import sys
import urllib.request
from pathlib import Path

SOURCE_URL = "https://conic.openstreetmap.de/api/model/4/3"
SOURCE_SHA256 = "6cbbb7cbbfe55c1d32d32df40c6d99b89db24b2eaa83960d686a779b3d75826f"
COPYRIGHT = (
    "CC0 1.0: Eiffel Tower by n42k, 3D Model Repository model 4 revision 3 "
    "(https://conic.openstreetmap.de/model/4, from https://www.blendswap.com/blends/view/67944), "
    "reduced by plugins/plugins/gltf-layer/scripts/prepare_eiffel_tower.py"
)

KEEP_MESHES = {0, 1, 2, 3, 5, 7}

# Linear base colors of the textured materials: the average of the texture over its opaque pixels (alpha >= 0.5),
# times the material's base color factor.
COLORS = {
    "frame3_": (0.191, 0.143, 0.107),
    "M_1": (0.319, 0.225, 0.165),
    "internal": (0.180, 0.139, 0.105),
    "profile": (0.272, 0.211, 0.150),
    "M_6": (0.297, 0.218, 0.166),
    "arch": (0.772, 0.693, 0.514),
    "middle": (0.329, 0.247, 0.192),
    "frame1_": (0.163, 0.122, 0.093),
    "frame": (0.196, 0.146, 0.110),
    "basestone": (0.287, 0.219, 0.176),
    "frame2_": (0.193, 0.144, 0.109),
    "M_4_8_": (0.180, 0.135, 0.102),
    "M_4_9_": (0.181, 0.136, 0.102),
    "M_4_5_": (0.183, 0.137, 0.103),
    "M_2": (0.339, 0.240, 0.175),
    "frame2_2_": (0.204, 0.152, 0.114),
    "frame2_3_": (0.205, 0.152, 0.115),
    "M_4_7_": (0.179, 0.134, 0.101),
    "frame2_2__frame2_2_.png.001": (0.225, 0.167, 0.125),
    "M_4_1_": (0.173, 0.130, 0.098),
    "frame2_1_": (0.221, 0.164, 0.123),
    "top": (0.164, 0.161, 0.154),
    "M_3": (0.244, 0.176, 0.129),
    "M_4_3_": (0.182, 0.137, 0.103),
    "M_4": (0.184, 0.137, 0.104),
    "M_4_4_": (0.181, 0.136, 0.102),
    "top_middle": (0.237, 0.195, 0.153),
    "_Gray6_": (0.136, 0.117, 0.093),
    "frame2_4_": (0.222, 0.165, 0.123),
    "M_4_6_": (0.178, 0.133, 0.101),
    "frame2": (0.204, 0.152, 0.114),
    "M_4_2_": (0.180, 0.135, 0.102),
    "entrance": (0.130, 0.114, 0.105),
    "window": (0.103, 0.085, 0.074),
    "_Charcoal_": (0.196, 0.213, 0.096),
}

FORMATS = {5126: "f", 5125: "I", 5123: "H"}
COMPONENTS = {"SCALAR": 1, "VEC3": 3}


def read_glb(data):
    magic, version, _ = struct.unpack_from("<4sII", data, 0)
    if magic != b"glTF" or version != 2:
        raise SystemExit("not a binary glTF 2.0 file")
    document = binary = None
    offset = 12
    while offset < len(data):
        size, kind = struct.unpack_from("<I4s", data, offset)
        chunk = data[offset + 8 : offset + 8 + size]
        if kind == b"JSON":
            document = json.loads(chunk)
        elif kind == b"BIN\0":
            binary = chunk
        offset += 8 + size
    return document, binary


def read_accessor(document, binary, index):
    """The elements of an accessor, as tuples."""
    accessor = document["accessors"][index]
    view = document["bufferViews"][accessor["bufferView"]]
    fmt = "<%d%s" % (COMPONENTS[accessor["type"]], FORMATS[accessor["componentType"]])
    start = view.get("byteOffset", 0) + accessor.get("byteOffset", 0)
    stride = view.get("byteStride", struct.calcsize(fmt))
    return [struct.unpack_from(fmt, binary, start + i * stride) for i in range(accessor["count"])]


def pad(data, filler):
    return data + filler * (-len(data) % 4)


def write_glb(document, binary):
    document_bytes = pad(json.dumps(document, separators=(",", ":")).encode("utf-8"), b" ")
    binary = pad(bytes(binary), b"\0")
    total = 12 + 8 + len(document_bytes) + 8 + len(binary)
    return (
        struct.pack("<4sII", b"glTF", 2, total)
        + struct.pack("<I4s", len(document_bytes), b"JSON")
        + document_bytes
        + struct.pack("<I4s", len(binary), b"BIN\0")
        + binary
    )


def prepare(source):
    document, binary = read_glb(source)
    assert all(not {"matrix", "translation", "rotation", "scale"} & node.keys() for node in document["nodes"])

    out_binary = bytearray()
    views, accessors, primitives, materials = [], [], [], []
    material_ids = {}

    def add(data, component_type, kind, target, count, bounds=None):
        out_binary.extend(b"\0" * (-len(out_binary) % 4))
        views.append({"buffer": 0, "byteOffset": len(out_binary), "byteLength": len(data), "target": target})
        accessor = {"bufferView": len(views) - 1, "componentType": component_type, "count": count, "type": kind}
        if bounds:
            accessor["min"], accessor["max"] = bounds
        accessors.append(accessor)
        out_binary.extend(data)
        return len(accessors) - 1

    for mesh_index in sorted(KEEP_MESHES):
        for primitive in document["meshes"][mesh_index]["primitives"]:
            assert primitive.get("mode", 4) == 4 and "indices" in primitive
            positions = read_accessor(document, binary, primitive["attributes"]["POSITION"])
            normals = read_accessor(document, binary, primitive["attributes"]["NORMAL"])
            indices = [i[0] for i in read_accessor(document, binary, primitive["indices"])]
            assert len(positions) < 65536, "a primitive needs more than 16-bit indices"

            source_material = document["materials"][primitive["material"]]
            name = source_material["name"]
            if name not in material_ids:
                pbr = source_material["pbrMetallicRoughness"]
                if "baseColorTexture" in pbr:
                    color = COLORS[name]
                else:
                    color = pbr["baseColorFactor"][:3]
                material_ids[name] = len(materials)
                materials.append(
                    {
                        "name": name,
                        "pbrMetallicRoughness": {
                            "baseColorFactor": [round(c, 4) for c in color] + [1.0],
                            "metallicFactor": 0.0,
                            "roughnessFactor": 0.9,
                        },
                        "doubleSided": True,
                    }
                )

            flat = [c for p in positions for c in p]
            bounds = ([min(p[a] for p in positions) for a in range(3)], [max(p[a] for p in positions) for a in range(3)])
            position_accessor = add(struct.pack("<%df" % len(flat), *flat), 5126, "VEC3", 34962, len(positions), bounds)
            flat = [c for n in normals for c in n]
            normal_accessor = add(struct.pack("<%df" % len(flat), *flat), 5126, "VEC3", 34962, len(normals))
            index_accessor = add(struct.pack("<%dH" % len(indices), *indices), 5123, "SCALAR", 34963, len(indices))
            primitives.append(
                {
                    "attributes": {"POSITION": position_accessor, "NORMAL": normal_accessor},
                    "indices": index_accessor,
                    "material": material_ids[name],
                    "mode": 4,
                }
            )

    result = {
        "asset": {"version": "2.0", "generator": "prepare_eiffel_tower.py", "copyright": COPYRIGHT},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [{"name": "Eiffel Tower", "mesh": 0}],
        "meshes": [{"name": "Eiffel Tower", "primitives": primitives}],
        "materials": materials,
        "accessors": accessors,
        "bufferViews": views,
        "buffers": [{"byteLength": len(out_binary)}],
    }
    return write_glb(result, out_binary)


def main():
    here = Path(__file__).resolve().parent
    if len(sys.argv) > 1:
        source = Path(sys.argv[1]).read_bytes()
    else:
        print("Downloading", SOURCE_URL)
        with urllib.request.urlopen(SOURCE_URL) as response:
            source = response.read()
    digest = hashlib.sha256(source).hexdigest()
    if digest != SOURCE_SHA256:
        raise SystemExit("The source is not the pinned revision: sha256 %s, expected %s" % (digest, SOURCE_SHA256))
    output = Path(sys.argv[2]) if len(sys.argv) > 2 else here / ".." / "render-tests" / "assets" / "eiffel-tower.glb"
    output.write_bytes(prepare(source))
    print("Wrote", output.resolve(), "(%d bytes)" % output.stat().st_size)


if __name__ == "__main__":
    main()
