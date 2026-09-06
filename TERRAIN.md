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
- Android test activities (the five "3D Terrain" entries under `activity/style`,
  in the app's Style section; all five share the `TerrainTestOptions` overflow
  menu - see Performance below):
  - `TerrainActivity` - self-contained style with color-relief + hillshade over
    3D terrain (Matterhorn); a bright hypsometric colour ramp to show color-relief.
  - `TerrainVectorMapActivity` - a full-planet vector basemap (OpenFreeMap
    Liberty) draped over terrain, DEM/hillshade/terrain added at runtime.
  - `TerrainOsmRasterActivity` - OSM raster draped over terrain (Innsbruck);
    exercises the raster-draping cases.
  - `TerrainDebugTilesActivity` - the gl-js terrain debug-tiles style (numbered
    tiles over synthetic terrain), for comparing tile zoom / draping against
    gl-js.
  - `TerrainFlightActivity` - an automated FPV "drone" camera flight starting on
    the ground at Innsbruck and touring the surrounding Alps, sweeping a range of
    zoom / altitude / pitch / rotation over terrain (baked, elevation-planned
    path that stays above ground); useful for eyeballing draping, occlusion, and
    frame pacing in motion, especially with the rendering-stats HUD on.

## Overview

Terrain rendering enables draping the map over 3D elevation data from Digital Elevation Model (DEM) tiles. This feature provides a 3D visualization of geographic terrain with realistic elevation. The architecture follows maplibre-gl-js (`src/render/terrain.ts`, `src/render/render_to_texture.ts`).

## Features

- **All Backends**: OpenGL, Metal, Vulkan, and WebGPU
- **DEM Support**: raster-dem tiles in Mapbox Terrain-RGB or Terrarium encoding,
  decoded via the source's unpack vector like hillshade and color-relief
- **Layer Draping**: background, fill, line, raster, hillshade, and color-relief
  layers render into per-tile render targets draped over the terrain mesh
- **Elevated Layers**: symbol, circle, and fill-extrusion layers sample the DEM in
  their vertex shaders and are displaced by the terrain elevation. **Caveat:
  fill-extrusion elevation is GL/WebGPU only** - see the backend parity section.
- **Exaggeration**: styled vertical exaggeration multiplier (1.0 = true scale)
- **Shared Elevation Function**: a `get_elevation()` helper in every backend's
  shader prelude

## Style Configuration

### Basic Example

```json
{
  "version": 8,
  "sources": {
    "osm": {
      "type": "raster",
      "tiles": ["https://tile.openstreetmap.org/{z}/{x}/{y}.png"],
      "tileSize": 256,
      "maxzoom": 19,
      "attribution": "&copy; OpenStreetMap Contributors"
    },
    "terrainSource": {
      "type": "raster-dem",
      "url": "https://tiles.mapterhorn.com/tilejson.json"
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
      "source": "osm"
    }
  ]
}
```

