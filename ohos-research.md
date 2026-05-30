# OpenHarmony Research Log

## Current status snapshot

- Branch: `ohos-research` (investigation; `platform/ohos/` is not yet committed).
- Sample bundle: `org.maplibre.native.demo` (AGC disallows `ohos` in bundle IDs).
- Device: Huawei MatePad Pro `MRDI-W00`, HarmonyOS `6.1.0.117`, API `23`
- Install without DevEco: `hvigorw assembleApp`, manual `hap-sign-tool.jar sign-app`
  (`platform/ohos/sample/sign/README.md`), `hdc install -r sign/entry-default-signed.hap`.
- Sample UI: legacy `libraryname: 'maplibre_native_ohos'` declarative `XComponent`
  (`onLoad` context). ArkUI node-handle `registerXComponentNode()` still fails on
  this device (`Could not create XComponent surface holder callback`).
- Sample native HTTP: `MLN_OHOS_WITH_HMS_RCP=ON` in
  `platform/ohos/sample/entry/src/main/cpp/CMakeLists.txt` (HMS `librcp_c.so`, not
  OpenHarmony `net_http`).
- **Confirmed on device (GLES + inline styles):**
  - **Diag** — solid red background (`#ff0000`); proves background fill path.
  - **Local** — blue background (`#004488`) + yellow GeoJSON polygon fill
    (`#ffdd00`); proves vector fill layers without network.
  - **Empty** — black (no background layer).
  - Map surface stable (`hasMap=yes`, EGL config probing in `egl_window_backend.cpp`).
- **Not working:**
  - **Remote** (`https://demotiles.maplibre.org/style.json`) — process exits ~3s
    after load; `hilog` shows `SIGSEGV` in `OnlineFileSource` worker inside
    `librcp_c.so` (`strlen` → `RcpToCppConfiguration` / `RcpToCppRequest`).
    Suspected cause: uninitialized `Rcp_Request` optional fields after
    `HMS_Rcp_CreateRequest` (not yet fixed; a one-off `initializeRcpRequest()`
    attempt was reverted).
  - **Remote with `net_http`** (earlier build) — `OH_HTTP_OUT_OF_MEMORY` (2300027)
    on style fetch.
- Sample diagnostics: periodic `hilog` from ArkTS (`MapLibreSample`, 3s) and native
  (`MapLibreNative`); buttons **Render**, **Remote**, **Empty**, **Diag**, **Local**,
  **Memory**. No auto-load of Remote on startup.
- Open blockers: Remote/RCP crash, remote MVT/glyphs/sprites, gestures, ImageKit decode.

## 2026-05-29

### Starting point

- Branch: `ohos-research` at `647636bf6115` (`origin/main`).
- GitHub issue: `maplibre/maplibre-native#3749`, still open. The newest comment is a November 12, 2025 thumbs-up; no newer technical findings or PRs were found.
- Related issue `maplibre/maplibre-native#3940` was opened November 12, 2025 and closed the same day as a duplicate/reference back to `#3749`; it does not add implementation details.
- User-provided SDK location: `/Users/sargunv/Downloads/command-line-tools`.

### SDK/tooling observed

- Command line tools version file reports:
  - Command Line Tools mac-arm64: `6.0.1.251`
  - HarmonyOS SDK: `6.0.1 Release`
  - OpenHarmony public SDK: `6.0.1.112`
  - API version: `21 Release`
- Native toolchain:
  - Toolchain file: `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native/build/cmake/ohos.toolchain.cmake`
  - Toolchain CMake: `3.28.2`
  - OHOS clang: `15.0.4`
  - Default native target: `aarch64-unknown-linux-ohos`
- Additional tools available in the command-line package:
  - `hvigorw` version `6.21.1`
  - `ohpm` version `6.0.1`
  - `hdc` version `3.2.0b`

### Local side effects

- Running `hvigorw --version` from the repo root created `.hvigor/outputs/build-logs/build.log`.
- No intentional changes were made outside this project directory.

### SDK sysroot notes

- Available in the OHOS native sysroot for `aarch64-linux-ohos`:
  - headers: `EGL/egl.h`, `GLES3/gl3.h`, `hilog/log.h`, `multimedia/image_framework/image/image_source_native.h`, `network/netstack/net_http.h`, `uv.h`
  - libraries: `libEGL.so`, `libGLESv2.so`, `libGLESv3.so`, `libhilog_ndk.z.so`, `libimage_source.so`, `libnet_http.so`, `libpixelmap.so`, `libuv.so`, `libz.so`
- Not found in the OHOS native sysroot:
  - `curl/curl.h`, `png.h`, `jpeglib.h`, WebP headers
  - `libcurl`, `libpng`, `libjpeg`, `libwebp`

### C++20 toolchain findings

- The SDK compiler accepts `-std=c++20` and reports `__cplusplus == 202002L`.
- The bundled libc++ still defines `_LIBCPP_HAS_NO_INCOMPLETE_RANGES`.
- `<source_location>` is not available.
- A small probe confirmed that `std::ranges::find` is unavailable with the OHOS SDK libc++.
- The same ranges probe succeeds with either `-D_LIBCPP_ENABLE_EXPERIMENTAL` or clang's `-fexperimental-library`.
  - `-fexperimental-library` is now the preferred workaround because it exposes the SDK's own guarded libc++ ranges algorithms without adding local definitions inside `std::ranges`.
  - The flag does not provide `<source_location>`; the guard call-site code now uses an internal `mbgl::SourceLocation` type instead of relying on the standard header.

### Build progress

- Configured a core-only OpenGL/EGL build with:

  ```sh
  pixi run cmake -S . -B build-ohos-core -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native/build/cmake/ohos.toolchain.cmake \
    -DOHOS_ARCH=arm64-v8a \
    -DOHOS_STL=c++_shared \
    -DMLN_WITH_CORE_ONLY=ON \
    -DMLN_WITH_OPENGL=ON \
    -DMLN_WITH_EGL=ON \
    -DMLN_WITH_GLFW=OFF \
    -DMLN_WITH_WERROR=OFF
  ```

- After temporary C++20 compatibility patches, `pixi run cmake --build build-ohos-core -j 8` linked:
  - `build-ohos-core/libmbgl-core.a`
  - `build-ohos-core/vendor/maplibre-tile-spec/cpp/libmlt-cpp.a`
- First checkpoint patch state before cleanup:
  - OHOS-only force include of the tile-spec polyfill for `mbgl-core` and `mlt-cpp`.
  - A `source_location` header availability check in `symbol_instance.hpp`.
  - A ranges algorithm shim inside the `vendor/maplibre-tile-spec` submodule.

### Direction

- Prefer Linux/default platform sources where possible.
- Do not reuse Android platform pieces unless the OHOS SDK/API shape actually matches Android better for that component.
- Move temporary compatibility code out of vendored submodules where possible.
- Add a dedicated OpenHarmony platform CMake file instead of relying on `MLN_WITH_CORE_ONLY`.

### Current patch status

- Added `CMakePresets.json` entries:
  - `ohos-opengl-core`: core-only OpenHarmony GLES/EGL build in `build-ohos-core`
  - `ohos-opengl`: OpenHarmony platform build in `build-ohos`
  - `ohos-opengl-link-smoke`: OpenHarmony platform build in `build-ohos-link-smoke` with a final shared-library link smoke target.
  - `ohos-opengl-napi-smoke`: OpenHarmony platform build in `build-ohos-napi-smoke` with a NAPI/XComponent shared-library smoke target.
  - `harmonyos-opengl`: HarmonyOS/HMS platform build in `build-harmonyos` with `MLN_OHOS_WITH_HMS_RCP=ON`
  - `harmonyos-opengl-link-smoke`: HarmonyOS/HMS platform build in `build-harmonyos-link-smoke` with a final shared-library link smoke target.
  - `harmonyos-opengl-napi-smoke`: HarmonyOS/HMS platform build in `build-harmonyos-napi-smoke` with a NAPI/XComponent shared-library smoke target.
- Added `platform/ohos/ohos.cmake`.
  - Uses default/Linux-style platform sources for filesystem, SQLite/offline storage, run loop, timers, compression, text, map snapshotting, and headless gfx.
  - Uses vendored `nunicode`, `sqlite`, and the built-in ICU fallback instead of requiring unavailable OHOS sysroot packages.
  - Uses Linux EGL/headless GL sources where possible.
  - Uses an optional `GLES3/gl3ext.h` include in `platform/linux/src/gl_functions.cpp` because the OHOS SDK provides `GLES3/gl3.h` but not `GLES3/gl3ext.h`.
  - Adds opt-in `MLN_OHOS_BUILD_LINK_SMOKE` to link `mbgl-core` into `libmbgl-ohos-link-smoke.so` with `--whole-archive` and `--no-undefined`.
  - Adds opt-in `MLN_OHOS_BUILD_NAPI_SMOKE` to link `mbgl-core` into `libmaplibre_native_ohos.so` with NAPI, XComponent, and native-window dependencies.
- Added OHOS-specific implementations:
  - `platform/ohos/src/logging_hilog.cpp`: routes MapLibre logs to `hilog`.
  - `platform/ohos/src/image.cpp`: decodes images with OHOS `ImageSourceNative`/`PixelmapNative`.
  - `platform/ohos/src/http_file_source.cpp`: public OpenHarmony HTTP backend using NetworkKit `net_http`.
  - `platform/ohos/src/http_file_source_hms_rcp.cpp`: optional HarmonyOS/HMS HTTP backend using `RemoteCommunicationKit/rcp.h`.
  - `platform/ohos/src/egl_window_backend.*`: EGL/GLES window backend for `OHNativeWindow*` XComponent surfaces.
  - `platform/ohos/src/renderer_frontend.*` and `platform/ohos/src/map_view.*`: native controller scaffold that owns the backend, renderer frontend, and `mbgl::Map`.
  - `platform/ohos/src/napi_xcomponent_smoke.cpp`: compile/link smoke NAPI module for XComponent surface registration, style loading, run-loop pumping, and manual frame rendering.
- Added maintainer-facing integration helpers:
  - `platform/ohos/README.md`: current build commands, status, and Harmony project integration notes.
  - `platform/ohos/arkts/types/libmaplibre_native_ohos/`: `.d.ts` and `oh-package.json5` metadata that can be copied into an app module's `src/main/cpp/types/` directory.
- Moved the C++20 compatibility work out of the tile-spec submodule:
  - OHOS builds pass `-fexperimental-library` for `mbgl-core`, `mlt-cpp`, and the NAPI smoke target, which enables the SDK libc++'s guarded ranges algorithms.
  - The temporary local `std::ranges` compatibility shim was removed.
  - `vendor/maplibre-tile-spec` is clean again.
- Replaced `std::source_location` usage in the symbol guard diagnostics with internal `mbgl::SourceLocation`.
  - This preserves file/function/line reporting for the guard checks.
  - It avoids defining a fallback type inside `namespace std` for SDKs such as OHOS API 21 that do not ship `<source_location>`.

### Verified builds

- Reconfigured and built core-only:

  ```sh
  env OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native \
    pixi run cmake --preset ohos-opengl-core
  pixi run cmake --build --preset ohos-opengl-core
  ```

- Reconfigured and built the OpenHarmony platform target:

  ```sh
  env OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native \
    pixi run cmake --preset ohos-opengl
  pixi run cmake --build --preset ohos-opengl
  ```

- Reconfigured and built the HarmonyOS/HMS RCP target:

  ```sh
  env OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native \
    HMOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/hms/native \
    pixi run cmake --preset harmonyos-opengl
  pixi run cmake --build --preset harmonyos-opengl -j 8
  ```

- Reconfigured and built the OpenHarmony final-link smoke target:

  ```sh
  env OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native \
    pixi run cmake --preset ohos-opengl-link-smoke
  pixi run cmake --build --preset ohos-opengl-link-smoke -j 8
  ```

- Reconfigured and built the HarmonyOS/HMS final-link smoke target:

  ```sh
  env OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native \
    HMOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/hms/native \
    pixi run cmake --preset harmonyos-opengl-link-smoke
  pixi run cmake --build --preset harmonyos-opengl-link-smoke -j 8
  ```

- Reconfigured and built the OpenHarmony NAPI/XComponent smoke target:

  ```sh
  env OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native \
    pixi run cmake --preset ohos-opengl-napi-smoke
  pixi run cmake --build --preset ohos-opengl-napi-smoke -j 8
  ```

- Reconfigured and built the HarmonyOS/HMS NAPI/XComponent smoke target:

  ```sh
  env OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native \
    HMOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/hms/native \
    pixi run cmake --preset harmonyos-opengl-napi-smoke
  pixi run cmake --build --preset harmonyos-opengl-napi-smoke -j 8
  ```

- The public OpenHarmony presets currently build `mbgl-core`.
- The current full platform output is `build-ohos/libmbgl-core.a`.
- The current HMS/RCP output is `build-harmonyos/libmbgl-core.a`.
- The link smoke outputs are:
  - `build-ohos-link-smoke/libmbgl-ohos-link-smoke.so`
  - `build-harmonyos-link-smoke/libmbgl-ohos-link-smoke.so`
- The NAPI/XComponent smoke target is named `mbgl-ohos-napi-smoke`, but its shared-library output name is aligned with the NAPI module name:
  - `build-ohos-napi-smoke/libmaplibre_native_ohos.so`
  - `build-harmonyos-napi-smoke/libmaplibre_native_ohos.so`
- After adding the native `MapView`/`RendererFrontend` scaffold, the OpenHarmony and HarmonyOS/HMS NAPI smoke targets were reconfigured and rebuilt successfully. The non-NAPI platform and final-link smoke presets were also reconfigured/rebuilt successfully.
- The only recurring compiler noise is the SDK toolchain warning that `--gcc-toolchain=.../llvm` is unused. This is non-fatal with `MLN_WITH_WERROR=OFF`.

### NAPI/XComponent findings

- The command-line SDK includes the native app surface pieces needed for compile/link iteration:
  - headers: `napi/native_api.h`, `ace/xcomponent/native_interface_xcomponent.h`, `arkui/native_node_napi.h`, `native_window/external_window.h`
  - libraries: `libace_napi.z.so`, `libace_ndk.z.so`, `libnative_window.so`
- The NAPI smoke module compile-tests both exposed XComponent shapes found in the SDK:
  - Legacy exported `OH_NATIVE_XCOMPONENT_OBJ` registration with `OH_NativeXComponent_RegisterCallback`.
  - ArkUI node-to-native conversion with `OH_ArkUI_GetNodeHandleFromNapiValue`, `OH_ArkUI_SurfaceHolder_Create`, surface callbacks, and `OH_ArkUI_XComponent_GetNativeWindow`.
- Added an OHOS EGL window backend scaffold for XComponent surfaces:
  - `platform/ohos/src/egl_window_backend.*` derives from the repo's GL `RendererBackend` and `gfx::Renderable` interfaces, following the GLFW/default renderer-backend shape rather than the Android GL table/path.
  - It creates an ES 3 EGL context and `eglCreateWindowSurface` directly from `OHNativeWindow*`, binds framebuffer 0, tracks viewport size, swaps with `eglSwapBuffers`, and references/unreferences the native window with `OH_NativeWindow_NativeObjectReference/Unreference`.
  - The NAPI smoke binding now creates/destroys this backend from ArkUI XComponent surface callbacks once a native window and nonzero surface size are available.
- Added an initial native MapLibre controller path:
  - `platform/ohos/src/renderer_frontend.*` implements MapLibre's `RendererFrontend` contract around the OHOS EGL backend and renders the latest `UpdateParameters` on demand.
  - `platform/ohos/src/map_view.*` owns `EGLWindowBackend`, `RendererFrontend`, and `mbgl::Map`, applies surface size changes, supports style URL/JSON loading, pumps `RunLoop::runOnce()`, and renders one frame when asked.
  - The NAPI smoke exports `registerXComponentNode(node)`, `setStyleUrl(binding, url)`, `setStyleJson(binding, json)`, `renderFrame(binding)`, and `runLoopOnce()`.
  - This proves the native ownership chain `ArkUI XComponent -> OHNativeWindow -> EGLWindowBackend -> RendererFrontend -> mbgl::Map` compiles and links against both the public OpenHarmony SDK and the HarmonyOS/HMS SDK.
- The legacy `libraryname` XComponent path now uses the same `MapView` controller:
  - `OH_NATIVE_XCOMPONENT_OBJ` is unwrapped during module initialization.
  - `OnSurfaceCreated`/`OnSurfaceChanged` query `OH_NativeXComponent_GetXComponentSize`, bind the `OHNativeWindow*`, and create/update `MapView`.
  - `setStyleUrl`, `setStyleJson`, and `renderFrame` work either with an explicit external binding from `registerXComponentNode(node)` or as methods on a legacy XComponent context object.
  - Both the legacy component path and ArkUI node path register the SDK's on-frame callbacks when available and render from those callbacks.
  - The ArkUI node path now rejects duplicate `registerXComponentNode(node)` calls and only erases `nodeBindings` during finalization when the finalized binding still owns that node entry.
- SDK quirk: `EGL/eglplatform.h` only typedefs `EGLNativeWindowType` as `struct NativeWindow*` when `OHOS_PLATFORM` is defined. The command-line toolchain did not define it by default, so `platform/ohos/ohos.cmake` now adds `OHOS_PLATFORM` for OHOS core and the NAPI smoke target.
- The NAPI smoke target also links `mbgl-compiler-options` so it matches the repo default `-fno-rtti`; otherwise the standalone smoke object emits RTTI references to `mbgl::gl::RendererBackend` that `mbgl-core` intentionally does not provide when `MLN_WITH_RTTI=OFF`.
- This suggests the command-line tools are enough to continue native build and NAPI/XComponent linker work without the full IDE.
- The standalone CMake smoke target still does not prove package generation, ArkTS declarations, lifecycle correctness, or actual MapLibre frame rendering into an XComponent on a device/emulator. The ignored Harmony project harness below exercises packaging separately.

### HTTP findings

- The OpenHarmony public SDK provides `network/netstack/net_http.h` and `libnet_http.so`.
- Its C callback API does not include per-request user data or request identity in `Http_ResponseCallback` or the event callbacks, which makes it a poor fit for MapLibre's concurrent `HTTPFileSource` contract.
- It is still usable with a bounded callback-slot dispatcher: each in-flight request gets one of 128 slots, and the C callbacks registered for that slot dispatch back to the owning request state.
- The HarmonyOS/HMS SDK also includes `RemoteCommunicationKit/rcp.h` and `librcp_c.so`.
  - RCP callbacks do include user data and look much more suitable for a real `HTTPFileSource`.
  - RCP is in the HMS SDK, not the OpenHarmony public sysroot, so it should probably be a separate HarmonyOS-specific option rather than the default OpenHarmony platform dependency.
- The public OpenHarmony `HTTPFileSource` now compiles and links against `libnet_http.so`.
  - It handles GET requests, conditional headers, byte ranges, streamed response body fallback, response cache headers, 200/206/204/304/404/429/5xx statuses, and basic netstack error mapping.
  - It is compile/link-tested only. It has not been exercised inside an OpenHarmony app with the required `ohos.permission.INTERNET` permission.
  - Later cleanup changed cancellation lifetime so explicit cancel releases the
    callback slot and self-reference immediately instead of waiting for a
    completion/cancellation callback.
- The optional HMS/RCP `HTTPFileSource` now compiles and links into `build-harmonyos/libmbgl-core.a`.
  - It handles GET requests, conditional headers, byte ranges, response cache headers, 200/206/204/304/404/429/5xx statuses, and basic curl-derived RCP error mapping.
  - It is compile-tested only. It has not been exercised inside a HarmonyOS app with the required network permissions.
  - Later cleanup replaced the raw request-state callback context with stable
    callback slots, so explicit cancel can release request state immediately
    and late SDK callbacks cannot dereference freed state.
- The HMS RCP final link initially failed because `-lrcp_c` was not on the linker search path even though the header was visible.
  - `platform/ohos/ohos.cmake` now resolves `librcp_c.so` with `find_library`.
  - In this SDK it resolves to `/Users/sargunv/Downloads/command-line-tools/sdk/default/hms/native/sysroot/usr/lib/aarch64-linux-ohos/librcp_c.so`.
  - The HarmonyOS link smoke target now verifies that absolute library path in a final shared-library link.

### DevEco 6 compiler check

- The HMS native toolchain is present at `/Users/sargunv/Downloads/command-line-tools/sdk/default/hms/native`.
- HMS `aarch64-unknown-linux-ohos-clang++ --version` reports:
  - `OHOS (BiSheng Mobile STD 203.2.0.B048) clang version 15.0.4`
- A direct C++20 probe with the HMS compiler still fails for:
  - `std::ranges::find`
  - `<source_location>`
- So DevEco 6 command-line tools do not expose the needed libc++ ranges algorithms by default and still do not provide `<source_location>`.
- The OpenHarmony SDK clang accepts `-fexperimental-library`, which enables the bundled libc++ ranges algorithms; the direct OpenHarmony and HarmonyOS/HMS CMake builds now use that flag instead of a local ranges shim.

### Command-line packaging check

- `ohpm --help` exposes package-management commands and `ohpm init`, but no full application template generation.
- `hvigorw --help` exposes build/task execution commands, but no project creation template in this command-line-tools install.
- A shallow search under `/Users/sargunv/Downloads/command-line-tools` found the tool packages and SDK metadata, but no sample/template app skeleton.
- Local side effect: `hvigorw --help` recreated `.hvigor/` in the repo root; it was removed again after the probe.
- Conclusion: the command-line tools are enough for native CMake, final-link work, and packaging when an existing HarmonyOS project skeleton is available. They still do not appear to be enough by themselves to generate a clean first app shell without either an existing skeleton or the full IDE/template payload.

### 2026-05-30 issue refresh

- `gh issue view 3749 --repo maplibre/maplibre-native` reports issue #3749 is still open, unassigned, unlabeled, and last updated on `2025-11-12T01:16:25Z`.
- GitHub's issue page still shows no linked development branch or pull request.

### Harmony project harness check

- Cloned the public XComponent codelab into ignored path `build-ohos-xcomponent-sample` for an app-layout harness.
  - Repo: `https://gitee.com/harmonyos_codelabs/XComponent.git`
  - Checked commit: `8dde2bebe73c17475c5c99f9bd855b5adf792523`
- The codelab targets HarmonyOS SDK `6.0.2(22)` and fails immediately against the installed SDK `6.0.1/API 21` with: `Unable to find the targetSdkVersion 6.0.2(22) in SDK Manager`.
- In the ignored clone only, retargeted `build-profile.json5` to `compatibleSdkVersion` and `targetSdkVersion` `6.0.1(21)`.
  - After that change, `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon` builds and packages the codelab successfully.
- In the ignored clone only, added the local MapLibre Native tree as a CMake subdirectory:
  - `-DMAPLIBRE_NATIVE_ROOT=/Users/sargunv/Code/maplibre-native`
  - `-DMLN_WITH_GLFW=OFF`
  - `-DMLN_WITH_WERROR=OFF`
  - `-DMLN_WITH_OPENGL=ON`
  - `-DMLN_WITH_EGL=ON`
  - `-DMLN_OHOS_BUILD_NAPI_SMOKE=ON`
- `hvigorw assembleApp --no-daemon` then built MapLibre Native inside the Harmony project and packaged the app successfully.
  - First full MapLibre native build inside hvigor took about `1 min 31 s`.
  - The packaged HAP was `build-ohos-xcomponent-sample/entry/build/default/outputs/default/entry-default-unsigned.hap`.
  - The HAP contains `libs/arm64-v8a/libmaplibre_native_ohos.so` and `libs/x86_64/libmaplibre_native_ohos.so`.
- In the ignored clone only, changed the sample XComponent `libraryname` to `maplibre_native_ohos`, imported `libmaplibre_native_ohos.so`, added a minimal `.d.ts`, and called the smoke module's legacy-context `setStyleJson`/`renderFrame` methods from ArkTS.
  - A clean `hvigorw clean --no-daemon && hvigorw assembleApp --no-daemon` still succeeds.
  - The clean HAP contains `libmaplibre_native_ohos.so` for `arm64-v8a` and `x86_64`, and no stale `libmbgl-ohos-napi-smoke.so`.
  - This proves the command-line tools can compile ArkTS, native CMake/Ninja, and package a HAP that points an XComponent at the MapLibre Native OHOS smoke module.
