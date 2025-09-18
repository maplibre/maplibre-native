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
- Deduplicated WGSL helper definitions so every shader now pulls shared math/unpack routines from the Prelude; Dawn no longer reports redeclaration errors.
- Corrected the fill-outline WGSL inputs to use `outline_color`/`outline_color_t`, fixing the "struct member not found" errors emitted by Dawn.
- Retain swapchain and depth-stencil views by calling `wgpuTextureViewAddRef` before wrapping them; this prevents use-after-free when the render pass grabs the current surface texture.
- Disabled direct command-encoder debug groups for now (Dawn forbids recording while a render pass encoder is open); render-pass markers will drive debugging once the plumbing lands.

## Observations
- `./run_webgpu.sh` renders the MapLibre demo style end-to-end with WebGPU/Dawn. There are no remaining Dawn validation errors in the steady state.
- For automated runs use `gtimeout 20 ./run_webgpu.sh` (part of GNU coreutils) to auto-exit after a short soak; otherwise close the GLFW window by hand.
- Linking still warns about the Dawn static libs targeting macOS 15.0 while the project is built for 14.3. Bumping `CMAKE_OSX_DEPLOYMENT_TARGET` silences the noise if desired.
- Verbose logging remains enabled in the backend and shaders; keep it on while stabilising the pipeline, then dial back once the regression suite is green.

## Earlier backend improvements
- WebGPU clipping masks now route through a dedicated WGSL program that consumes uniform buffers, and the context builds a Dawn pipeline to populate the stencil attachment before the opaque/translucent passes.
- `PaintParameters::renderTileClippingMasks` builds `ClipUBO` batches for WebGPU instead of skipping stencil work, matching the Metal path.
- Shared shader metadata (`UniformBlockInfo`, `AttributeInfo`, `TextureInfo`, `ClipUBO`) is guarded so Metal/WebGPU headers can coexist without duplicate type definitions.
- Buffer uploads go through Dawn queue writes with 4-byte padding, eliminating the earlier alignment warnings.
- The GLFW backend wires Dawn device/validation callbacks so WGSL or runtime errors land in the MapLibre log for easier diagnosis.

## What still needs attention
- Implement stencil-based clipping for the WebGPU path so tiles match Metal/Vulkan visuals in corner cases.
- Align the WebGPU shader-group selection and bind group layout caching with the Metal/Vulkan registries to reduce redundant pipeline creation.
- Trim logging/no-op debug group stubs once the backend hardens and we decide how to surface Dawn debug markers across platforms.