**Give the raster-dem source a `url`, not an inline `tiles` array.** The TileJSON
carries `encoding`, `tileSize` and `bounds`, and the source picks all three up; hand-
rolling them instead is the easiest way to get a blank map with no error in the log.
`tileSize` is the sharp edge - declaring 256 for a source that serves 512px tiles
meshes the cover at a shallower zoom than the DEM is loaded at, so mesh and DEM never
line up. This is the pattern the
[demo terrain tiles](https://github.com/maplibre/demotiles/tree/gh-pages/terrain-tiles#how-to-use-the-demo-terrain-tiles)
and every terrain test activity use.

Two DEM sources are used across this work:

| | coverage | encoding | tile size |
|---|---|---|---|
| [Mapterhorn](https://mapterhorn.com/data-access/) `tiles.mapterhorn.com/tilejson.json` | worldwide | terrarium | 512 |
| demotiles `demotiles.maplibre.org/terrain-tiles/tiles.json` | N047E011 only, to z12 | Terrain-RGB | 512 |

The Android test activities all use Mapterhorn, for planet-wide coverage to test
against. The demotiles DEM is the narrower option: it is fine around Innsbruck
(11.404, 47.265) and 404s everywhere else, even though its `tiles.json` advertises
worldwide `bounds`.

For the basemap, note that `demotiles.maplibre.org/tiles/{z}/{x}/{y}.png` does not
exist - that demotiles set is vector-only. Use OSM raster (above), or the full-planet
OpenFreeMap Liberty vector basemap (`TestStyles.OPENFREEMAP_LIBERTY`) that
`TerrainVectorMapActivity` drapes.

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

Terrarium encodes "no data" as RGB(0, 0, 0), which decodes to -32768 m.
`DEMData::get()` reads that back as 0 rather than letting a void drag the tile's
elevation range - and the frustum test built on it - below the sea floor
(maplibre-gl-js #6982). The GPU decode in `get_elevation()` is unguarded, as it is
in gl-js, so a void still displaces the mesh; only the CPU-side range and elevation
queries are protected.

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

Terrain uses a regular 128×128-quad grid mesh (129×129 = 16,641 vertices, 32,768 triangles):

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
- Mesh skirts on all four backends: each tile edge is extruded into a curtain
  that hangs below the surface by `u_elevation_offset` (gl-js `u_ele_delta`),
  hiding the cracks/stitches between neighbouring tiles at different zoom
  levels. The skirt flag rides in the mesh vertex's z (native analog of gl-js
  `a_pos3d.z`); the terrain depth pass drops the skirts too so they occlude
  correctly. Skirts can be turned off per map with `Map::setTerrainSkirtLength`
  (`TerrainSkirtLength::Auto` / `None`; Android `MapLibreMap.setTerrainSkirtLength`),
  matching gl-js's `MapOptions.terrainSkirtLength`. `None` suits a transparent
  background, where the skirts would otherwise show as vertical artifacts, at the
  cost of visible stitches; it builds the bare grid rather than shortening the
  curtain, and changing it at runtime rebuilds the mesh and every tile drawable.
  Render tests `terrain/skirts-auto` and `terrain/skirts-none` select it through the
  `terrainSkirtLength` test-metadata field, but do **not** currently cover it - their
  baselines hold no terrain, so both render identically either way (see Testing).
- The terrain **mesh** vertex shader now samples the DEM through the shared
  `get_elevation()` prelude helper (backfilled 1px border, NEAREST fetch +
  post-decode bilinear on pixel centres), the same path the elevated layers
  use, matching maplibre-gl-js
- Hillshade prepare-target lifetime fixed (no more OOM / monotonic GPU-memory
  growth while panning terrain with a hillshade layer - see Performance below)

## Backend parity (audited 2026-07-29, Android)

Terrain work has been developed and tested almost entirely on **OpenGL**. A first
audit against the other backends found the following. Only GL and Vulkan are
buildable/testable in this environment (Metal needs macOS; WebGPU builds via the
`webgpuDawn`/`webgpuWgpu` flavours but has not been run). The **Metal** column was
filled in by @daviani on an iPhone 16 Pro Max / iOS 26.6, built for device with
`--//:renderer=metal` ([#4190 review](https://github.com/maplibre/maplibre-native/pull/4190#issuecomment-5491176380)).

| | OpenGL | Vulkan | Metal | WebGPU |
|---|---|---|---|---|
| builds | yes | yes | **yes** (bazel, for device) | yes (Dawn) |
| terrain renders | yes | **yes, verified on device** | **yes, verified on device** | untested |
| terrain off->on (all 3 load modes) | yes | **yes, verified** | **yes** (via style JSON; load modes not separated) | untested |
| symbol occlusion | yes | looks correct on device | untested (code path is complete; see below) | untested |
| **fill-extrusion elevation** | yes | **yes** (2026-08-01, render test pixel-matches the GL baseline) | **yes** (2026-08-02, shaders sample the DEM; CI confirmation pending) | yes |
| instanced depth pass | yes (GL-only by design) | n/a (per-tile path) | n/a | n/a |

**Fill-extrusion terrain elevation on Metal/Vulkan (history; both now fixed).** Metal and
Vulkan use the instanced fill-extrusion path
(`MLN_USE_FILL_EXTRUSION_INSTANCING = METAL || VULKAN`, from upstream `1b6ed35`
"Vulkan fill extrusion instancing" #4310). Its layout vertex is
`<pos, decimals_ed>` with no `centroid`, so those shaders never sample the DEM:
buildings render at sea level while the terrain is elevated, i.e. they float
above / sink below it. GL and WebGPU (non-instanced path) are correct.

**Not a recent regression.** Checked out `c2d28b9` (2026-07-21, "fix-terrain-3d-memory-leak")
and built the Vulkan flavour: buildings float there too. That commit already has
Vulkan on the instanced path with no `get_elevation` in its FE shader, so the gap
dates from whenever Vulkan moved onto that path (`1b6ed35`, #4310) - not from any
of this session's terrain work.

**An attempted fix was reverted (`5864482` -> `fbf87d9`).** Adding `centroid` to
the instanced layout vertex changed its stride from 8 to 12 bytes and broke the
Vulkan instance/SSBO buffer outright - fill-extrusion stopped rendering
altogether (no extrusions with terrain on OR off; a colour diagnostic in the
shader emitted no fragments at all). Reverting restored the buildings, verified
on device.

**Second attempt, also reverted - the real trigger is the sampler, not the
stride.** A follow-up tried elevation *without* touching the instance record:
sample the DEM at each vertex's own outline position (`p1`/`p2`, already in the
shader), leaving `OutlineInstance` at 8 bytes. Buildings disappeared again,
exactly as before. Since that change altered no stride, the common factor across
both failures is **declaring a `sampler2D` in the plain instanced Vulkan FE
shader** (`DRAWABLE_IMAGE_SET_INDEX` binding 0) plus registering a `TextureInfo`
for it. The instanced path evidently has no image descriptor set bound for that
drawable, so adding the binding breaks pipeline creation / the draw and nothing
renders.

**2026-08-01 - FIXED on Vulkan (render test passes against the GL baseline).**
Both prior failures are now fully explained; neither was the elevation maths:

1. **Vulkan binds drawable textures by texture-id slot, not TextureInfo order.**
   `vulkan::ImageDescriptorSet::update` uses the drawable texture array index as
   `dstBinding` and never consults `getSamplerLocation`. The attempts declared
   `dem_sampler` at GLSL `binding = 0` while the tweaker set the DEM at
   `idFillExtrusionDEMTexture` (= 1), so the sampler read the backend's dummy
   texture - which unpacks to a ~1.6e6 m elevation and moved every building out
   of the frustum. They were never "not rendering"; they were in orbit. The
   pattern shader only worked because `idFillExtrusionImageTexture` happens to
   be 0. The DEM sampler must be declared at `binding = 1`.
2. **The OutlineInstance SSBO is the bucket's shared vertex buffer, aliased.**
   `vulkan::Drawable::setSharedBuffers` binds the raw vertex buffer as the
   storage buffer - there is no repacking. The GLSL struct must match
   `FillExtrusionLayoutVertex` byte-for-byte. The centroid therefore packs two
   int16 into one 32-bit word exactly like `pos` (C++ 12 B == std430 stride
   12 B); an `ivec2` would std430-align to 8 and desync the stride - the actual
   failure mode of attempt one.

Implementation (all verified via `terrain/fill-extrusion` on the local Windows
Vulkan runner, pixel-matching the GL baseline): centroid added to the
instancing-path `FillExtrusionLayoutVertex` (8->12 B) and packed by the bucket
on both paths; roof shader samples the DEM at a per-vertex `in_centroid`
attribute (location 5); wall shader reads `centroid_pos` from the aliased
instance record; both declare `dem_sampler` at binding 1 with
`TextureInfo{1, idFillExtrusionDEMTexture}`; same base/height lift as GL
(including the 10 m basement drop gated on dem_enabled). The plain FE suite on
Vulkan: 28 passed, 1 failed (`fill-extrusion-pattern/tile-buffer`) - confirmed
failing identically on the pre-change build, pre-existing.

**2026-08-02 - Metal done too.** Metal respects TextureInfo/getSamplerLocation,
so none of the Vulkan binding trickery applies: the DEM is declared
`texture2d<float, access::sample> demTexture [[texture(0)]]` +
`sampler demSampler [[sampler(0)]]` on the vertex function (mtl::Texture2D::bind
binds to both stages), registered as `TextureInfo{0, idFillExtrusionDEMTexture}`.
The roof shader reads a new per-vertex `short2 centroid [[attribute(5)]]`
(`AttributeInfo{5, Short2, fillExtrusionUBOCount + 0, idFillExtrusionCentroid
VertexAttribute}` - same buffer index as pos/decimals_ed, since all three are
interleaved in the bucket's 12-byte shared vertex); the wall shader reads
`centroid_pos` from the aliased `OutlineInstance` record (synced in 62e1fdc).
Both apply the GL lift verbatim. `mtl::circle.hpp` is the precedent for a
vertex-stage DEM sampler on Metal, including the terrain-off case: the tweaker
only calls `setTexture` when `parameters.terrain` exists, and `get_elevation`
early-returns on `dem_enabled == 0`, so the declared-but-unbound argument is
never sampled.

The **pattern** FE variants still do not elevate, on any backend - GL's
`fill_extrusion_pattern.vertex.glsl` has no `u_dem` either, so Metal matches.
Worth closing on all four backends together rather than diverging here.

*(Historical analysis below, kept for the record.)*

**So the first thing to fix is the texture binding on instanced FE drawables**,
not the elevation maths. Check whether `FillExtrusionLayerTweaker`'s
`drawable.setTexture(..., idFillExtrusionDEMTexture)` actually reaches the
instanced drawables, and whether the Vulkan backend allocates an image
descriptor set for a drawable whose shader declares textures on that path
(`FillExtrusionInstancedShaderSource::textures` was empty before these
attempts - the pattern-instanced variant does have one, so compare against it).
Once a texture can be bound at all, either elevation approach (per-vertex
outline position, or a real centroid) becomes viable.

Lessons for the next attempt:
- **Do not change the instanced vertex stride.** The Vulkan `OutlineInstance`
  SSBO and the C++ layout vertex must agree, and the backend builds that buffer
  from the registered instance attributes; the packing was never confirmed.
  Prefer carrying the centroid somewhere that does not resize the instance
  record (e.g. the drawable UBO, since the whole extrusion shares one centroid
  per polygon... note that is per-drawable, not per-polygon, so it may need a
  different approach), or fix the buffer description first.
- **Verify on device with terrain OFF first** - extrusions must still draw
  before elevation can be judged.
- A quick way to tell the failure modes apart: colour the extrusion fragments by
  `dem_enabled` / sampled elevation. No fragments at all means the draw is
  broken, not the elevation.

**Depth-convention divergence.** GL's terrain depth pass now packs clip-space NDC
z and its `unpack_depth()` does no `*2-1` remap; Metal/Vulkan/WebGPU still pack
window depth (`FragCoord.z`) and keep the remap. Each backend is internally
consistent, so occlusion works on each, but only GL has the corrected convention
and the tuned soft ramp - worth converging.

**Shared-code changes validated only on GL.** The opaque depth-tested terrain
surface (`setEnableDepth(true)`), the depth-pass gate (`depthDirty`/camera-moved)
and the per-mode mesh-tile cap all live in `render_terrain.cpp` and apply to every
backend. Vulkan was spot-checked (renders, re-enable OK); Metal/WebGPU were not.

**Low-end device result (US828, MediaTek GPU, OpenGL, 2026-07-30, git 5864482).**
The load-mode budgets do **not** help on the device they were designed for:

| mode | fps mean | worstMs med/p95/max | jank/s | enc p95/max |
|---|---|---|---|---|
| quality | 8.0 | 262/722/983 | 6.01 | 768/1354 |
| balanced | 8.2 | 265/732/990 | 6.14 | 735/1297 |
| performance | 8.2 | 270/701/865 | 6.23 | 698/1324 |

All three modes are within noise of each other, even though the per-mode tile
cap meant Performance drew 24 mesh tiles against Quality's 64 - a 62% cut in
terrain tiles for no measurable gain. Encode is 700-1350ms per frame and the
whole scene runs at ~8 fps (the Adreno 750 does ~105 fps on the same build).

This is the experiment the `TerrainLoadMode` budgets existed to justify, and it
says they do not work: the bottleneck on this hardware is not the number of
terrain mesh tiles or how many drapes re-render per frame. Something else
dominates - the 1024x1024 drape targets, the DEM uploads, or the non-terrain
layers - and should be measured before any more tuning of the budgets. Consider
whether Balanced/Performance earn their complexity at all in their current form.

**Vulkan performance** (SM-S948U, same flight/benchmark as GL, 90s/mode) is well
ahead of GL: quality 115.2 fps / 2.66 jank-per-s vs GL's 105.0 / 5.38, performance
113.9 / 2.58 vs 86.9 / 10.87.

**Open: one intermittent Vulkan crash.** A `SIGSEGV` in the render thread during a
Balanced-mode benchmark run (0 samples captured). Not reproduced in four
subsequent launches, so it is rare; no backtrace captured yet.

**Symbol occlusion on Metal: code-complete, still unverified on screen.** An audit
by @daviani alongside the device run above found the Metal path whole -
`src/mln/shaders/mtl/terrain_depth.cpp` is the same shape as the Vulkan and WebGPU
ones, all three symbol shaders bind `idSymbolDepthTexture` (slots 2/2/3 against
Vulkan's 3/3/3, which is Metal numbering without the gap Vulkan leaves for the icon
texture), and nothing in `render_terrain.cpp` branches away from Metal. What is
missing is a test that can *see* an occluded marker: markers placed along a line are
raised onto the surface and so are never behind it, and packing them across a ridge
puts them in a narrow band near the horizon at high pitch where out-of-frame and
occluded look alike. The Android answer is `TerrainFlightActivity`'s baked,
elevation-planned path (probes placed by querying terrain elevation rather than by
guessing coordinates); an iOS equivalent would fill this row.

**Metal performance** (iPhone 16 Pro Max, iOS 26.6, z12 Innsbruck, camera rotating
at 24 deg/s, 8 s per pass after a 2 s warm-up, `MLNRenderingStats`; two runs).
The cost scales with pitch, and it is not all extra tiles:

| pitch | fps, terrain | fps, no terrain | delta | encode CPU | draw calls |
|---|---|---|---|---|---|
| 25 deg | 54.8 / 58.8 | 61.2 / 58.9 | -11% / -0% | x1.39 / x1.50 | 6 vs 6 |
| 45 deg | 40.9 / 42.1 | 60.5 / 59.6 | -32% / -29% | x1.73 / x1.82 | 8 vs 8 |
| 65 deg | 21.5 / 22.0 | 53.8 / 56.2 | -60% / -61% | x2.35 / x2.30 | 28 vs 19 |

At 25 and 45 degrees the draw call count is *identical* with and without terrain,
yet encode time is still x1.4-1.8 - so that share of the cost is per-drawable work
(drape targets, tweakers, UBO churn), not tile count. Only at 65 degrees does the
cover itself grow, +47%, as the horizon pushes out. Caveats from the reporter: the
no-terrain passes sit on the 60 Hz vsync ceiling, so the deltas are a **lower
bound**; the style keeps a `hillshade` layer in both arms, so this is terrain *on
top of* hillshade rather than against a bare map; and memory counters were left out
as process-wide gauges that could not be attributed to a pass.

**FIXED - replacing terrain at runtime orphaned the old mesh.** Reported from the
same session: raising `exaggeration` behaved, lowering it blanked the map, and
neither turning terrain off nor any gesture brought it back - only restarting the
app did. That is not the Phase 4 camera-anchoring item it resembles.
`RenderOrchestrator::createRenderTree` dropped and rebuilt `RenderTerrain` whenever
the terrain impl changed, but called `deactivate()` only on the *removal* path. The
mesh layer group is a `shared_ptr` the orchestrator holds in
`layerGroupsByLayerIndex`, a **multimap**, so each replacement stacked a new group
beside the old one instead of evicting it, and every orphan kept drawing with its
own stale exaggeration and a drape texture the new terrain's pool had already
recycled. Every symptom follows: the orphans accumulate one per slider step; turning
terrain off removes only the current group and leaves the pile; per-frame cost grows
until the iOS main-thread render loop stalls and takes the gestures with it. So does
the direction asymmetry - raising exaggeration hides the shorter stale surfaces
behind the new one, while lowering it leaves them towering over and occluding the
real terrain. No Android test activity changes exaggeration at runtime (all five use
a fixed value and toggle only on/off, which took the working path), which is why
device testing never hit it. `TerrainTestOptions` now carries exaggeration controls
and an `exag_sweep_*` ramp so all five activities exercise the replacement path.
The orphaning itself is settled from the code and the fix is in; the symptom chain
above is inference from it and still wants a re-run on Metal to confirm.

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

- **Terrain tile cover** — **done** (`RenderTerrain::computeMeshCover`). Native
  used to derive terrain mesh tiles from the DEM source's `TilePyramid` render
  set and reverse-engineer the ideal cover out of it (`expandToDeepestCover`,
  now removed). It now computes the mesh/drape cover directly from the transform
  via `util::tileCover` with the DEM elevation provider and the frame's LOD
  params - an elevation-aware, LOD-based ideal cover, the way every other source
  is covered and the way gl-js's `coveringTiles({tileSize: 512, terrain})` does.
  Both the drape-target allocation (`Renderer::Impl`) and the mesh build
  (`RenderTerrain::update`) call it with the same `state`/`updateParameters`, so
  the two sets stay in lockstep. This fixed draped roads going blurry over a
  **sparse DEM** (e.g. mapterhorn): high-zoom DEM tiles 404 and native falls
  back to a lower-zoom DEM tile, and the old cover inherited that low zoom, so
  the fixed-size drape target spread over too much ground. The mesh cover now
  stays at the view's ideal zoom and only the DEM/drape *textures* fall back to
  ancestors per tile while exact tiles load. The LOD *zoom selection* inside
  `tileCover` is still not itself elevation-aware (see the near/bottom frontier
  skirt under Phase 3).
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

**Progressive loading budgets + `TerrainLoadMode` API.** Zoom-in / tilt / pan
that reveals new coverage otherwise stalls a frame while a burst of new tiles
build their drawables and dirty drape targets all re-render at once (measured
on PowerVR GE8320: worst interaction frames 713ms/233ms). Two per-frame budgets
spread that work across frames (`gfx::Context::allowNewTileBuild` for new-tile
drawable builds; the drape re-render cap in `Renderer::Impl::render` via
`RenderTarget::render`'s `canRerender`), each requesting a follow-up frame so
deferred work catches up. Because the trade (initial-load sharpness / a bit of
progressive fill-in vs smoother interaction) is hardware-dependent, it is a
per-map API knob, not a style property: `Map::setTerrainLoadMode` /
`TerrainLoadMode { Quality, Balanced, Performance }` (`include/mln/map/mode.hpp`,
`terrainLoadBudget()` maps each mode to a budget pair). Default **Quality =
unlimited** = the historical sharp snap-load, so nothing regresses; Balanced =
32 tiles / 16 drapes per frame, Performance = 8 / 4. Exposed on Android
(`MapLibreMap.setTerrainLoadMode`, `TerrainLoadMode` enum). All five "3D Terrain"
test activities share a `TerrainTestOptions` overflow menu that toggles terrain,
switches load mode, toggles the built-in rendering-stats HUD (FPS + frame
timings + counts), and toggles an above-ground-margin debug log (off by default,
gated so the sampling does not cost anything when unused); the same controls are
driveable over adb (`am broadcast ... --es cmd terrain_toggle`, plus launch
extras) for on-device testing. Not yet exposed on iOS/macOS.

**Fixed - runtime terrain re-enable left the map blank until panned.** Toggling
`style.setTerrain(null)` then back on over an otherwise static map left the map
blank until the view moved (re-entering the activity rebuilt correctly). Root
cause was *not* the draped `renderToTerrain` routing: on the first frame after
re-enable, `RenderTerrain::update` - which resolves the DEM source
`computeMeshCover` needs - has not run yet, so the drape cover is empty and no
drape targets are created; with nothing else loading, the map idles blank. Two
fixes in `Renderer::Impl::render`: (1) request a bounded number of follow-up
frames while an enabled terrain has an empty cover, so the next frame (DEM source
now resolved) covers normally; (2) force a drape re-bake on the frame the cover
reappears - the draped drawables still hold screen-space UBOs from rendering to
screen while terrain was off, so the tweakers must re-run before the fresh
targets are baked or the drape signature cache serves stale (blank) content.
Only observable on this branch (the drape signature cache is what serves the
stale content; upstream terrain-3d re-bakes every frame and never showed it).

**Fixed - hillshade prepare-target memory leak / OOM (`1efc0db`).** On a
terrain style with a hillshade layer, `RenderHillshadeLayer` appended every
per-tile "prepare" render target (a 512x512 Sobel target + its 514x514 DEM
input) to `activatedRenderTargets`, a list only cleared by the edge-triggered
`markLayerRenderable()`. While panning, the layer stays continuously
renderable, so the list was never cleared: live texture count climbed
monotonically (measured 175→634 in ~45s on PowerVR) until the process was
OOM-killed. This also degraded frame rate on lower-end GPUs as the working set
grew. Now `activatedRenderTargets` is reconciled against the tiles still in the
cover each frame (departed tiles' targets released, new ones registered, diffed
by identity so persistent tiles cause no churn), and
`RenderOrchestrator::addRenderTargets` drops any non-drape target it is the
sole owner of as a backstop. After the fix the same run plateaus and oscillates
within a band instead of growing.

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

**2026-07-19/20 update - Vulkan occlusion debugged and fixed on device; GL now
suspected broken by the same change.** Occlusion had *never* actually worked on
Vulkan (the "verified... on OpenGL" note above was accurate, but Vulkan/Metal/
WebGPU following "the same structure" turned out not to be enough). On-device
debugging (screenshots + targeted shader probes) found six independent Vulkan
defects, all now fixed:

1. The terrain depth layer group is never registered with the orchestrator (by
   design - it's rendered only from `RenderTerrain::renderDepth`), so the
   general upload pass skipped it: its drawables had no vertex buffers, and
   Vulkan's `bindAttributes` failed silently every frame. Fix: upload it
   explicitly in `Renderer::Impl::render`. GL builds attribute state at draw
   time, so it never hit this.
2. `RenderTerrain::renderDepth` created the depth render target lazily, after
   the symbol tweaker had already bound `getDepthTexture()` for the frame -
   symbols got the 1x1 far-plane placeholder for that frame, permanently so in
   single-frame renders (the render tests). Fix: `prepareDepthTarget()` runs
   before the upload phase now.
3. Vulkan's `unpack_depth` returned the stored `[0,1]` depth as-is, but symbol
   matrices carry no `[-1,1]->[0,1]` remap (only the terrain/drape matrices do
   - see Phase 2's original z-remap note above), so the compared z was still
   GL-convention and the comparison almost never fired. Fixed to `*2-1`,
   matching GL and Metal.
4. The visibility test was a hard binary z compare; on-surface labels z-fought
   it. Replaced with gl-js's actual `depthOpacity()`: a small bias, a soft fade
   over ~0.002 NDC, and a second sample above the anchor (a label just behind a
   ridge still shows if its glyphs poke above it). Ported to all four backends.
5. Occlusion was gated on `dem_enabled` (does *this tile* have DEM data), so
   labels from tiles outside the terrain cover - exactly the distant labels
   that pierce mountains near the horizon - skipped occlusion entirely. Added
   a global `depth_enabled` flag (terrain on) in `SymbolDrawableUBO`'s former
   padding slot; `dem_enabled` still gates only the elevation.
6. The position compared against the depth texture was `gl_Position` - for
   screen-aligned text that's post label-plane/`coord_matrix`, a different clip
   space than the depth pass (and than gl-js, which compares its tile-projected
   `projectedPoint`). Switched all four backends to pass `projectedPoint`, with
   Vulkan additionally running it through a new `surface_transformed()` helper
   (same transform `applySurfaceTransform()` applies to `gl_Position`).

Verified on Android/Vulkan on-device (`TerrainVectorMapActivity`, planet-vector
basemap over the Alps): distant city labels correctly hide behind the
Nordkette at both steep and shallow pitch, with gl-js's soft fade at ridge
crests. Committed as `aaa145cc85df`.

**Known issue - GL regressed by the same fix (#6 above), unconfirmed cause.**
After the Vulkan fix landed, GL symbol occlusion - previously verified working
on device - stopped hiding labels behind terrain (OpenGL flavour,
`TerrainVectorMapActivity`, reported on device 2026-07-20). Fixes #1-3 are
Vulkan/Metal-specific dead code on GL (its `unpack_depth` already had the
`*2-1`, and GL builds attribute state at draw time so #1 never applied), and
#4/#5 only *broaden* when occlusion applies, so they're unlikely culprits. The
prime suspect is #6: GL's occlusion was implemented and tuned against
`gl_Position` (the final label-plane-adjusted screen position), and switching
its `calculate_visibility` argument to `projectedPoint` (the raw tile-projected
anchor, matching gl-js and fixing Vulkan's shallow-pitch failure) may not be
correct for GL's particular label-plane/pitch/rotation pipeline, or may expose
a second, GL-specific bug that `gl_Position` happened to mask. Not yet
diagnosed on device - `projectedPoint` is provably what gl-js itself compares,
so the fix is more likely to find *why* it fails on GL than to revert it.
Next step: run the local `terrain/occlusion-debug` render test (committed in
`c9fea76` at `metrics/integration/render-tests/terrain/occlusion-debug/`)
against a GL build to check whether the depth texture populates on GL the way
it was confirmed to on Vulkan (see the diagnostic pattern in the aaa145c commit
message / this session's history), before assuming the coordinate-space
hypothesis is right.

**2026-07-23 - RESOLVED on GL, confirmed on device.** Symbol occlusion works on
the Android OpenGL flavour (`TerrainVectorMapActivity`, mapterhorn vector map):
distant city labels fade out completely as the camera approaches a mountain
that comes between them and the viewer, and no labels bleed through foreground
ridges. The earlier "GL regressed" report did not reproduce on the current
build - it was most likely against a build that predated one of the six Vulkan/
GL occlusion fixes (the `projectedPoint` / `depth_enabled` changes in
`aaa145c`). The GL and Vulkan occlusion paths are now both verified on device.
This known issue is closed.

### Phase 3 - Seams and quality

- **Drape target size** (`Renderer::Impl::texturePool`). Now `drapeTileSize *
  drapeQualityFactor` = 512 * 2 = **1024x1024**, matching maplibre-gl-js, which
  renders its render-to-texture tiles at `tileSize * qualityFactor` (a fixed
  `qualityFactor = 2`) *specifically* so draped thin lines are not pixelated/
  aliased when the mesh magnifies them (`src/render/terrain.ts`,
  `src/webgl/render_to_texture.ts` `rttSize`). This was the fix for draped roads
  looking aliased/"not antialiased": the line AA band is sized to the screen's
  device pixel ratio, so at 512 (qualityFactor 1) it fell sub-texel in the
  lower-res target and produced no effective AA. Like gl-js, this is a fixed
  factor independent of `pixelRatio`, so at very high DPR in the foreground the
  1024 target can still be below screen resolution (softer, but no longer
  aliased); `drapeQualityFactor` is the single knob to raise it (each step is 4x
  GPU memory per target) or drop back to 1 on constrained devices. Was the
  long-standing `// TODO: tile size`.
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
- **Fill-extrusion (buildings) are not occluded by terrain** (reported on
  device 2026-07-24): a building behind a hill renders *through* the hill
  instead of being hidden by it. Unlike symbols - 2D billboards that needed the
  explicit depth-texture `calculate_visibility` path - fill-extrusions are real
  3D geometry, so this is more likely a depth-test / draw-order gap against the
  terrain surface (the elevated building geometry not depth-testing against the
  nearer terrain mesh) than a need for the symbol depth-texture path.
  - **Attempted and reverted (2026-07-29).** A per-fragment version of the symbol
    path was tried: `unpack_depth()`/`depth_opacity()` added to the *fragment*
    prelude, the fill-extrusion vertex forwarding its clip position, and the
    fragment discarding where the packed terrain depth is nearer. Per fragment
    (not per drawable) so a ridge can cut through a building, keeping the part
    that pokes above - which is the behaviour we want. It compiled and ran, but
    on device it discarded **every** building fragment while terrain was on
    (buildings vanished entirely; they returned with terrain off), so it was
    reverted.
  - Prime suspect for the next attempt: the comparison is in the wrong space.
    The fill-extrusion drawable's matrix is built with `nearClipped = true`
    (`FillExtrusionLayerTweaker`), so its clip-space z is not on the same scale
    as the terrain depth pass's packed z; the symbol path compares a
    `projectedPoint` that does match. Verify the two depths agree (e.g. render
    the sampled depth as colour) *before* wiring the discard, and note the depth
    pass now packs clip-space NDC z with no `*2-1` remap.
- The terrain mesh/drape cover is now a direct elevation-aware ideal cover
  (`RenderTerrain::computeMeshCover`, see the tile-cover item above), but only
  *visibility* is elevation-aware. Native's tile LOD system
  (`tileLodMinRadius`/`tileLodScale`/`tileLodPitchThreshold`) still drives the
  *zoom* selection and is not elevation-aware.
- **Frontier skirt at the near/bottom screen edge under steep pitch — partially
  mitigated; the residual is Phase 4.** (Diagnosed on device 2026-07-24,
  mapterhorn vector map, 60° pitch.) At high pitch the near-bottom terrain rises
  into view but the tile there is not in the cover, so the last meshed row's
  skirts hang into the gap (with a sliver of background) until a pan pulls the
  tile in. On-device logging established the mechanism precisely: the cover is
  always fully meshed (`noDrawable=0`) but too *small* — the tile is not in the
  cover at all. `util::tileCover` is a top-down DFS that only descends into tiles
  whose **flat (z=0)** ancestor intersects the frustum, so a frontier tile whose
  terrain is in view but whose flat footprint is outside the sea-level frustum is
  never even visited.
  - **Mitigation (done, `computeMeshCover`):** a one-ring dilation of the cover
    (all 8 neighbours), trimmed by `util::frustumCull` with `DEMElevationProvider`
    reporting a conservative range for not-yet-loaded tiles. This re-tests the
    neighbours the DFS skipped, keeping the ones whose elevation-extended bounds
    intersect the frustum; the elevation *sign* picks the side, so it is correct
    for terrain above or below sea level (bathymetry) without hardcoding a
    direction, and the trim keeps it from tripling the cover. Measured: at zoom
    12 the skirt shrank from ~40% of the frame to a thin sliver at near-zero cost
    (+1–2 tiles). Drop the `frustumCull` line for a pure uniform 1-ring (kills the
    sliver but ~2.7× the cover).
  - **Residual (Phase 4):** at higher zoom over tall relief the gap is *several*
    tiles, not one, because the cover frustum is built from a camera anchored at
    z=0 while the true view is from high on the terrain — the near tiles fall well
    outside the sea-level frustum, and `frustumCull` correctly trims the dilated
    neighbours there, so no ring count fixes it (a big skirt plus a white gap at
    the very bottom, the near-rays-miss-terrain signature). This is the
    terrain-anchored-camera work in Phase 4; the dilation only covers the moderate
    (roughly one-tile) case.
- Other transitional artifacts while panning/zooming: brief flat/empty far-field
  areas before a tile loads, and re-resolve flicker when the LOD migrates an
  area between zoom levels. The content-hash render cache and the ideal cover
  reduce these.

### Phase 4 - Terrain-anchored camera (needs a complete fix)

At high pitch over tall terrain the near (bottom) part of the view goes black:
the camera sits at or below the terrain surface, so the near rays hit nothing
and clear to the background. (A blank map that *also* kills gestures and survives
turning terrain off is a different bug - the orphaned-mesh one fixed under Backend
parity above, not this.) This is a camera-model problem, not a tile/drape
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
  per-frame terrain-summary / `skippedNoDEM` logs in render_terrain.cpp.
- **(done)** Removed the terrain bring-up debug scaffolding: the `TERRAIN:`
  texture/upload logs in mtl/drawable.cpp and the `getName() == "terrain"` logs
  in mtl/layer_group.cpp (both back to their upstream bodies bar the
  `renderToTerrain` constructor arg); the checkerboard `createTestMapTexture`
  fallback, the dead `findDEMSource`, and the no-op `update(UpdateParameters)`
  in render_terrain.cpp; the opt-in `MLN_VALIDATE_UNIFORM_BLOCK_BINDINGS` block
  in gl/drawable_gl.cpp; and the Metal terrain shader's elevation-gradient debug
  fallback (it now samples the drape texture like the GL/Vulkan/WebGPU shaders).
  The `// TEMP: Disable depth testing` comment is gone — the drape pass not
  testing/writing the main depth buffer is intentional and unchanged. The
  duplicated DEM sub-tile mapping (`dz`/`scale`/`dx`/`dy`) in render_terrain.cpp
  is now a single `demSubTileOffset` helper. The remaining logs in
  render_terrain.cpp are one-time warning/error logs; terrain_layer_tweaker.cpp
  has no per-frame logging.
- Extend the draped-flag gamma handling to the line gradient/pattern/SDF variants
- Decide whether heatmap should be draped (gl-js does not drape it)
- Runtime styling API for terrain (`setTerrain`) on iOS/macOS (Android has it)

## Testing

- **Render tests**: `metrics/integration/render-tests/terrain/` contains
  `default`, `pitched-world`, `skirts-auto`, `skirts-none` (all ported from
  maplibre-gl-js) and `occlusion-debug` (the local symbol-occlusion diagnostic,
  `c9fea76`). They are un-ignored (no terrain entries left in `metrics/ignores/`)
  and run as part of the normal render-test suite, with native-rendered baselines
  captured on-device (`default`/`pitched-world` un-ignored in `5b17c5a`, skirts in
  `5303d65`; `pitched-world`'s baseline refreshed in `5d39f97`).
  - **2026-08-01 - ROOT CAUSE of the blank renders FOUND AND FIXED.** The prior
    explanation below (missing fixture tiles) was incomplete: still renders drew
    terrain **entirely blank** even where every needed tile was cached. The real
    cause was a first-frame ordering bug: `Renderer::Impl::render` builds the
    frame's drape-target pool from `computeMeshCover()` *before*
    `RenderTerrain::update()` runs, but `demSource` was only bound inside
    `update()` - so on the first frame after a style load the cover was empty,
    zero drape targets existed, `update()` then found no render target for any
    mesh tile (all `getRenderTarget()` misses) and created zero terrain
    drawables. Continuous rendering recovered on frame 2, which is why devices
    never showed it; single-frame still renders (every render test, local and
    CI) stayed permanently blank. Fixed by extracting the source binding into
    `RenderTerrain::prepareSource()` and calling it in `Renderer::Impl::render`
    before the pool is built. Diagnosed with temporary counters: `impl:
    drapeTargets=0` vs `update: meshTiles=6 rtMisses=6` in the same frame.
  - **2026-08-02 - the six terrain failures are THREE independent bugs, not one.**
    Established by diffing the local GL runner against the macOS/Metal CI images
    (node-ci run on the 62e1fdc merge) - the earlier single "missing fixtures"
    explanation is wrong as a blanket claim:

    | test | GL (local) | Metal (CI) | cause |
    |---|---|---|---|
    | occlusion-debug | renders correctly | black bands | Metal-only |
    | fill-extrusion | passes | black bands | Metal-only |
    | default | blank | blank | camera/exaggeration (below) |
    | skirts-auto / skirts-none | blank | blank | missing ancestor-pyramid fixtures (they now "pass" vacuously against blank baselines - see 2026-09-05 below) |
    | pitched-world | (unclassified) | over-filled | zoom -2.5, see zoom-0 relief item |

    1. **Metal black bands - ROOT CAUSE FOUND AND FIXED (96f0a95).** The black
       was the terrain *skirts*, drawn unoccluded and smearing the drape tiles'
       black border texel (u or v = 0) down every curtain. mtl::Drawable::draw
       sets no depth-stencil state for 3D drawables (the layer group owns it);
       mtl::TileLayerGroup does that job, but the plain mtl::LayerGroup that
       holds RenderTerrain's mesh and depth-pass drawables never did - so on
       Metal the terrain drew with the encoder default (depth test Always,
       write off) and triangle order decided visibility. GL/Vulkan pick
       depthModeFor3D inside the drawable and were unaffected. Established by
       a fragment-probe sequence (flat red -> UV visualisation -> skirt-flag
       blue) after drape dumps proved the drape textures pixel-identical to
       GL. The same fix should restore the Metal terrain depth pass (symbol
       occlusion). The next paragraph's earlier drape-texture theory is
       superseded.
       *(earlier, superseded analysis:)* Metal black bands = drape texture not bound. The Metal terrain fragment
       samples `mapTexture` (mtl/terrain.hpp fragmentMain); an unbound/never-rendered
       drape texture reads black. GL renders the identical scene perfectly, so mesh,
       cover and DEM are all fine - only Metal's per-tile drape texture binding fails
       for a subset of tiles. Same family as the Vulkan `binding = 1` bug fixed in
       62e1fdc, different mechanism. **Metal-specific; do not chase it in shared code.**
    2. **skirts-auto / skirts-none: genuinely missing fixtures, and the tile set
       differs from gl-js.** Their DEM source has **no `maxzoom`**, so
       `computeMeshCover`'s `zoomRange{0, demSource->getMaxZoom()}` lets tileCover
       DFS from z0 and native requests a whole **ancestor pyramid** -
       `terrain/6-11-25`, `7-23-50`, `8-47-100`, `9-94-200/201`, `9-95-200`,
       `10-189-400/403`, `10-190-403`, `10-191-401` - none of which are in
       `cache-style.db` (it holds `terrain/` at 16 entries only). gl-js requests only
       its ideal-zoom set for the same camera, which is why its fixture set has no
       such pyramid. This is the concrete form of the long-suspected
       native-vs-gl-js cover/zoom-level difference: **native over-requests
       ancestors**. Two ways out - make the cover stop over-requesting (the real
       fix, Phase 3 tile-cover convergence), or add the pyramid tiles to the cache.
    3. **terrain/default is NOT a fixture problem.** It reports **zero** cache
       misses and still renders blank. Its distinguishing parameters are
       `exaggeration: 2` + `pitch: 60` + `zoom: 13`; the two tests that DO render
       (occlusion-debug, fill-extrusion) both use `exaggeration: 0.45`. That points
       at camera-inside-terrain (Phase 4) rather than data: at 2x exaggeration the
       Alps terrain rises above the sea-level-anchored camera and the view ends up
       under the surface. Testable locally by lowering the exaggeration.

       **CONFIRMED 2026-08-02, and the root cause is a missing feature, not a
       bug: the camera is never anchored to the terrain.** Everything the
       renderer does is healthy - the diagnostics report `meshTiles=3 created=3
       noRenderTarget=0 demTextures=3 demDim=512`, and all three drape targets
       render real content (`drawCalls=3/6/1`, clear 1,1,1,1). The frame is
       white because the mesh clips away entirely. Editing only the
       exaggeration proves it: at `2` the output PNG is 850 B (blank), at `0.45`
       it is 6.3 kB of rendered terrain - and even then the framing is a
       close-up where gl-js's baseline shows a wide pitched vista, i.e. the
       camera is still too low, just no longer underground.

       `TransformState` has a `centerAltitude`, but grep shows it is only ever
       written from explicit `CameraOptions` / flyTo interpolation
       (`transform.cpp` 130/186/378) - **nothing derives it from the terrain**.
       gl-js runs `transform.updateElevation()` every frame, setting
       `_centerAltitude` from the DEM under the map centre. Without that, the
       ground rises toward a camera that stays at its sea-level altitude:
       Innsbruck's valley floor (~575 m) puts the camera ~260 m too low at
       exaggeration 0.45 (merely too close) and ~1150 m too low at 2, which is
       below the surrounding peaks (~2600 m, doubled) - hence the empty frame.
       This is exactly why every terrain test that passes today uses a small
       exaggeration.

       Fixing it is Phase 4 work, not a patch: the transform would have to learn
       the terrain elevation at the centre each frame (the renderer knows it,
       `TransformState` does not), and that value feeds `cameraToCenterDistance`,
       the projection matrices, free-camera, and the pitch limit that keeps the
       camera above ground. Until then `terrain/default` should stay enabled and
       failing - it is the regression guard for this gap.

  - **DEM encoding hygiene (2026-08-02).** The fixture DB holds **three distinct
    DEM tile families** plus a terrarium one, and every terrain test style omits
    `encoding`, so all of them decode as the `raster-dem` default `mapbox`
    (Terrain-RGB):

    | family | used by |
    |---|---|
    | `terrain-shading/{z}-{x}-{y}.terrain.png` | default, occlusion-debug, fill-extrusion |
    | `terrain/{z}-{x}-{y}.terrain.png` | skirts-auto, skirts-none |
    | `{z}-{x}-{y}.terrain.png` | pitched-world |
    | `{z}-{x}-{y}.terrarium.png` | (present in cache-style.db, unused by these tests) |

    **VERIFIED BY DECODING THE PIXELS (2026-08-02) - `tiles/terrain/` is
    genuinely MIXED.** Every tile was decoded both ways (terrarium
    `(R*256+G+B/256)-32768` vs Terrain-RGB `-10000+(R*65536+G*256+B)*0.1`) and
    classified by which yields a plausible land elevation:

    | family | tiles | encodings | used by |
    |---|---|---|---|
    | `terrain/` | 16 | **11 terrarium + 5 terrain-rgb - MIXED** | skirts-auto, skirts-none |
    | `terrain-shading/` | 40 | all terrain-rgb (uniform) | default, occlusion-debug, fill-extrusion |
    | bare `*.terrain.png` | 11 | all terrain-rgb (uniform) | pitched-world |
    | `*.terrarium.png` | 1 | terrarium | (unused) |

    The mixing is *inside one directory*, under identical `{z}-{x}-{y}.terrain.png`
    naming, so it is invisible from the filename - only the pixel values (or the
    tint: terrarium reads red/yellow, Terrain-RGB blue/green) reveal it. A
    misdecode is not subtle: terrarium tiles read as Terrain-RGB come out at
    ~870,000 m, and Terrain-RGB tiles read as terrarium at ~-32,000 m.

    **Consequence for skirts-auto / skirts-none:** they declare no `encoding` (so
    Terrain-RGB) but 11 of the 16 tiles they can load are terrarium. Any of those
    that loads decodes to ~870 km and throws the mesh far outside the frustum -
    a better explanation for their blank output than the missing ancestor pyramid
    alone. The two causes likely compound: missing ancestors AND garbage
    elevations from the tiles that are present. Fix by splitting the directory by
    encoding (or setting `encoding` per source) *and* supplying the ancestors.

    **Correction:** an earlier note here guessed `pitched-world`'s over-filled
    geometry was a terrarium tile misread as Terrain-RGB. That is disproven - its
    bare `*.terrain.png` family is uniformly Terrain-RGB. Its `zoom: -2.5` camera
    remains the open suspect (see the zoom-0 relief item).

    **Rule to keep:** one `raster-dem` source applies a single unpack vector to
    everything it loads, so a test must only ever see tiles of one encoding.
    Verify with the decode-both-ways check above when adding DEM fixtures; the
    failure mode is silent.

  - **2026-09-05 - `skirts-auto` / `skirts-none` pass, but VACUOUSLY. Do not read them
    as covering the skirt option.** Measured on the local Linux/WSL GL runner (they
    pass there with or without this branch's changes, so the "still fail" line below is
    stale for these two): both tests render **byte-identical** output (same md5), and
    their two `expected.png` are byte-identical to each other as well - the baselines
    were captured while terrain was not rendering at all, so each image is just the
    draped geojson line and polygon on a transparent background, with no terrain
    surface and therefore no skirts to differ over. They would pass whether or not
    `TerrainSkirtLength` works, and they did not detect the option being unimplemented
    for as long as it was. Re-baseline them only once terrain actually renders in the
    still-render path (the fixture problem below); until then the skirt option is
    covered only by on-device eyeballing (`skirts_toggle` in TerrainTestOptions).
  - **Current status after the fix (local Windows GL runner):**
    `terrain/fill-extrusion` (new, see below) **passes**; `default`,
    `pitched-world`, `skirts-auto`, `skirts-none`, `occlusion-debug` still fail -
    their baselines were captured on-device, and `default`'s actual is still
    blank even with the fix, so at least one further cause remains (its own
    fixture coverage / style URLs are suspects: `occlusion-debug` centers on the
    cached `terrain-shading` z12 block but its edge tiles are absent, and the
    missing-fixture analysis below still applies to the remaining diffs).
  - The earlier analysis, still relevant for the residual failures: the style
    JSON is byte-identical to the gl-js originals (only `default` differs, in
    whitespace). Native's terrain tile-cover requests tiles the fixture DB does
    not have - the overview pyramid and the near/frontier edges (e.g.
    `terrain-shading/12-2178-1433`, the `terrain/` z6-z9 ancestors) - and those
    exact tiles are absent from gl-js's own fixtures too (for the identical
    camera gl-js requests a *different* tile set). That part is a symptom of the
    Phase 3 tile-cover / Phase 4 terrain-anchored-camera work. Render tests read
    tiles from the SQLite `metrics/cache-style.db` (flat tiles under
    `metrics/integration/tiles/` are loaded into it, manually - there is no
    official populate tool; match the existing `tiles` table row format with
    `insert or replace`).
  - **`terrain/fill-extrusion` (new)**: guards fill-extrusion elevation on
    terrain - buildings must rest on the slope, not at sea level. Inline-GeoJSON
    buildings + the draped numbered raster over the cached Innsbruck
    `terrain-shading` block, camera pitch 45 / zoom 12 / exaggeration 0.45.
    Baseline generated with the local Windows OpenGL runner (`-p
    metrics/linux-opengl.json -u default`), the backend that elevates FE
    correctly today; provisional until Linux CI confirms. This test is the
    regression guard for the FE-elevation gap on the instanced path: it caught
    Vulkan (fixed 62e1fdc) and Metal (fixed 2026-08-02) rendering their
    buildings at sea level, and should FAIL on any backend that stops sampling
    the DEM.
  - The gl-js `terrain/symbol` test is still worth porting now that symbols are
    elevated (needs its DEM fixtures loaded into `metrics/cache-style.db`).
- **Android**: the test app has five "3D Terrain" activities in the Style
  category (`TerrainActivity`, `TerrainVectorMapActivity`,
  `TerrainOsmRasterActivity`, `TerrainDebugTilesActivity`, `TerrainFlightActivity`
  - see "Continuing this work" for what each covers). Developed and tested on the
  `opengl` flavour; the other flavours build but are far less exercised on device.
- **Load-mode benchmark**: `metrics/benchmarks/terrain-load-mode-bench.sh` runs the
  FPV-flight activity once per `TerrainLoadMode` (quality / balanced / performance)
  over the same deterministic baked path with the stats HUD on, capturing a clean
  steady-state window of the `PERF-HUD fps/worstFrameMs/jank/maxEncodeMs` log into a
  per-mode summary (fps mean/p5/min, worstMs median/p95/max, jank-per-second, encode
  p95/max). It is built for **cross-device comparison of the budgets**: run it on a
  high-end and a low-end device at the same commit and diff the summaries. Baseline on
  an Adreno 750 (SM-S948U): Quality is the smoothest (~89 fps, ~10 jank/s) and the
  throttling modes *add* jank (~15 jank/s, worse frame-time spikes) - the budgets are
  meant to help weaker GPUs where the snap-load burst stalls a frame, not fast ones.
  Usage: `metrics/benchmarks/terrain-load-mode-bench.sh <out-dir> [abi] [secs] [build]`.
- Or load any style with a `terrain` root property, e.g. the example above.

## API Usage

### C++ API

```cpp
// Create terrain configuration
auto terrain = std::make_unique<mln::style::Terrain>("demSourceID", 1.5f);

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
