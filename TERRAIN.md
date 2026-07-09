# Terrain Rendering (Metal Backend)

This document describes the 3D terrain rendering implementation for MapLibre Native's Metal backend.

## Overview

Terrain rendering enables draping the map over 3D elevation data from Digital Elevation Model (DEM) tiles. This feature provides a 3D visualization of geographic terrain with realistic elevation.

## Features

- **Metal Backend Only**: Initial implementation targets Metal rendering backend (iOS/macOS)
- **DEM Support**: Uses raster-dem tiles in Mapbox Terrain RGB format
- **Exaggeration**: Configurable vertical exaggeration multiplier
- **Shared Elevation Functions**: All Metal shaders can sample terrain elevation

## Style Configuration

### Basic Example

```json
{
  "version": 8,
  "sources": {
    "maplibre": {
      "type": "raster",
      "tiles": ["https://demotiles.maplibre.org/tiles/{z}/{x}/{y}.png"],
      "tileSize": 256
    },
    "terrainSource": {
      "type": "raster-dem",
      "tiles": ["https://demotiles.maplibre.org/terrain-tiles/{z}/{x}/{y}.png"],
      "tileSize": 256,
      "encoding": "terrarium"
    }
  },
  "terrain": {
    "source": "terrainSource",
    "exaggeration": 1.5
  },
  "layers": [
    {
      "id": "background",
      "type": "raster",
      "source": "maplibre"
    }
  ]
}
```

### Terrain Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `source` | string | (required) | ID of a raster-dem source to use for terrain |
| `exaggeration` | number | 1.0 | Vertical exaggeration multiplier for elevation |

## DEM Encoding

MapLibre Native terrain supports the **Mapbox Terrain RGB** encoding format:

```
elevation_meters = -10000 + ((R * 256 * 256 + G * 256 + B) * 0.1)
```

Where R, G, B are the RGB pixel values (0-255) from the DEM tile.

## Implementation Details

### Architecture

The terrain system consists of several key components:

1. **Style Configuration** (`style::Terrain`)
   - Parses terrain from style JSON
   - Stores source ID and exaggeration
   - Notifies observers of changes

2. **Render Manager** (`RenderTerrain`)
   - Generates terrain mesh geometry (128×128 grid)
   - Provides elevation lookups
   - Manages DEM source references

3. **Metal Shaders** (`TerrainShader`)
   - Vertex shader samples DEM texture
   - Displaces vertices by elevation
   - Applies exaggeration multiplier

4. **Common Shader Functions**
   - `decode_elevation()` - Decodes Terrain RGB format
   - `get_elevation()` - Samples elevation from DEM texture

### Files Modified/Created

**Style System:**
- `include/mbgl/style/terrain.hpp` - Public terrain API
- `src/mbgl/style/terrain.cpp` - Terrain implementation
- `src/mbgl/style/terrain_impl.hpp` - Immutable terrain data
- `include/mbgl/style/terrain_observer.hpp` - Change observer
- `include/mbgl/style/conversion/terrain.hpp` - JSON conversion
- `src/mbgl/style/conversion/terrain.cpp` - JSON parsing

**Rendering:**
- `src/mbgl/renderer/render_terrain.hpp` - Terrain rendering manager
- `src/mbgl/renderer/render_terrain.cpp` - Mesh generation and elevation lookups
- `src/mbgl/renderer/update_parameters.hpp` - Added terrain parameter
- `src/mbgl/renderer/render_orchestrator.hpp/cpp` - Terrain integration

**Shaders:**
- `include/mbgl/shaders/mtl/terrain.hpp` - Metal terrain shader
- `include/mbgl/shaders/terrain_layer_ubo.hpp` - Terrain UBO structures
- `include/mbgl/shaders/mtl/common.hpp` - Added elevation functions
- `shaders/manifest.json` - Added TerrainShader entry

**Build System:**
- `bazel/core.bzl` - Added terrain source files

### Mesh Generation

Terrain uses a regular grid mesh (128×128 vertices = 16,641 vertices, 32,768 triangles):

- Vertices span from (0,0) to (EXTENT, EXTENT) where EXTENT = 8192
- Z coordinate (elevation) is 0 in mesh data
- Vertex shader displaces Z based on DEM sampling
- Mesh is shared across all tiles for efficiency

### Shader Integration

All Metal shaders have access to terrain elevation through common functions defined in `mtl/common.hpp`:

```metal
// Get elevation at UV coordinates from DEM texture
float elevation = get_elevation(demTexture, demSampler, uv);
```

This enables future work to make all layers terrain-aware (draping symbols, lines, etc. over terrain).

## Current Status

Implemented:

- Terrain shaders and registration for all four backends (OpenGL, Metal, Vulkan, WebGPU)
- DEM decoding via the source's unpack vector (Mapbox Terrain-RGB and Terrarium),
  matching hillshade/color-relief and maplibre-gl-js
