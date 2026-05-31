# HarmonyOS

This platform support is experimental. It targets the HarmonyOS native SDK with
Vulkan or EGL/GLES rendering, ArkUI XComponent surfaces, and HMS
RemoteCommunicationKit networking.

The implementation has been validated on a HarmonyOS tablet with both Vulkan and
EGL/GLES backends. The DevEco emulator can be used for EGL/GLES checks; the
tested emulator image exposed the Vulkan loader but did not provide a compatible
Vulkan driver.

## Status

The current implementation provides:

- Vulkan rendering to `OHNativeWindow` surfaces through `VK_OHOS_surface`.
- EGL/GLES rendering to `OHNativeWindow` surfaces, preferring OpenGL ES 3 and
  falling back to OpenGL ES 2 when needed.
- ArkUI XComponent integration through legacy `libraryname` context loading.
- HMS RemoteCommunicationKit networking for `HTTPFileSource`.
- HarmonyOS image decoding through ImageSourceNative and PixelmapNative.
- Native MapLibre logging through hilog with tag `MapLibreNative`.
- Camera, free-camera, bounds, debug, client, resource, tile-cache,
  pixel-ratio, frame-rate, rendering-enable, and memory-reduction controls
  through NAPI.
- Basic one-finger pan, two-finger pinch, two-finger rotation, two-finger shove
  pitch, double-tap zoom, and single-finger fling gesture bridging.

## Build

Set the native SDK paths, then use the HarmonyOS CMake presets:

```sh
export HMOS_SDK_NATIVE=/path/to/harmonyos/native
export OHOS_SDK_NATIVE=/path/to/openharmony/native

cmake --preset harmonyos-opengl
cmake --build --preset harmonyos-opengl

cmake --preset harmonyos-vulkan
cmake --build --preset harmonyos-vulkan
```

The HarmonyOS toolchain wraps the OHOS base toolchain, so both SDK paths are
required. The configure and build presets are disabled until both
`HMOS_SDK_NATIVE` and `OHOS_SDK_NATIVE` are set.
Both presets build the ArkTS/NAPI module and HMS RCP networking path. Use
`harmonyos-opengl` for the EGL/GLES renderer and `harmonyos-vulkan` for the
Vulkan renderer.

The tested HarmonyOS native SDK ships Clang 15. The build keeps MapLibre
Native's current C++20 code path and enables the SDK libc++ experimental ranges
support for HarmonyOS targets.

Apps that load remote styles, tiles, sprites, or glyphs must declare the network
permission in `module.json5`:

```json5
{
  "module": {
    "requestPermissions": [
      { "name": "ohos.permission.INTERNET" }
    ]
  }
}
```

## Project Integration

For an existing HarmonyOS module with `externalNativeOptions`, add MapLibre
Native as a CMake subdirectory and enable the native module:

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
set(MLN_WITH_VULKAN ON CACHE BOOL "" FORCE)
set(MLN_WITH_OPENGL OFF CACHE BOOL "" FORCE)
set(MLN_WITH_EGL OFF CACHE BOOL "" FORCE)
set(MLN_OHOS_BUILD_NATIVE_MODULE ON CACHE BOOL "" FORCE)
add_subdirectory("${MAPLIBRE_NATIVE_ROOT}" "${CMAKE_BINARY_DIR}/maplibre-native")
```

For the EGL/GLES backend, set `MLN_WITH_OPENGL=ON`, `MLN_WITH_EGL=ON`, and
`MLN_WITH_VULKAN=OFF` instead.

The native module target builds `libmaplibre_native_ohos.so`, matching the NAPI
module name `maplibre_native_ohos`. The sample packages both
`libmaplibre_native_ohos.so` and `libc++_shared.so` for each configured ABI.

For sample ArkTS type metadata, add
`platform/ohos/arkts/types/libmaplibre_native_ohos` as an OH package dependency
or copy it into the app module's `src/main/cpp/types/` directory. Treat
`index.d.ts` in that package as experimental sample metadata, not a stable
public language binding.

## ArkTS Usage

Legacy `libraryname` XComponent usage:

```ts
import { common } from '@kit.AbilityKit';
import type { XComponentContext } from 'libmaplibre_native_ohos.so';

let map: XComponentContext | undefined;

XComponent({
  id: 'map',
  type: XComponentType.SURFACE,
  libraryname: 'maplibre_native_ohos',
})
  .onLoad((context?: object | XComponentContext) => {
    if (!context) {
      return;
    }

    map = context as XComponentContext;
    const hostContext = this.getUIContext().getHostContext() as common.Context | undefined;
    if (hostContext) {
      map.setResourceOptions({ cachePath: hostContext.cacheDir + '/maplibre-cache.db' });
    }
    map.setPixelRatio(2);
    map.setRenderingEnabled(true);
    map.jumpTo({ longitude: 0, latitude: 0, zoom: 2 });
    map.setStyleUrl('https://tiles.openfreemap.org/styles/bright');
  })
  .onDestroy(() => {
    map?.destroy();
    map = undefined;
  });
```

Apps should call `reduceMemoryUse()` from low-memory or background lifecycle
hooks where available. Apps can call `setRenderingEnabled(false)` while hidden
to suppress frame-callback rendering, then re-enable it from foreground
lifecycle hooks.

## Sample App

Open `platform/ohos/sample` in DevEco Studio. See
`platform/ohos/sample/README.md` for local signing and renderer selection.
