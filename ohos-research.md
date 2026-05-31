# HarmonyOS / Future OpenHarmony Research Notes

This note compresses the original research log into the pieces a contributor
needs in order to understand the current HarmonyOS port direction, what has
been proven, and which OpenHarmony follow-ups are worth remembering.

## Current Status

- Branch: `ohos-research`.
- Tracking issue: `maplibre/maplibre-native#3749` remains open. As of the last
  check on 2026-05-30, no newer technical issue comments or PRs had appeared.
- Platform code lives under `platform/ohos/`.
- Sample app lives under `platform/ohos/sample`.
- Native module output: `libmaplibre_native_ohos.so`.
- ArkTS type package:
  `platform/ohos/sample/entry/types/libmaplibre_native_ohos`.
- Sample bundle id: `org.maplibre.native.demo`. AppGallery Connect rejected
  bundle ids containing `ohos`, so the original sample id was renamed.
- Device validation target so far:
  - Huawei MatePad Pro `MRDI-W00`
  - HarmonyOS `6.1.0.117`
  - API `23`

Confirmed on the MatePad:

- Legacy declarative `XComponent` using
  `libraryname: 'maplibre_native_ohos'` works.
- Vulkan rendering through `VK_OHOS_surface` works into the XComponent surface
  and is the desired/default renderer for the sample.
- EGL/GLES rendering also works as a fallback path.
- Remote styles work with the HarmonyOS HMS RCP HTTP backend after fixing RCP
  request/session lifetime and configuration initialization:
  - **Demo**: `https://demotiles.maplibre.org/style.json`
  - **Bright**: `https://tiles.openfreemap.org/styles/bright`
  - **Liberty**: `https://tiles.openfreemap.org/styles/liberty`
- Bright is the current default style. It exercises remote glyphs, sprites,
  vector tiles, raster source metadata, density scaling, frame pacing, and
  style-sourced attribution rendering.
- Style switching, background/foreground, resize, and the current gesture set
  have been smoke-tested on device.

Confirmed on DevEco emulator:

- Target: `127.0.0.1:5555`
- Reported model/version:
  - model `emulator`
  - API `22`
  - software `emulator 6.0.0.130(SP7DEVC00E130R4P11)`
- Existing signed HAP installed successfully.
- Legacy XComponent path launches and creates a map surface.
- Remote style loads without the previous RCP crash.
- Background and symbol layers render on the OpenGL sample renderer.
- A historical OpenGL fill/line rendering gap was reproduced and narrowed down:
  - The emulator window framebuffer reports complete status but no usable depth
    or stencil bits:
    `rgba=0/0/0/0 depth=0 stencil=0 status=0x8cd5 renderer=Mali-G77`.
  - An offscreen color/depth/stencil FBO plus blit did **not** make fill/line
    geometry appear.
  - Disabling GL stencil clipping for OHOS makes both **Remote** country
    fills/boundaries and the **Local** GeoJSON polygon render on the emulator.
- DevEco Studio 6.0.2's macOS emulator exposes the Vulkan loader and
  `VK_OHOS_surface`, but fails `vkCreateInstance` with
  `VK_ERROR_INCOMPATIBLE_DRIVER`. That matches Huawei's published note that
  Vulkan emulator support starts in DevEco Studio 6.1.0 Beta2.

Not yet fully validated:

- OpenHarmony public SDK build/runtime support. The current PR is intentionally
  HarmonyOS/HMS-only; public OpenHarmony paths can be restored after validation
  against that SDK.
- DevEco Studio 6.1.0 Beta2-or-newer Vulkan emulator runtime.
- Release/profile performance on the MatePad. Current debug-device correctness
  is good, but heavy styles can still feel rough.
- OpenGL fallback behavior and whether the OHOS stencil-clipping bypass should
  remain unconditional, be gated, or be documented as fallback/emulator-specific.
- ArkUI `NodeContainer` / `registerXComponentNode(node)` runtime path on the
  MatePad.

## Quick Build And Run

Native CMake presets:

```sh
export OHOS_SDK_NATIVE=/path/to/openharmony/native
export HMOS_SDK_NATIVE=/path/to/hms/native

pixi run cmake --preset harmonyos-opengl
pixi run cmake --build --preset harmonyos-opengl

pixi run cmake --preset harmonyos-vulkan
pixi run cmake --build --preset harmonyos-vulkan
```

Sample app:

```sh
cd platform/ohos/sample
/Applications/DevEco-Studio.app/Contents/tools/hvigor/bin/hvigorw assembleApp --no-daemon
/Applications/DevEco-Studio.app/Contents/tools/hvigor/bin/hvigorw assembleApp -p product=opengl --no-daemon
```

The preferred developer flow is DevEco Studio: open `platform/ohos/sample`,
select either the default Vulkan product or the `opengl` product, then use the
IDE's normal build/install/run actions.

Local signing is generated from ignored files in `platform/ohos/sample/sign/`:

```sh
cd platform/ohos/sample
node sign/generate-signing-config.mjs
```

The script writes `sign/signing.local.json` and `sign/material/`. Keep both
uncommitted. On the MatePad, command-line `aa start` fails with error
`10106102` if the screen is locked; manually unlock the device first.

## Architecture

The port intentionally starts from the Linux/default platform implementation
where that matches the OHOS SDK:

- Default/Linux-style pieces reused:
  - filesystem/local files
  - SQLite/offline storage
  - run loop and timers
  - compression
  - text utilities
  - headless frontend pieces
  - GL function loading with a small optional `GLES3/gl3ext.h` include guard
- OHOS-specific pieces:
  - EGL window backend for `OHNativeWindow *`
  - Vulkan window backend using `VK_OHOS_surface`
  - image decoding through ImageSourceNative/PixelmapNative
  - HTTP backends using platform SDK APIs
  - sample-scoped XComponent/NAPI surface integration

The main device renderer path is:

```text
ArkUI XComponent
  -> sample NAPI/XComponent bridge
  -> sample MapView
  -> OHNativeWindow
  -> VulkanWindowBackend
  -> RendererFrontend
  -> mbgl::Map
```

The EGL/GLES backend remains available as an OpenGL fallback and emulator
investigation path.

Important files:

- `platform/ohos/ohos.cmake`: platform source selection, SDK libraries, native
  libraries, and whole-archive helper used by the sample native module.
- `platform/ohos/src/egl_window_backend.*`: EGL/GLES context and surface
  management for `OHNativeWindow`.
- `platform/ohos/src/vulkan_window_backend.*`: Vulkan surface and swapchain
  integration for `OHNativeWindow`.
- `platform/ohos/src/renderer_frontend.*`: thin `RendererFrontend` bridge.
- `platform/ohos/src/http_file_source_hms_rcp.cpp`: HarmonyOS HMS
  RemoteCommunicationKit backend.
- `platform/ohos/src/image.cpp`: ImageKit image decoding.
- `platform/ohos/src/logging_hilog.cpp`: native MapLibre logging routed through
  hilog.
- `platform/ohos/sample/entry/src/main/cpp/map_view.*`: sample embedding
  adapter that owns the backend, frontend, and `mbgl::Map`; stores desired
  state across XComponent surface recreation.
- `platform/ohos/sample/entry/src/main/cpp/native_module.cpp`: sample
  NAPI/XComponent lifecycle and ArkTS exports.
- `platform/ohos/sample/entry/src/main/cpp/native_values.*`: sample NAPI value
  parsing/object creation.
- `platform/ohos/sample/entry/src/main/cpp/gesture_handler.*`: sample
  XComponent touch input to MapLibre gesture calls.

## Toolchain Findings

Initial command-line tools:

- Command Line Tools mac-arm64: `6.0.1.251`
- HarmonyOS SDK: `6.0.1 Release`
- OpenHarmony public SDK: `6.0.1.112`
- API version: `21 Release`
- Native target: `aarch64-unknown-linux-ohos`
- Clang: `15.0.4`
- Hvigor: `6.21.1`
- ohpm: `6.0.1`
- hdc: `3.2.0b`

DevEco Studio 6.0.2 notes checked later:

- Latest visible 6.0.2 Release line was `6.0.2.660`.
- It still reports compile SDK `6.0.2(22)`, Node `18.20.1`, ohpm
  `6.0.2.640`, Hvigor `6.22.7`, emulator `6.0.2.210`.
- The 6.0.2 Release entries say no new enhanced features versus earlier 6.0.2
  Release builds; the useful additions were mostly in 6.0.2 Beta1: API 22
  project support, multi-device run, better HiLog filtering, App Killed logs,
  AppFreeze stack display, Database Inspector, GPU memory profiling, and some
  Hvigor config additions.

C++20 compatibility:

- The SDK compiler accepts `-std=c++20` and reports `__cplusplus == 202002L`.
- The bundled libc++ did not expose `std::ranges::find` by default.
- `<source_location>` was not available in the tested SDK.
- We kept current MapLibre Native instead of reverting to an older revision:
  - OHOS builds use `-fexperimental-library` to expose the SDK libc++ ranges
    algorithms.
  - Symbol guard diagnostics use `mbgl::SourceLocation`, which aliases
    `std::source_location` where available and falls back to file/function/line
    on incomplete SDKs.

Known recurring build noise:

- SDK CMake toolchain files warn about an old `cmake_minimum_required` floor.
- OHOS clang may warn that an SDK-injected `--gcc-toolchain=.../llvm` argument
  is unused. The platform suppresses that command-line diagnostic.
- Hvigor packaging emits Java warnings about `sun.misc.Unsafe::arrayBaseOffset`.

## CMake Presets

OpenHarmony/public SDK:

- Removed for now; this branch has only been validated on HarmonyOS. Public
  OpenHarmony presets can be restored after validation against that SDK.

HarmonyOS/HMS SDK:

- `harmonyos-opengl`
- `harmonyos-vulkan`

The root `harmonyos-*` presets configure the HarmonyOS platform sources and use
`librcp_c.so` instead of `libnet_http.so`. The DevEco/Hvigor sample build owns
the ArkTS/NAPI native module target. Its shared library output name is:

```text
libmaplibre_native_ohos.so
```

## Networking

The current PR uses only the HarmonyOS HMS RemoteCommunicationKit backend. A
public OpenHarmony `net_http` backend was explored and then removed from the
branch because this PR has not been validated against public OpenHarmony yet.
The useful notes to preserve for a future OpenHarmony pass:

- Header/library: `network/netstack/net_http.h`, `libnet_http.so`.
- Compile/link testing worked in the earlier prototype.
- The callback API does **not** carry per-request user data or request identity,
  so a robust backend needs an explicit design for matching SDK callbacks back
  to MapLibre requests.
- A bounded callback-slot prototype was not robust enough; it could associate a
  late response with the wrong request if slot reuse and callback timing lined
  up badly.
- An earlier runtime attempt on MatePad failed fetching the demotiles style with:

```text
OH_HTTP_OUT_OF_MEMORY (2300027)
```

This came from the SDK path, not from exhausting local callback slots.

### HarmonyOS HMS RCP

- Header/library: `RemoteCommunicationKit/rcp.h`, `librcp_c.so`.
- Better API fit because callbacks carry user context.
- Used by the committed HarmonyOS build path; the non-RCP HTTP path was removed
  until public OpenHarmony validation is restored.
- Initial remote-style load crashed in the RCP worker thread:

```text
SIGSEGV(SEGV_MAPERR)
Thread: OnlineFileSourc
#00 strlen
#01 librcp_c.so - std::string from null char*
#02 librcp_c.so - RcpToCppConfiguration
#03 librcp_c.so - RcpToCppRequest
```

Root cause hypothesis, validated by the fix:

- `Rcp_Request.configuration` is optional.
- Several nested `Rcp_Configuration` string fields are documented as defaulting
  to `""`, but zero-initialized C structs provide null pointers.
- The RCP bridge appears to convert those fields to `std::string` without a
  null check.

Current fix:

- `RcpConfigurationStorage` owns a mutable empty string and a default
  `Rcp_Configuration`.
- Both the session default config and each request config get non-null empty
  strings for documented-empty fields.
- Conservative defaults are set for redirect, timeout, path preference, proxy,
  certificate validation, and server auth.
- `request->configuration` is cleared before `HMS_Rcp_DestroyRequest`, so the
  SDK request destructor cannot try to own `RequestState` storage.

Result:

- Remote map loading works on the MatePad with the HMS RCP backend.

Historical Liberty stress finding:

- The OpenFreeMap Liberty style previously showed an obviously wrong tile at
  some zooms, then crashed.
- Captured crash:

```text
2026-05-30 16:40:13.821
record_id: 03851508495760502232
Reason: SIGSEGV(SEGV_MAPERR)@0x0000000000000024
Thread: OnlineFileSourc
#00 librcp_c.so HMS_Rcp_GetResponseCookieAttrEntries+24
#01 librcp_c.so OHOS::NetStackExt::RcpC::RcpToCppRequest(...)
#02 librcp_c.so OHOS::NetStackExt::RcpC::RcpSessionBasedFetchHandle(...)
#03 libmaplibre_native_ohos.so mbgl::HTTPFileSource::request(...)
#04 libmaplibre_native_ohos.so mbgl::OnlineFileSourceThread::activateRequest(...)
```