- The HAP remains unsigned because the codelab has no signing config.
- The SDK still warns that imported NAPI modules are not verified even with local type declarations. This is a warning only in the current command-line build.
- Added the same type metadata shape to the reviewable tree under `platform/ohos/arkts/types/libmaplibre_native_ohos/`.
- Added `platform/ohos/README.md` with the CMake preset commands and Harmony project integration outline.
- Re-running the direct OpenHarmony and HarmonyOS/HMS NAPI smoke builds after adding those docs/types was a no-op (`ninja: no work to do`), as expected.
- Copied the reviewable `platform/ohos/arkts/types/libmaplibre_native_ohos/` metadata into the ignored harness and rebuilt with `hvigorw assembleApp --no-daemon`.
  - The ArkTS compiler accepted this exact `.d.ts` shape.
  - The SDK still emits the same NAPI module verification warnings, but the build succeeds.
- After adding the duplicate-node guard/finalizer ownership check in `napi_xcomponent_smoke.cpp`:
  - `pixi run cmake --build --preset ohos-opengl-napi-smoke -j 8` rebuilt and linked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-napi-smoke -j 8` rebuilt and linked `libmaplibre_native_ohos.so`.
  - `hvigorw assembleApp --no-daemon` in the ignored harness rebuilt native code and packaged successfully.
  - The HAP contains `libs/arm64-v8a/libmaplibre_native_ohos.so` and `libs/x86_64/libmaplibre_native_ohos.so`.
- After replacing the local ranges shim with `-fexperimental-library`:
  - `env OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native pixi run cmake --build --preset ohos-opengl-napi-smoke -j 8` rebuilt and linked `libmaplibre_native_ohos.so`.
  - `env OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native HMOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/hms/native pixi run cmake --build --preset harmonyos-opengl-napi-smoke -j 8` rebuilt and linked `libmaplibre_native_ohos.so`.
  - Both builds still emit the SDK's known unused `--gcc-toolchain=.../llvm` clang warning.
- After replacing the public OpenHarmony HTTP placeholder with the `net_http` backend:
  - `env OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native pixi run cmake --build --preset ohos-opengl-napi-smoke -j 8` rebuilt `platform/ohos/src/http_file_source.cpp` and linked `libmaplibre_native_ohos.so`.
  - `env OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native HMOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/hms/native pixi run cmake --build --preset harmonyos-opengl-napi-smoke -j 8` reconfigured successfully; no HMS/RCP rebuild was needed.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon` in the ignored XComponent harness rebuilt native code and packaged successfully in `1 min 18 s`.
  - The resulting unsigned HAP contains `libs/arm64-v8a/libmaplibre_native_ohos.so` and `libs/x86_64/libmaplibre_native_ohos.so`.
  - `llvm-readelf -d` on both packaged ABI outputs shows `NEEDED` dependency `libnet_http.so`, confirming the public HTTP backend is included in the harness build.
- Added `ohos.permission.INTERNET` to the ignored XComponent harness `entry/src/main/module.json5` and rebuilt with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The rebuild succeeded in `3 s`.
  - `unzip -p entry-default-unsigned.hap module.json` shows `requestPermissions` includes `ohos.permission.INTERNET`.
  - The HAP still contains `libs/arm64-v8a/libmaplibre_native_ohos.so` and `libs/x86_64/libmaplibre_native_ohos.so`.

### 2026-05-30 direct build matrix refresh

- After gating `-fexperimental-library` to C++ compile units and after replacing the public HTTP placeholder with the `net_http` backend, all direct CMake build presets were rebuilt successfully:
  - `ohos-opengl-core`
  - `ohos-opengl`
  - `ohos-opengl-link-smoke`
  - `ohos-opengl-napi-smoke`
  - `harmonyos-opengl`
  - `harmonyos-opengl-link-smoke`
  - `harmonyos-opengl-napi-smoke`
- The OpenHarmony default builds use the public SDK `net_http` backend and link against `libnet_http.so`.
- The HarmonyOS/HMS builds use the optional RCP backend and link against the resolved HMS `librcp_c.so`.
- The final-link smoke outputs still build:
  - `build-ohos-link-smoke/libmbgl-ohos-link-smoke.so`
  - `build-harmonyos-link-smoke/libmbgl-ohos-link-smoke.so`
- The NAPI/XComponent smoke outputs still build:
  - `build-ohos-napi-smoke/libmaplibre_native_ohos.so`
  - `build-harmonyos-napi-smoke/libmaplibre_native_ohos.so`
- Hygiene after the matrix refresh:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "ranges_compat" . --glob '!build-*' --glob '!.git' --glob '!ohos-research.md'` found no remaining references.
- Recurring non-fatal warnings remain unchanged:
  - SDK toolchain CMake files use an old `cmake_minimum_required` compatibility floor.
  - Clang warns that the toolchain-provided `--gcc-toolchain=.../llvm` argument is unused.

### 2026-05-30 HMS/RCP cancellation cleanup

- Tightened the optional HMS/RCP `HTTPFileSource` request lifetime:
  - `cancel()` no longer calls `HMS_Rcp_CancelRequest` while holding the request mutex.
  - Completion/cancellation paths detach and destroy `Rcp_Request` outside the request mutex.
  - This avoids a possible deadlock if the HMS SDK reports cancellation synchronously from `HMS_Rcp_CancelRequest`.
- Rebuilt the HMS/RCP presets that cover this source:
  - `harmonyos-opengl`
  - `harmonyos-opengl-link-smoke`
  - `harmonyos-opengl-napi-smoke`
- The same known non-fatal clang warning remains: `--gcc-toolchain=.../llvm` is unused.

### 2026-05-30 portability and packaging refresh

- Guarded the new `__has_include` check in `platform/linux/src/gl_functions.cpp` with `defined(__has_include)`.
- `src/mbgl/layout/symbol_instance.hpp` briefly used the same guard while it still depended on `<source_location>`; the later source-location cleanup below removed that dependency entirely.
- Rebuilt the NAPI/XComponent smoke presets that exercise both touched headers/sources:
  - `ohos-opengl-napi-smoke`
  - `harmonyos-opengl-napi-smoke`
- Attempted a host Linux build with `pixi run cmake --build --preset linux-opengl-core -j 8`, but `build-linux-opengl` is not configured in this worktree, so that host-side check was not run.
- Checked for an attached OHOS target with:

  ```sh
  /Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets
  ```

  The output was `[Empty]`, so no device/emulator runtime install or launch was possible from this machine state.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `30 s`.
  - The unsigned HAP still contains `libs/arm64-v8a/libmaplibre_native_ohos.so` and `libs/x86_64/libmaplibre_native_ohos.so`.
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
- Hygiene after these changes:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.

### 2026-05-30 source location cleanup

- Replaced the temporary `std::source_location` compatibility shim with `src/mbgl/util/source_location.hpp`.
  - The new `mbgl::SourceLocation` is a small internal file/function/line carrier used by symbol guard diagnostics.
  - `SYM_GUARD_LOC` now expands to `::mbgl::SourceLocation{__FILE__, __func__, __LINE__}` when symbol guards are enabled.
  - This removes the need to include `<source_location>` or inject a fallback type into `namespace std`.
- Updated symbol guard signatures in:
  - `src/mbgl/layout/symbol_instance.hpp`
  - `src/mbgl/layout/symbol_instance.cpp`
  - `src/mbgl/renderer/bucket.hpp`
  - `src/mbgl/renderer/buckets/symbol_bucket.hpp`
  - `src/mbgl/renderer/buckets/symbol_bucket.cpp`
- Rebuilt the targets that exercise the affected layout/renderer code through the OHOS toolchains:
  - `ohos-opengl-napi-smoke`
  - `harmonyos-opengl-napi-smoke`
- Tried to configure a host Linux core check with `pixi run cmake --preset linux-opengl-core`, but this repo state has no configure preset by that name. The earlier `pixi run cmake --build --preset linux-opengl-core -j 8` failure was the same missing preset issue, not a compile failure.
- `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `29 s`.
  - The unsigned HAP still contains `libs/arm64-v8a/libmaplibre_native_ohos.so` and `libs/x86_64/libmaplibre_native_ohos.so`.
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.

### 2026-05-30 final smoke and host-build refresh

- Refreshed `maplibre/maplibre-native#3749` and searched current PRs/issues for `openharmony`, `harmonyos`, and `ohos`.
  - The issue is still the only open tracked item found for this work.
  - No matching PRs were found.
- Confirmed `platform/ohos/ohos.cmake` now keeps the OHOS `mbgl-core` links to `EGL` and `GLESv3` under `MLN_WITH_OPENGL`, matching the existing OpenGL source guard.
- Rechecked the NAPI/XComponent smoke presets after the source-location cleanup:
  - `ohos-opengl-napi-smoke`: no rebuild needed.
  - `harmonyos-opengl-napi-smoke`: no rebuild needed.
- Rebuilt the final-link smoke presets after CMake regenerated their build files:
  - `ohos-opengl-link-smoke` rebuilt `mbgl-core` and linked `libmbgl-ohos-link-smoke.so`.
  - `harmonyos-opengl-link-smoke` rebuilt `mbgl-core` and linked `libmbgl-ohos-link-smoke.so`.
  - Both builds still emit the SDK's known unused `--gcc-toolchain=.../llvm` clang warning.
- Tried a macOS host-side core build to make sure the source-location cleanup did not leave a host-only compile issue:
  - `pixi run cmake --list-presets` shows no visible `linux-opengl-core` configure preset on macOS, only a build preset.
  - `pixi run cmake --preset linux-opengl-core` fails because that configure preset is not present.
  - `pixi run cmake --build --preset linux-opengl-core -j 8` fails because `build-linux-opengl` has not been configured.
  - `pixi run cmake --preset macos-metal` configured successfully.
  - `pixi run cmake --build --preset macos-core -j 8` fails because the build preset references the hidden `macos` configure preset.
  - Reconfiguring `build-macos-metal` with `pixi run cmake -S . -B build-macos-metal -DCMAKE_CXX_COMPILER_LAUNCHER=` clears the local missing `ccache` launcher issue.
  - `pixi run cmake --build build-macos-metal --target mbgl-core -j 8` then fails in vendored ICU while compiling `vendor/icu/src/cmemory.cpp`, before reaching the touched source-location code. The errors are around `TRUE`, `FALSE`, and `U_NOEXCEPT` in the vendored ICU headers with the current pixi macOS toolchain.
- Final hygiene for the current tree:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 XComponent style lifecycle cleanup

- Checked the API 21 public `net_http` headers again:
  - `OH_Http_Request` still accepts only `Http_ResponseCallback` plus `Http_EventsHandler`.
  - `Http_ResponseCallback`, `Http_OnDataReceiveCallback`, `Http_OnHeaderReceiveCallback`, `Http_OnProgressCallback`, and `Http_OnVoidCallback` still have no user-data/context parameter.
  - The callback-slot table in the public OpenHarmony `HTTPFileSource` remains necessary unless the backend switches away from `net_http`.
- Tightened the NAPI/XComponent smoke wrapper's style lifecycle:
  - `SurfaceBinding` now tracks desired style kind, style generation, and applied style generation.
  - The wrapper reapplies the desired style when a new `mbgl::Map` is created for a recreated surface.
  - Resizing or otherwise updating the same native surface no longer reloads the same style repeatedly.
  - `MapView` now exposes the current `OHNativeWindow*` so the wrapper can detect when `setSurface` will recreate the map.
- Rebuilt the NAPI/XComponent smoke targets that cover this code:
  - `ohos-opengl-napi-smoke` rebuilt `map_view.cpp` and `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-napi-smoke` rebuilt the same files and linked `libmaplibre_native_ohos.so`.
  - Both builds still emit the SDK's known unused `--gcc-toolchain=.../llvm` clang warning.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 674 ms`.
  - The unsigned HAP still contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - The HAP/app remain unsigned because the ignored harness has no signing config.
- Checked for an attached OHOS target again with `hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 basic camera control

- Added a minimal `jumpTo(longitude, latitude, zoom?)` NAPI export to the XComponent smoke module.
  - The legacy XComponent context path can call `map.jumpTo(lon, lat, zoom)`.
  - The node-handle path can call `maplibreNative.jumpTo(binding, lon, lat, zoom)`.
  - The call stores desired camera state in `MapView`, so it can be made before a surface exists.
  - `MapView::onDidFinishLoadingStyle()` reapplies the desired camera after style load, so style default camera metadata does not permanently override the requested smoke camera.
- Updated the reviewable ArkTS type metadata in `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts`.
- Updated `platform/ohos/README.md` to show the new smoke `jumpTo` call.
- Synced the ignored XComponent codelab harness to call `jumpTo(0, 0, 2)` and copied the updated local type metadata into the harness.
- Rebuilt the NAPI/XComponent smoke targets that cover this code:
  - `ohos-opengl-napi-smoke` rebuilt `map_view.cpp` and `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-napi-smoke` rebuilt the same files and linked `libmaplibre_native_ohos.so`.
  - Both builds still emit the SDK's known unused `--gcc-toolchain=.../llvm` clang warning.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 631 ms`.
  - ArkTS accepted the new `jumpTo` call; the same existing NAPI module verification warnings remain.
  - The unsigned HAP still contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
- Checked for an attached OHOS target again with `hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 pixel ratio control

- Added `setPixelRatio(pixelRatio)` to the XComponent smoke module.
  - The legacy XComponent context path can call `map.setPixelRatio(pixelRatio)`.
  - The node-handle path can call `maplibreNative.setPixelRatio(binding, pixelRatio)`.
  - The value is validated as finite and positive.
  - `MapView` stores the pixel ratio and uses it when creating `RendererFrontend` and `MapOptions`.
  - If pixel ratio changes while a native surface is active, the smoke view recreates the `mbgl::Map` against the same `OHNativeWindow*`, then reapplies desired style/camera state.
- Updated the reviewable ArkTS type metadata in `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts`.
- Updated `platform/ohos/README.md` to show `setPixelRatio(2)` in the smoke setup.
- Synced the ignored XComponent codelab harness to call `setPixelRatio(2)` before `setStyleJson(...)` and copied the updated local type metadata into the harness.
- Rebuilt the NAPI/XComponent smoke targets that cover this code:
  - `ohos-opengl-napi-smoke` rebuilt `map_view.cpp` and `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-napi-smoke` rebuilt the same files and linked `libmaplibre_native_ohos.so`.
  - Both builds still emit the SDK's known unused `--gcc-toolchain=.../llvm` clang warning.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 476 ms`.
  - ArkTS accepted the `setPixelRatio`, `setStyleJson`, and `jumpTo` smoke setup; the same existing NAPI module verification warnings remain.
  - The unsigned HAP still contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
- Checked for an attached OHOS target again with `hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 camera bearing and pitch

- Extended the smoke `jumpTo` bridge to accept optional bearing and pitch:
  - Legacy XComponent context: `map.jumpTo(longitude, latitude, zoom?, bearing?, pitch?)`.
  - Node-handle path: `maplibreNative.jumpTo(binding, longitude, latitude, zoom?, bearing?, pitch?)`.
- `MapView` now stores desired camera state with center, zoom, bearing, and pitch, and continues to reapply it after map recreation and style load.
- Updated the reviewable ArkTS type metadata in `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts`.
- Updated `platform/ohos/README.md` and the ignored XComponent codelab harness to call `jumpTo(0, 0, 2, 15, 30)`.
- Rebuilt the NAPI/XComponent smoke targets that cover this code:
  - `ohos-opengl-napi-smoke` rebuilt `map_view.cpp` and `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-napi-smoke` rebuilt the same files and linked `libmaplibre_native_ohos.so`.
  - Both builds still emit the SDK's known unused `--gcc-toolchain=.../llvm` clang warning.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 578 ms`.
  - ArkTS accepted the expanded `jumpTo` signature; the same existing NAPI module verification warnings remain.
  - The unsigned HAP still contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
- Checked for an attached OHOS target again with `hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 legacy XComponent touch pan

- Added a minimal one-finger pan path to the legacy `libraryname` XComponent smoke module.
  - The legacy touch callback now reads `OH_NativeXComponent_TouchEvent`.
  - `DOWN` marks a gesture active and stores the active touch id/position.
  - `MOVE` ignores multi-touch and calls `MapView::moveBy(dx, dy)` for the active touch.
  - `UP`/`CANCEL` clears gesture state and ends `mbgl::Map::setGestureInProgress`.
- Added `MapView::setGestureInProgress(bool)` and `MapView::moveBy(double, double)` helpers.
  - `moveBy` persists the current camera through `map->getCameraOptions()` so a later surface recreation can reapply the latest panned camera.
- This is intentionally only a smoke-level gesture bridge:
  - The ArkUI node-handle XComponent path does not yet register touch input.
  - Pinch, rotate, fling/inertia, double-tap, pitch, and full gesture arbitration are still not implemented.
- Updated `platform/ohos/README.md` to document the current input boundary.
- Rebuilt the NAPI/XComponent smoke targets that cover this code:
  - `ohos-opengl-napi-smoke` rebuilt `map_view.cpp` and `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-napi-smoke` rebuilt the same files and linked `libmaplibre_native_ohos.so`.
  - Both builds still emit the SDK's known unused `--gcc-toolchain=.../llvm` clang warning.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `943 ms`.
  - The unsigned HAP still contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
- Checked for an attached OHOS target again with `hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 ArkUI node-handle touch pan

- Extended the one-finger pan bridge to the ArkUI node-handle XComponent path.
  - The node path now includes `arkui/native_node.h` and `arkui/ui_input_event.h`.
  - `registerXComponentNode(node)` registers `NODE_ON_TOUCH_INTERCEPT` with `OH_ArkUI_NativeModule_RegisterCommonEvent`.
  - The callback reads `ArkUI_UIInputEvent`, maps touch down/move/up/cancel to the same internal pan handler as the legacy XComponent path, and sets `HTM_BLOCK` for the intercepted pointer event.
- Tightened node binding cleanup:
  - The binding now tracks whether the frame callback and touch callback were registered.
  - Finalization unregisters only the callbacks that were actually registered.
  - `nodeBindings` remains the ownership/duplicate-registration map even if frame callback registration fails.
- Rebuilt the NAPI/XComponent smoke targets that cover this code:
  - `ohos-opengl-napi-smoke` rebuilt `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-napi-smoke` rebuilt the same file and linked `libmaplibre_native_ohos.so`.
  - Both builds still emit the SDK's known unused `--gcc-toolchain=.../llvm` clang warning.
- Updated `platform/ohos/README.md` to say both XComponent paths now have compile-tested one-finger touch pan.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 156 ms`.
  - Packaging emitted Java warnings from `app_packing_tool.jar` about a terminally deprecated `sun.misc.Unsafe::arrayBaseOffset` call, but the build completed.
  - The unsigned HAP still contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
- Checked for an attached OHOS target again with `hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 explicit node binding destroy

- Added explicit lifetime control for the ArkUI node-handle XComponent binding.
  - `registerXComponentNode(node)` now returns an opaque external handle that owns a `SurfaceBinding*`.
  - The external finalizer still releases the binding if ArkTS drops the handle.
  - New `destroy(binding)` releases the `SurfaceBinding` deterministically, unregistering node touch/frame callbacks and surface callbacks, then nulling the external handle to avoid a later double free.
  - Calling other binding-based exports with a destroyed handle now reports `Native XComponent binding has been destroyed`.
- Updated the reviewable ArkTS type metadata in `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts`.
- Updated `platform/ohos/README.md` to tell node-handle users to call `destroy(binding)` when the ArkTS owner is disposed.
- Synced the ignored XComponent codelab harness type metadata from the committed `platform/ohos/arkts/types/...` file.
- Rebuilt the NAPI/XComponent smoke targets that cover this code:
  - `ohos-opengl-napi-smoke` rebuilt `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-napi-smoke` rebuilt the same file and linked `libmaplibre_native_ohos.so`.
  - Both builds still emit the SDK's known unused `--gcc-toolchain=.../llvm` clang warning.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 599 ms`.
  - ArkTS emitted the same existing NAPI module verification warnings for `libnativerender.so` and `libmaplibre_native_ohos.so`.
  - Packaging emitted the same Java warnings from `app_packing_tool.jar` about a terminally deprecated `sun.misc.Unsafe::arrayBaseOffset` call, but the build completed.
  - The unsigned HAP still contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
- Checked for an attached OHOS target again with `hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 destroyed handle error path cleanup

- Audited the binding-based NAPI exports after adding `destroy(binding)`.
- Fixed a dispatch bug where a destroyed external binding could raise `Native XComponent binding has been destroyed`, then continue falling through to the legacy XComponent context lookup.
  - Added a small `hasPendingException(env)` helper around `napi_is_exception_pending`.
  - `renderFrame`, `setStyleUrl`, `setStyleJson`, `jumpTo`, and `setPixelRatio` now stop immediately when `getExternalBindingIfPresent` has raised a NAPI exception.
  - This preserves the more specific destroyed-handle error instead of replacing it with the generic `Expected a native XComponent binding handle or context`.
- Rebuilt the NAPI/XComponent smoke targets that cover this code:
  - `ohos-opengl-napi-smoke` rebuilt `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-napi-smoke` rebuilt the same file and linked `libmaplibre_native_ohos.so`.
  - Both builds still emit the SDK's known unused `--gcc-toolchain=.../llvm` clang warning.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 123 ms`.
  - Packaging emitted the same Java warnings from `app_packing_tool.jar` about a terminally deprecated `sun.misc.Unsafe::arrayBaseOffset` call, but the build completed.
  - The unsigned HAP still contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
- Checked for an attached OHOS target again with `hdc list targets`; the output is still `[Empty]`.

### 2026-05-30 OHOS HTTP user agent audit

- Audited the experimental OHOS HTTP backends after noticing they still had the same hardcoded `MapLibreNative/1.0` user agent as the Linux cURL default.
- Updated both backend implementations to use the platform `ClientOptions` stored on `HTTPFileSource::Impl` at request time.
  - If the embedding app supplies a client name/version, the request now prefixes the MapLibre token with that client identity.
  - The fallback token now includes `mbgl::version::revision` as `MapLibreNative/0.0.0 (<revision>; OpenHarmony)`.
  - The OpenHarmony `net_http` path and optional HarmonyOS HMS RCP path use the same formatting.
- Rebuilt the link-smoke targets that cover the two HTTP implementations:
  - `ohos-opengl-link-smoke` rebuilt `platform/ohos/src/http_file_source.cpp`, then linked `libmbgl-ohos-link-smoke.so`.
  - `harmonyos-opengl-link-smoke` rebuilt `platform/ohos/src/http_file_source_hms_rcp.cpp`, then linked `libmbgl-ohos-link-smoke.so`.
- Rebuilt the NAPI/XComponent smoke targets that package this core code:
  - `ohos-opengl-napi-smoke` rebuilt the OpenHarmony HTTP backend and linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-napi-smoke` rebuilt the HMS RCP backend and linked `libmaplibre_native_ohos.so`.
  - Both builds still emit the SDK's known unused `--gcc-toolchain=.../llvm` clang warning.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 258 ms`.
  - Packaging emitted the same Java warnings from `app_packing_tool.jar` about a terminally deprecated `sun.misc.Unsafe::arrayBaseOffset` call, and the same unsigned-output warning due to missing signing config.
- Checked for an attached OHOS target with `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`; the output is still `[Empty]`.
- Hygiene after this audit:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 shared OHOS HTTP user-agent helper

- Reduced duplication between the OpenHarmony `net_http` backend and optional HarmonyOS HMS RCP backend.
  - Added `platform/ohos/src/http_user_agent.hpp` as a small OHOS-private helper for `ClientOptions`-based request identity.
  - Both `platform/ohos/src/http_file_source.cpp` and `platform/ohos/src/http_file_source_hms_rcp.cpp` now call `ohos::buildUserAgent(...)` instead of carrying separate local copies of the same formatting logic.
