# OpenHarmony

This platform support is experimental. It targets the OpenHarmony/HarmonyOS
native SDK with EGL/GLES and ArkUI XComponent surfaces.

## Status

This support is still compile/package tested only. No device or emulator has
validated rendering, input, networking, or image decoding behavior yet.

The current implementation provides:

- EGL/GLES rendering to `OHNativeWindow` surfaces, preferring OpenGL ES 3 and
  falling back to OpenGL ES 2 when needed.
- ArkUI XComponent integration through both legacy `libraryname` context
  loading and `registerXComponentNode(node)`.
- XComponent lifecycle readback for the last reported surface size and whether
  a native window/map currently exists, plus map/style/render callback
  diagnostics, surface callback/error counters, lightweight resource callback
  counters, last missing-style-image id, last glyph/sprite error strings, the
  selected GLES context client version, frame/touch/gesture callback counters,
  ArkUI surface visibility counters, and last-error strings for runtime
  validation.
- OpenHarmony NetworkKit `net_http` networking by default.
- Optional HarmonyOS HMS RemoteCommunicationKit networking through the
  `harmonyos-*` presets.
- OHOS image decoding through ImageSourceNative and PixelmapNative, with
  ImageKit error names and decoded pixel metadata in decoder failure messages.
- Default stderr logging.
- Camera, free-camera, bounds, debug, client, resource, tile-cache,
  pixel-ratio, frame-rate, rendering-enable, and memory-reduction
  controls/readback through NAPI.
- Basic one-finger pan, two-finger pinch, two-finger rotation, double-tap zoom,
  and single-finger fling gesture bridging.

## Build

Set the native SDK paths, then use the CMake presets:

```sh
export OHOS_SDK_NATIVE=/path/to/openharmony/native
export HMOS_SDK_NATIVE=/path/to/hms/native

pixi run cmake --preset ohos-opengl
pixi run cmake --build --preset ohos-opengl

pixi run cmake --preset ohos-opengl-native
pixi run cmake --build --preset ohos-opengl-native
```

The configure and build presets are disabled until the required SDK environment
variables are set. This avoids creating or reusing a CMake cache with an empty
toolchain path.

The OpenHarmony native SDK tested so far ships Clang 15. The build keeps the
current MapLibre Native C++20 code path and enables the SDK's libc++
experimental ranges support for the OHOS targets instead of reverting to an
older MapLibre Native revision.

The platform wiring intentionally starts from the Linux/default source set:
filesystem, SQLite storage, local files, run loop, text utilities, headless
frontend, GL function loading, and the EGL headless backend all use the shared
default or Linux implementations. HTTP and image decoding are OHOS-specific
because the tested SDK sysroots do not provide cURL, libpng, libjpeg, or
libwebp, and this repository does not vendor those libraries for the default
readers.

The public OpenHarmony build uses NetworkKit `net_http` for `HTTPFileSource`.
Its callbacks do not carry request user data, so the backend dispatches through
a bounded callback-slot table and still needs runtime network validation. The
`harmonyos-*` presets switch to the optional HMS RemoteCommunicationKit HTTP
backend instead. That backend uses RCP's callback context with stable slots so
late cancellation callbacks cannot dereference freed request state. Both
backends include SDK error names and request URLs in network failure messages,
but still need runtime network validation:

```sh
pixi run cmake --preset harmonyos-opengl-native
pixi run cmake --build --preset harmonyos-opengl-native
```

Apps that load remote styles, tiles, sprites, or glyphs must declare the
network permission in `module.json5`:

```json5
{
  "module": {
    "requestPermissions": [
      { "name": "ohos.permission.INTERNET" }
    ]
  }
}
```

## Harmony Project Integration

For an existing HarmonyOS module with `externalNativeOptions`, add MapLibre
Native as a CMake subdirectory and enable the experimental native module:

```json5
{
  "buildOption": {
    "externalNativeOptions": {
      "path": "./src/main/cpp/CMakeLists.txt",
      "arguments": "-DOHOS_STL=c++_shared",
      "abiFilters": [
        "arm64-v8a",
        "x86_64"
      ]
    }
  }
}
```

