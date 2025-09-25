# WebGPU GLFW status

## Build configuration
- Reconfigured CMake with `MLN_WITH_WEBGPU=ON` and `MLN_WEBGPU_BACKEND=dawn`.
- Added `-DFETCHCONTENT_SOURCE_DIR_TINYOBJLOADER=$(pwd)/build/_deps/tinyobjloader-src` to avoid the FetchContent update that requires network access.
- Updated Dawn detection to use the in-tree `vendor/dawn` path on every platform; CMake now sets `DAWN_DIR=${PROJECT_SOURCE_DIR}/vendor/dawn` and pulls headers from the generated include directory.
- Non-Apple builds define `DAWN_ENABLE_BACKEND_VULKAN=1` so Dawn selects the Vulkan backend on Linux; the link step now pulls in `dl`/`pthread` alongside the Dawn static archives.
- WebGPU now opts into triangulated fill outlines (`MLN_TRIANGULATE_FILL_OUTLINES`), matching the Metal backend and avoiding thin-line rendering artefacts on Dawn/Vulkan.

## Code changes
- Treat `glfw_webgpu_backend.cpp` as Objective-C++ on macOS so Dawn/Metal headers compile, and avoid linking Dawn's optional `libdawn_glfw.a` when it is not produced.
- Normalised forward declarations (`AttributeBinding` now declared as `class`) to satisfy clang with `-Wmismatched-tags`.
- Removed the unused `swapBehaviour` member from the WebGPU headless backend and silence the unused-parameter warning.
- WebGPU pipelines now derive their primitive topology from each drawable’s draw mode, so line-based fill outlines use a `LineList` pipeline on Dawn instead of defaulting to triangles.
- WebGPU fill outlines clamp the hardware line width to 1px (wider lines are unsupported on Vulkan/Dawn), preventing the renderer from silently dropping the draws after the first frame.

## Recent WebGPU fixes (2025-09-30, hillshade)
- Reimplemented the WebGPU hillshade and hillshade-prepare WGSL to mirror the Metal pipeline: uniforms now route through the consolidated global-index/storage-buffer path, the prepare pass computes derivatives with the DEM unpack vector, and the lighting shader applies the full slope/aspect logic (intensity scaling, azimuth, accent/shadow mix).
- Updated the hillshade prepare bind group indices to use `idHillshadePrepareDrawableUBO`/`idHillshadePrepareTilePropsUBO` (bindings 2 and 4) so Dawn receives the per-draw uniforms the tweaker uploads; this eliminated the "No buffer found for binding 0" warnings and makes the prep pass valid on Dawn.
- Made WebGPU offscreen render targets expose a `RenderableResource` shim, allowing `CommandEncoder::present` to submit command buffers without trying to swap non-surface renderables; this stopped the `--style maptiler` hillshade crash on Linux.
- `./run_webgpu.sh --style https://api.maptiler.com/maps/outdoor-v2/style.json?key=$MLN_API_KEY` now renders with WebGPU (Dawn/Vulkan/X11). Dawn still emits "No render pass encoder available" once while the pipeline graph warms, but the map continues to render and hillshade layers remain visible.
- Offscreen textures now request `RenderAttachment` usage (and keep `CopySrc`) when backed by Dawn, so the hillshade prepare FBO actually records gradient data; this removes the constant-color shading and keeps the DEM tiles hidden under the lighting overlay.
- WebGPU render passes read the attachment formats directly from each renderable, and the command encoder now recreates a fresh Dawn encoder after every submit. With those two fixes the hillshade prepare pass renders every tile (no more grey fallback quads) while the main pass continues on the same frame. `timeout 20 ./run_webgpu.sh --style https://api.maptiler.com/maps/outdoor-v2/style.json?key=$MLN_API_KEY` completes cleanly with the map fully shaded.
- Matching the Metal/Vulkan backends, WebGPU textures now respect the requested channel type: half-float offscreen targets become `RGBA16Float` and single-channel buffers pick `R16Float`/`R32Float`. Heatmap accumulation uses the high precision target again, so `./run_webgpu.sh --style demotiles-heatmap.json` shows the expected gradients instead of clamped grey tiles.
- WebGPU offscreen textures now resize in step with the swapchain: `Texture2D` emits a size-change callback that rebuilds the Dawn texture views and updates the owning renderable. Resizing the GLFW window no longer blanks the heatmap pass.
- WebGPU render passes now obtain their color/depth attachments from each renderable resource instead of unconditionally binding the swapchain view, so hillshade-prepare tiles stay offscreen and no longer tint the map with the DEM gradient.
- `OffscreenTextureResource` and the GLFW surface resource expose `getColorTextureView`/`getDepthStencilTextureView`, and the viewport/scissor sizing comes from the renderable; Dawn receives the correct 512×512 hillshade framebuffers instead of stretching them to the window size.
- `timeout 25 ./run_webgpu.sh --style ./demotiles-hillshade.json` (Dawn/Vulkan/X11) now shows hillshade highlights/shadows that track the terrain while the DEM layer remains hidden.


