# Terrain Rendering

This document describes the 3D terrain rendering implementation for MapLibre Native.

## Overview

Terrain rendering enables draping the map over 3D elevation data from Digital Elevation Model (DEM) tiles. This feature provides a 3D visualization of geographic terrain with realistic elevation. The architecture follows maplibre-gl-js (`src/render/terrain.ts`, `src/render/render_to_texture.ts`).

## Features

- **All Backends**: OpenGL, Metal, Vulkan, and WebGPU
- **DEM Support**: raster-dem tiles in Mapbox Terrain-RGB or Terrarium encoding,
  decoded via the source's unpack vector like hillshade and color-relief
- **Layer Draping**: background, fill, line, raster, hillshade, and color-relief
  layers render into per-tile render targets draped over the terrain mesh
- **Elevated Layers**: symbol, circle, and fill-extrusion layers sample the DEM in
  their vertex shaders and are displaced by the terrain elevation
- **Exaggeration**: styled vertical exaggeration multiplier (1.0 = true scale)
- **Shared Elevation Function**: a `get_elevation()` helper in every backend's
  shader prelude

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

### Sharing a source between terrain and hillshade

maplibre-gl-js recommends giving terrain its own raster-dem source (even with
the same tile URLs as a hillshade/color-relief source) because gl-js keeps a
separate terrain source cache, and sharing one source between the terrain and
a hillshade layer makes two caches with different retention fight over one
tile set.

MapLibre Native does not have that constraint: there is exactly one tile
pyramid per style source, and terrain, hillshade, and color-relief consume the
same `RasterDEMTile`s and share the decoded DEM data, so sharing a source is
coherent and avoids downloading and decoding the tiles twice. A terrain-only
source (no layer attached) also works; the render orchestrator marks the
terrain's DEM source as needing rendering.

For styles that must also run on maplibre-gl-js, follow the gl-js
recommendation and declare separate sources; the test app examples do this.

## DEM Encoding

Terrain decodes elevations with the same per-source unpack vector used by the
hillshade and color-relief layers (`DEMData::getUnpackVector()`), so both
supported raster-dem encodings work:

- **Mapbox Terrain-RGB** (`"encoding": "mapbox"`, the default):
  `elevation = -10000 + ((R * 256 * 256 + G * 256 + B) * 0.1)`
- **Terrarium** (`"encoding": "terrarium"`):
  `elevation = (R * 256 + G + B / 256) - 32768`

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

3. **Terrain Shaders** (`TerrainShader`, all four backends)
   - Vertex shader samples the DEM texture and displaces vertices by elevation
   - Fragment shader samples the draped render-to-texture map tile
   - Applies the styled exaggeration multiplier

4. **Render-to-Texture Draping** (`TexturePool`, `RenderTarget`)
   - Draped layer groups (created with `renderToTerrain = true`) have their
     drawables moved into per-terrain-tile render targets each frame
   - The render targets are cleared to the map background color (the terrain
     mesh renders unblended) and have a depth attachment for correct
     inter-layer depth within the drape

5. **Elevated (non-draped) Layers** (`RenderTerrain::getTerrainData`)
   - Symbol, circle, and fill-extrusion tweakers bind the covering DEM tile's
     texture per drawable, with ancestor-tile fallback and a 1x1 placeholder
     when no tile is loaded
   - Their vertex shaders call the shared `get_elevation()` prelude helper,
     which does manual bilinear interpolation on DEM pixel centers using the
     1px backfilled tile border, matching maplibre-gl-js

6. **CPU Elevation Queries** (`RenderTerrain::getElevation`)
   - DEM tile lookup with ancestor fallback and bilinear interpolation,
     mirroring maplibre-gl-js `Terrain.getDEMElevation`

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
- `shaders/terrain.vertex.glsl` / `shaders/terrain.fragment.glsl` - GL terrain shader sources
- `include/mbgl/shaders/mtl/terrain.hpp`, `include/mbgl/shaders/vulkan/terrain.hpp`,
  `include/mbgl/shaders/webgpu/terrain.hpp` - backend terrain shaders
- `include/mbgl/shaders/terrain_layer_ubo.hpp` - Terrain UBO structures
- `shaders/_prelude.vertex.glsl`, `include/mbgl/shaders/{mtl,vulkan,webgpu}/common.hpp` -
  shared `get_elevation()` prelude helper
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

Every backend's shader prelude defines a shared elevation helper. Unlike
maplibre-gl-js (which uses global terrain uniforms), MapLibre Native carries the
DEM binding per drawable, so the values are passed as arguments:

```glsl
float ele = get_elevation(pos, u_dem, u_dem_coords, u_dem_unpack,
                          u_dem_dim, u_dem_exaggeration, u_dem_enabled);
```

Layers opt in by adding the `dem_*` fields to their drawable UBO, binding the
DEM texture in their layer tweaker via `RenderTerrain::getTerrainData()`, and
calling the helper in their vertex shader (see the circle, symbol, and
fill-extrusion layers for the pattern).

## Current Status

Implemented:

- Terrain shaders and registration for all four backends (OpenGL, Metal, Vulkan, WebGPU)
- DEM decoding via the source's unpack vector (Mapbox Terrain-RGB and Terrarium)
- Layer draping for background, fill, line, raster, hillshade, and color-relief,
  with a depth attachment on the drape render targets and per-drape clip masks
- Elevation for the non-draped layers: symbol (icon/SDF/text-and-icon), circle,
  and fill-extrusion on all four backends, via the shared `get_elevation()`
  prelude helper and per-drawable DEM bindings