- The exact deployed `libmaplibre_native_ohos.so` build ID was
  `9f14fedf944277f4b8ec23aae185f699441dc762`; local symbolization maps the
  MapLibre frames to `http_file_source_hms_rcp.cpp` and
  `online_file_source.cpp`.
- The crash is in the HarmonyOS RCP HTTP path, not Vulkan rendering.
- Nearby MapLibre logs before death included Liberty `invalid tag exception`
  vector tile failures and `OH_ImageSourceNative_CreatePixelmap` failures for
  `ne2_shaded` PNG raster tiles.
- Crash artifacts were saved locally in `/tmp/maplibre-ohos-crash/`.
- The wrong-tile artifact disappeared after uninstalling the app, which cleared
  the app sandbox and MapLibre on-disk cache. The likely cause was an earlier
  RCP callback user-data slot reuse bug that allowed a late response to be
  associated with a newer request and then cached under the newer resource key.
- The repeatable crash was fixed by keeping RCP session configuration storage
  alive for the session lifetime and keeping callback/request string storage
  alive for the request lifetime.

## Rendering And XComponent

### Working Path

The sample currently uses the legacy declarative XComponent path:

```ts
XComponent({
  id: 'maplibre_map',
  type: XComponentType.SURFACE,
  libraryname: 'maplibre_native_ohos'
})
```

The legacy `onLoad` context exposes sample NAPI methods such as
`map.setStyleUrl(...)`, `map.jumpTo(...)`, `map.renderFrame()`,
`map.getSurfaceState(...)`, and `map.destroy()`.

### ArkUI Node Path

`registerXComponentNode(node)` compiled and linked in an earlier prototype, but
the path has been removed from the current sample scope. On the MatePad runtime,
the programmatic node path failed early with:

```text
Could not create XComponent surface holder callback
```

The failure came from the `OH_ArkUI_SurfaceHolder_Create(node)` /
`OH_ArkUI_SurfaceCallback_Create()` path. Keep this as a follow-up
SurfaceHolder/API-23 investigation rather than part of the first PR.

### EGL Fixes

The first device render attempt failed before map creation:

```text
EGL_BAD_MATCH: pixel format (surface:..., config:...)
eglCreateWindowSurface error
```

Fixes that made rendering stable:

- Set the native window format to `NATIVEBUFFER_PIXEL_FMT_RGBA_8888`.
- Log the native window `GET_FORMAT`.
- Probe EGL configs in order:
  - RGBA8888
  - RGBX8888
  - RGB565
- Use the first config that succeeds with `eglCreateWindowSurface`.

The render button also originally called `cameraForBounds()` before a map
surface existed, which threw:

```text
Camera for bounds requires an active map surface
```

The sample now checks `getSurfaceState().hasMap` and uses deferred
`fitBounds()` for the normal render path.

## ArkTS/NAPI Surface

The sample API surface is documented by
`platform/ohos/sample/entry/types/libmaplibre_native_ohos/index.d.ts`.
It intentionally lives with the sample for now. The platform implementation is
the reusable native OHOS support; the ArkTS/NAPI bridge is one concrete
XComponent embedding.

Implemented categories:

- lifecycle:
  - `destroy`
  - `setRenderingEnabled`
  - `reduceMemoryUse`
- style:
  - `setStyleUrl`
- camera:
  - positional `jumpTo`
  - `getCameraOptions`
  - `setBounds`
- rendering/display:
  - `renderFrame`
  - `setPixelRatio`
  - `getPixelRatio`
  - `setFrameRateRange`
- resources:
  - `setClientOptions`
  - `setResourceOptions`
  - `setTileCacheEnabled`
- diagnostics:
  - `getSurfaceState`
  - `getStyleAttributions`
  - measured frame rates
  - renderer identity
  - last surface/map/render/glyph/sprite/image error strings

Gestures implemented in the bridge:

- one-finger pan
- two-finger pinch
- two-finger rotation
- two-finger vertical shove for pitch
- double-tap zoom using animated camera movement
- single-finger fling, including tuned behavior while pitched

These are enough for the sample and have been smoke-tested on the MatePad. They
are still sample-level gesture handling, not a full production SDK gesture
system.

## Image Decoding

The tested SDK sysroots do not provide cURL, libpng, libjpeg, or libwebp, and
this repo does not vendor those default image readers for OHOS.

The OHOS image backend uses:

- `multimedia/image_framework/image/image_source_native.h`
- `libimage_source.so`
- `libpixelmap.so`