## Running the sample
```sh
cmake -S . -B build -G Ninja \
  -DMLN_WITH_WEBGPU=ON \
  -DMLN_WEBGPU_BACKEND=dawn \
  -DFETCHCONTENT_SOURCE_DIR_TINYOBJLOADER="$(pwd)/build/_deps/tinyobjloader-src"
cmake --build build --target mbgl-glfw -- -j8
./run_webgpu.sh
```
- `run_webgpu.sh` now requests the WebGPU backend explicitly (`--backend webgpu`), only applies the Metal debug env vars on macOS, defaults to `https://demotiles.maplibre.org/style.json`, and accepts optional overrides via `--style <url>` or `STYLE_URL=...`.
- A 20s soak on Linux via `timeout 20 ./run_webgpu.sh` brings up the GLFW window on X11/XWayland, logs the selected Dawn adapter, and repeatedly renders the MapLibre demo tiles without validation errors.
- Expect the CLI to print the Dawn adapter scan and a "Successfully started" banner once `mbgl-glfw` survives the startup grace period.

## Recent WebGPU fixes (2025-09-24, fill extrusions)
- `webgpu::TileLayerGroup` now mirrors the Metal/Vulkan path for 3D drawables: it detects when a layer contains extrusions, requests a shared `depthModeFor3D`/`stencilModeFor3D`, and only falls back to per-tile clipping masks for 2D content. This removes the "TODO" stub that left WebGPU extrusions without depth/stencil setup.
- `webgpu::Drawable` exposes `setDepthModeFor3D`/`setStencilModeFor3D`, caching the layer-wide depth/stencil state and invalidating the Dawn pipeline when those change so extrusions render with the correct depth test.
- `./run_webgpu.sh --style https://demotiles.maplibre.org/style.json` (Dawn/Vulkan/X11) now boots cleanly and keeps the fill-extrusion pass alive without the "visited drawables but produced no draw calls" warnings; the GLFW window renders the MapLibre demo tiles with pitched extruded buildings.

## Recent WebGPU fixes (2025-09-26)
- WebGPU now treats stencil tests that omit an explicit read mask as requesting the full `0xFF` mask, mirroring the Metal and Vulkan backends. Dawn previously propagated a zero mask from `stencilModeForClipping`, so later drawables skipped the tile clip test and entire tiles intermittently adopted the colour of another layer.
- `webgpu::Drawable` now programs the stencil reference immediately before each draw (and resets it when stencilling is disabled) while asserting that the cached depth/stencil state matches the paint parameters. This brings the WebGPU path in line with Metal’s bookkeeping and prevents stale stencil refs when toggling per-drawable stencil usage.
- Re-synced the fill outline WGSL with the Metal implementation: `FillOutlineShader` now binds `GlobalPaintParamsUBO` and emits the world-space position varying so future anti-alias logic has the same inputs across backends.
- The WebGPU fill and line pipelines now hand the full clip-space position to Dawn (no manual divide-by-W); this matches Metal/Vulkan depth behaviour and eliminates the perspective loss that let later layers paint over entire tiles or line segments at high zoom.
- `./run_webgpu.sh --style ./debug-glitch.json` and `STYLE_URL=https://demotiles.maplibre.org/style.json ./run_webgpu.sh` (Dawn/Vulkan/X11) both render repeatedly without the solid tile flashes or colour bleeding seen previously.