- Rebuilt the affected native smoke targets:
  - `ohos-opengl-link-smoke`
  - `harmonyos-opengl-link-smoke`
  - `ohos-opengl-napi-smoke`
  - `harmonyos-opengl-napi-smoke`
  - All four rebuilt the relevant HTTP backend and linked successfully; all still emit only the SDK's known unused `--gcc-toolchain=.../llvm` clang warning.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 277 ms`.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - Packaging emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` warning and unsigned-output warning as before.
- Checked for an attached OHOS target with `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 XComponent pinch zoom compile path

- Added compile-tested two-finger pinch zoom support to the NAPI/XComponent smoke integration.
  - `MapView` now exposes `scaleBy(scale, anchorX, anchorY)` as a small bridge to `mbgl::Map::scaleBy`.
  - The shared touch handler now computes a two-pointer center/distance for both legacy `OH_NativeXComponent_TouchEvent` and ArkUI node-handle `ArkUI_UIInputEvent` paths.
  - Multi-touch move events now scale around the current two-finger center and apply center drift with `moveBy`.
  - One-finger pan behavior is unchanged. Rotation, velocity/inertia, and polished gesture arbitration are still not implemented.
- Updated `platform/ohos/README.md` so the XComponent caveat mentions compile-tested pinch zoom instead of saying pinch is fully open.
- Rebuilt the NAPI/XComponent smoke targets that cover the gesture code:
  - `ohos-opengl-napi-smoke`
  - `harmonyos-opengl-napi-smoke`
  - Both rebuilt `map_view.cpp` and `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
  - After the README/include-order cleanup, both rebuilt `napi_xcomponent_smoke.cpp` and linked again successfully.
- Rechecked the link-smoke targets:
  - `ohos-opengl-link-smoke`
  - `harmonyos-opengl-link-smoke`
  - Ninja reported no work to do, as expected because this change is NAPI/XComponent-only.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 28 ms`.
  - After the README/include-order cleanup, a second harness build succeeded in `2 s 99 ms`.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - Packaging emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` warning and unsigned-output warning as before.
- Checked for an attached OHOS target with `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 XComponent debug-options API

- Added a compile-tested debug overlay API to the NAPI/XComponent smoke integration for on-device bring-up.
  - `MapView` now stores `MapDebugOptions`, applies it to newly created maps, and exposes `setDebugOptions(...)`/`getDebugOptions()`.
  - The NAPI module now exports runtime `DebugOptions` constants matching the core `MapDebugOptions` bit values.
  - Added `setDebugOptions(binding?, options)` and `getDebugOptions(binding?)`.
  - Invalid option bits are rejected instead of being cast through to core.
- Updated `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts` and `platform/ohos/README.md` for the new API.
- Synced the ignored XComponent codelab harness type metadata from the committed `platform/ohos/arkts/types/.../index.d.ts`.
- Rebuilt the NAPI/XComponent smoke targets that cover the new API:
  - `ohos-opengl-napi-smoke`
  - `harmonyos-opengl-napi-smoke`
  - Both rebuilt `map_view.cpp` and `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
- Rechecked the link-smoke targets:
  - `ohos-opengl-link-smoke`
  - `harmonyos-opengl-link-smoke`
  - Ninja reported no work to do, as expected because this change is NAPI/XComponent-only.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 778 ms`.
  - ArkTS emitted the same existing NAPI module verification warnings for `libnativerender.so` and `libmaplibre_native_ohos.so`.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - Packaging emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` warning and unsigned-output warning as before.
- Checked for an attached OHOS target with `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 XComponent client-options API

- Wired the OHOS smoke integration to the `ClientOptions` path used by the HTTP backends.
  - `MapView` now stores client name/version strings and passes them into `mbgl::Map` construction.
  - Added `setClientOptions(name, version?)` to the NAPI module for both legacy XComponent context calls and ArkUI node-handle bindings.
  - If the map already has an active surface, changing client options recreates the map so future HTTP requests get the updated request identity.
  - The binding resets applied style generation after changing client options, so any previously requested style is loaded again after recreation.
- Updated `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts` and `platform/ohos/README.md` for the new API.
- Synced the ignored XComponent codelab harness type metadata from the committed `platform/ohos/arkts/types/.../index.d.ts`.
- Rebuilt the NAPI/XComponent smoke targets that cover the new API:
  - `ohos-opengl-napi-smoke`
  - `harmonyos-opengl-napi-smoke`
  - Both rebuilt `map_view.cpp` and `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
- Rechecked the link-smoke targets:
  - `ohos-opengl-link-smoke`
  - `harmonyos-opengl-link-smoke`
  - Ninja reported no work to do, as expected because this change is NAPI/XComponent-only.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 919 ms`.
  - ArkTS emitted the same existing NAPI module verification warnings for `libnativerender.so` and `libmaplibre_native_ohos.so`.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - Packaging emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` warning and unsigned-output warning as before.
- Checked for an attached OHOS target with `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 XComponent resource-options API

- Wired the OHOS smoke integration to configurable `ResourceOptions`.
  - `MapView` now stores a `ResourceOptions` object instead of constructing default options inside `createMap`.
  - Added `setResourceOptions({ apiKey?, cachePath?, assetPath? })` to the NAPI module for both legacy XComponent context calls and ArkUI node-handle bindings.
  - Unspecified fields preserve their current values; string fields present in the object update the stored options.
  - If the map already has an active surface, changing resource options recreates the map so future file-source/resource-loader construction uses the updated options.
  - The binding resets applied style generation after changing resource options, so any previously requested style is loaded again after recreation.
- Updated `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts` and `platform/ohos/README.md` for the new API.
- Synced the ignored XComponent codelab harness type metadata from the committed `platform/ohos/arkts/types/.../index.d.ts`.
- Rebuilt the NAPI/XComponent smoke targets that cover the new API:
  - `ohos-opengl-napi-smoke`
  - `harmonyos-opengl-napi-smoke`
  - Both rebuilt `map_view.cpp` and `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
- Rechecked the link-smoke targets:
  - `ohos-opengl-link-smoke`
  - `harmonyos-opengl-link-smoke`
  - Ninja reported no work to do, as expected because this change is NAPI/XComponent-only.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 375 ms`.
  - ArkTS emitted the same existing NAPI module verification warnings for `libnativerender.so` and `libmaplibre_native_ohos.so`.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - Packaging emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` warning and unsigned-output warning as before.
- Checked for an attached OHOS target with `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### 2026-05-30 XComponent camera readback API

- Added camera readback to the NAPI/XComponent smoke integration.
  - `MapView` now exposes `getCameraOptions()`, returning the live `mbgl::Map` camera when a map exists and the desired camera state otherwise.
  - The NAPI module now exports `getCameraOptions(binding?)`.
  - The returned ArkTS object includes `longitude`, `latitude`, `zoom`, `bearing`, and `pitch` when those fields are present.
  - This gives ArkTS a way to inspect state after `jumpTo`, pan, and pinch input without waiting for a full production camera API.
- Updated `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts` and `platform/ohos/README.md` for the new API.
- Synced the ignored XComponent codelab harness type metadata from the committed `platform/ohos/arkts/types/.../index.d.ts`.
- Rebuilt the NAPI/XComponent smoke targets that cover the new API:
  - `ohos-opengl-napi-smoke`
  - `harmonyos-opengl-napi-smoke`
  - Both rebuilt `map_view.cpp` and `napi_xcomponent_smoke.cpp`, then linked `libmaplibre_native_ohos.so`.
- Rechecked the link-smoke targets:
  - `ohos-opengl-link-smoke`
  - `harmonyos-opengl-link-smoke`
  - Ninja reported no work to do, as expected because this change is NAPI/XComponent-only.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `3 s 377 ms`.
  - ArkTS emitted the same existing NAPI module verification warnings for `libnativerender.so` and `libmaplibre_native_ohos.so`.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - Packaging emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` warning and unsigned-output warning as before.
- Checked for an attached OHOS target with `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`; the output is still `[Empty]`.
- Hygiene after this cleanup:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/CMake/source files found no matches.
  - `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'` found no remaining source-location dependency.

### Local side effects from harness checks

- Created ignored directory `build-ohos-xcomponent-sample/` under the repo root.
- Generated ignored build artifacts under that directory, including `.hvigor/`, `entry/build/`, and packaged unsigned HAP/app outputs.
- Created ignored direct-build directories `build-ohos-native/` and `build-harmonyos-native/` while verifying the renamed native-module presets.
- Created/reconfigured ignored host build directory `build-macos-metal/` during the host-side verification attempt.
- Modified the ignored harness CMake file at `build-ohos-xcomponent-sample/entry/src/main/cpp/CMakeLists.txt` to use `MLN_OHOS_BUILD_NATIVE_MODULE`.
- Modified the ignored harness module manifest at `build-ohos-xcomponent-sample/entry/src/main/module.json5` to include `ohos.permission.INTERNET`.
- Modified the ignored harness page at `build-ohos-xcomponent-sample/entry/src/main/ets/pages/Index.ets` to exercise the MapLibre smoke module.
- Synced the ignored harness type metadata at `build-ohos-xcomponent-sample/entry/src/main/cpp/types/libmaplibre_native_ohos/index.d.ts`.
- Running hvigor from the codelab initialized user-level hvigor wrapper state outside this project:
  - `/Users/sargunv/.hvigor/wrapper/tools/package.json`
  - `/Users/sargunv/.hvigor/wrapper/tools/package-lock.json`
  - `/Users/sargunv/.hvigor/wrapper/tools/node_modules/`
  - `/Users/sargunv/.hvigor/project_caches/`
  - Console output reported `Installing pnpm@8.13.1...` and `Pnpm install success`.

### 2026-05-30 GitHub and local status refresh

- Refreshed `maplibre/maplibre-native#3749` with `gh issue view`.
  - The issue is still open.
  - Its latest issue update is `2025-11-12T01:16:25Z`.
  - The latest comment is from `qmjy`: `I'll give it a thumbs up.`
- Searched related issue/PR state with `gh search`.
  - Related issues found: `#3749` open and `#3940` closed.
  - No matching pull requests were found for `OpenHarmony` or `HarmonyOS`.
- Rechecked the direct OpenHarmony NAPI smoke build:
  - `pixi run cmake --build --preset ohos-opengl-napi-smoke`
  - Ninja reported `no work to do`.
- Rebuilt the ignored XComponent codelab harness with `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  - The build succeeded in `1 s 70 ms`.
  - The unsigned HAP still contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
- Checked for an attached OHOS target again with `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`; the output is still `[Empty]`.

### 2026-05-30 native-module target cleanup

- Promoted the NAPI/XComponent wrapper from a smoke-only target name to an
  experimental native-module target:
  - `MLN_OHOS_BUILD_NATIVE_MODULE` is the new CMake option.
  - `maplibre-native-ohos` is the new CMake target.
  - The shared-library output remains `libmaplibre_native_ohos.so`, matching
    the ArkTS import and NAPI module name.
- Removed the old unshipped NAPI smoke option/presets from the reviewable patch
  surface rather than carrying a deprecated alias in a new platform port.
- Added direct presets:
  - `ohos-opengl-native`
  - `harmonyos-opengl-native`
- Renamed the wrapper source from `platform/ohos/src/napi_xcomponent_smoke.cpp`
  to `platform/ohos/src/native_module.cpp`.
- Updated `platform/ohos/README.md` and the ignored XComponent codelab harness
  CMake file to use `MLN_OHOS_BUILD_NATIVE_MODULE`.
- Verification:
  - Configuring `ohos-opengl-native` and `harmonyos-opengl-native` without
    `OHOS_SDK_NATIVE`/`HMOS_SDK_NATIVE` exported fails by looking for
    `/build/cmake/...`, as expected for the env-based presets.
  - With explicit SDK env, `ohos-opengl-native` configured and built from a
    fresh `build-ohos-native` directory, linking `libmaplibre_native_ohos.so`.
  - With explicit SDK env, `harmonyos-opengl-native` configured and built from
    a fresh `build-harmonyos-native` directory, linking
    `libmaplibre_native_ohos.so`.
  - After removing the aliases, `ohos-opengl-native` re-ran CMake and reported
    `ninja: no work to do`.
  - After removing the aliases, `harmonyos-opengl-native` re-ran CMake and
    reported `ninja: no work to do`.
  - The ignored XComponent codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `3 s 725 ms`.
  - After removing the aliases, the ignored harness rebuilt again; the build
    succeeded in `1 s 173 ms`.
  - The unsigned HAP still contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.
  - `pixi run cmake --list-presets` shows the new `ohos-opengl-native` and
    `harmonyos-opengl-native` presets and no `napi-smoke` presets.
  - `git diff --check` passed.
  - The source-location scan found no remaining dependency:
    `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'`

### 2026-05-30 NAPI argument validation cleanup

- Tightened the native-module string argument parsing to match the ArkTS type
  declarations.
  - `setStyleUrl(...)` now rejects non-string style URLs instead of treating
    them as an empty URL.
  - `setStyleJson(...)` now rejects non-string style JSON instead of treating
    it as an empty style.
  - `setClientOptions(...)` now requires a string client name and, when
    provided, a string client version.
  - `setResourceOptions(...)` already validated optional string properties and
    still does.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/native_module.cpp` and
    linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt `platform/ohos/src/native_module.cpp`
    and linked `libmaplibre_native_ohos.so`.
  - The ignored XComponent codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `3 s 284 ms`.
  - The unsigned HAP still contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so`
    - `libs/x86_64/libmaplibre_native_ohos.so`
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 lifecycle memory cleanup API

- Refreshed `maplibre/maplibre-native#3749` through the GitHub connector.
  - The issue is still open.
  - The latest technical context is still the earlier HarmonyOS/OpenHarmony
    GLES3/EGL/XComponent discussion and build notes; the newest later comment
    is a thumbs-up from November 12, 2025.
- Added a minimal memory-pressure hook to the OHOS native module.
  - `RendererFrontend::reduceMemoryUse()` forwards to
    `mbgl::Renderer::reduceMemoryUse()`.
  - `MapView::reduceMemoryUse()` exposes that frontend call.
  - The NAPI module now exports `reduceMemoryUse(binding?)`, available both as
    a legacy XComponent context method and for ArkUI node-handle bindings.
- Updated ArkTS metadata and README examples for `reduceMemoryUse()`.
- Updated the ignored XComponent codelab harness to exercise more of the
  current API:
  - calls `setClientOptions(...)`
  - calls `setResourceOptions(...)`
  - enables debug overlays
  - checks camera/debug readback
  - calls `reduceMemoryUse()` from `aboutToDisappear()` and `onPageHide()`
- Verification:
  - `ohos-opengl-native` rebuilt `renderer_frontend.cpp`, `map_view.cpp`, and
    `native_module.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same files and linked
    `libmaplibre_native_ohos.so`.
  - The first harness rebuild caught an ArkTS type issue in the ignored page:
    `setResourceOptions` was locally typed as accepting `object`, and ArkTS
    rejects untyped object literals. The harness page now declares an explicit
    `MapLibreResourceOptions` interface.
  - The ignored XComponent codelab harness rebuilt successfully with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `3 s 298 ms`.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,219,280` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`9,987,416` bytes)
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - The source-location scan found no remaining dependency:
    `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'`
  - The reviewable patch surface no longer references the old NAPI smoke names.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 lifecycle render-enable API

- Checked the installed command-line tools for a launchable emulator/simulator.
  - No files or directories matching `emulator` or `simulator` were found under
    `/Users/sargunv/Downloads/command-line-tools` at depth 4.
  - The available runtime-side tool remains `hdc`, and it still reports no
    connected targets.
- Added a binding-level rendering enable switch to the OHOS native module.
  - `SurfaceBinding` now tracks `renderingEnabled`.
  - `renderBindingFrame(...)` skips `MapView::renderFrame()` while rendering is
    disabled, covering frame callbacks and manual `renderFrame(...)` calls.
  - The NAPI module now exports `setRenderingEnabled(binding?, enabled)`.
  - Re-enabling rendering triggers one render attempt so foreground transitions
    can repaint promptly.
- Updated ArkTS metadata and README examples for `setRenderingEnabled(...)`.
- Updated the ignored XComponent codelab harness lifecycle hooks:
  - `aboutToAppear()` and `onPageShow()` call `setRenderingEnabled(true)`.
  - `aboutToDisappear()` and `onPageHide()` call
    `setRenderingEnabled(false)` before `reduceMemoryUse()`.
  - Initial XComponent load explicitly enables rendering after pixel-ratio
    setup.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/native_module.cpp` and
    linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt `platform/ohos/src/native_module.cpp`
    and linked `libmaplibre_native_ohos.so`.
  - The ignored XComponent codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `3 s 742 ms`.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,220,072` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`9,988,232` bytes)
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/log/harness files found
    no matches.
  - The source-location scan found no remaining dependency:
    `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'`
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 tile-cache memory API

- Added an OHOS binding for the core renderer tile-cache controls already used
  by mature platforms.
  - `RendererFrontend` now forwards `setTileCacheEnabled(...)` and
    `getTileCacheEnabled()`.
  - `MapView` stores the desired tile-cache state so callers can set it before
    the XComponent surface exists; new renderers receive the stored setting
    during map creation.
  - The NAPI module now exports `setTileCacheEnabled(binding?, enabled)` and
    `getTileCacheEnabled(binding?)`.
- Updated ArkTS metadata and README examples for the new API.
- Updated the ignored XComponent codelab harness to call
  `setTileCacheEnabled(true)` and `getTileCacheEnabled()` during XComponent
  load, which compile-tests the ArkTS-facing declarations.
- Verification:
  - `ohos-opengl-native` rebuilt `renderer_frontend.cpp`, `map_view.cpp`, and
    `native_module.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same files and linked
    `libmaplibre_native_ohos.so`.
  - The ignored XComponent codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `3 s 501 ms`.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,221,376` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`9,989,704` bytes)
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched OHOS/log/harness files found
    no matches.
  - The source-location scan found no remaining dependency:
    `rg -n "std::source_location|MLN_HAS_STD_SOURCE_LOCATION|__builtin_source_location|<source_location>" src include platform/ohos platform/linux --glob '!build-*' --glob '!.git'`
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 source-location compatibility cleanup

- Tightened the C++20 source-location workaround to be less invasive on
  toolchains with complete C++20 library support.
  - `mbgl::SourceLocation` is now an alias to `std::source_location` when
    `<source_location>` is available and advertises `__cpp_lib_source_location`.
  - The fallback class is only used on toolchains like the OpenHarmony API 21
    SDK libc++, where the header is not available.
  - Symbol guard call sites now use `MBGL_CURRENT_SOURCE_LOCATION`, preserving
    standard `std::source_location::current()` behavior on capable toolchains
    and using `__FILE__`/`__func__`/`__LINE__` only for the fallback path.
- Verification:
  - `ohos-opengl-native` rebuilt `mbgl-core` broadly, including
    `symbol_instance.cpp` and `symbol_bucket.cpp`, then linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same affected core code and linked
    `libmaplibre_native_ohos.so`.
  - A host syntax-only probe using `pixi run c++ -std=c++20` included
    `mbgl/util/source_location.hpp` and instantiated
    `MBGL_CURRENT_SOURCE_LOCATION`, proving the standard-library branch parses
    on the local host compiler.
  - The ignored XComponent codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `28 s 489 ms` after rebuilding native CMake outputs.
  - The unsigned HAP still contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,221,376` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`9,989,704` bytes)
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The only remaining `std::source_location` and `<source_location>` mentions
    in source are intentionally contained in
    `src/mbgl/util/source_location.hpp`.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 render-enable consistency cleanup

- Audited the native-module render paths after adding
  `setRenderingEnabled(...)`.
  - Surface creation/change and gesture handlers were still calling
    `MapView::renderFrame()` directly.
  - Those paths now go through `renderBindingFrame(...)`, the same helper used
    by frame callbacks and the public `renderFrame(...)` export.
  - This keeps camera/touch/surface state changes active while suppressing
    actual rendering when an app has disabled rendering for a hidden page.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/native_module.cpp` and
    linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same source and linked
    `libmaplibre_native_ohos.so`.
  - The ignored XComponent codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `3 s 376 ms`.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,221,408` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`9,989,800` bytes)
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The only remaining direct `mapView->renderFrame()` call in
    `platform/ohos/src/native_module.cpp` is inside `renderBindingFrame(...)`.
  - Checked for an attached OHOS target again with
  `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 public net_http cancellation cleanup

- Tightened the default OpenHarmony `net_http` cancellation lifetime handling.
  - The public SDK headers expose `OH_Http_Destroy(...)` for request teardown,
    plus an `onCanceled` event, but no separate cancel API was visible in
    `network/netstack/net_http.h`.
  - `Http_EventsHandler` callbacks such as `onDataReceive` and `onCanceled`
    still carry no user-data pointer in
    `network/netstack/net_http_type.h`, so the default backend continues to use
    bounded static callback slots.
  - Because those callbacks only carry a slot index, canceled requests now mark
    themselves canceled/finished, drop the MapLibre callback, destroy the
    request, and release the callback slot/self-reference immediately.
  - Late response callbacks find an empty or expired slot and destroy the
    response object; late data callbacks are ignored once the request is marked
    finished or canceled.
  - The optional HarmonyOS HMS/RCP backend was left unchanged because its
    callback object carries the raw request state pointer, so the request state
    must remain alive until the SDK callback path is finished with it.
- Verification:
  - `ohos-opengl-link-smoke` reported `ninja: no work to do` after the
    cancellation cleanup, confirming the previous link-smoke rebuild completed.
  - `ohos-opengl-native` and `harmonyos-opengl-native` both reported
    `ninja: no work to do`.
  - The ignored XComponent codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `960 ms`.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,221,440` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`9,989,864` bytes)
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS files found
    no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 XComponent frame-rate range API

- Added an ArkTS-facing `setFrameRateRange({ min, max, expected })` API to the
  experimental native module.
  - The SDK exposes `OH_NativeXComponent_SetExpectedFrameRateRange(...)` for
    legacy `libraryname` XComponents and
    `OH_ArkUI_XComponent_SetExpectedFrameRateRange(...)` for ArkUI node-handle
    XComponents.
  - The native module now validates positive ordered ranges
    (`min <= expected <= max`) and applies the range to either XComponent path.
  - The requested range is stored on the binding so it can be reapplied if the
    callback path is registered again for an existing binding.
  - This does not replace the XComponent on-frame render path with a separate
    native-vsync subsystem; it keeps rendering on the UI component callback
    path while giving apps a platform-supported way to tune callback cadence.
- Updated ArkTS metadata and README examples for the new API.
- Updated the ignored XComponent codelab harness to call
  `setFrameRateRange({ min: 30, max: 120, expected: 60 })`, compile-testing
  the ArkTS-facing declaration and NAPI export.
- Local side effects:
  - Regenerated ignored harness build outputs under
    `build-ohos-xcomponent-sample/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/` that were already noted earlier in this
    log.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/native_module.cpp` and
    linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same source and linked
    `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` reported `ninja: no work to do`, as expected
    because the change is native-module-only.
  - The ignored XComponent codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `4 s 720 ms`.
  - The harness emitted the same existing ArkTS NAPI verification warnings for
    `libnativerender.so` and `libmaplibre_native_ohos.so`, plus the same Java
    `sun.misc.Unsafe::arrayBaseOffset` and unsigned-output warnings as before.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,223,768` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`9,992,280` bytes)
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 EGL native-window buffer geometry

- Tightened the XComponent EGL backend's surface sizing path.
  - The OpenHarmony native-window header exposes `SET_BUFFER_GEOMETRY` through
    `OH_NativeWindow_NativeWindowHandleOpt(...)`.
  - `EGLWindowBackend` now applies that native producer-buffer geometry before
    creating the `EGLSurface` and whenever `setSize(...)` receives an updated
    XComponent size.
  - MapLibre was already updating its logical map size and GL viewport; this
    adds the matching platform window-buffer size so the rendered buffer is less
    likely to mismatch the ArkUI surface after creation or resize.
  - Failure to set buffer geometry is logged as an OpenGL warning rather than
    treated as fatal, because the XComponent window may already have usable
    platform-managed geometry.
- Local side effects:
  - Regenerated ignored harness build outputs under
    `build-ohos-xcomponent-sample/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/` that were already noted earlier in this
    log.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/egl_window_backend.cpp` and
    linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same source and linked
    `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` reported `ninja: no work to do`, as expected
    because the EGL window backend is native-module-only.
  - The ignored XComponent codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `3 s 289 ms`.
  - The harness emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` and
    unsigned-output warnings as before.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,224,464` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`9,992,984` bytes)
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 ArkUI node surface-size fallback

