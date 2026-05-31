# MapLibre Native HarmonyOS Sample

This is a minimal HarmonyOS XComponent app shell for the experimental
`maplibre_native_ohos` NAPI module.

Build it with the DevEco command-line tools:

```sh
cd platform/ohos/sample
(cd entry && /path/to/command-line-tools/bin/ohpm install)
/path/to/command-line-tools/bin/hvigorw assembleApp --no-daemon
/path/to/command-line-tools/bin/hvigorw assembleApp -p product=opengl --no-daemon
```

Install it once `hdc list targets` shows a device or emulator:

```sh
/path/to/command-line-tools/sdk/default/openharmony/toolchains/hdc install -r \
  entry/build/default/outputs/default/app/entry-default.hap
```

For device installs, configure local signing first with the instructions in
`sign/README.md`.

The sample builds MapLibre Native from this checkout through
`entry/src/main/cpp/CMakeLists.txt`, imports `libmaplibre_native_ohos.so` from
ArkTS, consumes the local `libmaplibre_native_ohos.so` type package, declares
`ohos.permission.INTERNET`, and packages `libc++_shared.so` with the native
module.

The default product uses the Vulkan renderer. Use `-p product=opengl` to build
the EGL/GLES product instead. The UI starts with
`https://tiles.openfreemap.org/styles/bright`, displays the backend label and
frame rates, shows style attribution, and includes remote style buttons:

- Demo: `https://demotiles.maplibre.org/style.json`
- Bright: `https://tiles.openfreemap.org/styles/bright`
- Liberty: `https://tiles.openfreemap.org/styles/liberty`

Runtime validation should confirm that the on-screen state reports a nonzero
surface size, `window=yes`, `map=yes`, a backend label, increasing frame rates
while the map is running, and no last map/render/glyph/sprite error.