Hardening already added:

- Reject empty decoded dimensions.
- Checked `size_t` arithmetic for row stride and total byte counts.
- Reject decoder row strides smaller than the tight RGBA/BGRA stride.
- Reject short `OH_PixelmapNative_ReadPixels(...)` reads.
- Include ImageKit error names and decoded pixel metadata in failure messages.

Bright and Liberty have exercised real remote glyphs, sprites, vector tiles, and
raster source metadata on the MatePad. Remaining image work is specific failure
investigation, such as Liberty's `ne2_shaded` PNG raster tiles returning
`IMAGE_UNSUPPORTED_OPERATION`, not basic remote image validation.

## Logging

The port uses an OHOS-specific `Log::platformRecord` implementation that routes
native MapLibre logs through hilog with tag `MapLibreNative`. This replaces the
earlier default stderr sink and the one-off native `OH_LOG_Print` probes used
during EGL bring-up.

Useful emulator command:

```sh
hdc shell "hilog -x -t app -T MapLibreNative"
```

Confirmed output includes normal `mbgl::Log` messages such as setup, EGL config,
default framebuffer, and GPU identifier. The sample keeps normal ArkTS logging
to warnings, such as failures to open attribution URLs or query display
properties, using tag `MapLibreSample`.

## Sample App

The committed sample app is a small HarmonyOS/OpenHarmony project under
`platform/ohos/sample`.

The sample now behaves more like the desktop GLFW demo: it loads the Bright
remote style on startup at a Chongqing camera, keeps rendering enabled while
visible, lets the XComponent frame callback drive `MapView::renderFrame()`, and
refreshes a compact status readout while the map is running.

Current buttons:

- **Demo**
- **Bright**
- **Liberty**

Current style probes:

- **Demo**: `https://demotiles.maplibre.org/style.json`.
- **Bright**: `https://tiles.openfreemap.org/styles/bright`.
- **Liberty**: `https://tiles.openfreemap.org/styles/liberty`.

The sample shows:

- overlaid style-sourced attribution anchored bottom-right
- compact overlaid status anchored top-left
- surface size
- density
- measured FPS
- XComponent tick rate
- renderer identity, either Vulkan diagnostics or OpenGL ES version
- render pending/idle state

The attribution overlay is populated from loaded style source attribution
strings through `getStyleAttributions()`. Link taps have been verified on
device.

The sample depends on the local type package through:

```json5
"libmaplibre_native_ohos.so": "file:types/libmaplibre_native_ohos"
```

## Signing And Device Install

AGC debug signing was made to work with DevEco Studio/Hvigor without committing
local credentials:

- Generate a local debug keystore/CSR with `hap-sign-tool.jar`.
- Upload the CSR to AppGallery Connect.
- Download the debug certificate/profile.
- Put local signing files under `platform/ohos/sample/sign/`.
- Run `node sign/generate-signing-config.mjs` from `platform/ohos/sample`.
- The script writes ignored local files:
  - `sign/signing.local.json`
  - `sign/material/`
- Hvigor reads `sign/signing.local.json` from `hvigorfile.ts`, so DevEco Studio
  and command-line Hvigor builds both sign through the normal build flow.

Important notes:

- OpenHarmony community signing (`OpenHarmony.p12` +
  `UnsgnedDebugProfileTemplate.json`) failed on the retail HarmonyOS MatePad
  with `code:9568257 fail to verify pkcs7 file`.
- DevEco Studio may write signing settings into `build-profile.json5` while
  configuring the local developer environment. Keep those generated settings out
  of the PR; the committed flow reads ignored local signing data from
  `sign/signing.local.json`.
- Passwords and generated signing material are intentionally kept out of source
  control.

## Runtime Timeline

1. Command-line SDK only:
   - Native CMake build/link path was established.
   - An ignored XComponent codelab harness proved Hvigor could compile ArkTS,
     build the local CMake native module, and package a HAP.
   - No device/emulator was attached.

2. Committed sample app:
   - A small sample was added under `platform/ohos/sample`.
   - It compiles the local checkout as a CMake subdirectory and imports the
     local type package.

3. First MatePad launch:
   - Programmatic ArkUI node XComponent path crashed at surface-holder callback
     creation.
   - Sample switched back to legacy declarative `libraryname` XComponent.

4. First legacy XComponent render attempt:
   - App stayed up until **Render**.
   - `eglCreateWindowSurface` failed with `EGL_BAD_MATCH`.
   - Sample also called `cameraForBounds()` with no active map.
   - EGL config probing/window format and render guard fixed this.