- Tightened the ArkUI node-handle XComponent surface lifecycle path.
  - The node-handle callback API reports surface dimensions in
    `onSurfaceChanged(...)`, while `onSurfaceCreated(...)` only provides the
    holder/window.
  - The SDK also exposes `OH_ArkUI_NodeUtils_GetLayoutSize(...)`, returning the
    node layout size in pixels.
  - The native module now uses that layout-size API as a fallback when a node
    surface callback has a native window but no nonzero callback dimensions yet.
  - This keeps the first surface-created event from clearing the `MapView`
    solely because the changed callback has not populated `width`/`height`
    yet; callback-provided dimensions remain authoritative when present.
- Local side effects:
  - Regenerated ignored harness build outputs under
    `build-ohos-xcomponent-sample/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/` that were already noted earlier in this
    log.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/native_module.cpp` and
    linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same source and linked
    `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` reported `ninja: no work to do`, as expected
    because the change is native-module-only.
  - The ignored XComponent codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `3 s 944 ms`.
  - The harness emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` and
    unsigned-output warnings as before.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,224,920` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`9,993,336` bytes)
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 surface teardown gesture reset

- Tightened XComponent surface teardown state handling for both native binding
  paths.
  - Added a shared `clearBindingSurface(...)` helper in the native module.
  - Surface clears now end any active MapLibre gesture before destroying the
    `MapView` surface.
  - The helper resets stored one-finger pan and two-finger pinch state whenever
    a surface is destroyed, a surface temporarily has no native window/size, or
    map surface creation fails.
  - This prevents stale touch/pinch state from carrying into a recreated
    XComponent surface after backgrounding, resize-to-zero, page navigation, or
    backend creation errors.
- Local side effects:
  - Regenerated ignored harness build outputs under
    `build-ohos-xcomponent-sample/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/` that were already noted earlier in this
    log.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/native_module.cpp` and
    linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same source and linked
    `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` reported `ninja: no work to do`, as expected
    because the change is native-module-only.
  - The ignored XComponent codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `3 s 914 ms`.
  - The harness emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` and
    unsigned-output warnings as before.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,225,176` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`9,993,640` bytes)
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 object camera API expansion

- Expanded the ArkTS-facing camera surface beyond the original positional
  `jumpTo(longitude, latitude, zoom, bearing, pitch)` smoke API.
  - `MapView` now accepts `mbgl::CameraOptions` directly and supports
    `jumpTo`, `easeTo`, and `flyTo`.
  - The native module now parses object camera options with top-level
    `longitude`/`latitude` or `center: { longitude, latitude }`, plus
    `centerAltitude`, `padding`, `anchor`, `zoom`, `bearing`, `pitch`, `roll`,
    and `fov`.
  - `easeTo` and `flyTo` accept a small `AnimationOptions` object with
    `duration` in milliseconds, `velocity`, and `minZoom`.
  - `setCameraOptions(...)` is exported as a clearer object-based immediate
    camera setter, while the existing positional `jumpTo(...)` signature remains
    for the legacy smoke harness path.
  - Desired camera state is merged with the current camera before being stored,
    so surface/map recreation continues to reapply the intended final camera
    state even when callers provide partial camera updates.
  - `getCameraOptions()` now returns object fields for padding, anchor,
    center altitude, roll, and fov when present.
- Updated ArkTS metadata under
  `platform/ohos/arkts/types/libmaplibre_native_ohos/` and kept the ignored
  XComponent harness metadata copy in sync.
- Updated the ignored codelab harness to compile-call `setCameraOptions(...)`
  and `easeTo(...)` in addition to the original positional `jumpTo(...)`.
- Local side effects:
  - Regenerated ignored harness build outputs under
    `build-ohos-xcomponent-sample/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/`.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/map_view.cpp` and
    `platform/ohos/src/native_module.cpp`, then linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same sources and linked
    `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` reported `ninja: no work to do`, as expected
    because this change only affects the native module and wrapper sources.
  - The ignored XComponent codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `4 s 443 ms`.
  - The harness emitted the same Java `sun.misc.Unsafe::arrayBaseOffset`,
    unsigned-output, and NAPI module verification warnings as before.
  - The unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,231,672` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`10,002,168` bytes)
  - `module.json` inside the HAP still contains `ohos.permission.INTERNET`.
  - The committed ArkTS metadata and ignored harness metadata copies compare
    equal with `cmp -s`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 committed minimal sample app

- Added `platform/ohos/sample` as a small committed HarmonyOS/OpenHarmony app
  shell.
  - The sample has its own Hvigor project/module metadata, `module.json5`,
    resources, and a minimal Stage `EntryAbility`.
  - `entry/src/main/cpp/CMakeLists.txt` builds the local MapLibre checkout via
    `add_subdirectory(...)` and enables `MLN_OHOS_BUILD_NATIVE_MODULE`.
  - The ArkTS page imports `libmaplibre_native_ohos.so`, hosts a surface
    `XComponent` with `libraryname: 'maplibre_native_ohos'`, and compile-calls
    client/resource options, tile cache, frame-rate range, pixel ratio,
    rendering enable/disable, style JSON, debug options, object camera options,
    `easeTo`, camera/debug/tile-cache readback, manual render, and memory
    reduction.
  - The sample manifest declares `ohos.permission.INTERNET`.
  - The sample uses the committed type package at
    `platform/ohos/arkts/types/libmaplibre_native_ohos` instead of carrying a
    second generated copy of the declarations.
- Updated `platform/ohos/README.md` with sample build instructions.
- Local side effects:
  - Building the sample generated ignored project-local outputs under
    `platform/ohos/sample/.hvigor/`, `platform/ohos/sample/build/`,
    `platform/ohos/sample/entry/.cxx/`, and
    `platform/ohos/sample/entry/build/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/`.
- Verification:
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `1 min 30 s 683 ms`.
  - The sample emitted the same Java `sun.misc.Unsafe::arrayBaseOffset`,
    unsigned-output, and NAPI module verification warnings seen in the ignored
    codelab harness.
  - The sample unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,231,672` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`10,002,168` bytes)
  - `module.json` inside the sample HAP reports bundle name
    `org.maplibre.native.ohos.sample`, HarmonyOS compile SDK `6.0.1.112`, and
    `ohos.permission.INTERNET`.
  - A second up-to-date sample `assembleApp` after documentation/log edits
    succeeded in `1 s 37 ms`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 image decoder bounds hardening

- Hardened the OHOS image decoder against malformed or surprising decoder
  metadata before runtime validation is available.
  - Empty decoded dimensions now fail explicitly instead of flowing into an
    invalid `PremultipliedImage`.
  - Row-stride and total pixel-buffer byte counts now use checked `size_t`
    arithmetic instead of relying on `uint32_t` multiplication.
  - A nonzero decoder row stride smaller than the RGBA/BGRA tight stride is now
    rejected.
  - `OH_PixelmapNative_ReadPixels(...)` short reads are now rejected before
    copying rows into MapLibre image storage.
- Local side effects:
  - Rebuilding the committed sample refreshed ignored outputs under
    `platform/ohos/sample/.hvigor/`, `platform/ohos/sample/build/`,
    `platform/ohos/sample/entry/.cxx/`, and
    `platform/ohos/sample/entry/build/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/`.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/image.cpp`, relinked
    `libmbgl-core.a`, and linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same source, relinked
    `libmbgl-core.a`, and linked `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` rebuilt `platform/ohos/src/image.cpp`, relinked
    `libmbgl-core.a`, and linked `libmbgl-ohos-link-smoke.so`.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 46 ms`.
  - The sample emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` and
    unsigned-output warnings as before.
  - The sample unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,232,632` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`10,003,160` bytes)
  - `module.json` inside the sample HAP still reports bundle name
    `org.maplibre.native.ohos.sample`, HarmonyOS compile SDK `6.0.1.112`, and
    `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 map bounds API

- Added persistent map bounds support to the OHOS wrapper.
  - `MapView` now stores desired `mbgl::BoundOptions`, applies them when a map
    is created or recreated, and exposes `setBounds(...)`/`getBounds()`.
  - The native module now parses ArkTS bounds options:
    `bounds: { west, south, east, north }`, `minZoom`, `maxZoom`, `minPitch`,
    and `maxPitch`.
  - The native module exports `setBounds(...)` and `getBounds(...)` for both
    legacy XComponent contexts and ArkUI node-handle bindings.
  - The existing object camera persistence remains separate: bounds are applied
    before desired camera state when a map is recreated.
- Updated ArkTS metadata under
  `platform/ohos/arkts/types/libmaplibre_native_ohos/` and kept the ignored
  XComponent harness metadata copy in sync.
- Updated the committed sample, ignored codelab harness, and
  `platform/ohos/README.md` to compile-call the bounds APIs.
- Local side effects:
  - Rebuilding the committed sample refreshed ignored outputs under
    `platform/ohos/sample/.hvigor/`, `platform/ohos/sample/build/`,
    `platform/ohos/sample/entry/.cxx/`, and
    `platform/ohos/sample/entry/build/`.
  - Rebuilding the ignored codelab harness refreshed ignored outputs under
    `build-ohos-xcomponent-sample/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/`.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/map_view.cpp` and
    `platform/ohos/src/native_module.cpp`, then linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same sources and linked
    `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` reported `ninja: no work to do`, as expected
    because the change is native-module/wrapper-only.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `4 s 150 ms`.
  - The ignored XComponent codelab harness rebuilt with the same `hvigorw`
    command from `build-ohos-xcomponent-sample`; the build succeeded in
    `4 s 275 ms`.
  - The harnesses emitted the same Java `sun.misc.Unsafe::arrayBaseOffset`,
    unsigned-output, and NAPI module verification warnings as before.
  - The committed sample unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,236,504` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`10,008,264` bytes)
  - `module.json` inside the sample HAP still reports bundle name
    `org.maplibre.native.ohos.sample`, HarmonyOS compile SDK `6.0.1.112`, and
    `ohos.permission.INTERNET`.
  - The committed ArkTS metadata and ignored harness metadata copies compare
    equal with `cmp -s`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 bounds-based camera fitting API

- Added bounds-based camera fitting on top of the map bounds controls.
  - `MapView` now exposes `fitBounds(...)` and `cameraForBounds(...)`.
  - `fitBounds(...)` stores the requested bounds fit and reapplies it when the
    native map is created or recreated; explicit camera updates clear the
    stored fit request.
  - `cameraForBounds(...)` returns the MapLibre-computed camera options for an
    active map surface, where viewport size is available.
  - The native module now parses `CameraBoundsOptions` with
    `bounds: { west, south, east, north }`, optional `padding`, optional
    `bearing`, and optional `pitch`.
  - `fitBounds(...)` and `cameraForBounds(...)` are exported for both legacy
    XComponent contexts and ArkUI node-handle bindings.
- Updated ArkTS metadata under
  `platform/ohos/arkts/types/libmaplibre_native_ohos/` and kept the ignored
  XComponent harness metadata copy in sync.
- Updated the committed sample, ignored codelab harness, and
  `platform/ohos/README.md` to compile-call the fitting APIs.
- Local side effects:
  - Rebuilding the committed sample refreshed ignored outputs under
    `platform/ohos/sample/.hvigor/`, `platform/ohos/sample/build/`,
    `platform/ohos/sample/entry/.cxx/`, and
    `platform/ohos/sample/entry/build/`.
  - Rebuilding the ignored codelab harness refreshed ignored outputs under
    `build-ohos-xcomponent-sample/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/`.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/map_view.cpp` and
    `platform/ohos/src/native_module.cpp`, then linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same sources and linked
    `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` reported `ninja: no work to do`, as expected
    because the change is native-module/wrapper-only.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 823 ms`.
  - The ignored XComponent codelab harness rebuilt with the same `hvigorw`
    command from `build-ohos-xcomponent-sample`; the build succeeded in
    `3 s 911 ms`.
  - The harnesses emitted the same Java `sun.misc.Unsafe::arrayBaseOffset`,
    unsigned-output, and NAPI module verification warnings as before.
  - The committed sample unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,240,360` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`10,012,088` bytes)
  - `module.json` inside the sample HAP still reports bundle name
    `org.maplibre.native.ohos.sample`, HarmonyOS compile SDK `6.0.1.112`, and
    `ohos.permission.INTERNET`.
  - The committed ArkTS metadata and ignored harness metadata copies compare
    equal with `cmp -s`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 two-finger rotation gesture

- Extended the existing two-finger gesture bridge from pinch/pan to
  pinch/pan/rotate.
  - `PinchState` now carries the angle between the first two touch points.
  - `SurfaceBinding` tracks the previous two-finger angle across move events.
  - `MapView` now exposes `rotateBy(previousAngle, currentAngle)`, translating
    the angle delta into MapLibre `Map::rotateBy(...)` points around the current
    viewport center and persisting the resulting camera state.
  - The two-finger move path now applies scale, rotation, and center movement
    before rendering a frame.
  - Gesture reset/teardown paths clear the stored two-finger angle along with
    the existing pinch state.
- Local side effects:
  - Rebuilding the committed sample refreshed ignored outputs under
    `platform/ohos/sample/.hvigor/`, `platform/ohos/sample/build/`,
    `platform/ohos/sample/entry/.cxx/`, and
    `platform/ohos/sample/entry/build/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/`.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/map_view.cpp` and
    `platform/ohos/src/native_module.cpp`, then linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same sources and linked
    `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` reported `ninja: no work to do`, as expected
    because the change is native-module/wrapper-only.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 126 ms`.
  - The sample emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` and
    unsigned-output warnings as before.
  - The committed sample unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,241,192` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`10,013,064` bytes)
  - `module.json` inside the sample HAP still reports bundle name
    `org.maplibre.native.ohos.sample`, HarmonyOS compile SDK `6.0.1.112`, and
    `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 double-tap zoom gesture

- Added a small double-tap zoom bridge to the shared touch handler.
  - `SurfaceBinding` now tracks a tap candidate, tap start position/time, and
    the last completed tap position/time.
  - Single-finger movement beyond a small slop cancels the tap candidate.
  - Long taps and moved taps are rejected before they can form a double tap.
  - Starting a two-finger pinch/rotate clears the pending tap state.
  - A valid second tap within the time and distance thresholds calls
    `MapView::scaleBy(...)` with a 2x scale anchored at the tap point and
    renders a frame.
  - Surface teardown/reset paths clear active and stored tap state.
- Local side effects:
  - Rebuilding the committed sample refreshed ignored outputs under
    `platform/ohos/sample/.hvigor/`, `platform/ohos/sample/build/`,
    `platform/ohos/sample/entry/.cxx/`, and
    `platform/ohos/sample/entry/build/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/`.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/native_module.cpp` and
    linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same source and linked
    `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` reported `ninja: no work to do`, as expected
    because the change is native-module-only.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 169 ms`.
  - The sample emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` and
    unsigned-output warnings as before.
  - The committed sample unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,241,736` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`10,014,104` bytes)
  - `module.json` inside the sample HAP still reports bundle name
    `org.maplibre.native.ohos.sample`, HarmonyOS compile SDK `6.0.1.112`, and
    `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 single-finger fling gesture

- Added a conservative single-finger fling path to the shared gesture bridge.
  - `SurfaceBinding` now tracks recent single-finger sample time and velocity.
  - Single-finger movement updates the latest velocity and still cancels tap
    candidates when movement exceeds tap slop.
  - On touch up after a real pan, sufficiently fast velocity is converted into
    a bounded animated `MapView::moveBy(...)`.
  - The fling uses a short MapLibre `AnimationOptions` duration and clamps the
    projected pixel distance to avoid large jumps from noisy input samples.
  - Tap, double-tap, cancel, pinch, rotate, and surface teardown paths suppress
    or clear fling state.
  - `MapView::moveBy(...)` now accepts optional MapLibre animation options so
    immediate pan and animated fling share the same wrapper call.
- Local side effects:
  - Rebuilding the committed sample refreshed ignored outputs under
    `platform/ohos/sample/.hvigor/`, `platform/ohos/sample/build/`,
    `platform/ohos/sample/entry/.cxx/`, and
    `platform/ohos/sample/entry/build/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/`.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/map_view.cpp` and
    `platform/ohos/src/native_module.cpp`, then linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same sources and linked
    `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` reported `ninja: no work to do`, as expected
    because the change is native-module/wrapper-only.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 184 ms`.
  - The sample emitted the same Java `sun.misc.Unsafe::arrayBaseOffset` and
    unsigned-output warnings as before.
  - The committed sample unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,242,376` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`10,014,808` bytes)
  - `module.json` inside the sample HAP still reports bundle name
    `org.maplibre.native.ohos.sample`, HarmonyOS compile SDK `6.0.1.112`, and
    `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 free-camera API bridge

- Exposed MapLibre's `FreeCameraOptions` through the OHOS wrapper and NAPI
  module.
  - `MapView` now stores a desired free-camera state for surface recreation,
    reads active free-camera state from `mbgl::Map`, and applies it after map
    creation/style reloads.
  - Standard camera commands, fit-bounds, and gesture movement clear the saved
    free-camera state so recreated surfaces do not replay conflicting camera
    intents.
  - The NAPI module exports `getFreeCameraOptions` and
    `setFreeCameraOptions`.
  - ArkTS callers use objects matching the core fields:
    `position: { x, y, z }` and `orientation: { x, y, z, w }`.
  - The parser accepts partial or empty options, rejects non-finite vector
    components, and rejects zero-length quaternions when an orientation is
    provided.
  - The committed sample and ignored codelab harness both read and reapply the
    current free-camera state as a compile/package smoke test.
- Local side effects:
  - Rebuilding the committed sample refreshed ignored outputs under
    `platform/ohos/sample/.hvigor/`, `platform/ohos/sample/build/`,
    `platform/ohos/sample/entry/.cxx/`, and
    `platform/ohos/sample/entry/build/`.
  - Rebuilding the ignored codelab harness refreshed ignored outputs under
    `build-ohos-xcomponent-sample/.hvigor/`,
    `build-ohos-xcomponent-sample/build/`,
    `build-ohos-xcomponent-sample/entry/.cxx/`, and
    `build-ohos-xcomponent-sample/entry/build/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/`.
- Verification:
  - `ohos-opengl-native` rebuilt `platform/ohos/src/map_view.cpp` and
    `platform/ohos/src/native_module.cpp`, then linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same sources and linked
    `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` reported `ninja: no work to do`, as expected
    because the change is native-module/wrapper-only.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 688 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `3 s 518 ms`.
  - The sample and harness emitted the same native-module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - The committed sample unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,245,928` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`10,019,368` bytes)
  - `module.json` inside the sample HAP still reports bundle name
    `org.maplibre.native.ohos.sample`, HarmonyOS compile SDK `6.0.1.112`, and
    `ohos.permission.INTERNET`.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 platform documentation refresh

- Refreshed `platform/ohos/README.md` after the native module API grew beyond
  the older notes.
  - Added a status section that explicitly says the platform support is still
    compile/package tested only.
  - Listed the current implemented pieces: EGL/GLES3 window rendering,
    XComponent integration, OpenHarmony/HMS network backends, OHOS image
    decoding, hilog, NAPI controls, and the currently compile-tested gestures.
  - Documented the current compiler strategy: keep current MapLibre Native
    C++20 and use the SDK's libc++ experimental ranges support instead of
    reverting to an older MapLibre Native revision.
  - Replaced the stale hand-written ArkTS interface block with guidance that
    treats `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts` as
    the authoritative API surface.
  - Updated the sample snippet to include `setRenderingEnabled` and
    free-camera read/write.
  - Removed stale text that said rotation, double-tap, and fling gestures were
    still open.
- Refreshed `platform/ohos/sample/README.md` so it mentions free-camera,
  bounds, and current gesture coverage.
- Re-checked GitHub issue `maplibre/maplibre-native#3749`.
  - It remains open as of this run.
  - `gh issue view 3749 -R maplibre/maplibre-native --json ...` reports title
    `Support OpenHarmony or provide documentation for cross-platform build`,
    no labels, no assignees, `updatedAt` `2025-11-12T01:16:25Z`, eight
    comments, and latest comment by `qmjy` on `2025-11-12T01:16:25Z`.
- Verification:
  - `ohos-opengl-native` reported `ninja: no work to do`.
  - `harmonyos-opengl-native` reported `ninja: no work to do`.
  - `ohos-opengl-link-smoke` reported `ninja: no work to do`.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `1 s 78 ms`.
  - The sample emitted the same unsigned-output warning as before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 ArkTS type package consumption

- Updated the committed sample app to import
  `type { XComponentContext }` from `libmaplibre_native_ohos.so`.
  - Removed the duplicated local MapLibre ArkTS interface declarations from
    `platform/ohos/sample/entry/src/main/ets/pages/Index.ets`.
  - The sample now proves the checked
    `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts` package is
    usable by ArkTS rather than only copying it beside the native module.
- Updated the ignored codelab harness the same way.
  - Added a local `libmaplibre_native_ohos.so` dev dependency in the ignored
    harness `entry/oh-package.json5`.
  - Removed the duplicate local MapLibre interface declarations from the
    harness page.
- Updated `platform/ohos/sample/README.md` to mention the local type package.
- Local side effects:
  - Rebuilding the committed sample refreshed ignored outputs under
    `platform/ohos/sample/.hvigor/`, `platform/ohos/sample/build/`,
    `platform/ohos/sample/entry/.cxx/`, and
    `platform/ohos/sample/entry/build/`.
  - Rebuilding the ignored codelab harness refreshed ignored outputs under
    `build-ohos-xcomponent-sample/.hvigor/`,
    `build-ohos-xcomponent-sample/build/`,
    `build-ohos-xcomponent-sample/entry/.cxx/`, and
    `build-ohos-xcomponent-sample/entry/build/`.
  - `hvigorw` may have refreshed the existing user-level wrapper/cache paths
    under `/Users/sargunv/.hvigor/`.
- Verification:
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 558 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `3 s 355 ms`.
  - ArkTS accepted `import type { XComponentContext } from
    'libmaplibre_native_ohos.so'` in both projects.
  - The builds emitted the same native-module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 lower-level platform preset verification

- Rechecked the generated-output hygiene after repeated sample builds.
  - `git check-ignore -v` confirms committed-sample generated outputs are
    covered by `platform/ohos/sample/.gitignore`:
    `.hvigor/`, `build/`, `entry/.cxx/`, and `entry/build/`.
  - The ignored codelab harness remains covered by the repo-level `/build-*`
    ignore rule.
- Rebuilt the lower-level platform presets that do not enable the native module.
  - `ohos-opengl` re-ran CMake, rebuilt 100 `mbgl-core` steps, compiled the
    OpenHarmony NetworkKit HTTP backend, and linked `libmbgl-core.a`.
  - `harmonyos-opengl` re-ran CMake, rebuilt 101 `mbgl-core` steps, compiled
    the HMS RCP HTTP backend, and linked `libmbgl-core.a`.
  - These builds exercise the platform/default-plus-OHOS source list directly,
    independently of the NAPI/XComponent native module target.
- Warnings observed:
  - The SDK toolchain files emit the same CMake deprecation warning about
    `cmake_minimum_required(VERSION < 3.10)`.
  - OHOS clang still emits the same unused
    `--gcc-toolchain=.../openharmony/native/llvm` warning.
