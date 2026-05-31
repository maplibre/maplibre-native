# MapLibre Native HarmonyOS Sample

This is a minimal HarmonyOS XComponent app shell for the experimental
`maplibre_native_ohos` NAPI module.

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
default`, and the generated HAP has no signature files. A real device install may
require adding a DevEco/Harmony signing config or signing the HAP with the target
device's accepted profile before running `hdc install`.

The sample builds MapLibre Native from this checkout through
`entry/src/main/cpp/CMakeLists.txt`, imports `libmaplibre_native_ohos.so` from
ArkTS, consumes the local `libmaplibre_native_ohos.so` type package, declares
`ohos.permission.INTERNET`, and packages `libc++_shared.so` with the native
module.

The sample defaults to the Vulkan renderer. Configure with
`-DMLN_OHOS_SAMPLE_RENDERER=OpenGL` to build the EGL/GLES backend instead. The UI
starts with `https://tiles.openfreemap.org/styles/bright`, displays the backend
label and frame rates, shows style attribution, and includes remote style buttons:

- Demo: `https://demotiles.maplibre.org/style.json`
- Bright: `https://tiles.openfreemap.org/styles/bright`
- Liberty: `https://tiles.openfreemap.org/styles/liberty`

Runtime validation should confirm that the on-screen state reports a nonzero
surface size, `window=yes`, `map=yes`, a backend label, increasing frame rates
while the map is running, and no last map/render/glyph/sprite error.
