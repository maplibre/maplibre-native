# Plugins

Plugins are C/C++ libraries that register new style layer types through the
[C plugin API](../include/mln/plugin/plugin_api.h). This directory separates the plugins, their generated platform
wrappers and the demo apps:

```
plugins/
  plugins/<id>/        the plugins: platform-independent code, tests, render tests and a plugin.json
  android/<id>/        generated: the Android library module of each plugin that supports Android
  ios/<id>/            generated: the iOS (Bazel) library of each plugin that supports iOS, and a registry of them all
  demo/assets/         styles and models the demo apps share
  demo/android/        the Android demo app, lists the demo of every plugin
  demo/ios/            the iOS demo app, lists the demo of every plugin
  tools/               the generator
  BUILD.bazel, CMakeLists.txt, render_test.cpp    shared build and render-test entry points
```

A plugin's directory is named after its `id`, so `plugins/plugins/fill-extrusion/` has the id `fill-extrusion`.

## plugin.json

Every plugin describes itself in `plugin.json`, the format of
[maplibre-native-plugins](https://github.com/louwers/maplibre-native-plugins), so a plugin can move between this
repository and a standalone plugin repository. Paths are relative to the plugin's directory.

```json
{
  "schemaVersion": 1,
  "id": "ngon-layer",
  "displayName": "N-gon layer",
  "description": "Source-bound regular polygon (n-gon) style layer",
  "version": "0.1.1",
  "registerFunction": "mln_ngon_layer_register",
  "header": "shared/include/ngon_layer.hpp",
  "sources": ["shared/cpp/ngon_layer.cpp"],
  "includeDirectories": ["shared/include", "shared/cpp"],
  "demo": {
    "style": "examples/ngon.json",
    "camera": {"latitude": 0, "longitude": 0, "zoom": 11.3}
  }
}
```

`registerFunction` is declared in `header` as `mln_plugin_status f(mln_plugin_register_function_v1, char*, size_t)`.
The Android wrappers compile `sources` with `includeDirectories`, so those must list everything the plugin needs. The
iOS wrappers link the plugin's own Bazel library instead (see [Generated wrappers](#generated-wrappers)). Differences
from the standalone format, all optional:

| Key | Meaning |
| --- | --- |
| `includeDirectories` | May point outside the plugin directory, anywhere inside this repository (for example a library under `../../../vendor/`). |
| `backends` | The renderers the plugin implements, a subset of `opengl`, `vulkan` and `metal` (default: all). A plugin with neither `opengl` nor `vulkan` gets no Android wrapper, and the generated `register()` returns `false` on a renderer outside the list. A plugin with neither `opengl` nor `metal` gets no iOS wrapper, and one without `opengl` only builds in a Metal build. |
| `demo.style` | A style file in the plugin directory, or an `https://` URL. |
| `demo.camera` | `latitude`, `longitude` and `zoom`, and optionally `pitch` and `bearing`. |

## Generated wrappers

`plugins/android/<id>/` is a Gradle library module generated from `plugin.json` by `plugins/tools/generate.py`: the
CMake build of the plugin sources, the JNI registration function, a Kotlin `register()`
(`org.maplibre.plugins.ngonlayer.NgonLayerPlugin.register()` for `ngon-layer`) and the demo metadata.

`plugins/ios/<id>/` is a Bazel library (`//plugins/ios/<id>`), generated the same way: an Objective-C class
`MLN<Name>Plugin` with the API of [maplibre-native-plugins](https://github.com/louwers/maplibre-native-plugins), the
demo metadata, and a `module_name` so Swift imports it:

```swift
import NgonLayer

try NgonLayerPlugin.registerPlugin()
```

It depends on the plugin's Bazel library, `//plugins/plugins/<id>`, which defines the sources and compiler flags of
every Bazel build (the SDK, the tests and the render tests) in one place. Compiling `sources` a second time instead
would define the plugin's symbols twice in an app whose SDK links the plugin. The library
must therefore exist, be named after the plugin and export the plugin's header.
`//plugins/ios:plugins` links the wrappers of all plugins and adds `PluginRegistry`, which registers one by its ID, and
`//plugins/ios:demos` collects their demo metadata.

The files are committed. After changing a `plugin.json`, a demo style or `plugins/tools/templates/`, regenerate them:

```sh
python3 plugins/tools/generate.py           # write
python3 plugins/tools/generate.py --check   # fail if they are out of date; CI runs this
```

The n-gon shaders are generated from `plugins/plugins/ngon-layer/scripts/generate-shaders.mjs` and checked in as
`shared/cpp/ngon_shader_sources.hpp`, so builds don't need Node.js. Regenerate the header after changing the script:

```sh
node plugins/plugins/ngon-layer/scripts/generate-shaders.mjs --output plugins/plugins/ngon-layer/shared/cpp/ngon_shader_sources.hpp
```

## Add a plugin

1. Create `plugins/plugins/<id>/` with the plugin's sources and a `plugin.json`.
2. Give it a Bazel library named `<id>` in `plugins/plugins/<id>/BUILD.bazel` (copy a neighbor's), so the SDK, the
   tests and the iOS wrapper can use it.
3. Run `python3 plugins/tools/generate.py` and commit the new `plugins/android/<id>/` and `plugins/ios/<id>/`.

The demo apps pick it up without further changes if its `plugin.json` has a `demo`. The Android demo's
`settings.gradle.kts` includes every module in `plugins/android/` and lists the demo metadata they contain, and the iOS
demo lists the `//plugins/ios:demos` metadata.

## Demo apps

The Android demo links the SDK built from source (`includeBuild("platform/android")`), so build a flavor that exports the
plugin API:

```sh
cd plugins/demo/android
./gradlew -Pmaplibre.abis=arm64-v8a :app:assembleVulkanWithPluginApiDebug
adb install -r -t app/build/outputs/apk/vulkanWithPluginApi/debug/app-vulkanWithPluginApi-debug.apk
adb shell am start -n org.maplibre.plugindemo.vulkan/org.maplibre.plugins.android.DemoActivity -e plugin fill-extrusion
```

`MainActivity` lists the demos, and `DemoActivity -e plugin <id>` opens one. A plugin registers process-wide, so the
fill-extrusion demo replaces the built-in layer until the app restarts. The iOS demo is built with
`bazel build //plugins/demo/ios:PluginDemo --//:renderer=metal --//:plugins=true` and described in
[demo/ios/README.md](demo/ios/README.md). Styles and models the demos share are in [demo/assets](demo/assets).
