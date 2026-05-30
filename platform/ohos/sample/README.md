# MapLibre Native OpenHarmony Sample

This is a minimal HarmonyOS/OpenHarmony XComponent app shell for the
experimental `maplibre_native_ohos` NAPI module. It declares support for
phone and tablet device types, matching the currently known issue target.

The sample bundle name is `org.maplibre.native.demo` (AGC rejects `ohos` in
bundle IDs). The UI uses the legacy declarative `XComponent` `libraryname` path
because `registerXComponentNode()` / `OH_ArkUI_SurfaceHolder_Create()` failed on
a HarmonyOS 6.1 MatePad with `Could not create XComponent surface holder
callback`.

Build it with the DevEco command-line tools:

```sh
cd platform/ohos/sample
(cd entry && /path/to/command-line-tools/bin/ohpm install)
/path/to/command-line-tools/bin/hvigorw assembleApp --no-daemon
```

Install it once `hdc list targets` shows a device or emulator:

```sh
/path/to/command-line-tools/sdk/default/openharmony/toolchains/hdc install -r \
  entry/build/default/outputs/default/app/entry-default.hap
```

The command-line build currently warns `No signingConfig found for product
default`, and the generated HAP has no signature files. A real device install
may require adding a DevEco/Harmony signing config or signing the HAP with the
target device's accepted profile before running `hdc install`.

The sample bundle name is `org.maplibre.native.demo`, and the entry ability is
`EntryAbility`. If the target shell provides `aa`, a candidate launch command is:

```sh
/path/to/command-line-tools/sdk/default/openharmony/toolchains/hdc shell \
  aa start -b org.maplibre.native.demo -a EntryAbility
```

The sample builds MapLibre Native from this checkout through
`entry/src/main/cpp/CMakeLists.txt`, imports `libmaplibre_native_ohos.so` from
ArkTS, consumes the local `libmaplibre_native_ohos.so` type package, declares
`ohos.permission.INTERNET`, and packages `libc++_shared.so` with the native
module. The sample defaults to the Vulkan renderer; configure with
`-DMLN_OHOS_SAMPLE_RENDERER=OpenGL` to build the EGL/GLES backend instead. It
starts as a pumped map app: the XComponent frame callback drives
`MapView::renderFrame()`, startup loads
`https://demotiles.maplibre.org/style.json`, and a compact status readout is
refreshed while the map is running.

On the DevEco emulator tested on 2026-05-30, the Vulkan loader advertises
Vulkan 1.3.275 with `VK_KHR_surface` and `VK_OHOS_surface`, but
`vkCreateInstance` returns `VK_ERROR_INCOMPATIBLE_DRIVER`. The same Vulkan
sample runs on a HarmonyOS tablet with a `Maleoon 920` Vulkan 1.3.275 device and
renders both the remote demotiles style and the local inline GeoJSON style. Use
the OpenGL sample renderer for emulator rendering checks unless a Vulkan-capable
emulator image is available. Huawei's DevEco Studio emulator specifications are
published at
<https://developer.huawei.com/consumer/en/doc/harmonyos-guides/ide-emulator-specification>.
They state that emulator versions before DevEco Studio 6.1.0 Beta2 do not
support Vulkan; DevEco Studio 6.1.0 Beta2 and newer support Vulkan APIs except
`vkGetSwapchainGrallocUsageOHOS`, `vkAcquireImageOHOS`, and
`vkQueueSignalReleaseImageOHOS`.

The status readout and logs still expose style read/write, camera, free-camera
read/write, bounds, client/resource options, debug, surface-state readback
including map/style/render callback diagnostics, surface callback/error
counters, surface visibility counters, selected GLES context version, renderer
diagnostics, frame/touch/gesture callback counters, and lightweight resource
callback counters plus the last missing-style-image id and last-error strings
for map/render/glyph/sprite failures, rendering, frame-rate, pixel-ratio,
tile-cache, and memory lifecycle hooks.

The native module also compile-tests XComponent touch input handling for
one-finger pan, two-finger pinch/rotate, double-tap zoom, and fling gestures.
The sample derives the tile cache path from the ability `cacheDir` instead of
hardcoding an app sandbox path. The `Remote` and `Local` buttons switch between
the remote demotiles style and a deterministic inline GeoJSON style. The `Fit`
button adjusts the camera without manually forcing a render; the native frame
pump should pick up the camera update.

Runtime validation should confirm that the on-screen state reports a nonzero
surface size, `window=yes`, `map=yes`, increasing frame counts while the map is
running, increasing touch/gesture counters after interacting with the map, and
no last map/render error. Use the on-screen glyph/sprite/tile/missing-image
counters and runtime logs to inspect native `MapLibre` loading behavior.
Network failures should include the SDK error name and request URL in the
reported error string. Image decoder failures should include the ImageKit error
name or decoded pixel metadata in the reported glyph/sprite/map error string.