- No source edits were needed for this verification pass.

### 2026-05-30 gesture bridge extraction

- Split the XComponent gesture translation out of `native_module.cpp`.
  - Added `platform/ohos/src/gesture_handler.hpp`.
  - Added `platform/ohos/src/gesture_handler.cpp`.
  - `SurfaceBinding` now owns a `mbgl::ohos::GestureState` instead of carrying
    all touch/tap/pinch/fling fields directly.
  - The extracted bridge handles legacy `OH_NativeXComponent_TouchEvent` and
    ArkUI `ArkUI_UIInputEvent` inputs and returns whether the binding should
    render a frame.
  - `native_module.cpp` now handles XComponent lifecycle, NAPI exports, and
    frame scheduling while gesture parsing and MapLibre gesture operations live
    in the smaller bridge file.
  - Added `gesture_handler.cpp` to the `maplibre-native-ohos` CMake target.
  - Ran `pixi run clang-format -i` on the touched C++ files.
- Verification:
  - `ohos-opengl-native` re-ran CMake, rebuilt `gesture_handler.cpp` and
    `native_module.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` re-ran CMake, rebuilt the same sources, then
    linked `libmaplibre_native_ohos.so`.
  - `ohos-opengl-link-smoke` re-ran CMake and reported `ninja: no work to do`.
  - `harmonyos-opengl-link-smoke` re-ran CMake, rebuilt `mbgl-core`, and
    linked `libmbgl-ohos-link-smoke.so`.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `2 s 725 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `2 s 672 ms`.
  - The builds emitted the same SDK toolchain, native-module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - The committed sample unsigned HAP contains:
    - `libs/arm64-v8a/libmaplibre_native_ohos.so` (`9,246,504` bytes)
    - `libs/x86_64/libmaplibre_native_ohos.so` (`10,019,768` bytes)
  - `module.json` inside the sample HAP still reports bundle name
    `org.maplibre.native.ohos.sample`, HarmonyOS compile SDK `6.0.1.112`, and
    `ohos.permission.INTERNET`.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 NAPI value conversion extraction

- Split ArkTS/N-API value parsing and object creation out of
  `native_module.cpp`.
  - Added `platform/ohos/src/native_values.hpp`.
  - Added `platform/ohos/src/native_values.cpp`.
  - Added `platform/ohos/src/camera_bounds_options.hpp` so `MapView` and the
    value conversion layer can share the camera-bounds request type without
    keeping it buried in `map_view.hpp`.
  - Moved string, number, boolean, debug-option, camera, bounds, animation, and
    free-camera conversions into `mbgl::ohos` helpers.
  - `native_module.cpp` is now focused on binding lookup, XComponent lifecycle,
    NAPI exports, and command dispatch; its line count dropped from `2,099` to
    `1,536`.
  - Added `native_values.cpp` to the `maplibre-native-ohos` CMake target.
- Verification:
  - `ohos-opengl-native` re-ran CMake, rebuilt `native_values.cpp`,
    `gesture_handler.cpp`, `map_view.cpp`, and `native_module.cpp`, then linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` re-ran CMake, rebuilt the same native-module
    sources, then linked `libmaplibre_native_ohos.so`.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `4 s 345 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `4 s 408 ms`.
  - The builds emitted the same SDK toolchain, Java
    `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 lifecycle/display getter coverage

- Added readback for several one-way native-module controls so sample ArkTS can
  verify desired state after configuration:
  - `getRenderingEnabled(...)`
  - `getPixelRatio(...)`
  - `getFrameRateRange(...)`
- The frame-rate getter returns the requested frame-rate range stored on the
  binding, or `undefined` if ArkTS has not set one yet. It does not claim to
  query the platform's effective frame cadence.
- Updated the ArkTS type package and both sample callers to exercise the new
  getters.
- Updated `platform/ohos/README.md` and `platform/ohos/sample/README.md` to
  include the new display/lifecycle readback coverage.
- Verification:
  - `ohos-opengl-native` rebuilt `native_module.cpp` and linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt `native_module.cpp` and linked
    `libmaplibre_native_ohos.so`.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `4 s 86 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `4 s 144 ms`.
  - The builds emitted the same NAPI module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 binding resolver cleanup

- Reduced repeated binding-resolution code in the OHOS native module.
  - Added a small `ResolvedBinding` helper and shared `resolveBinding(...)`
    function for NAPI calls that accept either an explicit native binding handle
    or the legacy XComponent context as `this`.
  - Applied it to no-payload commands and getters:
    `renderFrame`, `reduceMemoryUse`, `getDebugOptions`,
    `getTileCacheEnabled`, `getRenderingEnabled`, `getPixelRatio`,
    `getFrameRateRange`, `getBounds`, `getCameraOptions`, and
    `getFreeCameraOptions`.
  - Left argument-heavy setters on their existing explicit paths for now, so
    their current validation and error messages remain unchanged.
- Verification:
  - `ohos-opengl-native` rebuilt `native_module.cpp` and linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt `native_module.cpp` and linked
    `libmaplibre_native_ohos.so`.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 973 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `4 s 32 ms`.
  - The builds emitted the same SDK toolchain,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 client/resource readback

- Added stored-configuration readback for client and resource options.
  - `MapView` now exposes the stored client name/version alongside its existing
    stored `ResourceOptions`.
  - Added `getClientOptions(...)` and `getResourceOptions(...)` to the NAPI
    module for both explicit binding handles and legacy XComponent contexts.
  - Added `createClientOptionsObject(...)` and
    `createResourceOptionsObject(...)` to the native value conversion helper.
  - Updated the ArkTS type package and both sample callers to exercise the new
    getters.
  - Updated the OHOS README files to describe client/resource readback coverage.
- Semantics:
  - These getters report the desired configuration stored by the wrapper. They
    do not prove that network requests have succeeded or that the platform HTTP
    backend has used those values at runtime.
  - Empty resource option strings are omitted from the returned ArkTS object,
    matching the optional shape accepted by `setResourceOptions(...)`.
- Verification:
  - `ohos-opengl-native` rebuilt `native_values.cpp`, `gesture_handler.cpp`,
    `map_view.cpp`, and `native_module.cpp`, then linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same native-module sources, then
    linked `libmaplibre_native_ohos.so`.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `4 s 143 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `4 s 252 ms`.
  - The builds emitted the same NAPI module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 style readback

- Added desired-style readback to the OHOS native module.
  - Added `getStyleUrl(...)`, which returns the stored style URL only when the
    current desired style was set with `setStyleUrl(...)`; otherwise it returns
    `undefined`.
  - Added `getStyleJson(...)`, which returns the stored style JSON only when
    the current desired style was set with `setStyleJson(...)`; otherwise it
    returns `undefined`.
  - Added a small `createStringValue(...)` helper to the native value
    conversion layer.
  - Updated the ArkTS type package and both sample callers to exercise the new
    getters.
  - Updated the OHOS README and sample README to describe style read/write
    coverage.
- Semantics:
  - These getters report the wrapper's desired style state, not a runtime
    confirmation that the style finished loading on device.
- Verification:
  - `ohos-opengl-native` rebuilt `native_values.cpp` and `native_module.cpp`,
    then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same sources, then linked
    `libmaplibre_native_ohos.so`.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 755 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `3 s 855 ms`.
  - The builds emitted the same NAPI module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 surface-state readback

- Added XComponent lifecycle/surface introspection to the OHOS native module.
  - Added `getSurfaceState(...)` for both explicit binding handles and legacy
    XComponent contexts.
  - The returned object contains:
    - `width` and `height`: the last surface dimensions reported by the
      XComponent callbacks or layout-size fallback.
    - `hasWindow`: whether the binding currently has an `OHNativeWindow*`.
    - `hasSurface`: whether the binding currently has a native window and
      non-zero dimensions.
    - `hasMap`: whether `MapView` currently owns a created `mbgl::Map`.
  - Updated the ArkTS type package and both sample callers to exercise the new
    getter.
  - Updated the OHOS README files to describe surface-state readback coverage.
- Semantics:
  - This is diagnostic state for future runtime validation. It can help
    distinguish NAPI module load, XComponent surface creation, and MapLibre map
    creation once a target is attached.
- Verification:
  - `ohos-opengl-native` rebuilt `native_module.cpp` and linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt `native_module.cpp` and linked
    `libmaplibre_native_ohos.so`.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 679 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `3 s 786 ms`.
  - The builds emitted the same NAPI module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 MapView creation helper cleanup

- Reduced repeated lazy `MapView` construction in the OHOS NAPI module.
  - Added a local `ensureMapView(...)` helper in `native_module.cpp`.
  - Reused it in the command/getter paths that already created a `MapView`.
  - Left non-creating paths unchanged, including `renderFrame(...)`,
    `reduceMemoryUse(...)`, `getStyleUrl(...)`, `getStyleJson(...)`, and
    `getSurfaceState(...)`.
  - Left `setPixelRatio(...)` with direct construction so a first-created
    `MapView` still receives the newly requested pixel ratio.
- Verification:
  - `ohos-opengl-native` rebuilt `native_module.cpp` and linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt `native_module.cpp` and linked
    `libmaplibre_native_ohos.so`.
  - Both native builds emitted the known unused `--gcc-toolchain` warning.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 372 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `3 s 428 ms`.
  - The HAP builds emitted the same Java `sun.misc.Unsafe::arrayBaseOffset`
    and unsigned-output warnings as before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 NAPI binding-call resolver cleanup

- Reduced repeated optional-binding dispatch code in the OHOS NAPI module.
  - Added a local `ResolvedBindingCall` helper path that resolves either an
    explicit `NativeBinding` first argument or the legacy XComponent context.
  - The helper also returns the payload-argument offset, so overloaded calls
    keep the same ArkTS call shapes while avoiding repeated external-handle and
    legacy-context checks in every setter/command.
  - Updated style, camera, bounds, rendering, frame-rate, tile-cache, client,
    resource, debug, free-camera, and pixel-ratio command paths to use it.
  - Left the no-payload getter/utility paths on the smaller `ResolvedBinding`
    helper.
- Verification:
  - `ohos-opengl-native` rebuilt `native_module.cpp` and linked
    `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt `native_module.cpp` and linked
    `libmaplibre_native_ohos.so`.
  - Both native builds emitted the known unused `--gcc-toolchain` warning.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 382 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `3 s 437 ms`.
  - The HAP builds emitted the same Java `sun.misc.Unsafe::arrayBaseOffset`
    and unsigned-output warnings as before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 surface-state render diagnostics

- Extended OHOS surface-state readback with render-loop diagnostics for future
  device validation.
  - `RendererFrontend::renderFrame()` now reports whether it actually rendered
    an update rather than only returning `void`.
  - `RendererFrontend` exposes whether an update is pending render.
  - `MapView` tracks a rendered-frame count for the current surface/map
    lifetime and resets it when the surface is cleared.
  - `getSurfaceState(...)` now returns `needsRender` and
    `renderedFrameCount` alongside the existing size/window/map fields.
  - Updated the ArkTS type package, README, committed sample, and ignored
    codelab harness to read the new fields.
- Verification:
  - `ohos-opengl-native` rebuilt `renderer_frontend.cpp`,
    `gesture_handler.cpp`, `map_view.cpp`, and `native_module.cpp`, then
    linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same sources, then linked
    `libmaplibre_native_ohos.so`.
  - Both native builds emitted the known unused `--gcc-toolchain` warning.
  - The first HAP rebuilds caught an ArkTS strictness issue: the new
    `surfaceState` local needed an explicit `SurfaceState` annotation because
    the SDK treats NAPI imports as unverified. The committed sample and ignored
    harness were updated to import and use that type explicitly.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 639 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `3 s 735 ms`.
  - The HAP builds emitted the same NAPI module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 MapObserver callback diagnostics

- Extended OHOS surface-state readback with core `MapObserver` callback
  diagnostics for first-device debugging.
  - `MapView` now tracks whether `onDidFinishLoadingStyle()`,
    `onDidFinishLoadingMap()`, and `onDidBecomeIdle()` have fired for the
    current surface/style lifecycle.
  - `MapView` exposes `Map::isFullyLoaded()` through the surface-state path
    when a map exists.
  - `MapView` tracks core render-frame callback count, the last frame's
    repaint/full-frame status, map-load error count, and render error count.
  - `getSurfaceState(...)` now returns `styleLoaded`, `mapLoaded`,
    `fullyLoaded`, `idle`, `lastFrameNeededRepaint`, `lastFrameComplete`,
    `coreFrameCount`, `mapLoadErrorCount`, and `renderErrorCount`.
  - Updated the ArkTS type package, README, committed sample, and ignored
    codelab harness to compile-test the expanded surface-state shape.
- Verification:
  - `ohos-opengl-native` rebuilt `gesture_handler.cpp`, `map_view.cpp`, and
    `native_module.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same sources, then linked
    `libmaplibre_native_ohos.so`.
  - Both native builds emitted the known unused `--gcc-toolchain` warning.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 897 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `3 s 979 ms`.
  - The HAP builds emitted the same NAPI module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 resource callback diagnostics

- Extended OHOS surface-state readback with lightweight resource callback
  counters for first-device debugging.
  - `MapView` now counts `MapObserver` source-change, style-image-missing,
    glyph request/load/error, tile action, and sprite request/load/error
    callbacks for the current surface/style lifecycle.
  - `getSurfaceState(...)` now returns `sourceChangedCount`,
    `styleImageMissingCount`, `glyphsRequestedCount`, `glyphsLoadedCount`,
    `glyphsErrorCount`, `tileActionCount`, `spritesRequestedCount`,
    `spritesLoadedCount`, and `spritesErrorCount`.
  - Updated the ArkTS type package, README, committed sample, and ignored
    codelab harness to compile-test the expanded resource diagnostic shape.
- Verification:
  - `ohos-opengl-native` rebuilt `gesture_handler.cpp`, `map_view.cpp`, and
    `native_module.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same sources, then linked
    `libmaplibre_native_ohos.so`.
  - Both native builds emitted the known unused `--gcc-toolchain` warning.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `4 s 11 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `4 s 132 ms`.
  - The HAP builds emitted the same NAPI module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 surface-state last-error strings

- Extended OHOS surface-state readback with last-error text for first-device
  debugging.
  - `MapView` now stores the last map-load error message reported through
    `onDidFailLoadingMap(...)`.
  - `MapView` now stores the last render error string reported through
    `onRenderError(...)`.
  - `getSurfaceState(...)` includes optional `lastMapLoadError` and
    `lastRenderError` properties when those strings are non-empty.
  - Updated the ArkTS type package, README, committed sample, and ignored
    codelab harness to compile-test the optional string properties.
- Verification:
  - `ohos-opengl-native` rebuilt `gesture_handler.cpp`, `map_view.cpp`, and
    `native_module.cpp`, then linked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-native` rebuilt the same sources, then linked
    `libmaplibre_native_ohos.so`.
  - Both native builds emitted the known unused `--gcc-toolchain` warning.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `platform/ohos/sample`; the build succeeded in `3 s 977 ms`.
  - The ignored codelab harness rebuilt with the same `hvigorw` command from
    `build-ohos-xcomponent-sample`; the build succeeded in `4 s 27 ms`.
  - The HAP builds emitted the same NAPI module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 command-line runtime tooling inspection

- Inspected the installed command-line tools under
  `/Users/sargunv/Downloads/command-line-tools`; no changes were made outside
  this repository.
- The bundle is enough for the current build/package path:
  - `bin/hvigorw` exposes build tasks such as `assembleApp`.
  - `bin/ohpm` exposes package install/update/list/run/cache commands.
  - `sdk/default/openharmony/toolchains/hdc` exposes install, uninstall,
    shell, hilog, and target-management commands.
- The installed DevEco 6 command-line native compilers are still Clang 15.0.4:
  - `sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang++`
    reports `OHOS (dev) clang version 15.0.4`.
  - `sdk/default/hms/native/BiSheng/bin/aarch64-unknown-linux-ohos-clang++`
    reports `OHOS (BiSheng Mobile STD 203.2.0.B048) clang version 15.0.4`.
- `hdc` still has no attached runtime target:
  - Running
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`
    returned `[Empty]`.
- The command-line SDK does contain previewer binaries:
  - `sdk/default/openharmony/previewer/common/bin/Previewer`
  - `sdk/default/openharmony/previewer/liteWearable/bin/Simulator`
  - `sdk/default/hms/previewer/liteWearable/bin/Simulator`
- The OpenHarmony `Previewer` binary is a macOS ARM64 host linked against
  previewer/mock libraries such as `libability_simulator.dylib`,
  `libpreviewer_window.dylib`, `libglfw_render_context.dylib`, and
  `libskia_canvaskit.dylib`.
- Its embedded strings point at an ArkUI preview workflow rather than a general
  app/device runtime:
  - It loads `module.json`, `modules.abc`, `resources.index`, and `resources`.
  - It initializes GLFW and a `previewer`/`preview_surface` window.
  - It reports `previewPath` and device-type adaptation messages.
- No separate emulator, QEMU, virtual-device, or `hdc` target-producing runtime
  binary was found in the command-line SDK search.
- Conclusion: the installed command-line tools are sufficient for
  configure/build/package, and should be enough to install/log/shell once a
  physical or emulator target appears in `hdc`; they do not appear sufficient by
  themselves to create a local OHOS runtime target for the native XComponent
  sample.

### 2026-05-30 issue refresh and build hygiene

- Refreshed GitHub issue
  `https://github.com/maplibre/maplibre-native/issues/3749`.
  - The issue is still open.
  - The latest issue update is `2025-11-12`.
  - The issue still has eight comments.
  - The actionable target context is unchanged: HarmonyOS 5.0+ phones/tablets,
    aarch64, GLES2/GLES3, ArkUI, and XComponent plus EGL for native rendering.
  - Maintainer feedback still supports documenting findings or opening a PR for
    build configuration changes.
- Added generated-tooling hygiene rules:
  - Root `/.hvigor` is ignored because invoking DevEco/Hvigor from the
    repository root can create that cache directory.
  - `platform/ohos/sample/oh_modules/` is ignored for OHPM dependency state.
- Inspected already-built native module ELF dependencies:
  - `build-ohos-native/libmaplibre_native_ohos.so` has SONAME
    `libmaplibre_native_ohos.so` and depends on `libace_napi.z.so`,
    `libace_ndk.z.so`, `libEGL.so`, `libGLESv3.so`,
    `libnative_window.so`, `libhilog_ndk.z.so`, `libimage_source.so`,
    `libnet_http.so`, `libpixelmap.so`, `libuv.so`, `libz.so`,
    `libc++_shared.so`, and `libc.so`.
  - `build-harmonyos-native/libmaplibre_native_ohos.so` has the same native
    module/rendering/image dependencies, but uses `librcp_c.so` instead of
    `libnet_http.so` for networking.
- Attempting `hdc shell aa help`, `hdc install -h`, or `hdc hilog -h` without a
  connected target returns `ExecuteCommand need connect-key`; only top-level
  `hdc -h` is available until `hdc list targets` reports a device/emulator.
- Verification:
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - `git ls-files -o --exclude-standard platform/ohos` shows only source,
    sample, type-package, and documentation files; generated `.hvigor`,
    `build`, `.cxx`, and future `oh_modules` content are ignored.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 ArkTS module binding type tightening

- Tightened the ArkTS type package so module-level map operations require an
  explicit `NativeBinding`.
  - The C++ NAPI resolver supports two call shapes:
    - Explicit binding calls such as
      `maplibreNative.setStyleJson(binding, json)`.
    - Legacy XComponent context method calls such as `context.setStyleJson(json)`,
      where the native resolver receives the XComponent context as `this`.
  - The previous `index.d.ts` exposed no-binding overloads on the module-level
    functions too, which allowed calls such as `maplibreNative.setStyleJson(json)`
    to type-check even though they would fail at runtime without a legacy
    XComponent context.
  - `XComponentContext` keeps its no-binding methods.
  - Exported module functions now require `NativeBinding` for map-specific
    operations; `runLoopOnce`, `DebugOptions`, `registerXComponentNode`, and
    `destroy(binding)` remain module-level utilities.
  - Updated the README with an explicit-binding `registerXComponentNode(node)`
    example and clarified that no-binding methods are only for the legacy
    XComponent onLoad context.
  - Synced the ignored codelab harness copy of the type package.
- Verification:
  - The committed sample rebuilt from clean with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw clean --no-daemon`
    followed by `hvigorw assembleApp --no-daemon`; the build succeeded in
    `3 s 422 ms` and reran `CompileArkTS`.
  - The ignored codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    from `build-ohos-xcomponent-sample`; the build succeeded in `4 s 56 ms`
    and reran `CompileArkTS`.
  - Both HAP builds emitted the known NAPI module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS/harness files
    found no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 ArkUI NodeController explicit binding sample

- Updated the committed sample app to create its map surface through
  `NodeContainer`/`NodeController` and `typeNode.createNode(..., 'XComponent',
  { type: XComponentType.SURFACE })`.
  - The sample now calls `registerXComponentNode(frameNode)` and stores the
    returned `NativeBinding`.
  - All map operations in the sample now use module-level explicit-binding
    calls such as `maplibreNative.setStyleJson(binding, json)`.
  - The sample destroys the explicit binding from `aboutToDisappear`.
  - This keeps the legacy `libraryname` XComponent path available and
    documented, but makes the committed sample compile-check the newer
    node-handle bridge.
- Updated the README's node-handle snippet to show a complete
  `NodeController`/`NodeContainer` example instead of an abstract `node`
  placeholder.
- Verification:
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `3 s 915 ms` and reran `CompileArkTS`.
  - The build emitted the known NAPI module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS files found
    no matches.

### 2026-05-30 binding lifecycle hardening

- Hardened the node-handle binding path ahead of device runtime testing.
  - Made the ArkTS `NativeBinding` type opaque with a private brand field so
    arbitrary objects no longer satisfy module-level binding parameters by
    structural typing.
  - Added `aboutToDisappear` and `onDetach` cleanup hooks to the sample
    `NodeController`, keeping explicit `destroy(binding)` idempotent with the
    page lifecycle cleanup.
  - Reordered native explicit-binding disposal so ArkUI touch/frame/surface
    callbacks are unregistered and holder user data is cleared before the
    `MapView` is destroyed.
  - Synced the ignored codelab harness copy of the ArkTS type package.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `platform/ohos/src/native_module.cpp` and linked
    `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt the same
    native module path and linked `libmaplibre_native_ohos.so`.
  - Both native builds emitted only the known unused
    `--gcc-toolchain=.../openharmony/native/llvm` warning.
  - The committed sample rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `4 s 599 ms` and reran native Ninja plus
    `CompileArkTS`.
  - The ignored codelab harness rebuilt with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `4 s 654 ms` and reran native Ninja plus
    `CompileArkTS`.
  - The HAP builds emitted the known NAPI module verification,
    Java `sun.misc.Unsafe::arrayBaseOffset`, and unsigned-output warnings as
    before.
  - `git diff --check` passed.
  - A trailing-whitespace scan across the touched source/log/OHOS files found
    no matches.
  - The committed ArkTS type package and ignored harness type copy compare
    equal.
  - Checked for an attached OHOS target again with
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
    the output is still `[Empty]`.

### 2026-05-30 HTTP and image backend audit

- Re-read the OpenHarmony `net_http` backend against the SDK headers.
  - The callback-slot table is still needed because `net_http` response/data/
    cancel callbacks do not expose a user-data pointer.
  - The request wrapper releases its slot on cancel/completion, destroys copied
    response bodies after converting to `mbgl::Response`, and suppresses the
    MapLibre callback if the `AsyncRequest` is canceled before the run loop
    delivers the response.
  - No code change was made in this pass; without a target, changing the
    completion/cancel ordering would be speculative and could make the current
    cancellation semantics worse.
- Re-read the HMS RCP backend against the HMS SDK headers.
  - At this point RCP still used a raw callback context and depended on RCP
    delivering a callback after `HMS_Rcp_CancelRequest`.
  - The later HMS/RCP callback lifetime cleanup superseded that design with
    stable callback slots.