- Layer draping: background, fill, line, raster, hillshade, and color-relief layer
  groups render into per-tile render targets that are draped over the terrain mesh
  (the same layer set maplibre-gl-js drapes)
- Style parsing of the `terrain` root property (`source`, `exaggeration`) per the style spec,
  with exaggeration applied as styled (1.0 = true scale)
- CPU elevation queries (`RenderTerrain::getElevation`) with DEM tile ancestor fallback
  and bilinear interpolation, mirroring maplibre-gl-js `Terrain.getDEMElevation`

## Remaining Work for Production

The reference for each phase is the maplibre-gl-js implementation
(`src/render/terrain.ts`, `src/render/render_to_texture.ts`, `src/shaders/_prelude.vertex.glsl`).

### Phase 1 - Elevation for non-draped layers (required)

Symbols, circles, and fill-extrusion currently render at z=0, so they float over
valleys and sink into peaks. gl-js elevates them with a `get_elevation()` helper
in the vertex shader (`TERRAIN3D` sections of `_prelude.vertex.glsl`).

Status:
- **circle** — implemented on all four backends (per-drawable DEM texture plus
  `dem_*` fields in `CircleDrawableUBO`, bound by the circle layer tweaker via
  `RenderTerrain::getTerrainData`).
- **fill-extrusion** — `dem_*` added to `FillExtrusionDrawableUBO` (all backends,
  so the consolidated SSBO stride matches) and elevation implemented for the
  **OpenGL** non-pattern shader; the fill-extrusion tweaker binds the DEM texture
  and fills `dem_*`. Still TODO: Metal/Vulkan/WebGPU non-pattern (incl. the Metal/
  Vulkan instanced side-geometry variants) and all pattern variants — their UBO
  struct is already grown, so each is just the shader elevation logic + textures.
- **symbol** — not started.

Symbol and fill-extrusion follow the circle pattern:

- Bind the covering DEM tile's texture plus a `TerrainElevationUBO`
  (dem-tile matrix, unpack vector, exaggeration, dem dimension) to symbol,
  circle, and fill-extrusion drawables when terrain is enabled
- Sample the DEM with manual bilinear interpolation on pixel centers
  (`dim + 2` with the backfilled 1px border, as gl-js does) and displace
  the vertex z by the decoded elevation
- Apply to all four backends' symbol (icon/sdf/text-and-icon), circle, and
  fill-extrusion shaders

### Phase 2 - Symbol occlusion (required)

gl-js renders a depth pass of the terrain mesh into a packed-RGBA texture and
fades symbols that are behind terrain (`calculate_visibility`/`depthOpacity`).
Without it, symbols show through mountains.

### Phase 3 - Seams and quality

- Backfilled DEM tile borders and pixel-center sampling to remove seams
  between DEM tiles
- Mesh skirts (gl-js `a_pos3d.z` flag + `u_ele_delta`) to hide cracks between
  neighboring tiles at different zoom levels
- Camera-terrain collision using `RenderTerrain::getElevation`
- Coordinate picking against the terrain (gl-js coords/depth framebuffers)

### Cleanup before merging

- Remove the per-frame `Log::Info` debug logging (renderer, drawable, tweaker paths)
- Extend the draped-flag gamma handling to the line gradient/pattern/SDF variants
- Decide whether heatmap should be draped (gl-js does not drape it)

## Testing

To test terrain rendering:

1. Use the example style JSON above
2. Ensure Metal renderer is enabled: `--//:renderer=metal`
3. Build the iOS app: `bazel build //platform/ios:App --//:renderer=metal`
4. Load a style with terrain configuration
5. Verify terrain parsing in debug logs

## API Usage

### C++ API

```cpp
// Create terrain configuration
auto terrain = std::make_unique<mbgl::style::Terrain>("demSourceID", 1.5f);

// Apply to style
style->setTerrain(std::move(terrain));

// Query terrain
auto* currentTerrain = style->getTerrain();
if (currentTerrain) {
    float exaggeration = currentTerrain->getExaggeration();
    std::string sourceID = currentTerrain->getSource();
}

// Remove terrain
style->setTerrain(nullptr);
```

### Style JSON API

```json
{
  "terrain": {
    "source": "terrain-source-id",
    "exaggeration": 2.0
  }
}
```

## References

- [MapLibre GL JS Terrain](https://maplibre.org/maplibre-gl-js/docs/examples/3d-terrain/)
- [Mapbox Terrain RGB Format](https://docs.mapbox.com/data/tilesets/reference/mapbox-terrain-rgb-v1/)
- [Digital Elevation Model](https://en.wikipedia.org/wiki/Digital_elevation_model)

## Credits

Implementation based on maplibre-gl-js terrain rendering architecture.
Generated with assistance from Claude Code.