5. First successful local rendering:
   - Sample reached `hasMap=yes`.
   - Inline styles proved background and vector fill rendering.
   - Debug overlays rendered poorly on the current GLES path, so sample default
     debug options were set to `0`.

6. Remote with public `net_http`:
   - Failed with `OH_HTTP_OUT_OF_MEMORY (2300027)` on style fetch.

7. Remote with HMS RCP before config fix:
   - Requests progressed further but crashed in `RcpToCppConfiguration`.

8. Remote with HMS RCP after config fix:
   - Remote demotiles style displayed a working map on the MatePad.

9. DevEco emulator run:
   - Existing signed HAP installed and launched on `127.0.0.1:5555`.
   - Emulator reports API `22` and software
     `emulator 6.0.0.130(SP7DEVC00E130R4P11)`.
   - Remote style loads and symbols render; sample counters after **Remote**
     showed `window=yes`, `map=yes`, `gl=3`, `style=loaded`, `loaded=yes`,
     `glyphs=1/1/0`, `sprites=0/1/0`, and tile activity.
   - Visual result is incomplete: countries and other fill/line geometry are
     missing. The same problem appears with the inline **Local** GeoJSON style,
     so this is not only a remote vector-tile data problem.

10. Sample pump cleanup:
   - The sample starts directly on the remote demotiles style.
   - Status refreshes every 500 ms while visible, with hilog diagnostics every
     sixth tick.
   - At this point in the investigation, the sample still had temporary
     controls for manual render, fit, diagnostics, and a local style. Those were
     later removed or replaced during sample polish.
   - Emulator after this change still reports a healthy loaded map, e.g.
     `Live: 1308x2177 window=yes map=yes render=idle`,
     `frames=59/59 gl=3`, `style=loaded`, `loaded=yes`, `idle=yes`,
     `glyphs=1/1/0`, `sprites=0/1/0`, `tiles=129`, but still shows labels over
     blue background with missing fill/line geometry.

11. DevEco emulator stencil investigation:
   - Runtime EGL config diagnostics showed the chosen config requested color,
     depth, and stencil, but the active default framebuffer reported
     `rgba=0/0/0/0 depth=0 stencil=0 status=0x8cd5 err=0x0 vendor=ARM renderer=Mali-G77`.
   - Creating an offscreen RGBA8 + DEPTH24_STENCIL8 framebuffer and blitting it
     back to the window did not restore fill/line rendering.
   - Disabling `DrawableGL` stencil clipping under `OHOS_PLATFORM` restored both
     remote country fills/boundaries and the local yellow GeoJSON fill.
   - The offscreen FBO experiment was removed again; the current workaround is
     only the OHOS stencil-mode bypass plus runtime diagnostics.

12. Vulkan backend bring-up:
   - Added an OHOS Vulkan window backend using MapLibre's shared Vulkan
     renderer and `VK_OHOS_surface` for `OHNativeWindow` presentation.
   - Added the `harmonyos-vulkan` preset and a sample renderer switch. The
     sample now defaults to Vulkan and can be configured back to OpenGL with
     `MLN_OHOS_SAMPLE_RENDERER=OpenGL`.
   - Updated the vendored Khronos Vulkan-Headers submodule to the latest
     `v1.4.x` patch so MapLibre's shared Vulkan C++ wrapper headers include the
     official `VK_OHOS_surface` declarations. The HarmonyOS SDK has C Vulkan
     headers and `libvulkan.so`, but not Vulkan-Hpp; using the vendored Khronos
     C/C++ header set keeps `vulkan.hpp`, `vulkan_core.h`, and
     `vulkan_ohos.h` coherent.
   - The Vulkan-Hpp update moved dynamic loader and object-destroy helper types
     under its configured dispatcher/detail types, so MapLibre now aliases
     `VULKAN_HPP_DISPATCH_LOADER_DYNAMIC_TYPE` and the matching destroyer at the
     Vulkan backend boundary instead of spelling old top-level Vulkan-Hpp names
     throughout platform code.
   - A `VK_LAYER_KHRONOS_validation` request was removed from GLFW shared
     context **device** creation while updating the Vulkan-Hpp type names. This
     does not disable validation. Validation layers are still requested during
     Vulkan **instance** creation by `RendererBackend::getLayers()` and by the
     GLFW shared `VkContext` instance path; device layers have been deprecated
     and ignored by modern Vulkan implementations.
   - DevEco emulator runtime logs show `Vulkan loader api=1.3.275
     instanceExtensions=8 VK_KHR_surface=yes VK_OHOS_surface=yes`, followed by
     `vk::createInstanceUnique: ErrorIncompatibleDriver`. The emulator exposes
     the loader and OHOS surface extension, but apparently not a compatible
     Vulkan driver/runtime.
   - HarmonyOS MatePad target `59GYD25808200129` successfully creates a
     `Maleoon 920` Vulkan 1.3.275 device. Remote demotiles renders with country
     fills, borders, and labels; the local inline GeoJSON style renders its
     yellow polygon over the blue background.
   - The tested IDE is DevEco Studio 6.0.2 Release, build 6.0.2.642, built on
     March 4, 2026. That is still before the 6.1.0 Beta2 emulator Vulkan support
     boundary in Huawei's emulator specification, so the emulator
     `VK_ERROR_INCOMPATIBLE_DRIVER` result is consistent with the published
     support matrix even though the loader reports `VK_OHOS_surface`.