```cmake
set(MLN_WITH_GLFW OFF CACHE BOOL "" FORCE)
set(MLN_WITH_OPENGL ON CACHE BOOL "" FORCE)
set(MLN_WITH_EGL ON CACHE BOOL "" FORCE)
set(MLN_OHOS_BUILD_NATIVE_MODULE ON CACHE BOOL "" FORCE)
add_subdirectory("${MAPLIBRE_NATIVE_ROOT}" "${CMAKE_BINARY_DIR}/maplibre-native")
```

The native module target builds `libmaplibre_native_ohos.so`, matching the NAPI
module name `maplibre_native_ohos`. The shared C++ runtime setting is important:
the sample HAP packages both `libmaplibre_native_ohos.so` and
`libc++_shared.so` for each configured ABI. The OHOS SDK 6.0.1 toolchain
injects a `--gcc-toolchain` argument that clang reports as unused, so the
platform build suppresses only that command-line diagnostic while keeping
warnings-as-errors enabled.

The public OpenHarmony native module should link against platform libraries
only, plus the packaged shared C++ runtime. Its expected dynamic dependencies
are `libEGL.so`, `libGLESv3.so`, `libace_napi.z.so`, `libace_ndk.z.so`,
`libc++_shared.so`, `libc.so`, `libimage_source.so`, `libnative_window.so`,
`libnet_http.so`, `libpixelmap.so`, `libuv.so`, and `libz.so`. The
HarmonyOS/HMS RCP variant replaces `libnet_http.so` with `librcp_c.so`.

For ArkTS type metadata, add
`platform/ohos/arkts/types/libmaplibre_native_ohos` as an OH package dependency
or copy it into the app module's `src/main/cpp/types/` directory. Treat
`index.d.ts` in that package as the authoritative API surface while this
platform support is experimental.

## ArkTS Usage

Legacy `libraryname` XComponent usage:

```ts
import { common } from '@kit.AbilityKit';
import maplibreNative from 'libmaplibre_native_ohos.so';
import type { SurfaceState, XComponentContext } from 'libmaplibre_native_ohos.so';

XComponent({
  id: 'map',
  type: XComponentType.SURFACE,
  libraryname: 'maplibre_native_ohos',
})
  .onLoad((context?: object | XComponentContext) => {
    if (!context) {
      return;
    }

    const map = context as XComponentContext;
    map.setClientOptions('ExampleApp', '1.0.0');
    const hostContext = this.getUIContext().getHostContext() as common.Context | undefined;
    if (hostContext) {
      map.setResourceOptions({ cachePath: hostContext.cacheDir + '/maplibre-cache.db' });
    }
    map.getClientOptions();
    map.getResourceOptions();
    map.setTileCacheEnabled(true);
    map.setFrameRateRange({ min: 30, max: 120, expected: 60 });
    map.setPixelRatio(2);
    map.setRenderingEnabled(true);
    map.getRenderingEnabled();
    map.getPixelRatio();
    map.getFrameRateRange();
    map.setStyleJson('{"version":8,"sources":{},"layers":[]}');
    map.getStyleJson();
    map.getStyleUrl();
    const surfaceState: SurfaceState = map.getSurfaceState();
    const observedCoreCallbacks: boolean =
      surfaceState.styleLoaded || surfaceState.mapLoaded || surfaceState.coreFrameCount > 0;
    const observedResourceCallbacks: boolean =
      surfaceState.glyphsRequestedCount > 0 ||
      surfaceState.spritesRequestedCount > 0 ||
      surfaceState.tileActionCount > 0 ||
      surfaceState.styleImageMissingCount > 0;
    const observedErrors: boolean =
      surfaceState.lastSurfaceError !== undefined ||
      surfaceState.lastMapLoadError !== undefined ||
      surfaceState.lastRenderError !== undefined;
    if (surfaceState.needsRender &&
        (surfaceState.renderedFrameCount === 0 ||
         observedCoreCallbacks ||
         observedResourceCallbacks ||
         observedErrors)) {
      maplibreNative.runLoopOnce();
    }
    map.setBounds({
      bounds: { west: -180, south: -85, east: 180, north: 85 },
      minZoom: 0,
      maxZoom: 22,
      minPitch: 0,
      maxPitch: 60,
    });
    map.fitBounds({
      bounds: { west: -10, south: -10, east: 10, north: 10 },
      padding: { top: 24, left: 24, bottom: 24, right: 24 },
    });
    map.setDebugOptions(
      maplibreNative.DebugOptions.TileBorders |
      maplibreNative.DebugOptions.ParseStatus
    );
    map.setCameraOptions({ longitude: 0, latitude: 0, zoom: 2, bearing: 15, pitch: 30 });
    map.easeTo({ zoom: 2.5 }, { duration: 0 });
    map.setFreeCameraOptions(map.getFreeCameraOptions());
    map.renderFrame();
    // Retain `map` in the owning component and call `map.destroy()` when that
    // owner is disposed.
  });
```

