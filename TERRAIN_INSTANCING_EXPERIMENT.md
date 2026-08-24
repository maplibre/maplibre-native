# Terrain-mesh instancing — feasibility / design (experiment branch)

Branch: `experiment/terrain-mesh-instancing`. Scratch design notes for collapsing the
per-tile terrain draws into a single instanced draw, to attack the CPU-encode bottleneck.
Not wired up yet — this is the "is it something we can do?" writeup.

## Prior art: WifiDB PR #5 "Terrain depth instancing" already does the depth half

`WifiDB/maplibre-native#5` implements this concept for the terrain **depth** pass and confirms
it is feasible on GL/PowerVR. It already contains the infra this doc calls for — so **build on
it rather than re-deriving the depth half here**:

- `src/mln/gl/texture_2d_array.{cpp,hpp}` — a GL-only `GL_TEXTURE_2D_ARRAY` of RGBA8 layers
  (pieces §1 below), packing the per-tile DEMs so one instanced draw samples a layer per tile.
- `TerrainDepthInstanceUBO[TERRAIN_MAX_INSTANCES=64]` indexed by `gl_InstanceID` (not a per-
  instance vertex attribute — the GL backend never binds divisor-1 attributes), with a
  `dem_layer` per instance (§3).
- `DrawableGL::setArrayTexture` + instanced draw (§2); instanced `terrain_depth.vertex/fragment`
  sampling `sampler2DArray u_dem_array` (§6).
- Heavy PowerVR profiling (`pvr_depth*.csv`, perfetto traces) — check those for the measured win.

**What #5 leaves for later (and what this branch should focus on): the color / drape pass.**
The depth pass only needs geometry + DEM, so it never packs the **drape** RTT into an array.
Instancing the visible terrain (`terrain.vertex/fragment` + the per-tile map drape) is the
harder, higher-payoff half — the drapes are the bulk of the terrain draw calls — and needs the
drape RTT rendered into array layers (§4–5). The `TERRAIN_MAX_INSTANCES=64` cap pairs with the
`coverZoomShift` shipped on the PR branch to keep tile counts under the array-layer limit.

## Why

The FPV-flight benchmark on a CPU-encode-bound low-end GPU (PowerVR, OpenGL) showed the
**GPU idle** (`Rendering time ≈ 0.004ms`) while **encode ran 25–61ms**; frame rate tracks
draw-call count, not fill. Encode cost ≈ `~15ms baseline + ~0.35ms × draw calls`. In the
wide overview the terrain alone is ~100+ of the ~128 draw calls — and every one of those
draws the **same shared 128×128 mesh**, differing only by a per-tile matrix + DEM + drape
texture. Instancing collapses those N draws into ~1: the "loop over tiles" moves from the
CPU into the GPU's instance loop. `mode.hpp`'s per-mode `coverZoomShift` (already landed on
the PR branch) cuts N; instancing removes the per-tile-draw cost of N entirely.

## How terrain draws today

`RenderTerrain::update` computes the cover (`computeMeshCover`) and, per cover tile, calls
`createDrawableForTile(context, shaders, tileID, demTexture, drapeTexture)` → one drawable in
the terrain layer group (plus a twin in the depth layer group for occlusion). Each drawable:

- shares the one 128×128 grid mesh (`RenderTerrain::getMesh`),
- carries a per-tile `u_matrix` (tile transform) in its drawable UBO,
- binds that tile's **DEM texture** (`u_dem`) and **drape texture** (the map RTT for that tile),
- carries per-tile DEM coords/unpack/exaggeration in the UBO (incl. ancestor-DEM fallback).

So per instance we need: a transform, DEM sampler+coords, and a drape sampler — all per-tile.

## The crux: per-instance textures

Instancing shares geometry + shader, but each instance must sample a **different** DEM and a
**different** drape. Options:

1. **2D array textures** (`sampler2DArray`, layer = `gl_InstanceID`). All cover tiles' DEMs in
   one DEM array, all drapes in one drape array; the instanced shader indexes by layer. This
   is the clean fit and what this design assumes.
2. Texture atlas (one big texture, per-instance UV offset). Avoids array textures but needs
   atlas packing + guard bands; awkward for the 1024² drapes.
3. Bindless textures — not available on GL ES.

Memory is **not** extra: the drapes are already N separate 1024² RGBA textures (~171MB on the
PowerVR run); an array is the same bytes, reorganized. The DEM array is small.

## Backend status (target first: GL)

- **GL ES 3.0** (our shader target) supports `glDrawElementsInstanced`, `gl_InstanceID`,
  `sampler2DArray`, and layered FBO rendering — so it's API-feasible on the PowerVR device.
- **But gfx/GL has neither piece yet**: no instanced-draw path (GL isn't in
  `MLN_USE_FILL_EXTRUSION_INSTANCING`, which is Metal/Vulkan only) and **no `Texture2DArray`
  abstraction** (`context.hpp` has only a uniform-buffer *array*, not a texture array).
- Metal/Vulkan/WebGPU would follow the same shape later; GL is the one that matters for the
  bottlenecked device, so prototype there.

## Pieces to build

1. **`gfx::Texture2DArray`** (+ GL impl): allocate `GL_TEXTURE_2D_ARRAY`, upload a layer,
   attach a layer as an FBO color target (for drape rendering into layers).
2. **GL instanced draw**: `glDrawElementsInstanced` with an instance count; per-instance
   attribute divisor for the instance data buffer.
3. **Per-instance data**: pack per-tile `{matrix (or tile offset+scale), dem_coords, dem layer,
   drape layer, dem_enabled, exaggeration}` into a per-instance vertex-attribute buffer or an
   instance-indexed UBO/SSBO. This replaces the per-drawable UBO.