## Recent WebGPU fixes (2025-09-27)
- Ported the fill-extrusion WGSL from the Metal/Vulkan implementation: normals now split top vs. base vertices via `glMod`, lighting matches the shared luminance/vertical-gradient formula, and the shader respects the `HAS_UNIFORM_*` toggles so constant layers avoid unused attributes.
- Updated the WebGPU attribute descriptors so `FillExtrusionBase`/`FillExtrusionHeight` use `Float2`. The old single-float declaration starved Dawn of the interpolation pair, letting the edge-distance attribute drive Z, which manifested as neon-pink skyscrapers and runaway buffer churn.
- `timeout 25 ./run_webgpu.sh` (Dawn/Vulkan/X11, demotiles style) now produces correctly lit extrusions; the terminal’s "device lost" line is from the enforced timeout rather than a rendering fault.
- Depth state stays sourced from `PaintParameters::depthModeFor3D()`/`stencilModeFor3D()` via `webgpu::TileLayerGroup::setDepthModeFor3D`, matching Metal/Vulkan—no backend-specific overrides required.

## Recent WebGPU fixes (2025-09-24)
- Matched the WGSL `GlobalPaintParamsUBO` layout used by `FillPattern`/`FillOutlinePattern` with the shared std140 struct so Dawn reads the real `pixel_ratio` instead of the atlas width. Pattern fills now honor the style scale rather than repeating at texture-atlas frequency on WebGPU.
- `cmake --build build --target mbgl-glfw -- -j8` followed by `./run_webgpu.sh --style https://demotiles.maplibre.org/style.json` (Dawn/Vulkan on X11) rebuilds, launches, and streams patterned fill tiles; the window logs continuous draw submissions until closed, matching the Metal/Vulkan output.

## Recent WebGPU fixes (2025-09-22)
- Align the WebGPU `SymbolIconShader` vertex stage with Metal: when `HAS_UNIFORM_u_opacity` is defined we now skip sampling evaluated property uniforms and drop the extra varyings so the fragment shader no longer declares location 2. Dawn stops rejecting the module with "Invalid ShaderModule" / "fragment input has no matching vertex output" during `CreateRenderPipeline`.
- Corrected the symbol rotation matrices (`SymbolIcon`, `SymbolSDF`, `SymbolTextAndIcon`) to match Metal's column-major construction. Dawn now rotates glyph quads clockwise like the Metal/Vulkan backends, so road labels follow the street geometry instead of appearing flipped.
- Mirrored the Metal `FillPattern`/`FillOutlinePattern` macro guards: the WGSL now sources pattern UVs and opacity from tile uniforms when `HAS_UNIFORM_u_*` is defined, preventing Dawn from sampling zeroed vertex attributes and restoring patterned fills.
- Dropped the WebGPU-specific `adjustSymbolShader` string surgery; symbol WGSL now matches the other backends by using `HAS_UNIFORM_u_*` guards for optional varyings.
- Added an inline WGSL pre-pass in the WebGPU shader group to evaluate those guards before pipeline creation, keeping Dawn’s vertex layouts consistent with the compiled shaders while still sharing the existing `ProgramParameters` define plumbing.
- Updated the icon, SDF, and text+icon WGSL to gate their vertex inputs, varyings, and fragment fallbacks with the new macros so constant paint properties rely solely on uniforms.
- `./run_webgpu.sh` (Dawn/Vulkan on X11) rebuilds and launches successfully with symbol layers rendering; the previously-logged fill pipeline validation errors no longer appear.

## Recent WebGPU fixes (2025-09-21)
- Packed the circle shader varyings into two vectors (`circle_data` + `stroke_data`) so Dawn sees only five user interpolants; the earlier layout tripped validation by emitting nine varyings and the render pipeline was rejected.
- Reused the shared WGSL prelude for both stages, so the circle vertex/fragment entry points now reference the same consolidated UBO/SSBO bindings (`globalIndex`, `drawableVector`, `props`).
- Mirrored the GL/Metal circle transform path: the WebGPU vertex shader now samples `paintParams` to respect pitch/scale alignment flags, so viewport-aligned bubbles no longer shear when the map tilts.
- Added a circle expression mask to `CircleEvaluatedPropsUBO` (packed into the existing padding slot) and taught the WGSL to fall back to uniform values when a paint property is constant; Dawn no longer sees undefined vertex attributes when the style omits per-feature data.
- Circle smoke test: `timeout 5 ./build/platform/glfw/mbgl-glfw --backend webgpu --style $(pwd)/demotiles-circle.json --zoom 3 --benchmark`.
- Demo tiles regression run: `timeout 10 ./build/platform/glfw/mbgl-glfw --backend webgpu --style https://demotiles.maplibre.org/style.json --zoom 3 --benchmark`.

