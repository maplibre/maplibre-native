# WebGPU GLFW status

## Build configuration
- Reconfigured CMake with `MLN_WITH_WEBGPU=ON` and `MLN_WEBGPU_BACKEND=dawn`.
- Added `-DFETCHCONTENT_SOURCE_DIR_TINYOBJLOADER=$(pwd)/build/_deps/tinyobjloader-src` to avoid the FetchContent update that requires network access.
- Rebuilt Dawn previously bundled under `vendor/dawn/build` is reused; no additional toolchain work needed.

## Code changes
- Treat `glfw_webgpu_backend.cpp` as Objective-C++ on macOS so Dawn/Metal headers compile, and avoid linking Dawn's optional `libdawn_glfw.a` when it is not produced.
- Normalised forward declarations (`AttributeBinding` now declared as `class`) to satisfy clang with `-Wmismatched-tags`.
- Removed the unused `swapBehaviour` member from the WebGPU headless backend and silence the unused-parameter warning.

## Running the sample
```sh
cmake -S . -B build -G Ninja \
  -DMLN_WITH_WEBGPU=ON \
  -DMLN_WEBGPU_BACKEND=dawn \
  -DFETCHCONTENT_SOURCE_DIR_TINYOBJLOADER="$(pwd)/build/_deps/tinyobjloader-src"
cmake --build build --target mbgl-glfw -- -j8
./run_webgpu.sh
```
- `run_webgpu.sh` launches `mbgl-glfw` against the MapLibre demo tiles. On headless CI/macOS the window appears on the local console; close it manually to stop the run.
- Expect the CLI to print the Dawn adapter scan and a "Successfully started" message when the swapchain is ready.

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
- Guarded the layer-group warning so it only fires when drawables actually fail to emit draw calls, eliminating the noisy "visited N drawables but drew none" logs during mismatched render passes.
- Fill WGSL now falls back to the uniform color/opacity values when a tile uses constant styling, so constant fills like the Crimea overlay no longer disappear after the first frame.

## Observations
- `./build/platform/glfw/mbgl-glfw --backend=webgpu --style=https://demotiles.maplibre.org/style.json` now reports `WebGPU: selected surface format BGRA8Unorm` and renders the MapLibre demo tiles correctly on macOS. The translucent/opaque passes log non-zero drawable counts, confirming geometry is making it through the pipeline.
- `./run_webgpu.sh` renders the MapLibre demo style end-to-end with WebGPU/Dawn. There are no remaining Dawn validation errors in the steady state.
- For automated runs use `gtimeout 20 ./run_webgpu.sh` (part of GNU coreutils) to auto-exit after a short soak; otherwise close the GLFW window by hand.
- A 20-second soak via `./build/platform/glfw/mbgl-glfw --style https://demotiles.maplibre.org/style.json` (captured in `webgpu_run_new.log`) completes without any WebGPU warnings, and boundary/outline geometry renders cleanly.
- Confirmed the same soak after the constant-fill fix: Crimea and other constant fill tiles stay colored while the style updates, and line layers remain visible across the whole world.
- Linking still warns about the Dawn static libs targeting macOS 15.0 while the project is built for 14.3. Bumping `CMAKE_OSX_DEPLOYMENT_TARGET` silences the noise if desired.
- Verbose logging remains enabled in the backend and shaders; keep it on while stabilising the pipeline, then dial back once the regression suite is green.
- Stencil clipping is active again: geometry outside a tile’s clip polygon is discarded, matching Metal/Vulkan visuals when panning across tile seams.
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
