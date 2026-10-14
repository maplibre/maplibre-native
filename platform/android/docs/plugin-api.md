# Plugin API

!!! warning

    The plugin API is experimental and does not yet have stability guarantees.

MapLibre Native has a C plugin API for adding style layer types implemented in native code. After a plugin registers its layer types, styles can use them like the built-in layer types, for example `"type": "ngon"`. The API is defined in [`plugin_api.h`](https://github.com/maplibre/maplibre-native/blob/main/include/mln/plugin/plugin_api.h).

## Artifacts

Only the [artifacts](artifacts.md) with the `-with-plugin-api` postfix export the plugin API. Apart from that, they are the same as the artifacts without it. For example, use `android-sdk-with-plugin-api` instead of `android-sdk`:

```gradle
dependencies {
    implementation 'org.maplibre.gl:android-sdk-with-plugin-api:<version>'
}
```

## Writing a plugin

A plugin is a native library that registers its layer types with `mln_plugin_register_v1`:

- Include `<mln/plugin/plugin_api.h>`. The AAR provides it through the header-only Prefab module `maplibre`: enable `buildFeatures { prefab = true }`, then use `find_package(MapLibreAndroid REQUIRED CONFIG)` and `MapLibreAndroid::maplibre` in CMake.
- Look up `mln_plugin_register_v1` in the MapLibre native library that is already loaded, with `dlopen` and `dlsym`. This library is `libmaplibre.so`, except with the OpenGL ES backend of the `-vulkan-opengl-with-plugin-api` artifact, which uses `libmaplibre-opengl.so`.
- Register after `MapLibre.getInstance()` and before loading a style that uses the plugin's layer types.

The [n-gon layer](https://github.com/maplibre/maplibre-native/tree/main/plugins/ngon-layer) in the repository is an example plugin, with an [Android demo app](https://github.com/maplibre/maplibre-native/tree/main/plugins/android).