13. Sample polish after Vulkan validation:
   - Fixed density scaling and set the sample frame-rate range from the display
     refresh rate instead of hard-coding 60 Hz.
   - Changed render scheduling so XComponent frame callbacks drive rendering;
     this made FPS telemetry reflect real rendered frames and improved gesture
     feel.
   - Removed the inline **Local** style and the **Fit** button. The sample now
     exposes **Demo**, **Bright**, and **Liberty** style buttons.
   - Added shove pitch, animated double-tap zoom, and tuned tilted fling
     behavior.
   - Changed the default style to Bright with a Chongqing default camera.
   - Added a style-sourced attribution overlay anchored bottom-right and a
     compact telemetry overlay anchored top-left. Attribution link taps were
     verified on device.
   - Moved `MapView`, `CameraBoundsOptions`, gesture handling, NAPI bindings,
     and the ArkTS type package into the sample. The reusable platform layer is
     now limited to the OHOS window backends, renderer frontend, networking,
     image decoding, logging, and CMake integration.
   - Renamed the native CMake target to `maplibre_native_ohos` so it matches
     `NAPI_MODULE(maplibre_native_ohos, Init)` and the generated
     `libmaplibre_native_ohos.so` output name.
   - Added separate Vulkan/OpenGL sample products in DevEco/Hvigor and selected
     the BiSheng native compiler for both products.

## Verification Snapshot

Recent useful checks:

```sh
OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native \
HMOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/hms/native \
pixi run cmake --preset harmonyos-opengl

OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native \
HMOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/hms/native \
pixi run cmake --build --preset harmonyos-opengl

cd platform/ohos/sample
/Applications/DevEco-Studio.app/Contents/tools/hvigor/bin/hvigorw assembleApp --no-daemon

/Applications/DevEco-Studio.app/Contents/tools/hvigor/bin/hvigorw assembleApp -p product=opengl --no-daemon
```

Observed:

- `libmaplibre_native_ohos.so` builds for HarmonyOS/HMS.
- Sample HAP packages `libmaplibre_native_ohos.so` and `libc++_shared.so` for
  configured ABIs.
- HarmonyOS native module links `librcp_c.so`, not `libnet_http.so`.
- Signed HAP installed successfully on MatePad target `59GYD25808200129`.
- Signed HAP installed and launched successfully on DevEco emulator target
  `127.0.0.1:5555`.
- DevEco emulator screenshots confirmed the pumped sample loads remote
  resources and renders remote fill/line geometry after the stencil bypass.
- Earlier DevEco emulator screenshots confirmed the temporary inline local
  style rendered its yellow GeoJSON polygon after the same stencil bypass.
- `git diff --check` passed after the latest code/doc changes before this
  condensation.
- The DevEco/Hvigor sample builds and signs both products:
  - default Vulkan product
  - `opengl` EGL/GLES product
- The Vulkan sample HAP builds, signs, installs, and launches on the DevEco
  emulator, but fails before map creation with `VK_ERROR_INCOMPATIBLE_DRIVER`
  from `vkCreateInstance`.
- The same signed Vulkan HAP installs and runs on HarmonyOS MatePad target
  `59GYD25808200129`; logs report `Vulkan device name="Maleoon 920" api=1.3.275`
  and screenshots confirmed both remote styles and the earlier temporary local
  style test rendered correctly.
- Tablet gestures work: pan, pinch zoom, rotate, shove pitch, double tap, and
  fling all behave acceptably in the sample.