## Recent WebGPU fixes (2025-09-20)
- `CircleShader` WGSL now mirrors the consolidated UBO pattern: the per-drawable data is fetched from a storage buffer (binding `2`), evaluated props sit at binding `4`, and the global index UBO at binding `1`. Circle layers render without forcing changes in the common renderer code, honoring WebGPU’s bind-group semantics instead of relying on backend-specific hacks.
- Fixed the consolidated line UBO layout: WebGPU now wraps every `Line*DrawableUBO` in a padded storage entry so Dawn sees the 128-byte stride produced by the C++ union, and similarly pads the shared tile-props buffer to 64 bytes. Per-drawable matrices no longer alias between tiles, so `LineShader`, `LineGradientShader`, `LinePatternShader`, and `LineSDFShader` all render across every tile.
- Corrected the `LinePattern` tweaker to write the gap-width and offset interpolation factors to the matching UBO slots; dashed and image-pattern lines now respect their spacing instead of collapsing to solid strokes.
- Validation run: `timeout 10 ./build/platform/glfw/mbgl-glfw --backend webgpu --style https://demotiles.maplibre.org/style.json --zoom 3 --benchmark` (no Dawn validation errors; solid and dashed layers appear on every tile).

## Recent WebGPU fixes (2025-09-19)
- Consolidated vertex attribute bindings so a drawable never binds more than eight unique WebGPU vertex buffers; Dawn no longer complains about exceeding the limit, and the fill layer now reuses shared buffers instead of allocating duplicates per attribute.
- Uniform buffers are allocated with 256-byte alignment before upload, satisfying Dawn's minimum binding size (the global index UBO now binds as 256 bytes instead of 16).
- Pipelines for placeholder shaders (for example, `FillOutlineTriangulatedShader`) are skipped when no WGSL entry point exists, preventing repeated "entry point missing" validation errors while the WGSL is still being authored.
- Removed the leftover Metal-style Y flip in WebGPU vertex shaders (fill, background, line, clipping mask), so clip-space now matches Dawn’s convention and maps render right-side up.
- Detect the Dawn surface's preferred color format and propagate it through the renderer backend. The GLFW WebGPU backend now queries `wgpuSurfaceGetCapabilities`, selects a renderable format (preferring `BGRA8Unorm` for broad compatibility), updates the CAMetalLayer pixel format accordingly, and shares the choice with shader pipeline creation.
- Maintain the swapchain `viewFormats` storage inside the GLFW backend so Dawn sees a stable pointer across reconfigurations, and lock the Metal layer to sRGB color space to avoid color-management surprises.
- WebGPU drawables once again honour the layer-provided colour mask: the WebGPU-specific `Drawable::setColorMode` now forwards to the base implementation instead of discarding the request, so blend/state configuration matches Metal/Vulkan and fragments actually land in the swapchain.
- Shader programs now derive their color attachment format from the backend rather than assuming `BGRA8Unorm`, preventing the swapchain/pipeline mismatch that produced black frames on adapters that default to sRGB outputs.
- Deduplicated WGSL helper definitions so every shader now pulls shared math/unpack routines from the Prelude; Dawn no longer reports redeclaration errors.
- Corrected the fill-outline WGSL inputs to use `outline_color`/`outline_color_t`, fixing the "struct member not found" errors emitted by Dawn.
- Retain swapchain and depth-stencil views by calling `wgpuTextureViewAddRef` before wrapping them; this prevents use-after-free when the render pass grabs the current surface texture.
- Disabled direct command-encoder debug groups for now (Dawn forbids recording while a render pass encoder is open); render-pass markers will drive debugging once the plumbing lands.
- Render pipelines now consume the layer-provided depth/stencil modes, so tile draws respect the per-tile stencil masks written by the clipping pass.
- WebGPU tile layer groups no longer guess shaders from layer names; drawables keep the shader assigned by their builders, matching the Metal/Vulkan flow.
- Reconciled every WebGPU shader's `AttributeInfo` indices and data types with the WGSL `@location` declarations. Dawn no longer flags the clipping mask pipeline (or any other program) as invalid, and the GLFW demo presents a full map instead of a black surface.

