# C++ Plugins Design Proposal

## Motivation

MapLibre Native currently requires new layer types and style-loading behavior to be implemented inside the core engine or platform SDKs. This makes small rendering experiments, proprietary overlays, and platform-specific extensions expensive to maintain because they need to live in a fork or wait for upstream layer support.

The C++ plugins work adds a narrow extension point for registering plugin-provided layer types and style preprocessors at runtime.

## Proposed Change

The change introduces a core plugin API under `mbgl::plugin`:

* `MapLayerType` describes a custom style layer type, its paint/property metadata, and how to create the runtime layer object.
* `MapLayer` is the plugin-owned runtime layer implementation. It receives drawing state, evaluated properties, memory events, and a platform rendering context.
* `StylePreprocessor` can transform style JSON before the core style parser processes it.
* Platform code provides a `RenderingContext` subtype so plugin layers can render with the active backend, such as Metal on Darwin or OpenGL on Android.

`PluginManager` is the central runtime registry for these plugin extension points. It is a singleton so plugin code can register itself before maps or styles are loaded. For layer plugins, it stores registered `MapLayerType` instances and forwards them into the existing `LayerManager` by creating plugin layer factories. When the style parser later encounters that custom layer type, the factory creates a `PluginLayer`, wires it to the plugin's `MapLayer`, passes evaluated properties to the plugin, and calls the plugin's update and render callbacks during normal map rendering.

For style preprocessors, `PluginManager` stores a list of `StylePreprocessor` instances. During style load, the style implementation runs the loaded style JSON through the registered preprocessors before parsing. This allows plugins to adapt style documents without changing the core parser.

## API Modifications

The feature adds C++ APIs for:

* registering plugin map layer types,
* registering style preprocessors,
* defining plugin layer properties,
* receiving drawing context and platform rendering context callbacks.

Darwin and Android add platform glue and examples showing how SDK/application code can register plugin layers and pass backend-specific rendering objects to plugin code.

## Use Cases

This enables:

* custom rendered overlays that should participate in the map render loop,
* proprietary or experimental layer types outside the core style specification,
* platform-specific rendering integrations using Metal, OpenGL, or future backends,
* style JSON preprocessing for compatibility layers, feature flags, or plugin-specific style syntax,
* plugin example libraries that can be developed and shipped independently from the core SDK.

## Migration Plan and Compatibility

The changes are additive. Existing styles and applications continue to work without registering plugins. Plugin layers are only used when application code registers a plugin layer type and a loaded style references that custom layer type.

## Rejected Alternatives

The main alternative is to keep adding every custom layer type directly to core. That keeps the style parser simpler but forces experimentation and product-specific rendering into the main engine. A runtime plugin registry keeps the core API smaller while still allowing advanced users to extend rendering behavior.
