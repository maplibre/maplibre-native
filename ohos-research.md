# OpenHarmony/HarmonyOS Research Notes

This note compresses the original research log into the pieces a contributor
needs in order to understand the current OpenHarmony/HarmonyOS port direction,
what has been proven, and which dead ends are worth remembering.

## Current Status

- Branch: `ohos-research`.
- Tracking issue: `maplibre/maplibre-native#3749` remains open. As of the last
  check on 2026-05-30, no newer technical issue comments or PRs had appeared.
- Platform code lives under `platform/ohos/`.
- Sample app lives under `platform/ohos/sample`.
- Native module output: `libmaplibre_native_ohos.so`.
- ArkTS type package: `platform/ohos/arkts/types/libmaplibre_native_ohos`.
- Sample bundle id: `org.maplibre.native.demo`. AppGallery Connect rejected
  bundle ids containing `ohos`, so the original sample id was renamed.
- Device validation target so far:
  - Huawei MatePad Pro `MRDI-W00`
  - HarmonyOS `6.1.0.117`
  - API `23`

Confirmed on the MatePad:

- Legacy declarative `XComponent` using
  `libraryname: 'maplibre_native_ohos'` works.
- EGL/GLES rendering works into the XComponent surface.
- Local inline styles render:
  - **Empty**: black framebuffer with no layers.
  - **Diag**: red background layer.
  - **Local**: blue background plus yellow GeoJSON polygon fill.
- Remote `https://demotiles.maplibre.org/style.json` works with the HarmonyOS
  HMS RCP HTTP backend after initializing RCP request/session configuration
  with non-null empty strings.

Confirmed on DevEco emulator:

- Target: `127.0.0.1:5555`
- Reported model/version:
  - model `emulator`
  - API `22`
  - software `emulator 6.0.0.130(SP7DEVC00E130R4P11)`
- Existing signed HAP installed successfully.
- Legacy XComponent path launches and creates a map surface.
- Remote style loads without the previous RCP crash.
- Background and symbol layers render.
- A fill/line rendering gap was reproduced and narrowed down:
  - The emulator window framebuffer reports complete status but no usable depth
    or stencil bits:
    `rgba=0/0/0/0 depth=0 stencil=0 status=0x8cd5 renderer=Mali-G77`.
  - An offscreen color/depth/stencil FBO plus blit did **not** make fill/line
    geometry appear.
  - Disabling GL stencil clipping for OHOS makes both **Remote** country
    fills/boundaries and the **Local** GeoJSON polygon render on the emulator.

Not yet fully validated:

- OpenHarmony public `net_http` backend at runtime.
- Remote glyph/sprite/tile counters from a clean captured run.
- ImageKit decode on real remote sprite/image traffic.
- Gesture behavior on a real map beyond initial sample interaction checks.
- Whether the OHOS stencil-clipping bypass has unacceptable tile-edge artifacts
  during pan/zoom/rotate or on real devices that provide a usable stencil
  attachment.
- ArkUI `NodeContainer` / `registerXComponentNode(node)` runtime path on the
  MatePad.

## Quick Build And Run

Native CMake presets:

```sh
export OHOS_SDK_NATIVE=/path/to/openharmony/native
export HMOS_SDK_NATIVE=/path/to/hms/native

pixi run cmake --preset ohos-opengl-native
pixi run cmake --build --preset ohos-opengl-native

pixi run cmake --preset harmonyos-opengl-native
pixi run cmake --build --preset harmonyos-opengl-native
```

Sample app:

```sh
cd platform/ohos/sample
/path/to/command-line-tools/bin/hvigorw assembleApp --no-daemon
```

Manual debug signing is documented in
`platform/ohos/sample/sign/README.md`. The short version is:

```sh
java -jar "$SIGN_LIB/hap-sign-tool.jar" sign-app \
  -mode localSign \
  -keyAlias maplibre_debug \
  -keyPwd 'YOUR_PASSWORD' \
  -signAlg SHA256withECDSA \
  -appCertFile ./sign/maplibre-debug.cer \
  -profileFile ./sign/maplibre-debug.p7b \
  -inFile entry/build/default/outputs/default/entry-default-unsigned.hap \
  -keystoreFile ./sign/maplibre-debug.p12 \
  -keystorePwd 'YOUR_PASSWORD' \
  -compatibleVersion 21 \
  -outFile ./sign/entry-default-signed.hap

HDC=/path/to/command-line-tools/sdk/default/openharmony/toolchains/hdc
$HDC install -r ./sign/entry-default-signed.hap
$HDC shell aa start -b org.maplibre.native.demo -a EntryAbility
```

On the MatePad, `aa start` fails with error `10106102` if the screen is locked.
Manually unlock the device first.

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
  - XComponent/NAPI surface integration
  - EGL window backend for `OHNativeWindow *`
  - image decoding through ImageSourceNative/PixelmapNative
  - HTTP backends using platform SDK APIs

The main native ownership chain is:

```text
ArkUI XComponent
  -> OHNativeWindow
  -> EGLWindowBackend
  -> RendererFrontend
  -> mbgl::Map
```