- Reworked the symbol WGSL to fetch `SymbolDrawableUBO`/`SymbolTilePropsUBO` from storage-buffer arrays keyed by the shared `GlobalIndexUBO`, eliminating the duplicate `GlobalPaintParamsUBO` definition that Dawn flagged and aligning with the consolidated UBO uploads.
- Swapped the symbolic `@binding(id…)` decorations in the symbol shaders for literal binding indices, allowing the WebGPU shader introspection to build the correct bind-group layouts.
- Per-draw `GlobalUBOIndex` uploads now happen inside the WebGPU drawable, fixing the frozen fill tiles that appeared when every draw sampled the first UBO entry.
- Bind groups now retain both the WGSL `@group` id and the pipeline slot index; `Drawable::draw()` sets the bind group using the slot that the pipeline layout expects. This keeps Dawn’s layout matching stable even when WGSL skips group numbers, so outlines stay on-screen after the first frame.
- Guarded the layer-group warning so it only fires when drawables actually fail to emit draw calls, eliminating the noisy "visited N drawables but drew none" logs during mismatched render passes.
- Fill WGSL now falls back to the uniform color/opacity values when a tile uses constant styling, so constant fills like the Crimea overlay no longer disappear after the first frame.

## WebGPU fixes (2025-09-20)
- Dashed line layers were blank because the WebGPU WGSL for `LineGradient`, `LinePattern`, and `LineSDF` always sampled per-vertex attributes, even when a property was constant. The corresponding binders then uploaded only uniforms, leaving width, gap, and opacity at zero. The shaders now consult `LineEvaluatedPropsUBO.expressionMask` (matching the simple line path) and fall back to uniform values whenever a property is constant. See `include/mbgl/shaders/webgpu/line.hpp`.
- Rebuilt and ran `timeout 30 ./run_webgpu.sh` (demotiles style) after the change; Dawn launches the GLFW demo cleanly and line dash layers render alongside the rest of the MapLibre map.

## Text alpha investigation (2025-09-20)
- OpenGL 3.3/ES 3.0 builds now bind `GL_RED` glyph atlases with a swizzle of `r,r,r,r`, so legacy `.a` sampling continues to work without relying on deprecated `GL_ALPHA` formats.
- Glyph atlases land in `R8Unorm` textures on WebGPU. Dawn promotes the missing channels to `0,0,1`, so sampling `.a` always returns `1`. The WGSL symbol shaders already sample `.r`, matching Metal/Vulkan.
- The blocky text edges stemmed from `wgpuQueueWriteTexture` uploads that used the glyph width as `bytesPerRow`. Dawn requires 256-byte alignment when `height > 1`, so every row past the first was being dropped.
- `webgpu::Texture2D::upload()` and `uploadSubRegion()` now pad rows into a 256-byte-aligned staging buffer before calling Dawn. The helper copies the glyph data into the padded span and zero-fills the slack, restoring the expected SDF gradients.
- Vulkan glyph textures now swizzle the red source channel into both `.r` and `.a`, so the existing `.a` sampling paths continue to work after the GL_ALPHA→GL_R8 switch. (The shader changes were kept aligned with WebGPU.)
- Rebuilt and ran `./run_webgpu.sh` after the change; the GLFW sample still renders the MapLibre demo tiles. Dawn prints no copy-stride validation errors and labels regain smooth alpha.

## WebGPU fixes (2025-09-21)
- `GLFWWebGPUBackend` now initialises and tracks the framebuffer size through its base `Renderable`, so the backend size that the renderer sees matches the GLFW framebuffer. The GLFW WebGPU demo launched with `--zoom 5` now logs `TransformParameters ... scale=32` and `TilePyramid::update ... currentZoom=5`, matching the Metal run instead of dropping to zoom 2.
- Removed the stray padding member from the WebGPU `SymbolDrawableUBO` WGSL struct; Dawn no longer expects a 272-byte binding, and the symbol pipelines consume the 256-byte UBO allocations without validation errors.
- Validated with `timeout 25 ./build/platform/glfw/mbgl-glfw --backend webgpu --style https://demotiles.maplibre.org/style.json --zoom 5` and via `./run_webgpu.sh`. Both runs rendered the demotiles style, logged the corrected zoom, and produced no Dawn validation errors before the scripted timeout terminated the app.