- Tablet stress checks with pan/zoom/rotate and higher zooms did not reveal
  correctness issues. Frame rate is rough enough to revisit later, probably in
  release builds and/or after reviewing the current run-loop pump.
- Tablet background/foreground, resize, and repeated style switching work.
- Bright loads on the MatePad Vulkan path and exercises OpenFreeMap glyphs,
  sprites, vector tiles, and raster source metadata without style load failure.
- Style-sourced attribution renders over the map, and attribution link taps open
  correctly on device.
- Liberty loads on the MatePad Vulkan path but exposes remaining data/decoder
  gaps: `invalid tag exception` for some `openmaptiles` vector tiles and
  `IMAGE_UNSUPPORTED_OPERATION` for some `ne2_shaded` PNG raster tiles.
- A stale wrong-tile artifact seen in Bright/Liberty disappeared after
  uninstalling the app, which clears the app sandbox and MapLibre's on-disk
  resource cache. The likely cause was an earlier RCP callback user-data slot
  reuse bug that allowed a late response to be associated with a newer request
  and then cached under the newer resource key.
- A repeatable Liberty crash was fixed in the RCP backend. Fault logs showed
  `OnlineFileSourc` crashing in `strlen` from
  `OHOS::NetStackExt::RcpC::RcpToCppRequest+100`; disassembly showed that
  offset reads `Rcp_SessionConfiguration::baseUrl`. `HMS_Rcp_CreateSession`
  appears to retain the provided session configuration, so the local stack
  `Rcp_SessionConfiguration` in `RcpSession` became dangling. Keeping the
  session configuration as an `RcpSession` member fixed the crash. The backend
  also now keeps RCP callback contexts process-alive and keeps request URL/header
  strings alive for the request lifetime.
- Rebuilt and signed the sample, installed on MatePad target `59GYD25808200129`,
  selected Liberty, and observed no new `org.maplibre.native.demo` CppCrash
  records over a 20 second smoke test after the session configuration lifetime
  fix. Final validated native build ID:
  `28c415f0d6ad8aac8fceae8c436b55db6faaa93c`.

## Remaining Work

High value:

- Investigate Liberty style failures if Liberty needs to be a fully green
  sample style. Bright already exercises remote glyphs, sprites, vector tiles,
  and raster source metadata successfully.
- Validate release/profile performance on the MatePad. Current Vulkan
  correctness looks good, but frame rate is still rough with heavier styles in
  the debug sample.
- Decide the OpenGL fallback policy: keep the unconditional OHOS stencil bypass,
  gate it, or document it as fallback/emulator-specific behavior.
- Public OpenHarmony and `net_http` validation are deferred until an Oniro or
  similar OpenHarmony SDK/device setup is available.

Before upstreaming:

- Keep the first PR scoped to the platform implementation and demo app. Broader
  public language bindings should be split out.
- Re-test with the updated DevEco Studio SDK/emulator and record exact versions.
- Find or configure a DevEco emulator image that provides a compatible Vulkan
  runtime. The tested emulator advertises the OHOS surface extension but fails
  `vkCreateInstance`. Useful reference:
  <https://developer.huawei.com/consumer/en/doc/harmonyos-guides/ide-emulator-specification>.
  It says emulator versions before DevEco Studio 6.1.0 Beta2 do not support
  Vulkan; DevEco Studio 6.1.0 Beta2 and newer support Vulkan APIs except
  `vkGetSwapchainGrallocUsageOHOS`, `vkAcquireImageOHOS`, and
  `vkQueueSignalReleaseImageOHOS`.

## Local Side Effects To Know About

Generated and ignored paths used during research:

- `build-ohos-*`
- `build-harmonyos-*`
- `build-ohos-xcomponent-sample/`
- `build-macos-metal/`
- `platform/ohos/sample/.hvigor/`
- `platform/ohos/sample/build/`
- `platform/ohos/sample/entry/.cxx/`
- `platform/ohos/sample/entry/build/`

User-level Hvigor wrapper/cache paths may have been populated:

- `/Users/sargunv/.hvigor/wrapper/tools/`
- `/Users/sargunv/.hvigor/project_caches/`

Local signing credentials and generated AGC signing material intentionally live
outside source control or under the sample signing directory according to
`platform/ohos/sample/sign/README.md`.

This worktree also has ignored local `mise.local.toml` convenience tasks for
the repeated emulator loop:

```sh
mise run ohos-sample-build
mise run ohos-sample-install
mise run ohos-sample-launch
mise run ohos-sample-restart
```
