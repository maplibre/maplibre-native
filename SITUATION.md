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

## Running the sample
```sh
cmake -S . -B build -G Ninja \
  -DMLN_WITH_WEBGPU=ON \
  -DMLN_WEBGPU_BACKEND=dawn \
  -DFETCHCONTENT_SOURCE_DIR_TINYOBJLOADER="$(pwd)/build/_deps/tinyobjloader-src"
cmake --build build --target mbgl-glfw -- -j8
./run_webgpu.sh
```
- `run_webgpu.sh` now requests the WebGPU backend explicitly (`--backend webgpu`) and only applies the Metal debug env vars on macOS.
- A 20s soak on Linux via `timeout 20 ./run_webgpu.sh` brings up the GLFW window on X11/XWayland, logs the selected Dawn adapter, and repeatedly renders the MapLibre demo tiles without validation errors.
- Expect the CLI to print the Dawn adapter scan and a "Successfully started" banner once `mbgl-glfw` survives the startup grace period.

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
- Extend the new depth/stencil plumbing to 3D drawables (extrusions, terrain) once the upstream hooks are ready.
- Align the WebGPU shader-group selection and bind group layout caching with the Metal/Vulkan registries to reduce redundant pipeline creation.
- Trim logging/no-op debug group stubs once the backend hardens and we decide how to surface Dawn debug markers across platforms.