## Raster WGSL update (2025-09-22)
- Aligned the WebGPU raster shader with the Metal/GL uniform layout: the WGSL now reads `RasterDrawableUBO` and `RasterEvaluatedPropsUBO` through the consolidated UBO array, reconstructs the parent cross-fade coordinates, and applies hue/brightness adjustments per pixel. See `include/mbgl/shaders/webgpu/raster.hpp`.
- Built and ran `timeout 15 ./build/platform/glfw/mbgl-glfw --backend webgpu --style satstyle.json --zoom 2`; Dawn compiled the new raster pipeline without validation errors and the satellite raster tiles render in the GLFW WebGPU sample.
- WebGPU's drawable builder now clears its cached texture slots after each flush (`src/mbgl/webgpu/drawable_builder.cpp`), forcing raster drawables to bind their own `Texture2D` handles. The previous cache leaked the first tile's texture across subsequent drawables, so every tile rendered the same imagery.

## Observations
- `./build/platform/glfw/mbgl-glfw --backend=webgpu --style=https://demotiles.maplibre.org/style.json` now reports `WebGPU: selected surface format BGRA8Unorm` and renders the MapLibre demo tiles correctly on macOS. The translucent/opaque passes log non-zero drawable counts, confirming geometry is making it through the pipeline.
- Linux run (aarch64) selects the Vulkan-backed Dawn adapter, prints `WebGPU: selected surface format BGRA8Unorm`, and the render loop logs fill/background/line bind groups for all visible tiles—confirming we see map content rather than a blank swapchain.
- `./run_webgpu.sh` renders the MapLibre demo style end-to-end with WebGPU/Dawn. There are no remaining Dawn validation errors in the steady state.
- For automated runs use `timeout 20 ./run_webgpu.sh` on Linux (or `gtimeout` on macOS) to auto-exit after a short soak; otherwise close the GLFW window by hand.
- A 20-second soak via `./build/platform/glfw/mbgl-glfw --style https://demotiles.maplibre.org/style.json` (captured in `webgpu_run_new.log`) completes without any WebGPU warnings, and boundary/outline geometry renders cleanly.
- Confirmed the same soak after the constant-fill fix: Crimea and other constant fill tiles stay colored while the style updates, and line layers remain visible across the whole world.
- Linking still warns about the Dawn static libs targeting macOS 15.0 while the project is built for 14.3. Bumping `CMAKE_OSX_DEPLOYMENT_TARGET` silences the noise if desired.
- Verbose logging remains enabled in the backend and shaders; keep it on while stabilising the pipeline, then dial back once the regression suite is green.
- Stencil clipping is active again: geometry outside a tile’s clip polygon is discarded, matching Metal/Vulkan visuals when panning across tile seams.
- The current soak shows `Line draw bind global index=0..17` for every frame, confirming the consolidated UBO array is read per-tile and that line outlines now render on all visible tiles instead of jumping between them.
- 2025-09-19 run via `./run_webgpu.sh` on macOS 14.5 renders the MapLibre demo style; Dawn logs ~30k trace lines while warming caches and occasionally reports `Device was destroyed` when the window closes, but no validation errors appear during steady rendering.

## Earlier backend improvements
- WebGPU clipping masks now route through a dedicated WGSL program that consumes uniform buffers, and the context builds a Dawn pipeline to populate the stencil attachment before the opaque/translucent passes.
- `PaintParameters::renderTileClippingMasks` builds `ClipUBO` batches for WebGPU instead of skipping stencil work, matching the Metal path.
- Shared shader metadata (`UniformBlockInfo`, `AttributeInfo`, `TextureInfo`, `ClipUBO`) is guarded so Metal/WebGPU headers can coexist without duplicate type definitions.
- Buffer uploads go through Dawn queue writes with 4-byte padding, eliminating the earlier alignment warnings.
- The GLFW backend wires Dawn device/validation callbacks so WGSL or runtime errors land in the MapLibre log for easier diagnosis.

## What still needs attention
- Audit the terrain and globe renderers once they migrate to the shared 3D depth/stencil hooks; only fill extrusions are wired up today.
- Align the WebGPU shader-group selection and bind group layout caching with the Metal/Vulkan registries to reduce redundant pipeline creation.
- Trim logging/no-op debug group stubs once the backend hardens and we decide how to surface Dawn debug markers across platforms.
