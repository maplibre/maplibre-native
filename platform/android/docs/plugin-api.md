# Plugin API

!!! warning

    The plugin API is experimental and does not yet have stability guarantees.

MapLibre Native has a C plugin API for adding style layer types implemented in native code. After a plugin registers its layer types, styles can use them like the built-in layer types, for example `"type": "ngon"`. The API is defined in [`plugin_api.h`](https://github.com/maplibre/maplibre-native/blob/main/include/mln/plugin/plugin_api.h).

## Artifacts

Only the artifacts with the `-with-plugin-api` postfix export the plugin API. Apart from that, they are the same as the [other artifacts](data/rendering-engine.md):

| Artifact | Backend |
|---|---|
| `org.maplibre.gl:android-sdk-with-plugin-api` | Vulkan (the current default) |
| `org.maplibre.gl:android-sdk-opengl-with-plugin-api` | OpenGL ES |
| `org.maplibre.gl:android-sdk-vulkan-with-plugin-api` | Vulkan |
| `org.maplibre.gl:android-sdk-vulkan-opengl-with-plugin-api` | Both, selected at runtime |

Use one of them instead of the artifact without the postfix:

```gradle
dependencies {
    implementation 'org.maplibre.gl:android-sdk-with-plugin-api:<version>'
}
```

## Writing a plugin

A plugin is a native library that registers its layer types with `mln_plugin_register_v1`:

- Include `<mln/plugin/plugin_api.h>`. The AAR provides it through the header-only Prefab module `maplibre` (enable `buildFeatures { prefab = true }`, then use `find_package(MapLibreAndroid REQUIRED CONFIG)` and `MapLibreAndroid::maplibre` in CMake).
- Look up `mln_plugin_register_v1` in the already loaded MapLibre library (`libmaplibre.so`) with `dlopen` and `dlsym`. With the `-vulkan-opengl` artifact, the OpenGL ES renderer is `libmaplibre-opengl.so`.
- Register after `MapLibre.getInstance()` and before loading a style that uses the plugin's layer types.

The [n-gon layer](https://github.com/maplibre/maplibre-native/tree/main/plugins/ngon-layer) in the repository is an example plugin, with an [Android demo app](https://github.com/maplibre/maplibre-native/tree/main/plugins/android).