- Style parsing of the `terrain` root property (`source`, `exaggeration`) per the
  style spec, with exaggeration applied as styled (1.0 = true scale)
- CPU elevation queries (`RenderTerrain::getElevation`) with DEM tile ancestor
  fallback and bilinear interpolation
- Line antialiasing gamma handled per drawable (1.0 when draped into a terrain
  render target, perspective ratio otherwise)

## Remaining Work for Production

The reference for each phase is the maplibre-gl-js implementation
(`src/render/terrain.ts`, `src/render/render_to_texture.ts`, `src/shaders/_prelude.vertex.glsl`).

### Drape routing rework (required)

**Defect**: draped drawables are routed into exactly one terrain render target
(`renderer_impl.cpp` / `TexturePool::getRenderTargetAncestorOrDescendant`, which
returns the single deepest ancestor-or-descendant match). A drape tile at a
lower zoom than the terrain cover — a raster/vector parent tile standing in for
unloaded children, or a 256px-tile raster source whose pyramid runs one zoom
deeper than the DEM — covers several terrain tiles but is drawn into only one
of them. The rest render the clear color where that layer should be, producing
patchy, per-frame-changing drapes (very visible in the "3D Terrain (OSM
raster)" Android activity). The zoom-mismatch matrix math itself
(`getTerrainRttPosMatrix`) is correct; the routing multiplicity is the gap.

**Reference**: gl-js `render_to_texture.ts` iterates each terrain tile and
draws *all* overlapping source tiles into its framebuffer, setting `u_matrix`
per draw call, so a parent tile paints into every child target it covers.

**Design (per-target transform)**: keep draped drawables in their layer
groups instead of moving them into per-target groups. The drape projection is
orthographic, so placing a tile into a target is an affine transform in NDC:

- Each render target owns a small per-target UBO with its tile id, updated
  once per frame and bound in `RenderTarget::render` (which already binds
  global UBOs per pass), so deferred backends (Metal/Vulkan/WebGPU) see no
  mid-frame buffer hazards.
- Draped drawable UBOs carry a tile-local ortho matrix plus the drawable's
  tile coordinates (constant per frame, no per-target variance).
- A shared vertex prelude helper applies `pos.xy * scale + offset` computed
  from (drawable tile, target tile) — same mechanical shader sweep as
  `get_elevation`.
- `RenderTarget::render` iterates the draped layer groups and enables only the
  drawables whose tile overlaps its target tile (ancestor/equal/descendant),
  letting one drawable record into every target it covers with zero copies.

This keeps all routing logic in shared renderer code; the alternative
(cloning drawables per extra target) would require duplication support inside
all four backends' drawable implementations.

### Phase 2 - Symbol occlusion (required)

gl-js renders a depth pass of the terrain mesh into a packed-RGBA texture and
fades symbols that are behind terrain (`calculate_visibility`/`depthOpacity`).
Without it, symbols show through mountains.

### Phase 3 - Seams and quality

- Backfilled DEM tile borders and pixel-center sampling in the terrain mesh
  shader itself (the elevated layers already sample this way)
- Mesh skirts (gl-js `a_pos3d.z` flag + `u_ele_delta`) to hide cracks between
  neighboring tiles at different zoom levels
- Camera-terrain collision using `RenderTerrain::getElevation`
- Coordinate picking against the terrain (gl-js coords/depth framebuffers)
- Elevation for CPU-projected along-line labels in viewport alignment
  (gl-js applies it in the CPU symbol projection)
- Terrain-aware tile cover: gl-js consults the elevation data when computing
  covering tiles (horizon/occlusion aware). Native has its own tile LOD system
  (`tileLodMinRadius`/`tileLodScale`/`tileLodPitchThreshold`) for reducing
  distant-tile zoom at pitch, but it is not elevation-aware.

### Cleanup before merging

- Remove the per-frame `Log::Info` debug logging (render_terrain.cpp,
  terrain_layer_tweaker.cpp, mtl/drawable.cpp)
- Extend the draped-flag gamma handling to the line gradient/pattern/SDF variants
- Decide whether heatmap should be draped (gl-js does not drape it)
- Evict DEM textures for unused tiles (`RenderTerrain::demTextures` currently
  grows unbounded)
- Runtime styling API for terrain (`setTerrain`) on iOS/macOS (Android has it)

## Testing

- **Render tests**: `metrics/integration/render-tests/terrain/` contains
  `default` and `pitched-world`, ported from maplibre-gl-js. They are in
  `metrics/ignores/platform-all.json` until native-rendered baselines are
  captured (the expected images are gl-js renders). The gl-js `terrain/symbol`
  test is worth porting now that symbols are elevated (needs its DEM fixtures
  loaded into `metrics/cache-style.db`).
- **Android**: the test app has a "3D Terrain" activity (Style category)
  showing terrain + hillshade + color-relief from the Mapterhorn raster-dem
  source, camera pitched at the Matterhorn. Works with both the `opengl` and
  `vulkan` flavors.
- Or load any style with a `terrain` root property, e.g. the example above.

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

### Android API

```kotlin
// Requires a raster-dem source in the style (its TileJSON may carry the encoding)
style.addSource(RasterDemSource("terrain-source", "https://tiles.mapterhorn.com/tilejson.json"))
style.setTerrain(Terrain("terrain-source", exaggeration = 1.0f))

val terrain = style.getTerrain() // null when terrain is not enabled
style.setTerrain(null)           // disable terrain
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