Important files:

- `platform/ohos/ohos.cmake`: platform source selection, SDK libraries, native
  module target, link smoke target.
- `platform/ohos/src/egl_window_backend.*`: EGL/GLES context and surface
  management for `OHNativeWindow`.
- `platform/ohos/src/renderer_frontend.*`: thin `RendererFrontend` bridge.
- `platform/ohos/src/map_view.*`: owns the backend, frontend, and `mbgl::Map`;
  stores desired state across surface recreation.
- `platform/ohos/src/native_module.cpp`: NAPI/XComponent lifecycle and ArkTS
  exports.
- `platform/ohos/src/native_values.*`: NAPI value parsing/object creation.
- `platform/ohos/src/gesture_handler.*`: XComponent touch input to MapLibre
  gesture calls.
- `platform/ohos/src/http_file_source.cpp`: public OpenHarmony `net_http`
  backend.
- `platform/ohos/src/http_file_source_hms_rcp.cpp`: HarmonyOS HMS
  RemoteCommunicationKit backend.
- `platform/ohos/src/image.cpp`: ImageKit image decoding.

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

- `ohos-opengl-core`
- `ohos-opengl`
- `ohos-opengl-link-smoke`
- `ohos-opengl-native`

HarmonyOS/HMS SDK:

- `harmonyos-opengl`
- `harmonyos-opengl-link-smoke`
- `harmonyos-opengl-native`

The `harmonyos-*` presets set `MLN_OHOS_WITH_HMS_RCP=ON`, so they use
`librcp_c.so` instead of `libnet_http.so`.

The native module target is controlled by:

```cmake
MLN_OHOS_BUILD_NATIVE_MODULE=ON
```

The shared library output name stays:

```text
libmaplibre_native_ohos.so
```

## Networking

Two HTTP backends exist because the public OpenHarmony and HarmonyOS/HMS SDK
surfaces differ materially.

### Public OpenHarmony `net_http`

- Header/library: `network/netstack/net_http.h`, `libnet_http.so`.
- Compile/link tested.
- The callback API does **not** carry per-request user data or request identity.
- The backend uses a bounded 128-slot callback dispatcher to map SDK callbacks
  back to MapLibre requests.
- Earlier runtime attempt on MatePad failed fetching the demotiles style with:

```text
OH_HTTP_OUT_OF_MEMORY (2300027)
```

This came from the SDK path, not from exhausting the local callback slots.

### HarmonyOS HMS RCP

- Header/library: `RemoteCommunicationKit/rcp.h`, `librcp_c.so`.
- Better API fit because callbacks carry user context.
- Used by the committed sample through `MLN_OHOS_WITH_HMS_RCP=ON`.
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

The legacy `onLoad` context exposes no-binding methods such as
`map.setStyleJson(...)`, `map.setStyleUrl(...)`, `map.renderFrame()`, and
`map.destroy()`.

### ArkUI Node Path

`registerXComponentNode(node)` compiles and links, and the native module still
contains this path for `NodeContainer`/`NodeController` integration. On the
MatePad runtime, however, the programmatic node path failed early with:

```text
Could not create XComponent surface holder callback
```

The failure came from the `OH_ArkUI_SurfaceHolder_Create(node)` /
`OH_ArkUI_SurfaceCallback_Create()` path. Keep this as a follow-up
SurfaceHolder/API-23 investigation rather than using it for first validation.

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

The experimental API surface is documented by
`platform/ohos/arkts/types/libmaplibre_native_ohos/index.d.ts`.

Implemented categories:

- lifecycle:
  - `destroy`
  - `setRenderingEnabled`
  - `getRenderingEnabled`
  - `reduceMemoryUse`
- style:
  - `setStyleJson`
  - `setStyleUrl`
  - `getStyleJson`
  - `getStyleUrl`
- camera:
  - positional `jumpTo`
  - object `setCameraOptions`
  - `getCameraOptions`
  - `easeTo`
  - `flyTo`
  - `fitBounds`
  - `cameraForBounds`
  - `setBounds`
  - `getBounds`
  - free-camera get/set
- rendering/display:
  - `renderFrame`
  - `runLoopOnce`
  - `setPixelRatio`
  - `getPixelRatio`
  - `setFrameRateRange`
  - `getFrameRateRange`
  - `setDebugOptions`
  - `getDebugOptions`
- resources:
  - `setClientOptions`
  - `getClientOptions`
  - `setResourceOptions`
  - `getResourceOptions`
  - `setTileCacheEnabled`
  - `getTileCacheEnabled`
- diagnostics:
  - `getSurfaceState`
  - `logSurfaceState`
  - style/map/frame/resource counters
  - last surface/map/render/glyph/sprite/image error strings

Gestures implemented in the bridge:

- one-finger pan
- two-finger pinch
- two-finger rotation
- double-tap zoom
- single-finger fling

These are enough for smoke testing, but not a polished production gesture
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

Still needs runtime validation with real remote sprites/images.

## Logging

