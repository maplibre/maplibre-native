# Terrain Rendering

This document describes the 3D terrain rendering implementation for MapLibre Native.

## Continuing this work

- The reference implementation is upstream **maplibre-gl-js**; when in doubt,
  match its behaviour - it has been in production a long time. Key files:
  `src/render/terrain.ts`, `src/render/render_to_texture.ts` (render-to-texture
  draping), `src/geo/projection/covering_tiles.ts` (elevation-aware tile cover),
  `src/ui/camera.ts` (`_elevateCameraIfInsideTerrain`), and the terrain vertex
  prelude.
- Most development and testing has been on **Android / OpenGL**; the other
  renderers are implemented but far less exercised. Build a specific renderer
  flavour rather than `assembleDebug` (which fans out to every flavour and
  currently fails on WebGPU's Gradle config):
  - `cd platform/android && ./gradlew :MapLibreAndroidTestApp:installOpenglDebug -Pmaplibre.abis=<abi>`
    (`arm64-v8a` for a phone, `x86_64` for the emulator). `assembleOpenglDebug`
    builds without installing; `:MapLibreAndroid:assembleVulkanDebug` /
    `assembleMetalDebug` to check the other backends build.
- Android test activities (under `activity/style`, in the app's Style section):
  - `TerrainActivity` - self-contained style with color-relief + hillshade over
    3D terrain (Matterhorn); a bright hypsometric colour ramp to show color-relief.
  - `TerrainVectorMapActivity` - a full-planet vector basemap (OpenFreeMap
    Liberty) draped over terrain, DEM/hillshade/terrain added at runtime.
  - `TerrainOsmRasterActivity` - OSM raster draped over terrain (Innsbruck);
    exercises the raster-draping cases.
  - `TerrainDebugTilesActivity` - the gl-js terrain debug-tiles style (numbered
    tiles over synthetic terrain), for comparing tile zoom / draping against
    gl-js.

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
   - Each terrain tile has a persistent render target; the orchestrator's draped
     layer groups render into every target their tiles overlap (they are not
     moved out of the orchestrator)
   - Targets are cleared to the map background color and carry depth **and
     stencil** attachments; every overlapping source tile is drawn and the layer
     groups' tile clipping masks resolve parent/child overlap
     (`PaintParameters::clipMatrixForTile` builds the masks with the drape
     placement)
   - A content-hash render cache (`RenderTarget::DrapeCoverage`) skips
     re-rendering a target whose draped content, zoom, and evaluated properties
     are unchanged, so panning does not re-render every drape every frame

5. **Terrain tile cover** (`RenderTerrain::expandToDeepestCover`,
   `util::frustumCull`, `DEMElevationProvider`)
   - Mesh tiles come from the DEM source's render tiles expanded to the ideal
     cover, then frustum-culled with each tile's DEM min/max elevation so
     relief facing the camera is kept and off-screen low-zoom tiles are dropped
   - The same elevation feeds `Frustum::intersectsElevated` in `util::tileCover`,
     so every source requests the tiles the terrain mesh needs

6. **Symbol occlusion** (`TerrainDepthShader`, `RenderTerrain::renderDepth`)
   - A depth pass of the terrain mesh is packed into an RGBA texture; symbol
     shaders fade labels that fall behind the terrain

7. **Elevated (non-draped) Layers** (`RenderTerrain::getTerrainData`)
   - Symbol, circle, and fill-extrusion tweakers bind the covering DEM tile's
     texture per drawable, with ancestor-tile fallback and a 1x1 placeholder
     when no tile is loaded
   - Their vertex shaders call the shared `get_elevation()` prelude helper,
     which does manual bilinear interpolation on DEM pixel centers using the
     1px backfilled tile border, matching maplibre-gl-js

8. **CPU Elevation Queries** (`RenderTerrain::getElevation`)
   - DEM tile lookup with ancestor fallback and bilinear interpolation,
     mirroring maplibre-gl-js `Terrain.getDEMElevation`

### Key files

Not exhaustive - terrain touches many layers/shaders across four backends, and
the git history is the source of truth. The main pieces:

**Style:** `style/terrain.{hpp,cpp}`, `style/terrain_impl.hpp`,
`style/terrain_observer.hpp`, `style/conversion/terrain.{hpp,cpp}`.

**Rendering:**
- `renderer/render_terrain.{hpp,cpp}` - mesh generation, DEM caching, mesh tile
  cover, depth pass, elevation lookups
- `renderer/render_target.{hpp,cpp}`, `renderer/texture_pool.{hpp,cpp}` - drape
  render targets (depth+stencil), the content-hash render cache
- `renderer/dem_elevation_provider.{hpp,cpp}`, `util/tile_cover.*`,
  `util/bounding_volumes.*` - elevation-aware tile cover and frustum cull
- `renderer/paint_parameters.{hpp,cpp}` - drape clip-mask matrix
  (`clipMatrixForTile`)
- `renderer/layers/terrain_layer_tweaker.{hpp,cpp}` and the circle/symbol/
  fill-extrusion tweakers - per-drawable DEM binding and elevation
- `renderer/render_orchestrator.*`, `renderer/update_parameters.hpp` - integration

**Shaders:** `shaders/terrain*.glsl` and `shaders/terrain_depth*.glsl` (GL
sources), `shaders/{mtl,vulkan,webgpu}/terrain{,_depth}.*`, the shared
`get_elevation()` / `apply_drape_transform()` helpers in each backend's prelude,
`shaders/terrain_layer_ubo.hpp`, `shaders/manifest.json`.

**Build:** `bazel/core.bzl` and the top-level `CMakeLists.txt` both list terrain
sources explicitly - new `src/`/`include/` files must be added to both.

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
  with depth+stencil attachments on the drape render targets. Every overlapping
  tile is drawn and the layer groups' tile clipping masks resolve parent/child
  overlap (gl-js `coordsAscending`), so a drape target is only ever empty when
  the source genuinely has no tile for it - no more black-while-panning.
- Elevation-aware tile cover: covering tiles are tested against the frustum with
  the height of their DEM, so relief leaning towards the camera is requested
  (`DEMData` min/max, `util::TileElevationProvider` / `DEMElevationProvider`,
  `Frustum::intersectsElevated`), and the terrain mesh is frustum-culled so a
  sparse DEM's low-zoom ancestor tiles are not meshed off-screen.
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

### Drape routing rework (implemented on all backends; OpenGL device-tested)

Implemented following gl-js `render_to_texture.ts` semantics. Draped layer
groups stay in the orchestrator; each drape `RenderTarget` renders every
draped drawable whose tile overlaps its target tile. The drape projection is
orthographic, so placement into a zoom-mismatched target is an affine
transform in NDC, applied in the vertex shader (`apply_drape_transform` in
the prelude): the drawable's tile rides in the unused third column of its
tile-local drape matrix, and the target tile is carried in
`GlobalPaintParamsUBO::drape_tile` via a per-target copy of the global paint
params bound in the target's own render pass. On top of that:

- Every overlapping tile is drawn into the target and the layer groups' own
  tile clipping masks resolve parent-over-child overlap, exactly as gl-js does
  (`coordsAscending` + `renderTileClippingMasks`). This is what fixed drape
  targets rendering black when their exact tile was not loaded: a parent tile
  standing in for unloaded children now reaches every child target it covers.
  It required giving drape targets a **stencil** attachment (gl-js's RTT
  framebuffer has one) and building the masks with the drape placement rather
  than the camera matrix - `PaintParameters::clipMatrixForTile` evaluates the
  `apply_drape_transform` affine CPU-side, and the mask cache is invalidated on
  entering/leaving a drape (it is keyed only on the tile set). This replaced an
  earlier "pick one consistent tile per layer group" heuristic that returned
  nothing, hence black, whenever a target's tile was outside the loaded set.
- A drape target keeps its previously rendered content while the available
  coverage is temporarily worse than what is already baked in, so panning
  does not degrade already-seen detail (`RenderTarget::DrapeCoverage`).
- Terrain mesh tiles stay at the ideal cover (`RenderTerrain::expandToDeepestCover`),
  then are **frustum-culled** (`util::frustumCull`) so a sparse DEM's large
  low-zoom ancestor tile is not meshed across its off-screen extent - those
  off-screen meshes had no on-screen source to drape and rendered near-black.
  Only DEM/drape textures fall back to ancestors while tiles load.

Backend status:

- **OpenGL**: complete (per-target GlobalPaintParams copy bound per pass).
- **Metal**: complete (same mechanism; globals bind per encoder, so the
  per-target buffer swap works).
- **Vulkan**: complete, but the target tile travels in the **push constants**
  next to the consolidation `ubo_index` (32-byte range in the general
  pipeline layout, `PaintParameters::currentDrapeTile` at draw-record time):
  the global descriptor set is one per frame-in-flight with a dirty flag, so
  a per-target buffer swap would be silently ignored.
- **WebGPU**: complete (same per-target buffer mechanism as GL/Metal). This
  works because drawables currently rebuild their bind groups on every draw,
  picking up the per-pass buffer swap; if bind-group caching is introduced
  later, drape targets will need per-target bind groups or dynamic uniform
  offsets instead.

### Convergence with maplibre-gl-js (ask before doing)

Places where the native implementation reaches gl-js behavior through
different mechanisms. Each is working; converging would simplify review
against the gl-js reference, but the native variant may actually be the
better fit for the drawable architecture — **discuss/decide before spending
time on any of these**:

- **Terrain tile cover**: native derives terrain mesh tiles from the
  `TilePyramid` render set and reverse-engineers the ideal cover out of it
  (`expandToDeepestCover`); gl-js computes the ideal cover directly in a
  dedicated terrain source cache. A direct terrain tile cover (reusing
  native's `tileCover`/LOD machinery) would remove the expansion step.
- **Drawable tile id transport**: the tile id rides in the unused third
  column of the drape matrix to avoid a 12-struct UBO layout sweep across
  four backends. Explicit `drape_tile` UBO fields would be the boring,
  reviewable version. (gl-js needs neither: WebGL sets `u_matrix` per draw
  call; native's prebaked-UBO/deferred-recording architecture cannot.)

### Performance target

Downstream users include real-time applications (e.g. aviation/flight
planning on Android) where sustained frame rate matters more than fast
initial load. The content-hash render cache (above) already skips re-rendering
unchanged drape targets. Remaining levers: the per-frame CPU work in
`RenderTarget::computeDrapeCoverage` / `renderDrapedLayerGroups` (drawable
visits scale with targets × drawables, run even on a cache hit), the hardcoded
512x512 drape target size, and DEM texture upload scheduling. Real-device
profiling welcome.

### Phase 2 - Symbol occlusion (done, all backends)

Implemented following gl-js: a depth pass of the terrain mesh is rendered into a
packed-RGBA texture, and the symbol shaders fade symbols that are behind the
terrain (`calculate_visibility`), so labels no longer show through mountains.

- `TerrainDepthShader` re-renders the terrain meshes (same DEM displacement as
  the terrain shader, sharing its UBO layout) into a viewport-sized target,
  packing the fragment depth into RGBA8. It is driven by
  `RenderTerrain::renderDepth` after the drape targets, from a depth twin of
  each mesh drawable held in a separate layer group; the existing terrain
  tweaker fills both groups' UBOs.
- `RenderTerrain::getDepthTexture` hands symbol tweakers the packed texture, or
  a 1x1 far-plane placeholder (everything visible) before the pass has run.
- Symbol drawables bind it at `idSymbolDepthTexture`; the test reuses the
  `dem_enabled` flag, since occlusion is exactly terrain-enabled.

**Backend note - the depth lookup's V coordinate is not the same everywhere.**
OpenGL samples the depth texture with the unflipped `ndc * 0.5 + 0.5`; Metal and
WebGPU have y-up NDC but a top-left texture origin, so they flip V; Vulkan needs
no flip because its y-down NDC and top-left origin agree, provided the position
is taken after `applySurfaceTransform()` (which is where its symbol shaders
already compute fade opacity). The convention was confirmed per backend against
the existing `heatmap_texture` shaders, which sample a render target the same
way: GL flips `a_pos.y`, Metal and WebGPU do not.

Verified working on OpenGL on device (icons behind terrain are hidden);
Metal/Vulkan/WebGPU follow the same structure but are not yet run on hardware.

### Phase 3 - Seams and quality

- **Drape target size is hardcoded to 512x512** (`Renderer::Impl::texturePool`,
  the long-standing `// TODO: tile size`). It ignores `pixelRatio` and the
  source tile size, so on a high-DPI device (e.g. pixel ratio 3.5) a terrain
  tile covering a large screen area has its draped map content rendered at
  512x512 and upscaled onto the mesh, costing sharpness. gl-js exposes a
  terrain quality factor for this. Sizing this up trades GPU memory directly
  (one texture per terrain tile), so it wants measuring alongside the render
  cache below rather than a blind bump.
- Backfilled DEM tile borders and pixel-center sampling in the terrain mesh
  shader itself (the elevated layers already sample this way)
- Mesh skirts (gl-js `a_pos3d.z` flag + `u_ele_delta`) to hide cracks between
  neighboring tiles at different zoom levels
- Camera-terrain collision: a collision-only fix was prototyped and reverted
  (felt worse - it corrected only after the gesture); the real fix is the
  terrain-anchored camera in Phase 4
- **Zoom-0 intensity bug (terrain *and* hillshade)**: at zoom 0 the relief is
  rendered far more intense than at zoom 1+ - terrain elevation looks
  over-exaggerated and the existing hillshade layer is over-shaded; zooming in
  past zoom 1 both look normal. Long-standing: the hillshade version predates
  this terrain work, so it is likely one shared cause - a zoom-dependent scale in
  the elevation/derivative math (e.g. a metres-per-pixel or DEM-derivative term
  that blows up when the whole world is a single tile at zoom 0), not something
  specific to terrain. Reproduces on device; gl-js does not show it. Worth fixing
  for both layers together. Not yet investigated.
- Coordinate picking against the terrain (gl-js coords/depth framebuffers)
- Elevation for CPU-projected along-line labels in viewport alignment
  (gl-js applies it in the CPU symbol projection)
- Elevation-aware tile cover is **done** (see Current Status / the tile-cover
  component above), but only *visibility* is elevation-aware. Native's tile LOD
  system (`tileLodMinRadius`/`tileLodScale`/`tileLodPitchThreshold`) still drives
  the *zoom* selection and is not elevation-aware.
- Transitional artifacts while panning/zooming (observed in emulator testing):
  brief flat/empty far-field areas while the DEM cover has no render tile yet,
  and re-resolve flicker when the LOD migrates an area between zoom levels. The
  content-hash render cache and elevation-aware cover reduce these; the remaining
  case is the terrain mesh cover deriving from the DEM source's render set rather
  than a dedicated ideal cover (see the tile-cover convergence item above).

### Phase 4 - Terrain-anchored camera (needs a complete fix)

At high pitch over tall terrain the near (bottom) part of the view goes black:
the camera sits at or below the terrain surface, so the near rays hit nothing
and clear to the background. This is a camera-model problem, not a tile/drape
one — confirmed on device with a clean tile diagnostic (all tiles covered, mesh
== drawables) while the bottom of the frame was black below a terrain silhouette.

Research into why a quick fix is not enough (2026-07, Innsbruck, Android GL):

- **maplibre-gl-js** decouples `center` (a fixed lng/lat anchor) from `elevation`
  (the terrain height at that anchor). "Centre clamped to ground"
  (`getCenterClampedToGround`, default on) sets `transform.elevation =
  terrain.getElevationForLngLatZoom(center)` each terrain update, so the whole
  view rides the terrain surface and the camera stays above it. A separate
  safety, `_elevateCameraIfInsideTerrain`, lifts the camera via a fully
  recomputed `CameraOptions` (`calculateCameraOptionsFromTo`) only when it dips
  below terrain.

- **maplibre-native** has no such decoupling. `TransformState` defines the map
  centre as *where the camera ray meets the z=0 sea-level plane*
  (`updateStateFromCamera`, which also forces centre altitude back to 0), and
  the projection treats `z` as `cameraToSeaLevelDistance = cameraToCenterDistance
  + |z|/cos(pitch)`. Feeding a terrain height into `setCenterAltitude` therefore
  interacts with the sea-level-anchored model so that the resolved centre shifts,
  which re-samples a higher elevation, which shifts it again — a runaway pan
  (observed: the centre jumping ~40 km per frame, "the map pans as soon as you
  touch it").

So a correct "camera rides the terrain" feature (the gl-js
`getCenterClampedToGround` behaviour) requires teaching native's camera model
that the centre sits on terrain rather than sea level — a real change to
`TransformState`'s centre/plane math, not a wrapper around `setCenterAltitude`.
That is the Phase 4 work.

The **collision-only** subset (gl-js `_elevateCameraIfInsideTerrain`: recompute a
consistent `CameraOptions` and `jumpTo` only when the camera is below terrain,
which never touches the core plane math) was prototyped and then reverted - it
kept panning stable and did not run away, but only corrected *after* the gesture
(a brief black flash while zooming in, then a pop back out), which felt worse
than leaving the camera alone. A render→map elevation channel
(`RendererObserver::onTerrainCameraCollision`, forwarded per platform) plus
`TransformState::getCameraLatLng` / `getCameraAltitude` /
`cameraCollisionCorrection` and `RenderTerrain::getElevationForLatLng` were the
building blocks; see the reverted commits for the reference implementation. The
right Phase 4 fix is the terrain-anchored centre above, which prevents the camera
from entering the terrain in the first place rather than correcting afterwards.

### Cleanup before merging

- **(done)** Removed the TEMP terrain diagnostics: the throttled drape-coverage
  warning and `DrapeCoverage::emptyGroups` in render_target.cpp, and the
  per-frame terrain-summary / `skippedNoDEM` logs in render_terrain.cpp. The
  remaining `Log::Info`/`Warning` in render_terrain.cpp are one-time setup and
  error logs, not per-frame; terrain_layer_tweaker.cpp has no per-frame logging.
- Still to review: the `// TEMP: Disable depth testing` in the terrain drawable
  setup (render_terrain.cpp) - it is functional, so confirm the depth mode is
  correct before changing it. Check mtl/drawable.cpp for any leftover debug logs.
- Extend the draped-flag gamma handling to the line gradient/pattern/SDF variants
- Decide whether heatmap should be draped (gl-js does not drape it)
- Runtime styling API for terrain (`setTerrain`) on iOS/macOS (Android has it)

## Testing

- **Render tests**: `metrics/integration/render-tests/terrain/` contains
  `default` and `pitched-world`, ported from maplibre-gl-js. They are in
  `metrics/ignores/platform-all.json` until native-rendered baselines are
  captured (the expected images are gl-js renders). The gl-js `terrain/symbol`
  test is worth porting now that symbols are elevated (needs its DEM fixtures
  loaded into `metrics/cache-style.db`).
- **Android**: the test app has four terrain activities in the Style category
  (`TerrainActivity`, `TerrainVectorMapActivity`, `TerrainOsmRasterActivity`,
  `TerrainDebugTilesActivity` - see "Continuing this work" for what each covers).
  Developed and tested on the `opengl` flavour; the other flavours build but are
  far less exercised on device.
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