- Re-read the OHOS ImageSourceNative/PixelmapNative decoder.
  - The decoder requests `PIXEL_FORMAT_RGBA_8888`, accepts `RGBA_8888` and
    `BGRA_8888`, honors row stride, swizzles BGRA to RGBA, and premultiplies
    only when the pixel map reports `PIXELMAP_ALPHA_TYPE_UNPREMULTIPLIED`.
  - No code change was made in this pass; real PNG/JPEG/WebP samples still need
    device-side validation.

### Remaining gaps

- Runtime is not tested on a device or emulator.
- No runtime-proven production render-loop integration is present yet.
  - The committed minimal sample packages a node-handle XComponent HAP, and the
    ignored codelab harness still packages a legacy `libraryname` XComponent
    HAP, but neither has run on a device/emulator.
  - The NAPI/XComponent native module now creates `mbgl::Map`, has
    manual/frame-callback rendering, exposes surface-state including
    pending-render/rendered-frame, core MapObserver callback diagnostics, and
    resource callback counters plus last-error strings, desired style readback,
    pixel-ratio, client/resource options plus readback, debug overlays, camera
    readback, and `jumpTo`
    center/zoom/bearing/pitch control, compile-tests one-finger pan,
    two-finger pinch/rotate, double-tap zoom, and fling input for both
    XComponent paths, exposes tile-cache controls plus
    `setFrameRateRange`/`getFrameRateRange`, `setRenderingEnabled`/
    `getRenderingEnabled`, pixel-ratio readback, and `reduceMemoryUse`
    lifecycle hooks, and has explicit destroy for node-handle bindings. It also
    exposes object-based camera updates plus `easeTo` and `flyTo`, map bounds
    controls, and raw free-camera read/write, but still needs runtime
    validation, production app lifecycle integration, and a polished
    ArkTS-facing API.
- Image decoding compiles against OHOS NDK APIs but has not been validated with
  real PNG/JPEG/WebP inputs on device.
- Network loading now has compile-tested implementations, but still needs
  runtime validation:
  - OpenHarmony default: NetworkKit `net_http` backend with callback-slot dispatch.
  - HarmonyOS/HMS option: RCP backend, proprietary SDK-specific.
- The link smoke target proves native shared-library link closure for the
  current `mbgl-core` target and selected platform sources.
- The NAPI/XComponent native-module target proves NAPI/XComponent/native-window
  link closure, and the ignored codelab harness proves ArkTS/HAP packaging for
  the native module. Neither proves emulator/device behavior or a correct
  production MapLibre render lifecycle.

### 2026-05-30 Werror re-check

- Clean rebuilt the OpenHarmony and HarmonyOS native-module presets before
  changing warning policy. Both native builds completed successfully with
  `MLN_WITH_WERROR=OFF`, and the only repeated diagnostic was the SDK-injected
  `--gcc-toolchain=.../openharmony/native/llvm` argument:
  `[-Wunused-command-line-argument]`.
- Removed the OHOS preset and sample CMake `MLN_WITH_WERROR=OFF` overrides.
  Added an OHOS-only `-Wno-unused-command-line-argument` compile option for C
  and C++ so the toolchain argument does not mask real warnings when
  warnings-as-errors are enabled.
- Reconfigured the existing OpenHarmony and HarmonyOS native-module build
  directories with `MLN_WITH_WERROR=ON`; both rebuilt and linked
  `libmaplibre_native_ohos.so` successfully.
- The shell used for this pass did not have `OHOS_SDK_NATIVE` or
  `HMOS_SDK_NATIVE` exported, so the HarmonyOS preset needed those SDK roots
  passed explicitly during reconfigure. The preset remains environment-driven;
  no machine-local SDK path was baked into tracked files.
- Rebuilt the committed sample app with
  `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`.
  The first post-clean run still used stale native `.cxx` CMake caches with
  `MLN_WITH_WERROR=OFF`, so both ABI caches were regenerated after unsetting
  that cache entry. The resulting caches report `MLN_WITH_WERROR=ON`, compile
  commands contain both `-Werror` and `-Wno-unused-command-line-argument`, and
  the sample `assembleApp` run succeeded in `1 min 13 s 607 ms`.
  The only remaining sample warnings were the known NAPI verification and
  unsigned-output warnings.

### 2026-05-30 preset environment hardening

- Rechecked issue 3749 on GitHub. It is still open, and there are no new
  technical requirements after the November 2025 comment. The original issue
  asks for official or documented OpenHarmony support; maintainer feedback
  invited a PR for build config or documentation.
- The current shell and `pixi run env` do not export `OHOS_SDK_NATIVE` or
  `HMOS_SDK_NATIVE`. Without those variables, CMake presets expand toolchain
  paths to `/build/cmake/...`, which can poison an existing build directory
  cache.
- Added CMake preset conditions so OpenHarmony presets are disabled unless
  `OHOS_SDK_NATIVE` is set, and HarmonyOS/HMS presets are disabled unless both
  `OHOS_SDK_NATIVE` and `HMOS_SDK_NATIVE` are set. This keeps the tracked
  presets environment-driven while preventing accidental empty SDK paths.
- Verified preset visibility after the condition change:
  - `pixi run cmake --list-presets` without SDK variables no longer lists the
    OpenHarmony or HarmonyOS configure presets.
  - With explicit `OHOS_SDK_NATIVE` and `HMOS_SDK_NATIVE`, CMake lists
    `ohos-opengl`, `ohos-opengl-core`, `ohos-opengl-link-smoke`,
    `ohos-opengl-native`, `harmonyos-opengl`,
    `harmonyos-opengl-link-smoke`, and `harmonyos-opengl-native`.
- Reconfigured all seven OpenHarmony/HarmonyOS presets above with SDK
  variables provided explicitly and `MLN_WITH_WERROR=ON`.
- Rebuilt the Werror-enabled link-smoke and core presets:
  - `ohos-opengl-link-smoke` linked `libmbgl-ohos-link-smoke.so`.
  - `harmonyos-opengl-link-smoke` linked `libmbgl-ohos-link-smoke.so`.
  - `ohos-opengl-core` linked `libmbgl-core.a`.
  - `ohos-opengl` linked `libmbgl-core.a` against the OpenHarmony
    NetworkKit HTTP backend.
  - `harmonyos-opengl` linked `libmbgl-core.a` against the HarmonyOS HMS RCP
    HTTP backend.
- `hdc list targets` from the command-line tools still reports `[Empty]`, so
  runtime validation remains pending until a device or emulator is available.

### 2026-05-30 packaging and host compatibility check

- Rebuilt the current OpenHarmony and HarmonyOS native-module targets after
  the preset environment hardening:
  - `ohos-opengl-native`: `ninja: no work to do`.
  - `harmonyos-opengl-native`: `ninja: no work to do`.
- Rebuilt the committed sample app with
  `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
  from `platform/ohos/sample`; it completed successfully in `1 s 254 ms`.
- Inspected
  `platform/ohos/sample/entry/build/default/outputs/default/entry-default-unsigned.hap`.
  It contains:
  - `libs/arm64-v8a/libmaplibre_native_ohos.so`
  - `libs/arm64-v8a/libc++_shared.so`
  - `libs/x86_64/libmaplibre_native_ohos.so`
  - `libs/x86_64/libc++_shared.so`
  - `module.json`
  - `pack.info`
- Verified both generated sample ABI CMake caches still have
  `MLN_WITH_WERROR:BOOL=ON`.
- Added integration docs noting that app modules should pass
  `-DOHOS_STL=c++_shared` through `externalNativeOptions` so the HAP packages
  the shared C++ runtime with the native module.
- Tried a host macOS Metal `mbgl-core` build to sanity-check the shared
  `mbgl::SourceLocation` compatibility change. The first attempt failed before
  compiling project code because the preset's `ccache` launcher is not present
  in the pixi environment. After reconfiguring only the generated
  `build-macos-metal` directory with empty compiler launchers, the build still
  failed in vendored ICU under pixi clang 22 with pre-existing `U_NOEXCEPT` and
  `TRUE`/`FALSE` errors before reaching touched MapLibre source files.
- Ran a focused host compile/run probe for `src/mbgl/util/source_location.hpp`
  with `pixi run c++ -std=c++20`; it passed. The transient probe binary was
  written to `/tmp/maplibre-source-location-probe` and then removed.
- Checked for an attached OHOS target again with
  `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
  the output is still `[Empty]`.

### 2026-05-30 API surface and first-run checklist

- Mechanically compared exported NAPI property names in
  `platform/ohos/src/native_module.cpp` with the top-level exported names in
  `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts`; there were
  no name mismatches.
- Rechecked the local `hdc -h` output. It confirms `install [-r|-s|-cwd] src`
  for `.hap`/`.hsp` packages, `hilog [-h]`, and `shell [COMMAND...]`.
- Added a first-run section to `platform/ohos/sample/README.md` with:
  - the package install command for
    `entry/build/default/outputs/default/app/entry-default.hap`;
  - the sample bundle name `org.maplibre.native.ohos.sample`;
  - the entry ability `EntryAbility`;
  - a candidate `aa start` launch command, explicitly conditional on the
    target shell providing `aa`;
  - the on-screen `SurfaceState` signals to check during first runtime
    validation.
- Attempting `hdc shell help` and `hdc hilog -h` without a connected target
  still fails with `ExecuteCommand need connect-key`, so target-side launch/log
  command behavior remains unverified.
- `hdc list targets` still reports `[Empty]`.

### 2026-05-30 build preset environment hardening

- Rechecked preset visibility after noticing that build presets were still
  listed without SDK environment variables even though configure presets were
  hidden.
- Added the same SDK environment conditions to the OpenHarmony and HarmonyOS
  build presets:
  - OpenHarmony build presets require `OHOS_SDK_NATIVE`.
  - HarmonyOS/HMS build presets require both `OHOS_SDK_NATIVE` and
    `HMOS_SDK_NATIVE`.
- Verified the behavior:
  - Without SDK variables, neither `pixi run cmake --list-presets` nor
    `pixi run cmake --build --list-presets` lists OpenHarmony/HarmonyOS
    presets.
  - With only `OHOS_SDK_NATIVE`, `pixi run cmake --build --list-presets` lists
    only the OpenHarmony build presets.
  - With both SDK variables, CMake lists all OpenHarmony and HarmonyOS
    configure/build presets.
- Re-ran the env-gated native-module build presets:
  - `ohos-opengl-native`: `ninja: no work to do`.
  - `harmonyos-opengl-native`: `ninja: no work to do`.
- `hdc list targets` still reports `[Empty]`.

### 2026-05-30 build preset inheritance cleanup

- Rechecked the CMake presets manual bundled in the pixi environment. Build
  presets support hidden base presets, inheritance, and inherited conditions.
- Replaced the duplicated environment conditions on each OpenHarmony/HarmonyOS
  build preset with two hidden base build presets:
  - `ohos-build`, requiring `OHOS_SDK_NATIVE`;
  - `harmonyos-build`, requiring both `OHOS_SDK_NATIVE` and `HMOS_SDK_NATIVE`.
- Re-verified the same visibility matrix:
  - no SDK variables: no OpenHarmony/HarmonyOS configure or build presets are
    listed;
  - only `OHOS_SDK_NATIVE`: only OpenHarmony build presets are listed;
  - both SDK variables: all OpenHarmony/HarmonyOS configure and build presets
    are listed.
- Re-ran the env-gated native-module build presets:
  - `ohos-opengl-native`: `ninja: no work to do`.
  - `harmonyos-opengl-native`: `ninja: no work to do`.
- `hdc list targets` still reports `[Empty]`.

### 2026-05-30 source-location header probe

- Rechecked the shared core compatibility patch around `mbgl::SourceLocation`.
- A focused compile/run probe for `src/mbgl/util/source_location.hpp` passed
  with `pixi run c++ -std=c++20`.
- Header compile probes for `src/mbgl/layout/symbol_instance.hpp` and
  `src/mbgl/renderer/bucket.hpp` passed once the repo's normal vendor include
  paths for `geometry.hpp`, `variant`, and `rapidjson` were supplied.
- Removed the transient probe outputs from `/tmp` after the checks.

### 2026-05-30 sample runtime observability

- Updated the committed sample ArkTS page to display a compact native
  `SurfaceState` readout below the XComponent. It now shows surface size,
  native-window/map presence, pending-render state, rendered/core frame counts,
  style/map load state, idle state, and the last native map/render error if one
  is available.
- The sample refreshes that state after initial configuration, page
  show/hide, manual render, and memory-reduction actions. This should make the
  first device/emulator run easier to diagnose without attaching a debugger.
- Rebuilt the sample with
  `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
  from `platform/ohos/sample`; it completed successfully in `3 s 925 ms`.
  The only warnings were the known NAPI verification warnings for
  `libmaplibre_native_ohos.so`, Java `Unsafe` warnings from the SDK packaging
  tool, and missing signing config warnings for the unsigned sample output.
- Rechecked the unsigned HAP contents after the sample UI change. It still
  packages `libmaplibre_native_ohos.so` and `libc++_shared.so` for both
  `arm64-v8a` and `x86_64`.
- Checked for an attached OHOS target again with
  `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/toolchains/hdc list targets`;
  the output is still `[Empty]`.

### 2026-05-30 sample remote-style runtime path

- Rechecked GitHub issue 3749. It is still open; the latest comment is from
  2025-11-12 and does not add new technical requirements beyond the existing
  HarmonyOS 5.0+, aarch64, GLES2/GLES3, ArkUI/XComponent/EGL direction.
- The sample app still starts with the empty inline style so first surface and
  render-loop validation does not depend on network access.
- Added `Remote` and `Empty` sample buttons. `Remote` switches to
  `https://demotiles.maplibre.org/style.json`; `Empty` switches back to the
  inline style. This gives a first device/emulator run an explicit way to
  exercise the OHOS HTTP backend plus glyph, tile, sprite, and image loading
  paths after the basic surface is known to work.
- Verified the remote style URL from this machine with `curl -I`; it returned
  HTTP 200 with `content-type: application/json; charset=utf-8`.
- Updated `platform/ohos/README.md` and `platform/ohos/sample/README.md` to
  document the remote-style runtime path and what to inspect after tapping it.
- Rebuilt the sample with
  `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
  from `platform/ohos/sample`; it completed successfully in `3 s 278 ms`.
  The warnings were unchanged: current NAPI import verification warnings,
  SDK Java `Unsafe` packaging warnings, and missing signing config warnings for
  the unsigned sample output.

### 2026-05-30 sample resource-counter readout

- Rechecked the sample diagnostics after adding the remote-style path. The
  native `SurfaceState` already exposed glyph, sprite, tile, source-change, and
  missing-image counters, but the sample page did not display them.
- Added a third compact status line to the sample UI so first device/emulator
  runs can inspect those counters directly after tapping `Remote`, without
  needing a debugger.
- Updated `platform/ohos/sample/README.md` to point first-runtime checks at the
  on-screen glyph/sprite/tile counters.
- Rebuilt the sample with
  `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
  from `platform/ohos/sample`; it completed successfully in `3 s 251 ms`.
  The warnings were unchanged: current NAPI import verification warnings,
  SDK Java `Unsafe` packaging warnings, and missing signing config warnings for
  the unsigned sample output.

### 2026-05-30 HMS/RCP callback lifetime cleanup

- Re-read both OHOS HTTP backends against the SDK headers and current code.
  The public `net_http` backend already releases its callback slot and
  self-reference immediately on explicit cancel, so the older cancellation leak
  note for that path is obsolete.
- Tightened the optional HarmonyOS/HMS RCP backend so it no longer passes
  `RequestState*` directly as the SDK callback context. RCP callbacks now use
  stable slot contexts, and the slot table owns only weak references to active
  request state.
- Explicit RCP cancel now marks the request finished, clears the MapLibre
  callback, calls `HMS_Rcp_CancelRequest`, destroys the request, releases the
  slot, and drops the self-reference immediately. If the SDK reports a late
  cancellation/response callback, the static context still exists but the slot
  is empty, so the callback destroys only the SDK response object.
- This removes the compile-visible dependency on RCP always delivering a
  cancellation callback. It still needs runtime validation because request
  destruction immediately after `HMS_Rcp_CancelRequest` is only verified
  against the SDK headers and successful linking, not a live HarmonyOS runtime.
- Rebuilt the targets that cover the changed backend:
  - `ohos-opengl-native`: `ninja: no work to do`.
  - `harmonyos-opengl-native`: rebuilt `http_file_source_hms_rcp.cpp`, relinked
    `libmbgl-core.a`, and relinked `libmaplibre_native_ohos.so`.

### 2026-05-30 HTTP status cleanup and preset rebuild

- Cleaned up stale HTTP lifetime notes in this log so the current status
  snapshot reflects the latest state:
  - public `net_http` cancellation releases its callback slot/self-reference
    immediately;
  - optional HMS/RCP cancellation uses stable callback slots rather than a raw
    `RequestState*` callback context.
- Rebuilt the remaining separate CMake preset directories that cover the RCP
  backend:
  - `harmonyos-opengl`: rebuilt `http_file_source_hms_rcp.cpp` and relinked
    `libmbgl-core.a`;
  - `harmonyos-opengl-link-smoke`: rebuilt `http_file_source_hms_rcp.cpp`,
    relinked `libmbgl-core.a`, and relinked `libmbgl-ohos-link-smoke.so`;
  - `ohos-opengl-link-smoke`: `ninja: no work to do`.

### 2026-05-30 API/package consistency re-check

- Mechanically compared the NAPI properties exported from
  `platform/ohos/src/native_module.cpp` with the top-level exports declared in
  `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts`.
  - Native exports: 36.
  - ArkTS type exports: 36.
  - No native-only or type-only export names were found.
- Mechanically compared `maplibreNative.*` calls in the committed sample page
  against the ArkTS type package.
  - The sample currently exercises 34 exported names.
  - No sample-only undeclared calls were found.
- Rechecked the committed sample HAP contents at
  `platform/ohos/sample/entry/build/default/outputs/default/app/entry-default.hap`.
  It still contains:
  - `libs/arm64-v8a/libmaplibre_native_ohos.so`
  - `libs/arm64-v8a/libc++_shared.so`
  - `libs/x86_64/libmaplibre_native_ohos.so`
  - `libs/x86_64/libc++_shared.so`
  - `module.json`
  - `pack.info`
- The committed ArkTS type package and ignored XComponent harness metadata copy
  still compare equal.

### 2026-05-30 HMS/RCP session lifetime cleanup

- Tightened the optional HarmonyOS/HMS RCP backend teardown path.
  - Before this cleanup, `HTTPFileSource::Impl` owned the raw `Rcp_Session*`
    while each outstanding `AsyncRequest` stored only a borrowed session
    pointer for cancellation.
  - That relied on the file source outliving every request destructor. If a
    teardown path ever closed the RCP session first, a later request cancel
    could pass a stale session pointer back to `HMS_Rcp_CancelRequest`.
  - Added a small shared `RcpSession` wrapper around session create, fetch,
    cancel, and close.
  - `HTTPFileSource::Impl` and each `RequestState` now share that wrapper, so
    the session stays alive until outstanding request state has released it.
  - The RCP response callback object now uses field-named initialization
    instead of positional aggregate initialization against the SDK C struct.
  - This is still compile/link validation only; live HarmonyOS runtime testing
    must confirm that canceling and destroying an RCP request in this order is
    accepted by the SDK.
- Verification:
  - `harmonyos-opengl-native` rebuilt `http_file_source_hms_rcp.cpp`, relinked
    `libmbgl-core.a`, and relinked `libmaplibre_native_ohos.so`.
  - `harmonyos-opengl-link-smoke` rebuilt the same source, relinked
    `libmbgl-core.a`, and relinked `libmbgl-ohos-link-smoke.so`.
  - `harmonyos-opengl` rebuilt the same source and relinked `libmbgl-core.a`.
  - After the field-named callback-object cleanup, the same three HarmonyOS
    build presets rebuilt successfully again.
  - A focused `clang-tidy` attempt on the RCP source was not usable as a gate:
    the pixi-host clang-tidy invocation could not resolve the OHOS C++ standard
    library include path and also promoted style diagnostics from HMS SDK
    headers to fatal warnings.

### 2026-05-30 sample phone/tablet packaging

- Rechecked the committed sample metadata against the current issue target.
  Issue #3749's actionable target context is HarmonyOS 5.0+ phones/tablets,
  but the sample manifest only declared `phone`.
- The installed OpenHarmony/HMS SDK schemas list `tablet` as a valid
  `deviceTypes` value.
- Updated `platform/ohos/sample/entry/src/main/module.json5` to declare both
  `phone` and `tablet`.
- Updated `platform/ohos/sample/README.md` to state that the sample targets
  both device types.
- Verification:
  - Rebuilt the committed sample with
    `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`;
    the build succeeded in `4 s 152 ms`.
  - The usual warnings remain: NAPI import verification warnings for
    `libmaplibre_native_ohos.so`, SDK Java `sun.misc.Unsafe::arrayBaseOffset`
    warnings from the packing tool, and missing signing config warnings.
  - The packaged HAP `module.json` now contains
    `"deviceTypes":["phone","tablet"]`.
  - The packaged HAP still contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`.
  - The packaged HAP still declares `ohos.permission.INTERNET`.

### 2026-05-30 OHOS HTTP backend consistency pass

- Compared the public OpenHarmony `net_http` backend and the optional
  HarmonyOS/HMS RCP backend against the existing default, Darwin, Android, Qt,
  and local-file range handling.
  - `Resource::dataRange` is an inclusive `[first, second]` byte range.
  - The public `net_http` backend keeps using `resumeFrom`/`resumeTo`.
  - The HMS RCP backend keeps emitting the same `Range: bytes=first-second`
    header used by the other network backends. The RCP C SDK also exposes
    `Rcp_TransferRange`, but using it for async requests would require an
    owned range object with SDK-specific lifetime rules; the explicit header is
    simpler and matches the Linux/default path.
- Centralized RCP response cache-header parsing into an `applyCacheHeaders`
  helper matching the public backend shape.
- Adjusted both OHOS HTTP backends so `Expires` is parsed first, then
  `Cache-Control: max-age` overrides it when present. This removes an
  HTTP-cache precedence edge case without changing status-code handling, range
  handling, conditional request headers, or response body ownership.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `http_file_source.cpp`, relinked `libmbgl-core.a`, and relinked
    `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `http_file_source_hms_rcp.cpp`, relinked `libmbgl-core.a`, and relinked
    `libmaplibre_native_ohos.so`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `4 s 163 ms`.
  - The HAP still packages `libmaplibre_native_ohos.so` and `libc++_shared.so`
    for both `arm64-v8a` and `x86_64`, declares `phone` and `tablet`, and
    declares `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched HTTP sources or this log.
  - `hdc list targets` still returned `[Empty]`; no runtime validation was
    possible yet.

### 2026-05-30 shared-library dependency and signing audit

- Rechecked the live upstream issue state:
  - `maplibre/maplibre-native#3749` is still open.
  - Its newest comment is still the November 12, 2025 thumbs-up; no newer
    technical findings were found.
  - A pull request search for `ohos`, `harmony`, and `openharmony` found no
    relevant OpenHarmony/HarmonyOS PRs.
- Audited the dynamic dependencies of the native-module shared libraries with
  the OHOS SDK `llvm-readobj --needed-libs`.
  - `build-ohos-native/libmaplibre_native_ohos.so` is `elf64-littleaarch64`
    and links the expected platform libraries, including `libnet_http.so`.
  - `build-harmonyos-native/libmaplibre_native_ohos.so` is
    `elf64-littleaarch64` and has the same dependency shape except that
    `librcp_c.so` replaces `libnet_http.so`.
  - The committed sample's arm64-v8a and x86_64 CMake outputs both use the
    public `libnet_http.so` backend and link only platform libraries plus
    `libc++_shared.so`.
