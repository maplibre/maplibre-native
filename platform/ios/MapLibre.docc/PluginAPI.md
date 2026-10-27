# Plugin API

Extend MapLibre iOS with native code.

> Warning: The plugin API is experimental and does not yet have stability guarantees.

## Overview

MapLibre Native has a C plugin API for extending it with native code. Currently, plugins can add style layer types. After a plugin registers its layer types, styles can use them like the built-in layer types, for example `"type": "ngon"`. Layers of plugin types appear as ``MLNStyleLayer`` in `MLNStyle.layers`. The API is defined in [`plugin_api.h`](https://github.com/maplibre/maplibre-native/blob/main/include/mln/plugin/plugin_api.h), which documents the ownership and threading rules that plugins must follow.

A plugin describes a layer type with the structures of `plugin_api.h`:

- The name of the layer type, as used for `"type"` in a style.
- The paint properties of the layer, with their type, default value and the kinds of expressions they accept.
- The shaders of the layer. On iOS, which renders with Metal, a plugin provides the complete Metal Shading Language source with its entry points.
- Callbacks that turn the features of a tile into vertex data, update the uniforms of the shaders and answer queries for rendered features.

## Add MapLibre with the plugin API

Only the build of MapLibre iOS distributed as the Swift package [maplibre-ios-with-plugin-api](https://github.com/maplibre/maplibre-ios-with-plugin-api) exports the plugin API. Apart from that, it is the same as the standard build, and applications keep using `import MapLibre`. Use it instead of the standard package, not in addition to it.

The package has two products:

| Product | Use it in | Contents |
| --- | --- | --- |
| `MapLibre` | Applications | `MapLibre.xcframework` with the plugin API enabled. |
| `MapLibrePluginApi` | Plugin packages | The C plugin API, included as `<mln/plugin/plugin_api.h>`. |

The GitHub release of each MapLibre iOS version also includes a static XCFramework of this build, `MapLibre.static.plugins.xcframework.zip`.

## Write a plugin

A plugin package depends only on `MapLibrePluginApi`, so the application decides which MapLibre framework it links. Both products come from the same package, so Swift Package Manager resolves the header and the framework to the same version.

```swift
dependencies: [
    .package(url: "https://github.com/maplibre/maplibre-ios-with-plugin-api", exact: "<version>"),
],
targets: [
    // Application
    .target(name: "App", dependencies: [.product(name: "MapLibre", package: "maplibre-ios-with-plugin-api")]),
    // Plugin
    .target(name: "MyLayer", dependencies: [.product(name: "MapLibrePluginApi", package: "maplibre-ios-with-plugin-api")]),
]
```

Because the plugin does not link MapLibre, it does not call `mln_plugin_register_v1` itself. Instead it exposes a function that receives the registration function and passes its descriptor to it:

```c
#include <mln/plugin/plugin_api.h>

mln_plugin_status my_layer_register(mln_plugin_register_function_v1 register_plugin,
                                    char *error_message,
                                    size_t error_message_capacity) {
    if (!register_plugin) return MLN_PLUGIN_STATUS_NOT_FOUND;
    return register_plugin(&descriptor, error_message, error_message_capacity);
}
```

Here `descriptor` is the `mln_plugin_descriptor_v1` of the plugin. Set `struct_size` of every structure to the size of its type, as `plugin_api.h` describes.

## Register a plugin

The application passes `mln_plugin_register_v1` to the plugin. Register all plugins before creating a map that loads a style using their layer types:

```objc
#import <MapLibre/MapLibre.h>
#include "my_layer.hpp" // includes <mln/plugin/plugin_api.h>

char error[512] = {};
mln_plugin_status status = my_layer_register(&mln_plugin_register_v1, error, sizeof(error));
if (status != MLN_PLUGIN_STATUS_OK && status != MLN_PLUGIN_STATUS_ALREADY_REGISTERED) {
    NSLog(@"Failed to register plugin: %s", error);
}
```

Registration is process-wide and thread-safe. Registering the same plugin again returns `MLN_PLUGIN_STATUS_ALREADY_REGISTERED`. Plugins cannot be unregistered.

## Example

The [n-gon layer](https://github.com/maplibre/maplibre-native/tree/main/plugins/ngon-layer) in the repository is an example plugin. It draws regular polygons at point features and registers the `ngon` layer type with `mln_ngon_layer_register`.