The port currently uses default stderr logging rather than an OHOS-specific
hilog backend. The sample itself logs useful ArkTS-side state through hilog
with tag `MapLibreSample`, and the native module exposes explicit surface-state
logging/diagnostics for bring-up.

Earlier iterations used `libhilog_ndk.z.so`; later cleanup favored Linux/default
logging as part of keeping platform differences minimal. Recheck the final
dynamic dependency list before upstreaming. One recent packaged HarmonyOS build
still showed `libhilog_ndk.z.so` in `NEEDED`, likely through current native
module/sample logging paths or SDK linkage.

## Sample App

The committed sample app is a small HarmonyOS/OpenHarmony project under
`platform/ohos/sample`.

The sample now behaves more like the desktop GLFW demo: it loads the remote
demotiles style on startup, keeps rendering enabled while visible, lets the
XComponent frame callback drive `MapView::renderFrame()`, and refreshes the
compact status readout while the map is running.

Current buttons:

- **Fit**
- **Remote**
- **Local**

Current style probes:

- **Local**: inline GeoJSON polygon with blue background and yellow fill.
- **Remote**: `https://demotiles.maplibre.org/style.json`.

The sample shows:

- surface/window/map status
- frame counters
- GLES context version
- surface lifecycle counters
- style/load/idle flags
- glyph/sprite/tile/resource counters
- last error strings
- EGL config and default framebuffer diagnostics, including depth/stencil bits
  while the emulator rendering issue is under investigation

The sample depends on the local type package through:

```json5
"libmaplibre_native_ohos.so": "file:../../arkts/types/libmaplibre_native_ohos"
```

## Signing And Device Install

AGC debug signing was made to work without DevEco Studio:

- Generate a local debug keystore/CSR with `hap-sign-tool.jar`.
- Upload the CSR to AppGallery Connect.
- Download the debug certificate/profile.
- Put local signing files under `platform/ohos/sample/sign/`.
- Build unsigned with Hvigor.
- Sign manually with `hap-sign-tool.jar sign-app`.
- Install with `hdc install -r`.

Important notes:

- OpenHarmony community signing (`OpenHarmony.p12` +
  `UnsgnedDebugProfileTemplate.json`) failed on the retail HarmonyOS MatePad
  with `code:9568257 fail to verify pkcs7 file`.
- Hvigor signing config expects encrypted 32+ character passwords in
  `build-profile.json5`, so manual signing is currently simpler.
- Passwords are intentionally kept outside the repo in
  `~/Downloads/maplibre-ohos-sample-signing-credentials.txt`.

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
   - The old manual **Render** control is now **Fit**; camera/style changes
     should be picked up by the native frame pump.
   - The old **Empty** and **Diag** controls were removed; **Local** is enough
     for deterministic non-network rendering checks.
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

## Verification Snapshot

Recent useful checks:

```sh
OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native \
HMOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/hms/native \
pixi run cmake --preset harmonyos-opengl-native

OHOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/openharmony/native \
HMOS_SDK_NATIVE=/Users/sargunv/Downloads/command-line-tools/sdk/default/hms/native \
pixi run cmake --build --preset harmonyos-opengl-native

cd platform/ohos/sample
/Users/sargunv/Downloads/command-line-tools/bin/hvigorw assembleApp --no-daemon
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
- DevEco emulator screenshot confirmed the inline **Local** style renders its
  yellow GeoJSON polygon after the same stencil bypass.
- `git diff --check` passed after the latest code/doc changes before this
  condensation.

## Remaining Work

High value:

- Capture a clean `hilog` session for the now-working Remote path and record
  style/glyph/sprite/tile counters.
- Exercise pan/zoom/rotate/fling on the remote map and record which gestures
  behave acceptably.
- Validate ImageKit sprite/image decoding on real remote style assets.
- Re-check dynamic dependencies for public OpenHarmony and HarmonyOS/HMS builds.
- Decide whether the public `net_http` backend is worth keeping as default, or
  whether upstream docs should present HMS RCP as the practical HarmonyOS path.
- Stress the OHOS stencil-clipping bypass with pan/zoom/rotate and higher zoom
  levels. The likely risk is tile-edge overdraw or geometry crossing tile
  boundaries; current whole-world and local-polygon checks look correct.
- Decide whether the first upstreamable version should keep the unconditional
  OHOS stencil bypass, or invest in plumbing framebuffer stencil capability into
  render-time state so devices with usable stencil attachments can keep normal
  clipping.

Before upstreaming:

- Refresh `platform/ohos/README.md`; parts of it still describe the platform as
  compile/package tested only, which is now stale after MatePad validation.
- Reduce the sample API surface to a coherent demo instead of a broad compile
  harness, or clearly label it as a diagnostic sample.
- Add a concise maintainer-facing design note explaining why the port starts
  from Linux/default sources and only specializes HTTP, image decoding, and
  XComponent/EGL.
- Decide how much of the experimental ArkTS/NAPI API belongs in the first PR.
- Re-test with the updated DevEco Studio SDK/emulator and record exact versions.

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
mise run ohos-sample-sign
mise run ohos-sample-install
mise run ohos-sample-launch
mise run ohos-sample-restart
```