- Rechecked the sample HAP contents:
  - The HAP packages `libmaplibre_native_ohos.so` and `libc++_shared.so` for
    both `arm64-v8a` and `x86_64`.
  - `pack.info` reports bundle `org.maplibre.native.ohos.sample`, device types
    `phone` and `tablet`, compatible API 21, and target API 21.
  - The generated HAP has no signature files. This matches the hvigor warning
    `No signingConfig found for product default`; a real install may require a
    DevEco/Harmony signing profile or post-build signing.
- Documentation cleanup:
  - `platform/ohos/README.md` now records the expected native-module dynamic
    dependencies for the public OpenHarmony and HarmonyOS/HMS variants.
  - `platform/ohos/sample/README.md` now notes the current unsigned package
    state before the `hdc install` path.

### 2026-05-30 surface lifecycle diagnostics

- Re-read the ArkUI node-handle XComponent lifecycle path in
  `platform/ohos/src/native_module.cpp`.
  - The explicit `destroy(binding)` path unregisters node touch and frame
    callbacks, removes the surface callback, clears holder user data, disposes
    the callback/holder objects, clears the MapLibre surface, resets the native
    map view, and nulls the external binding handle.
  - The existing cleanup path is reasonable for first-device testing, so this
    pass focused on making failures easier to see when the surface never gets
    far enough to create a map.
- Added binding-level surface diagnostics to `getSurfaceState(...)`:
  - `surfaceCreatedCount`
  - `surfaceChangedCount`
  - `surfaceDestroyedCount`
  - `surfaceErrorCount`
  - optional `lastSurfaceError`
- `lastSurfaceError` captures exceptions thrown while creating/updating the
  `OHNativeWindow`/EGL-backed map surface. This covers failures that happen
  before `MapObserver::onRenderError(...)` or map load callbacks can run, so
  the sample can now show EGL/native-window setup failures on screen instead of
  relying only on `hdc hilog`.
- Updated the ArkTS type package, committed sample page, and README files to
  include the expanded surface-state shape.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `native_module.cpp` and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `native_module.cpp` and relinked `libmaplibre_native_ohos.so`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `4 s 223 ms`; the usual NAPI
    verification, Java `Unsafe`, and missing signing config warnings remain.
  - Native NAPI exports and ArkTS type exports still match: 36 names each, no
    native-only or type-only exports.
  - The rebuilt HAP still packages `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, still declares
    `phone` and `tablet`, and still declares `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched native module, ArkTS type
    file, sample page, README files, or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 legacy XComponent cleanup parity

- Rechecked the legacy `libraryname` XComponent path against the public SDK
  headers.
  - The SDK exposes `OH_NativeXComponent_UnregisterOnFrameCallback(...)`.
  - The SDK does not expose a matching unregister function for the legacy
    `OH_NativeXComponent_Callback` surface/touch callback struct.
- Added cleanup parity for the legacy XComponent context:
  - `map.destroy()` now works when called as a no-binding method on the legacy
    XComponent context.
  - For the explicit ArkUI node-handle path, `maplibreNative.destroy(binding)`
    remains unchanged.
  - Legacy context cleanup clears the MapLibre surface/map state, unregisters
    the legacy frame callback when possible, and erases the binding from the
    module's legacy binding map. If the platform later invokes the still-static
    legacy surface callback, it finds no binding and no-ops.
  - The `XComponentContext` ArkTS type now declares `destroy: () => void`.
  - `platform/ohos/README.md` now documents the legacy cleanup behavior and the
    public SDK's missing legacy surface-callback unregister API.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `native_module.cpp` and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `native_module.cpp` and relinked `libmaplibre_native_ohos.so`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `3 s 55 ms`; the usual Java
    `Unsafe` and missing signing config warnings remain.
  - Native NAPI exports and ArkTS top-level exports still match: 36 names each,
    no native-only or type-only exports.
  - The `XComponentContext` type includes `destroy`.
  - `llvm-readobj --dyn-symbols build-ohos-native/libmaplibre_native_ohos.so`
    shows both `OH_NativeXComponent_RegisterOnFrameCallback` and
    `OH_NativeXComponent_UnregisterOnFrameCallback` as dynamic symbols.
  - The rebuilt HAP still packages `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, still declares
    `phone` and `tablet`, and still declares `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched native module, ArkTS type
    file, README, or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 legacy destroy idempotence

- Rechecked the cleanup behavior after adding legacy `map.destroy()` support.
  The explicit ArkUI node-handle path already nulls the external native binding
  handle after `destroy(binding)`, so a second explicit destroy call is a no-op.
- Split legacy context detection from live legacy binding lookup:
  - `getLegacyComponent(...)` now resolves the legacy XComponent from the N-API
    `this` value without requiring the module to still have a live binding.
  - `getLegacyBinding(...)` now delegates to `getLegacyComponent(...)` and then
    looks up the live binding.
  - The method-form legacy `map.destroy()` now treats a recognized legacy
    XComponent context with no remaining binding as an already-destroyed context
    and returns without throwing.
  - Other no-binding legacy methods still require a live binding through the
    normal resolver and continue to throw after destroy.
- Updated `platform/ohos/README.md` to document that calling
  `map.destroy()` again on the same legacy XComponent context is a no-op.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `native_module.cpp` and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `native_module.cpp` and relinked `libmaplibre_native_ohos.so`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `3 s 106 ms`; the usual Java
    `Unsafe` and missing signing config warnings remain.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS module interface still has 36 exported members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched native module, README, or
    this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 API consistency and legacy frame teardown pass

- Re-audited the native N-API surface against the ArkTS declaration file and
  sample usage.
  - `runLoopOnce()` is intentionally global: the native implementation only
    drains `mbgl::util::RunLoop::Get()->runOnce()` and does not require a
    binding.
  - Module-level map operations still require an explicit `NativeBinding`.
  - Legacy XComponent context methods still resolve their binding from `this`.
- Tightened legacy frame callback cleanup:
  - Added `legacyFrameRegistered` to `SurfaceBinding`.
  - Set it only when `OH_NativeXComponent_RegisterOnFrameCallback(...)`
    succeeds.
  - `map.destroy()` now calls `OH_NativeXComponent_UnregisterOnFrameCallback(...)`
    only for a binding whose legacy frame callback was actually registered, so
    teardown warnings now point at real unregister failures instead of earlier
    register failures.
- Broadened the verification matrix beyond the sample path:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt and relinked
    `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt and
    relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset ohos-opengl` rebuilt public-HTTP
    `mbgl-core`.
  - `pixi run cmake --build --preset harmonyos-opengl` rebuilt HMS/RCP
    `mbgl-core`.
  - `pixi run cmake --build --preset ohos-opengl-link-smoke` rebuilt
    `mbgl-core` and linked `libmbgl-ohos-link-smoke.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-link-smoke` rebuilt
    `mbgl-core` and linked `libmbgl-ohos-link-smoke.so`.
  - `pixi run cmake --build --preset ohos-opengl-core` reported
    `ninja: no work to do`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `3 s 160 ms`; the usual Java
    `Unsafe` and missing signing config warnings remain.
- Artifact checks:
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`.
  - The HAP contains `module.json`, `pack.info`, and `ets/modules.abc`.
  - `module.json` still declares `deviceTypes` `phone` and `tablet`.
  - `module.json` still declares `ohos.permission.INTERNET`.
  - `llvm-readobj --dyn-symbols build-ohos-native/libmaplibre_native_ohos.so`
    shows both legacy and ArkUI frame callback register/unregister symbols.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS module interface still has 36 exported members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched native module, README, or
    this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 pinch rotation anchor pass

- Rechecked the OHOS gesture bridge after older notes in this log mentioned
  incomplete gesture coverage.
  - The current bridge already handles one-finger pan, double-tap zoom, fling,
    two-finger pinch zoom, and two-finger rotation for both legacy
    `OH_NativeXComponent_TouchEvent` and ArkUI `ArkUI_UIInputEvent` paths.
  - The remaining issue found in this pass was rotation anchoring: pinch zoom
    and pinch pan used the current pinch center, but pinch rotation changed
    bearing through `Map::rotateBy(...)`, whose core transform rotates around
    the map's viewport center.
- Updated OHOS `MapView::rotateBy(...)` to accept the current pinch center and
  apply the incremental bearing change with
  `CameraOptions().withBearing(...).withAnchor(...)`.
  - The sign matches the existing `Transform::rotateBy(...)` behavior by
    subtracting the screen-space angle delta from the current camera bearing.
  - `gesture_handler.cpp` now passes the pinch center into `MapView::rotateBy(...)`,
    so two-finger zoom, rotation, and pan all use the same gesture focal point.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `gesture_handler.cpp`, `map_view.cpp`, `native_module.cpp`, and relinked
    `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `gesture_handler.cpp`, `map_view.cpp`, `native_module.cpp`, and relinked
    `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset ohos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `pixi run cmake --build --preset harmonyos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `3 s 217 ms`; the usual Java
    `Unsafe` and missing signing config warnings remain.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS module interface still has 36 exported members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched gesture handler, map view
    files, or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 EGL ES2 fallback audit

- Rechecked the OHOS EGL backend against the intended GLES2/3 device target.
  The backend previously requested only an OpenGL ES 3 config and an ES 3
  context.
- Added an ES2 fallback while preserving ES3 as the preferred path:
  - `DisplayConfig` now first chooses an `EGL_OPENGL_ES3_BIT` window config.
  - If no ES3 config is available, it chooses an `EGL_OPENGL_ES2_BIT` config and
    logs that it is falling back to ES2.
  - If ES3 config selection succeeds but ES3 context creation fails, the backend
    retries with the ES2 config/context when one is available.
  - `eglCreateWindowSurface(...)` uses the config that matched the successfully
    created context.
- Also checked whether the module could link against `libGLESv2.so` to make the
  fallback fully reflected in dynamic dependencies.
  - The OHOS SDK ships both `libGLESv2.so` and `libGLESv3.so` for the configured
    architectures.
  - Temporarily switching the OHOS CMake link target from `GLESv3` to `GLESv2`
    failed under `--no-undefined` because `platform/linux/src/gl_functions.cpp`
    references ES3 entry points including `glReadBuffer`, `glDrawRangeElements`,
    `glTexImage3D`, and query APIs.
  - The link target was restored to `GLESv3`; the final change is therefore a
    runtime EGL config/context fallback, not a dynamic dependency change.
- Updated `platform/ohos/README.md` to describe EGL/GLES rendering as ES3
  preferred with ES2 fallback.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` re-ran CMake after the
    CMake link-target audit and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` re-ran CMake and
    reported `ninja: no work to do` for the final state.
  - `pixi run cmake --build --preset ohos-opengl-link-smoke` re-ran CMake and
    reported `ninja: no work to do`.
  - `pixi run cmake --build --preset harmonyos-opengl-link-smoke` re-ran CMake
    and reported `ninja: no work to do`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `1 s 250 ms`; the usual missing
    signing config warning remains.
  - The public native module still has the expected dynamic dependencies,
    including `libEGL.so`, `libGLESv3.so`, `libnet_http.so`, `libnative_window.so`,
    and `libc++_shared.so`.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - `llvm-readobj --dyn-symbols build-ohos-native/libmaplibre_native_ohos.so`
    still shows the expected EGL entry-point references.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS module interface still has 36 exported members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched EGL backend, OHOS CMake
    file, README, or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 input callback diagnostics

- Rechecked first-device observability for XComponent input and frame callback
  delivery.
  - The existing `SurfaceState` exposed render, surface, style, resource, and
    last-error state.
  - It did not show whether XComponent frame callbacks or touch callbacks were
    arriving, which would make a first device run harder to triage if gestures
    did not move the map.
- Added binding-level callback counters to `SurfaceState`:
  - `frameCallbackCount`: increments from both ArkUI node and legacy
    XComponent on-frame callbacks after resolving a live binding.
  - `touchEventCount`: increments from both ArkUI node and legacy XComponent
    touch callbacks after resolving a live binding.
  - `gestureHandledCount`: increments when the native gesture bridge reports
    that the touch event changed map state and requested a render.
- Updated the ArkTS type declaration, sample status readout, main OHOS README,
  and sample README so first device tests can inspect callback delivery without
  relying only on `hdc hilog`.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `native_module.cpp` and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `native_module.cpp` and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset ohos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `pixi run cmake --build --preset harmonyos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `3 s 564 ms`; the usual NAPI
    verification, Java `Unsafe`, and missing signing config warnings remain.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - The public native module still has the expected dynamic dependencies,
    including `libEGL.so`, `libGLESv3.so`, `libnet_http.so`, `libnative_window.so`,
    and `libc++_shared.so`.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS module interface still has 36 exported members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched native module, ArkTS type
    file, sample page, README files, or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 ArkTS type package lockfile pass

- Audited the sample's ArkTS type dependency for `libmaplibre_native_ohos.so`.
  The sample entry module already declared a local `file:` dependency on
  `platform/ohos/arkts/types/libmaplibre_native_ohos`, but the type package's
  `oh-package.json5` had an empty `version` field and there was no committed
  lockfile for the sample entry module.
- Updated the type package metadata:
  - `version` is now `0.1.0` instead of an empty string.
  - Added explicit `license` and `author` fields matching the sample package
    metadata.
- Ran `/Users/sargunv/Downloads/command-line-tools/bin/ohpm install` from
  `platform/ohos/sample/entry`.
  - `ohpm` accepted the local type package.
  - It created `platform/ohos/sample/entry/oh-package-lock.json5`.
  - The lockfile records `libmaplibre_native_ohos.so` version `0.1.0` with
    `registryType: "local"`.
  - `ohpm` created an ignored `entry/oh_modules/libmaplibre_native_ohos.so`
    symlink to the local type package.
- Updated the main OHOS README and the sample README to run the entry-module
  `ohpm install` step before `hvigorw assembleApp`.
- Verification:
  - `(cd entry && /Users/sargunv/Downloads/command-line-tools/bin/ohpm install)`
    succeeded from `platform/ohos/sample`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw clean assembleApp --no-daemon`
    succeeded from `platform/ohos/sample` in `1 min 31 s 120 ms`; `CompileArkTS`
    ran from scratch and the previous NAPI type-verification warning did not
    appear after the local type package was installed.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    then succeeded incrementally in `979 ms`; only the usual missing signing
    config warning remained.
  - `pixi run cmake --build --preset ohos-opengl-native` reported
    `ninja: no work to do`.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS module interface still has 36 exported members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched type package metadata,
    entry lockfile, README files, or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 ArkUI surface visibility diagnostics

- Rechecked the OpenHarmony SDK 6.0.1 XComponent header after the callback
  diagnostics pass.
  - In addition to create/change/destroy callbacks, the ArkUI node-handle
    `OH_ArkUI_SurfaceCallback` API exposes surface show/hide events.
  - These callbacks are useful for first-device validation because an
    XComponent can keep a native window and dimensions while the surface is
    hidden.
- Wired ArkUI surface show/hide events into the native binding:
  - `SurfaceState.surfaceVisible` tracks whether native frame rendering should
    run for the current surface.
  - `surfaceShownCount` and `surfaceHiddenCount` count the ArkUI show/hide
    callbacks.
  - ArkUI and legacy surface create/change paths mark the surface visible;
    surface destroy paths mark it hidden.
  - Frame-callback rendering now requires both `renderingEnabled` and
    `surfaceVisible`, so hidden ArkUI surfaces stop rendering even if frame
    callbacks continue.
  - A hide callback also ends any active gesture and resets the gesture state.
  - A show callback refreshes the native window/layout fallback and calls
    `updateSurface(...)`, allowing the map to render again when the surface
    becomes visible.
- Updated the ArkTS type package, sample status text, main OHOS README, and
  sample README to expose the new visibility diagnostics.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `native_module.cpp` and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `native_module.cpp` and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset ohos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `pixi run cmake --build --preset harmonyos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `4 s 506 ms`; the usual Java
    `Unsafe` and missing signing config warnings remain.
  - `llvm-readobj --dyn-symbols build-ohos-native/libmaplibre_native_ohos.so`
    shows the expected ArkUI surface show/hide callback setter references along
    with the legacy and ArkUI frame callback register/unregister references.
  - The public native module still has the expected dynamic dependencies,
    including `libEGL.so`, `libGLESv3.so`, `libnet_http.so`, `libnative_window.so`,
    and `libc++_shared.so`.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - The app bundle also packages `entry-default.hap` and `pack.info`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS module interface still has 36 exported members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched native module, ArkTS type
    file, sample page, README files, or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 GLES context version diagnostics

- Rechecked the EGL backend after adding ES3-preferred/ES2-fallback context
  creation.
  - The backend selected either an ES3 or ES2 context internally, but ArkTS
    runtime diagnostics could not report which path was actually used.
  - On a first device run, that makes it harder to tell whether rendering is
    using the intended ES3 path or the fallback path.
- Added a narrow diagnostic surface for the selected context client version:
  - `EGLWindowBackend` now stores the GLES client version that successfully
    created the EGL context.
  - `MapView::getGlesContextClientVersion()` returns that value when an
    EGL-backed map surface exists, otherwise `0`.
  - `SurfaceState.glesContextClientVersion` exposes the value to ArkTS.
  - The sample status line now includes `gl=<version>`.
  - The README documents `3` as the preferred ES3 context, `2` as the ES2
    fallback, and `0` as no EGL-backed map surface yet.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt the touched
    OHOS native module sources and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt the
    touched HMS/RCP native module sources and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset ohos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `pixi run cmake --build --preset harmonyos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `3 s 585 ms`; the usual Java
    `Unsafe` and missing signing config warnings remain.
  - The public native module still has the expected dynamic dependencies,
    including `libEGL.so`, `libGLESv3.so`, `libnet_http.so`, `libnative_window.so`,
    and `libc++_shared.so`.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS module interface still has 36 exported members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched EGL backend, map view,
    native module, ArkTS type file, sample page, README files, or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 resource error string diagnostics

- Rechecked the remote-style validation path in the OHOS `MapObserver`
  bridge.
  - The native state already counted glyph and sprite requests/loads/errors.
  - `onGlyphsError(...)` and `onSpriteError(...)` receive exception pointers,
    but the OHOS bridge discarded the exception text.
  - That would make a first device run harder to diagnose if the remote style
    exercises HTTP/image decoding and only increments a resource error counter.
- Added last-error strings for resource failures:
  - `MapView` now stores `lastGlyphsError` and `lastSpritesError`, reset with
    the rest of the per-style runtime state.
  - `SurfaceState` exposes optional `lastGlyphsError` and `lastSpritesError`
    properties when those strings are present.
  - The sample resource line now shows `glyph error: ...` or `sprite error: ...`
    ahead of the compact resource counters.
  - The sample's early run-loop probe treats these resource error strings as
    observed errors too.
  - The OHOS README and sample README now mention glyph/sprite last-error
    diagnostics.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `map_view.cpp`, `native_module.cpp`, and relinked
    `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `map_view.cpp`, `native_module.cpp`, and relinked
    `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset ohos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `pixi run cmake --build --preset harmonyos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `4 s 157 ms`; the usual Java
    `Unsafe` and missing signing config warnings remain.
  - The public native module still has the expected dynamic dependencies,
    including `libEGL.so`, `libGLESv3.so`, `libnet_http.so`, `libnative_window.so`,
    and `libc++_shared.so`.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS module interface still has 36 exported members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched map view, native module,
    ArkTS type file, sample page, README files, or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 missing style image id diagnostics

- Rechecked GitHub issue 3749 before continuing this pass.
  - The issue is still open.
  - The latest comment is from 2025-11-12 and does not add new technical
    integration requirements beyond the earlier HarmonyOS 5+/aarch64/GLES2+3/
    ArkUI XComponent notes.
- Rechecked the remote-style resource observer path in the OHOS bridge.
  - `onStyleImageMissing(...)` already received the missing image id from core,
    but the OHOS diagnostics only exposed a count.
  - On a first device run, that would make sprite/image failures harder to
    distinguish from benign missing optional icons in a style.
- Added last missing-style-image diagnostics:
  - `MapView` now stores the last missing style image id and resets it with the
    rest of the per-style runtime state.
  - `SurfaceState` exposes optional `lastStyleImageMissing` when an id is
    available.
  - The sample resource line now shows `missing image: <id>` when no glyph or
    sprite error string is present.
  - The sample's early run-loop probe now treats missing-image callbacks as
    observed resource activity.
  - The OHOS README and sample README now mention missing-style-image id
    diagnostics alongside the existing resource counters.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `gesture_handler.cpp`, `map_view.cpp`, `native_module.cpp`, and relinked
    `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `gesture_handler.cpp`, `map_view.cpp`, `native_module.cpp`, and relinked
    `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset ohos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `pixi run cmake --build --preset harmonyos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `4 s 520 ms`; the usual Java
    `Unsafe` and missing signing config warnings remain.
  - The OHOS native module still links `libnet_http.so`; the HarmonyOS native
    module still links `librcp_c.so`. Both variants still link `libEGL.so`,
    `libGLESv3.so`, `libnative_window.so`, `libimage_source.so`,
    `libpixelmap.so`, and `libc++_shared.so`.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - The rebuilt unsigned app package contains `entry-default.hap` and
    `pack.info`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS module interface still has 36 exported members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched map view, native module,
    ArkTS type file, sample page, README files, or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 HTTP error name diagnostics

- Rechecked the two OHOS `HTTPFileSource` implementations because remote-style
  loading is still one of the highest-risk unvalidated runtime paths.
  - The public OpenHarmony path uses NetworkKit `net_http`.
  - The optional HarmonyOS path uses HMS RemoteCommunicationKit RCP.
  - Both backends already preserve callback lifetime through stable slots and
    still need a real device/emulator to validate actual network behavior.