4. **DEM as an array**: upload each cover tile's DEM (incl. ancestor fallback) into a DEM array
   layer; record the layer per instance. Border backfill unchanged (still per tile).
5. **Drapes as an array**: render each tile's drape RTT into a drape-array layer instead of a
   standalone target (the RTT loop already exists in `RenderTarget`; retarget it to a layer).
6. **Instanced terrain + terrain_depth shaders**: `#version 300 es`, `gl_InstanceID` picks the
   instance row + array layers; sample `u_dem`/`u_drape` as `sampler2DArray`.

## Risks / open questions

- **`GL_MAX_ARRAY_TEXTURE_LAYERS`** (device-dependent, usually ≥256) must exceed the cover tile
  count. With the Performance-mode `coverZoomShift`, counts stay well under that; add a fallback
  to the current per-tile path if a cover ever exceeds the limit.
- **Drape RTT into array layers**: needs layered FBO attachment; verify PowerVR/GL ES driver
  support and that the per-layer clear/stencil still works (drape targets carry depth+stencil).
- **Ancestor-DEM fallback** and the render cache (`DrapeCoverage`) are currently per-target;
  they'd move to per-instance-row / per-layer bookkeeping.
- This is a **large, cross-cutting change** (new gfx primitive + new draw path + reworked
  DEM/drape lifecycle + new shaders), not a quick patch. Estimate: a multi-step effort, GL only
  to start.

## Verdict + suggested first step

**Feasible on the target (GL ES 3.0), but it's real infra work.** The de-risking first step,
before committing to the full DEM/drape array rework, is a **thin spike**: add a minimal
`gfx::Texture2DArray` + a GL instanced-draw path, and render the terrain mesh instanced with a
per-instance **transform only** (DEM/drape still bound the old way for a single tile, or a flat
mesh), purely to confirm on-device that `glDrawElementsInstanced` collapses the terrain draw
calls and the encode time drops as predicted. If the encode win shows up, build out the DEM +
drape arrays (pieces 4–5); if the driver fights the array-FBO/instancing path, fall back to the
`coverZoomShift` mitigation already shipped.

_Device testing required at each step; none of this has been run yet._


## ROOT CAUSE of the white-frame flicker in this port (2026-07-29)

The port on this branch renders intermittent **fully blank/white frames** while the camera
moves (zoom-out, and the FPV flight). PR #5's own branch does **not** — so the feature works
at source and the defect is in this port being **incomplete**.

### Evidence (Android/OpenGL, SM-S948U, Quality mode)

Automated metric: run the FPV flight 40s, sample 15fps, count frames whose map area is >85%
near-white. Motion is reported to validate that a capture actually exercised the map.

| build | blanks | motion | note |
|---|---|---|---|
| `98336c7` baseline (no port) | **0** / 600 | 20.4 | |
| `e362eb4` + our perf changes | 0 (user-verified) | | not the cause |
| `c69d31d` GL infra (instanced draw, texture_2d_array, FBO invalidate) | 0 (user-verified) | | not the cause |
| `ea18a7f` + instanced depth **shader** | **0** / 600 | 20.5 | shaders are innocent |
| `2a4022d` + instanced depth **C++** + bridge | **38** / 599 | 23.3 | regression appears here |

On a blank frame the renderer is healthy (84 draw calls, ~102 FPS, 7.4ms encode) and symbol
labels still draw - only the **draped** content is missing, and texture count/memory drop
(122->71, 142MB->77MB) as drape targets are released.

Ruled out by experiment: Performance-mode budgets; the drape-signature cache; the offscreen
FBO invalidate (depth/stencil only, correct usage); the per-tile depth guard (correctly
`#if !MLN_RENDER_BACKEND_OPENGL`); DEM-array reallocation (0 events logged); DEM-array size
(64->16 layers changed nothing); disabling the depth render, the DEM-array packing, or the
instanced drawable (all made it *worse*, 51-53 blanks, because they leave GL with **no depth
pass at all** - `58a9716` removes the per-tile GL depth path).

### The missing prerequisite: `604f293ca88f`

"perf(terrain): gate depth pass, cap mesh tiles, opaque depth-tested surface" was **not
cherry-picked**, but the instanced depth work depends on it:

- **`if (!depthDirty && !cameraMoved) return;`** - the *consumer* of `depthDirty`. In this port
  the flag is write-only (added by the bridge commit), so the depth pass runs **every frame**.
- **`builder->setEnableDepth(false)` -> `setEnableDepth(true)`** - "the terrain surface is
  opaque 3D geometry", replacing the old depth-off "2D for now" hack, explicitly for tiled GPUs.
  **This is the prime suspect**: the instanced pass writes depth while the terrain surface is
  still depth-disabled, so the surface intermittently fails to draw -> blank frame.
- **mesh-tile cap** (keep tiles nearest the centre) - bounds the cover, which matters because
  `maxDepthInstances`/`maxDEMArrayLayers` are 64.

`560a3d77a7f9` ("wip: session perf edits") is likely also needed - it carries the real
`buildMesh`/`getDepthMesh` that the bridge commit stubbed out. (Note: PR #5's
`terrainDepthMeshGridSize()` defaults to **128**, i.e. full-res - "coarser breaks symbol
occlusion" - so the bridge's alias to the full mesh is *equivalent*, not a deviation.)

### Next step

Cherry-pick `604f293ca88f` (10 conflicts across render_terrain.{cpp,hpp}, one ~120 lines) and
re-measure with the flight metric above; then `560a3d77a7f9` if `buildMesh`/`depthDirty` are
still stubbed. Resolve carefully - the whole failure mode here was a **half-ported** state.

_The PR branch `terrain-3d-color-relief` is unaffected by any of this._