The module also exposes `registerXComponentNode(node)` for ArkUI node-handle
integration. This is the preferred path for `NodeContainer`/`NodeController`
XComponents created from ArkTS. Release the returned native binding with
`destroy(binding)` when the ArkTS owner is disposed. Module-level map operations
require this explicit binding:

```ts
import { NodeController, typeNode } from '@kit.ArkUI';
import maplibreNative from 'libmaplibre_native_ohos.so';
import type { FrameNode, UIContext } from '@kit.ArkUI';
import type { NativeBinding } from 'libmaplibre_native_ohos.so';

class MapNodeController extends NodeController {
  private binding?: NativeBinding;

  makeNode(uiContext: UIContext): FrameNode | null {
    const node = typeNode.createNode(uiContext, 'XComponent', {
      type: XComponentType.SURFACE,
    });
    this.binding = maplibreNative.registerXComponentNode(node);
    maplibreNative.setStyleJson(this.binding, '{"version":8,"sources":{},"layers":[]}');
    return node;
  }

  aboutToDisappear(): void {
    if (this.binding) {
      maplibreNative.destroy(this.binding);
      this.binding = undefined;
    }
  }
}

NodeContainer(new MapNodeController());
```

Only the legacy XComponent onLoad context exposes no-binding methods such as
`map.setStyleJson(json)` and `map.destroy()`, because the native resolver
receives that XComponent context as `this`. The public legacy XComponent API
does not expose a surface-callback unregister function, so `map.destroy()`
clears the MapLibre surface/map state, unregisters the frame callback, and
removes the binding from this module so later static surface callbacks no-op.
Calling `map.destroy()` again on the same legacy context is a no-op.

For ArkUI node-handle XComponents, `SurfaceState.surfaceVisible`,
`surfaceShownCount`, and `surfaceHiddenCount` report the native surface
show/hide callback state. Rendering from frame callbacks is suppressed while
the surface is hidden.

`SurfaceState.glesContextClientVersion` reports `3` for the preferred OpenGL
ES 3 context, `2` when the EGL backend had to use its ES 2 fallback, and `0`
before an EGL-backed map surface exists.

Apps should call `reduceMemoryUse()` from low-memory or background lifecycle
hooks where available. Apps can call `setRenderingEnabled(false)` while hidden
to suppress frame-callback rendering, then re-enable it from foreground
lifecycle hooks. Apps can call `setFrameRateRange({ min, max, expected })` to
pass their desired XComponent on-frame callback rate to the platform.

## Sample App

`platform/ohos/sample` contains a minimal HarmonyOS/OpenHarmony app shell that
builds the native module from this checkout, imports
`libmaplibre_native_ohos.so` from ArkTS, and packages a HAP:

```sh
cd platform/ohos/sample
(cd entry && /path/to/command-line-tools/bin/ohpm install)
/path/to/command-line-tools/bin/hvigorw assembleApp --no-daemon
```

The sample is intended as a build and packaging integration check until device
or emulator runtime validation is available. It starts with an empty inline
style for deterministic surface/render validation and includes a `Remote`
button that switches to `https://demotiles.maplibre.org/style.json` for first
device checks of HTTP, glyph, tile, sprite, and image loading behavior. Missing
style image callbacks report both a count and the last requested image id.