- Improved network failure diagnostics without changing request behavior:
  - NetworkKit errors now report the `OH_HTTP_*` SDK error name, numeric code,
    and requested URL in the `Response::Error` message.
  - RCP errors now report the documented `RCP_ERROR_*` name, numeric code, and
    requested URL in the `Response::Error` message.
  - Unknown SDK error codes still report the numeric code and requested URL.
  - Error reason classification, request headers, range handling, callbacks,
    and cancellation behavior were left unchanged.
  - The OHOS README and sample README now mention that network failures should
    include SDK error names and URLs.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `http_file_source.cpp`, relinked `libmbgl-core.a`, and relinked
    `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `http_file_source_hms_rcp.cpp`, relinked `libmbgl-core.a`, and relinked
    `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset ohos-opengl-link-smoke` rebuilt
    `http_file_source.cpp`, relinked `libmbgl-core.a`, and relinked
    `libmbgl-ohos-link-smoke.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-link-smoke` rebuilt
    `http_file_source_hms_rcp.cpp`, relinked `libmbgl-core.a`, and relinked
    `libmbgl-ohos-link-smoke.so`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `3 s 630 ms`; the usual Java
    `Unsafe` and missing signing config warnings remain.
  - The OHOS native module still links `libnet_http.so`; the HarmonyOS native
    module still links `librcp_c.so`. Both variants still link `libEGL.so`,
    `libGLESv3.so`, `libnative_window.so`, `libimage_source.so`,
    `libpixelmap.so`, and `libc++_shared.so`.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - The rebuilt unsigned app package contains `entry-default.hap` and
    `pack.info`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS module interface still has 36 exported members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched HTTP backends, README
    files, or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 image decoder error diagnostics

- Rechecked the OHOS image decoder after the earlier bounds-hardening pass.
  - It already rejects empty dimensions, short pixel reads, unsupported decoded
    formats, and unsafe row strides.
  - Its SDK call failures still reported only raw `Image_ErrorCode` numbers,
    and local validation failures did not include the decoded pixelmap metadata.
- Improved image decode failure messages without changing decode behavior:
  - `checkImageResult(...)` now reports stable ImageKit `IMAGE_*` error names
    with their numeric values when SDK calls fail.
  - Unsupported decoded pixel formats now report the SDK pixel format name,
    alpha type name, dimensions, and row stride.
  - Empty decoded dimensions, unsafe row strides, and short pixel reads now
    include the same decoded pixelmap metadata.
  - The decoder still requests `PIXEL_FORMAT_RGBA_8888`, still accepts
    `PIXEL_FORMAT_RGBA_8888` and `PIXEL_FORMAT_BGRA_8888`, still converts BGRA
    to RGBA, and still premultiplies only when the pixelmap reports
    `PIXELMAP_ALPHA_TYPE_UNPREMULTIPLIED`.
  - The OHOS README and sample README now mention ImageKit error names and
    decoded pixel metadata in image decoder failure messages.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt `image.cpp`,
    relinked `libmbgl-core.a`, and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `image.cpp`, relinked `libmbgl-core.a`, and relinked
    `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset ohos-opengl-link-smoke` rebuilt
    `image.cpp`, relinked `libmbgl-core.a`, and relinked
    `libmbgl-ohos-link-smoke.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-link-smoke` rebuilt
    `image.cpp`, relinked `libmbgl-core.a`, and relinked
    `libmbgl-ohos-link-smoke.so`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `3 s 139 ms`; the usual Java
    `Unsafe` and missing signing config warnings remain.
  - The OHOS native module still links `libnet_http.so`; the HarmonyOS native
    module still links `librcp_c.so`. Both variants still link `libEGL.so`,
    `libGLESv3.so`, `libnative_window.so`, `libimage_source.so`,
    `libpixelmap.so`, and `libc++_shared.so`.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - The rebuilt unsigned app package contains `entry-default.hap` and
    `pack.info`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS module interface still has 36 exported members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched image decoder or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 option setter no-op lifecycle cleanup

- Refreshed GitHub issue #3749 before continuing.
  - The issue is still open.
  - The only activity after the August 2025 technical notes is a
    2025-11-12 thumbs-up comment; no new platform constraints were added there.
- Rechecked the surface recreation path in the OHOS native module.
  - `SurfaceBinding` already preserves desired style kind, style payload, and
    style generation independently from the current `MapView`.
  - `updateSurface(...)` already resets the applied style generation when a map
    is first created or the native window changes, then calls
    `applyDesiredStyle(...)`.
  - No surface recreation patch was needed for style reapplication.
- Fixed a smaller lifecycle issue in repeated option setters.
  - Added a helper that compares the exposed `ResourceOptions` fields supported
    by the ArkTS API: `apiKey`, `cachePath`, and `assetPath`.
  - `setPixelRatio(...)` now returns early when the effective pixel ratio is
    unchanged, including the default no-map case.
  - `setClientOptions(...)` now returns early when the client name/version are
    unchanged, and skips creating a `MapView` for the empty default values.
  - `setResourceOptions(...)` now skips empty default updates and returns early
    when the exposed resource options would not change.
  - The changed setters no longer reset `appliedStyleGeneration` or reapply the
    style for no-op updates, avoiding unnecessary style reloads and map
    recreation.
- Verification:
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `native_module.cpp` and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `native_module.cpp` and relinked `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset ohos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `pixi run cmake --build --preset harmonyos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `3 s 261 ms`; the usual Java
    `Unsafe` and missing signing config warnings remain.
  - The OHOS native module still links `libnet_http.so`; the HarmonyOS native
    module still links `librcp_c.so`. Both variants still link `libEGL.so`,
    `libGLESv3.so`, `libnative_window.so`, `libimage_source.so`,
    `libpixelmap.so`, and `libc++_shared.so`.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - The rebuilt unsigned app package contains `entry-default.hap` and
    `pack.info`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS `MapLibreNativeModule` interface still has 36 members.
  - `git diff --check` passed.
  - No trailing whitespace was found in `native_module.cpp` or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 C++20 workaround cleanup

- Rechecked the cross-platform C++20 compatibility edits needed by the OHOS SDK
  6.0.1 command-line compiler.
  - The `mbgl::SourceLocation` shim remains local to MapLibre code and avoids
    adding replacement definitions inside namespace `std`.
  - The OHOS SDK still needs `-fexperimental-library` for libc++ ranges
    algorithms, including MapLibre core sources and `mlt-cpp`.
- Centralized the OHOS libc++ experimental flag in CMake:
  - Added `mbgl_enable_ohos_libcxx_experimental(target)` in the top-level
    CMake file.
  - Replaced separate `-fexperimental-library` blocks for `mbgl-core`,
    `mlt-cpp`, and `maplibre-native-ohos` with calls to that helper.
  - Kept the flag target-scoped instead of pushing MapLibre's warning policy
    into third-party CMake targets.
- Verification:
  - Reconfigured `ohos-opengl-native`, `harmonyos-opengl-native`,
    `ohos-opengl-link-smoke`, and `harmonyos-opengl-link-smoke`. The SDK
    toolchain still emits its CMake minimum-version deprecation warning.
  - `pixi run cmake --build --preset ohos-opengl-native` reported
    `ninja: no work to do`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` reported
    `ninja: no work to do`.
  - `pixi run cmake --build --preset ohos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - `pixi run cmake --build --preset harmonyos-opengl-link-smoke` reported
    `ninja: no work to do`.
  - The regenerated compile commands still contain `-fexperimental-library` for
    representative `mbgl-core`, `mlt-cpp`, and `maplibre-native-ohos` compile
    commands in both native variants.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `1 s 358 ms`; the missing signing
    config warning remains.
  - The OHOS native module still links `libnet_http.so`; the HarmonyOS native
    module still links `librcp_c.so`. Both variants still link `libEGL.so`,
    `libGLESv3.so`, `libnative_window.so`, `libimage_source.so`,
    `libpixelmap.so`, and `libc++_shared.so`.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - The rebuilt unsigned app package contains `entry-default.hap` and
    `pack.info`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - The native NAPI descriptor table still has 36 exports.
  - The ArkTS `MapLibreNativeModule` interface still has 36 members.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched CMake files or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 source-location source inventory cleanup

- Rechecked the CMake source inventory after adding
  `src/mbgl/util/source_location.hpp`.
  - The header was used by `symbol_instance.hpp`, but was not listed in
    `SRC_FILES`, so IDE/source-group metadata and target source inventory would
    miss the new internal utility header.
- Added `src/mbgl/util/source_location.hpp` to the top-level `SRC_FILES` list
  beside the other `src/mbgl/util` headers.
- Verification:
  - Reconfigured `ohos-opengl-native` and `harmonyos-opengl-native`; the SDK
    toolchain still emits its CMake minimum-version deprecation warning.
  - `pixi run cmake --build --preset ohos-opengl-native` reported
    `ninja: no work to do`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` reported
    `ninja: no work to do`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `1 s 460 ms`; the missing signing
    config warning remains.
  - The rebuilt unsigned HAP contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - The rebuilt unsigned app package contains `entry-default.hap` and
    `pack.info`.
  - `module.json` still declares `compileSdkVersion` `6.0.1.112`, `phone` and
    `tablet` device types, and `ohos.permission.INTERNET`.
  - `git diff --check` passed.
  - No trailing whitespace was found in `CMakeLists.txt` or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 sample cache path cleanup

- Rechecked sample-generated-file hygiene before touching runtime sample code.
  The sample-local `.gitignore` covers `.hvigor/`, `oh_modules/`, `build/`,
  `entry/.cxx/`, and `entry/build/`; root `.gitignore` covers a top-level
  `/.hvigor`. The generated `entry/oh-package-lock.json5` uses a relative path
  to the local ArkTS type package and does not embed command-line tool or SDK
  absolute paths.
- Replaced the sample's hardcoded
  `/data/storage/el2/base/maplibre-cache.db` resource cache path with a path
  derived from the current ability context:
  `this.getUIContext().getHostContext()?.cacheDir + '/maplibre-cache.db'`.
  This keeps the sample in the app sandbox without assuming the concrete
  storage prefix.
- Updated the OHOS README usage snippet and sample README to describe deriving
  the cache path from the ability `cacheDir`.
- Verification:
  - The OpenHarmony SDK 6.0.1 ArkTS declarations expose
    `UIContext.getHostContext(): common.Context | undefined`, and
    `common.Context.cacheDir: string`.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `4 s 98 ms`; the missing signing
    config warning remains.
  - `rg -n "/data/storage/el2/base|maplibre-cache\\.db|cacheDir|getHostContext"`
    now finds only the derived `cacheDir` paths and documentation references.
  - `git diff --check` passed.
  - No trailing whitespace was found in the touched ArkTS/README files or this
    log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 upstream issue and main recheck

- Rechecked GitHub issue
  `maplibre/maplibre-native#3749` with `gh issue view --comments`.
  - The issue is still open, with no labels, assignee, milestone, or linked
    pull request.
  - The last visible update is still a non-technical comment from
    2025-11-12. No new platform requirements or blocker reports were added
    after the original August 2025 OpenHarmony/HarmonyOS 5 discussion.
- Fetched `origin/main`.
  - Local branch base before fetch: `647636bf6115` (`Expose Metal texture from
    headless backend (#4267)`).
  - Current `origin/main` after fetch: `a4072d746775` (`Replace link with
    MapLibre Plugins Android (#4313)`).
  - There are 15 upstream commits newer than the current OHOS branch base.
    Notable potentially relevant core/platform changes include the Android
    renderer asynchronous deletion option, a core `BackendScope` lifetime fix,
    an `ImageManager` callback use-after-free guard, and fill-extrusion
    instancing work.
- Conflict/rebase risk check:
  - The newer upstream commits do not touch the currently modified OHOS
    integration files: top-level OHOS CMake entries, OHOS presets, Linux GL
    function table fallback, symbol guard source-location edits,
    `vendor/maplibre-tile-spec.cmake`, `platform/ohos`, or
    `src/mbgl/util/source_location.hpp`.
  - A future rebase should still rebuild because upstream core/shader changes
    can expose compile issues even without textual conflicts.

### 2026-05-30 DevEco 6 compiler recheck

- Checked the native compilers packaged in the installed DevEco 6 command-line
  tools.
  - OpenHarmony native compiler:
    `/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native/llvm/bin/clang++`
    reports `OHOS (dev) clang version 15.0.4`.
  - HarmonyOS HMS native package does not have `native/llvm/bin/clang++`; its
    compiler is under `native/BiSheng/bin/clang++` and reports
    `OHOS (BiSheng Mobile STD 203.2.0.B048) clang version 15.0.4`.
- Conclusion: DevEco 6 command-line tools did not resolve this branch's C++20
  issue by moving to a newer Clang/libc++ major version. The targeted
  `-fexperimental-library` workaround remains necessary for OHOS libc++ ranges
  algorithms in `mbgl-core`, `mlt-cpp`, and `maplibre-native-ohos`.

### 2026-05-30 default logging simplification

- Revisited the earlier platform-source choice in light of the preference to
  reuse Linux/default pieces wherever possible.
  - The tested OpenHarmony and HMS native SDK sysroots do not provide cURL,
    libpng, libjpeg, or libwebp, and this repository does not vendor those
    libraries for the default `HTTPFileSource` or image readers. Keeping the
    OHOS NetworkKit/HMS RCP HTTP backends and ImageSourceNative/PixelmapNative
    decoder is still justified.
  - Logging does not require an OHOS-specific backend. Switched `mbgl-core` to
    `platform/default/src/mbgl/util/logging_stderr.cpp` and removed the
    experimental `platform/ohos/src/logging_hilog.cpp` source plus the
    `hilog_ndk.z` native link dependency.
- Updated the OHOS README and sample README:
  - The README now says OHOS starts from the Linux/default source set and
    documents why HTTP and image decoding are the exceptions.
  - The expected dynamic dependency list no longer includes
    `libhilog_ndk.z.so`.
  - The sample runtime notes now refer to runtime logs generically instead of
    promising native `MapLibre` output through `hdc hilog`.
- Verification:
  - Reconfigured `ohos-opengl-native` and `harmonyos-opengl-native`; the SDK
    toolchain still emits its CMake minimum-version deprecation warning.
  - `pixi run cmake --build --preset ohos-opengl-native` rebuilt
    `logging_stderr.cpp`, `libmbgl-core.a`, and `libmaplibre_native_ohos.so`.
  - `pixi run cmake --build --preset harmonyos-opengl-native` rebuilt
    `logging_stderr.cpp`, `libmbgl-core.a`, and `libmaplibre_native_ohos.so`.
  - Reconfigured and rebuilt `ohos-opengl-link-smoke` and
    `harmonyos-opengl-link-smoke`; both linked successfully.
  - `/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon`
    rebuilt the sample app successfully in `3 s 375 ms`; the missing signing
    config warning remains.
  - The OHOS native module now links `libnet_http.so` and no longer links
    `libhilog_ndk.z.so`.
  - The HarmonyOS native module now links `librcp_c.so` and no longer links
    `libhilog_ndk.z.so`.
  - Both native variants still link `libEGL.so`, `libGLESv3.so`,
    `libnative_window.so`, `libimage_source.so`, `libpixelmap.so`, `libuv.so`,
    `libz.so`, `libace_napi.z.so`, `libace_ndk.z.so`, `libc++_shared.so`, and
    `libc.so`.
  - The rebuilt unsigned HAP still contains `libmaplibre_native_ohos.so` and
    `libc++_shared.so` for both `arm64-v8a` and `x86_64`, plus `module.json`,
    `pack.info`, and `ets/modules.abc`.
  - `git diff --check` passed.
  - No trailing whitespace was found in `platform/ohos/ohos.cmake`, the touched
    README files, or this log.
  - `hdc list targets` still returned `[Empty]`; runtime validation remains
    pending.

### 2026-05-30 API surface and command-line runtime audit

- Rechecked the OHOS NAPI/ArkTS API contract for drift.
  - The native `napi_property_descriptor` names in
    `platform/ohos/src/native_module.cpp` match the top-level ArkTS exports in
    `platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts`.
  - The native descriptor names also match the `MapLibreNativeModule`
    interface. The only repeated top-level type declaration is the expected
    `jumpTo` overload pair.
  - The legacy `XComponentContext` interface intentionally excludes
    `DebugOptions` and `registerXComponentNode`; it exposes no-binding methods
    for calls resolved through the legacy XComponent context.
  - The native `SurfaceState` object property set matches the ArkTS
    `SurfaceState` interface exactly, including optional last-error fields.
- Rechecked the local type package wiring:
  - `platform/ohos/arkts/types/libmaplibre_native_ohos/oh-package.json5`
    points `"types"` at `./index.d.ts`.
  - The sample entry package depends on that local type package through a
    relative `file:../../arkts/types/libmaplibre_native_ohos` path.
  - The generated `entry/oh-package-lock.json5` still contains only the
    relative local package path and no SDK/tool absolute paths.
- Probed the installed DevEco command-line runtime tools without launching a
  long-running UI session.
  - The bundle has `hdc`, OpenHarmony Previewer, and liteWearable Simulator
    binaries, but no general phone/tablet emulator binary was found.
  - `hdc list targets` still returns `[Empty]`.
  - The OpenHarmony Previewer binary does not print CLI help for `--help` or
    `-h` and stays running until killed.
  - The liteWearable Simulator rejects `--help` with an error that no `-j` app
    path was specified, and its embedded option strings describe a lightweight
    JS app preview flow rather than HAP installation or a phone/tablet
    HarmonyOS runtime.
- Conclusion: the command-line tools remain sufficient for configure, native
  build, ArkTS compile, HAP/app packaging, and static artifact inspection. They
  still do not provide a usable runtime validation target for this phone/tablet
  XComponent native module on this machine.

### 2026-05-30 MatePad signing, install, and first runtime crash

- Connected a Huawei MatePad Pro (`MRDI-W00`) on HarmonyOS `6.1.0.117` (API
  `23`) through `hdc`.
- The device was already registered in AppGallery Connect from an earlier auto-sign
  entry.
- AGC bundle naming constraint: `ohos` is not allowed in bundle IDs, so the
  sample bundle was renamed to `org.maplibre.native.demo` in
  `platform/ohos/sample/AppScope/app.json5`.
- Debug signing without DevEco Studio:
  - Generated `~/Downloads/maplibre-ohos-sample-debug.{csr,p12}` with
    `hap-sign-tool.jar` (`keyAlias` `maplibre_debug`, keystore password recorded
    in `~/Downloads/maplibre-ohos-sample-signing-credentials.txt`).
  - Uploaded the CSR in AGC, downloaded `debug.cer` and `debugDebug.p7b`, and
    copied them to `platform/ohos/sample/sign/maplibre-debug.{cer,p7b,p12}`.
  - OpenHarmony SDK community signing (`OpenHarmony.p12` +
    `UnsgnedDebugProfileTemplate.json`) still fails install on this retail
    HarmonyOS device with `code:9568257 fail to verify pkcs7 file`.
  - `hvigor` `SignHap` expects encrypted 32+ character passwords in
    `build-profile.json5`, so the current workflow is `hvigorw assembleApp`
    (unsigned HAP) followed by manual `hap-sign-tool.jar sign-app`. Steps are
    documented in `platform/ohos/sample/sign/README.md`.
- First signed install succeeded:

  ```sh
  hdc install -r platform/ohos/sample/sign/entry-default-signed.hap
  hdc shell aa start -b org.maplibre.native.demo -a EntryAbility
  ```

- First runtime result: the app opened briefly, then exited with a JS runtime
  error before any map rendering could be validated.
  - `hilog` shows `org.maplibre.native.demo is about to exit due to RuntimeError`
  - Error message: `Could not create XComponent surface holder callback`
  - ArkTS stack: `Index.ts` `makeNode` → `registerXComponentNode(node)`
  - Native stack includes `libmaplibre_native_ohos.so` from
    `registerXComponentNode()` when `OH_ArkUI_SurfaceHolder_Create(node)` or
    `OH_ArkUI_SurfaceCallback_Create()` returns null
  (`platform/ohos/src/native_module.cpp`).
- Interpretation: the committed sample used `NodeContainer` +
  `typeNode.createNode(..., 'XComponent')` + `registerXComponentNode()`. That
  ArkUI node-handle path compiles and links, but the MatePad runtime rejects
  surface-holder creation for the programmatic node.
- Mitigation in progress: switch the sample UI back to the legacy
  `libraryname: 'maplibre_native_ohos'` declarative `XComponent` `onLoad`
  context path for first device validation; keep `registerXComponentNode()` in the
  native module for follow-up API 23 / SurfaceHolder investigation.
- Still unvalidated on device after the crash: EGL map draw, gestures, `net_http`
  remote style loading, and ImageKit decode.

### 2026-05-30 MatePad Render-button crash diagnosis

- After switching the sample to legacy `libraryname` XComponent, install/launch succeeded and the UI stayed up
  until the `Render` button was tapped.
- `hilog` for the Render crash:

  ```text
  Error message: Camera for bounds requires an active map surface
  at anonymous (entry|entry|1.0.0|src/main/ets/pages/Index.ts:317:26)
  ```

  The native frame includes `libmaplibre_native_ohos.so` from `cameraForBounds()`.
- Earlier log lines from the same session show the underlying setup failure:

  ```text
  EGL_BAD_MATCH: pixel format (surface:0x<private> config:0x<private>)
  egl.eglCreateWindowSurface error.
  ```

  `updateSurface()` catches the resulting `eglCreateWindowSurface` exception, records
  `lastSurfaceError`, and leaves `hasMap=false`, but the sample called `cameraForBounds()` anyway.
  HarmonyOS treats the thrown NAPI error as fatal, so the process exits.
- Mitigations applied on the branch:
  - Sample `Render` handler checks `getSurfaceState().hasMap` and uses `fitBounds()` (deferred) instead of
    `cameraForBounds()` when no map exists.
  - `platform/ohos/src/egl_window_backend.cpp` now sets `SET_FORMAT` to `NATIVEBUFFER_PIXEL_FMT_RGBA_8888`,
    logs `GET_FORMAT`, and tries multiple EGL configs (RGBA8888, RGBX8888, RGB565) against the actual
    `OHNativeWindow` until `eglCreateWindowSurface` succeeds.
- Next device check: rebuild/reinstall the sample HAP, confirm `hasMap=yes` and nonzero frame counts without
  tapping `Render`, then retry `Render` and `Remote`.

### 2026-05-30 MatePad first successful map render

- Rebuilt/reinstalled the signed sample with legacy XComponent + EGL config probing.
- Device result: app stays up, status reports `Rendered: 1644x954 window=yes map=yes render=idle`.
- Visual result with debug options enabled in the sample:
  - Black map background with illegible white label-like geometry (debug `ParseStatus` /
    `TileBorders` overlays, not map style labels; OHOS `LocalGlyphRasterizer` still returns
    `canRasterizeGlyph=false`).
- `Remote` button result:
  - `map error: loading style failed: OpenHarmony HTTP request failed with OH_HTTP_OUT_OF_MEMORY (2300027) for https://demotiles.maplibre.org/style.json`
  - `glyphs=0/0/0 sprites=0/0/0 tiles=0`
  - `2300027` is the NetworkKit `OH_HTTP_OUT_OF_MEMORY` code from `net_http.h`, returned by the
    SDK during `OH_Http_CreateRequest` / header setup / `OH_Http_Request`, not from exhausting the
    128 callback slots in `http_file_source.cpp`.
- Sample change: `configureMap()` now passes `setDebugOptions(0)` so the next install should show a
  clean empty map without debug overlay text.
- Next experiments: retry `Remote` after reinstall; if OOM persists, try the `harmonyos-opengl-native`
  build with HMS RCP HTTP, or fetch a smaller test URL; confirm whether inline style tile borders
  look correct without debug mode.

### 2026-05-30 MatePad sample switched to HMS RCP HTTP

- Sample native build (`platform/ohos/sample/entry/src/main/cpp/CMakeLists.txt`) now sets
  `MLN_OHOS_WITH_HMS_RCP=ON`; hvigor already uses `hmos.toolchain.cmake` and links `librcp_c.so`
  (no `libnet_http.so` in `libmaplibre_native_ohos.so` NEEDED list).
- Rebuilt, signed, and installed `entry-default-signed.hap` on device.
- HMS RCP removed the `net_http` OOM on the style URL path (HTTP requests proceed), but
  **Remote** still crashes the process after a few seconds (see below).

### 2026-05-30 MatePad inline style rendering matrix

- Added sample style buttons and inline JSON in `platform/ohos/sample/entry/src/main/ets/pages/Index.ets`:
  - **Diag** — `background-color: #ff0000` only.
  - **Local** — GeoJSON polygon + `background-color: #004488` + `fill-color: #ffdd00`
    (fixed an extra `}` JSON typo that had caused `loading style failed` at offset 217).
  - **Empty** — no layers (black framebuffer).
- **Local** and **Diag** validate GLES background fill and vector fill without network.
- **Remote** still fails at runtime with a native crash (not a style-parse error):

  ```text
  Reason: Signal: SIGSEGV(SEGV_MAPERR) @ 0x0001000000000000
  Thread: OnlineFileSourc
  #00 strlen
  #01 librcp_c.so — std::string from null char*
  #02 librcp_c.so — RcpToCppConfiguration
  #03 librcp_c.so — RcpToCppRequest
  #04–#06 libmaplibre_native_ohos.so
  ```

- Crash capture command (device):

  ```sh
  HDC=/path/to/command-line-tools/sdk/default/openharmony/toolchains/hdc
  $HDC shell aa force-stop org.maplibre.native.demo
  ($HDC shell hilog > /tmp/maplibre-crash.hilog) &
  sleep 1
  $HDC shell aa start -b org.maplibre.native.demo -a EntryAbility
  # tap Remote, wait ~5s, kill hilog; grep CPP_CRASH librcp_c OnlineFileSourc
  ```

- Attempted fix (reverted on break): zero-initialize `Rcp_Request` optional pointers /
  attach a static empty `Rcp_Configuration` before `HMS_Rcp_Fetch`; plus
  `AUTO_LOAD_REMOTE_STYLE` for unattended repro — both removed; sample stays manual
  **Remote** only.
- `http_file_source_hms_rcp.cpp` still releases the session mutex before
  `HMS_Rcp_Fetch` / `HMS_Rcp_CancelRequest` to avoid callback deadlock (kept).
- Next session: fix Remote by fully initializing `Rcp_Request` per `rcp.h`, compare
  with HMS RCP samples, then confirm demotiles MVT/sprites/glyphs on device.
