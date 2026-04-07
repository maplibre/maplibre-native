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

## Current Limitations

1. **Metal Only**: OpenGL/Vulkan backends not yet supported
2. **No Drawable Creation**: Terrain mesh data generated but not yet rendered
3. **No DEM Source Lookup**: Source binding not yet implemented
4. **No Depth/Coords Passes**: Advanced picking and occlusion not yet implemented
5. **No Layer Draping**: Other layers don't yet sample elevation for 3D positioning

## Future Enhancements

### Near Term
- Complete drawable creation and rendering
- Implement DEM source lookup and binding
- Add DEM texture sampling in terrain shader
- Test with real DEM data sources

### Medium Term
- Add terrain depth pass for proper 3D occlusion
- Implement coordinate picking for terrain features
- Enable layer draping (make fill, line, symbol layers terrain-aware)
- Add terrain to OpenGL backend

### Long Term
- Implement tile-specific LOD (Level of Detail)
- Add terrain caching and optimization
- Support additional DEM encoding formats
- Implement terrain lighting and shading

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
