# MapLibre Native plugin interface

Status: development ABI v1, Android OpenGL/Vulkan and Darwin Metal.

## Scope

A plugin registers a **new source-bound layer type**. It cannot add properties
to a built-in layer, intercept its render callbacks, or borrow private GPU
buffers. The former fill-extrusion shadow extension is removed. Built-in circle,
heatmap, and hillshade remain unchanged alongside the independent plugin types.

New plugins normally need no core changes. A new generic capability is added to
the host only when an actual cross-platform use case needs it.

## C++ foundation and C boundary

`LayerManager::registerLayerFactories` atomically registers owned C++ factories.
Type metadata has factory lifetime, duplicate names (including built-in names)
are rejected, and normal style/layout/bucket/render construction uses the same
factory lookup. This API is for code compiled against the same core, not a DSO
ABI. The C descriptor adapter implements a regular `LayerFactory`.

The public plugin boundary is the pure-C header `mln/plugin/plugin_api.h`.
Android obtains it from the renderer-independent `android-plugin-api` Prefab
artifact; Bazel uses `//:plugin-api`; Darwin exposes it through `MLNPluginAPI.h`.
No MapLibre C++ classes, STL values, exceptions, RTTI, or allocation ownership
cross that boundary. Host and plugin may each use private static C++ runtimes.

The host owns:

- registration, copied metadata, validation, and style construction;
- typed values, expressions, transitions, feature state, and dynamic binders;
- tile scheduling, feature indexing, bucket validation, and GPU uploads;
- drawable lifetime, shader variants, backend resource bindings, and render graphs;
- file-source requests, offscreen targets, DEM textures, and color-ramp textures.

The plugin owns property meanings, CPU layout, shader source, descriptor code,
and platform convenience wrappers. It issues no backend rendering commands.

## Registration

`mln_plugin_descriptor_v1` identifies a plugin/version and its layer types.
Each layer declares its source kind, geometry mask, render stage, 3D behavior,
properties, shaders, and layout callbacks or RasterDEM render graph.

`mln_plugin_register_v1` copies metadata and strings, but the native library owns
callback code and must remain loaded. Register before loading dependent styles.
Registration is process-wide and thread-safe; unloading and unregistration are
not supported. Identical repeated registration returns `ALREADY_REGISTERED`.
Conflicts and malformed descriptors return a status and a caller-owned diagnostic.

The API is unpublished and remains v1 during development. `struct_size` and ABI
fields validate matching headers; they do not promise compatibility with earlier
development snapshots. Build the host and plugins against matching revisions.

## Properties

Only the dedicated plugin layer implementation stores plugin values. Generated
built-in layer setters, getters, and immutable state have no registry fallback or
plugin property bag. An unregistered plugin type remains an unsupported type.

Values are boolean, float, float2, color, UTF-8 string, float/color arrays, or a
color-ramp expression. Descriptors declare expression dependencies, numeric
constraints, enum strings, array limits, and transition support. Missing values
use defaults without serialization; explicitly set values, including false,
survive clone and serialization. Updates use normal layer observers and repaint.

Dynamic paint bindings declare uniform ranges, per-vertex endpoint attributes,
and interpolation-factor ranges. The host evaluates feature values and
composite zoom endpoints, refreshes feature-state ranges, and supplies current
camera interpolation factors. Shader variants replace constant attributes with
uniforms. Tile geometry does not rebuild merely because paint values change.
Scalar/boolean/enum endpoint pairs may share a float2 attribute; float2 endpoints
may share a float4 attribute. Declared enum strings have an explicit ordinal
encoding, so map/viewport options can also be feature-driven. The host validates
attribute IDs against the portable sixteen-attribute limit.
See [dynamic-property-binders.md](dynamic-property-binders.md).

## Layout and ownership

Geometry plugins use GeoJSON or vector sources. Worker-thread `create_layout`,
`layout_feature`, `finish_layout`, and `destroy_layout` callbacks generate CPU
vertex streams, indices, segments, drawables, and feature-to-vertex ranges.
MapLibre validates and copies those arrays before callback storage is destroyed.
Features and property snapshots are borrowed only for their callback duration.

Resource requests use the configured `FileSource` and cache. Layout requests are
completed through a dedicated run loop; response bytes are borrowed during the
resource callback and must be copied if retained. Failed layout does not produce
a partial bucket. Plugins must not throw exceptions across callbacks.

## Shaders and rendering

Every shader declares attributes, uniform blocks, textures, stages, and scopes.
Core assigns resource bindings and injects numeric GLSL/MSL binding macros.
`update_uniform_block` fills host-owned bytes from borrowed tile/camera/property
inputs. CPU bucket geometry becomes normal host drawables with declared depth,
blend, stencil, and cull state; the host handles updates, removal, and context
replacement. No implicit shader layout or raw graphics context is exposed.

A declarative, topologically ordered graph can render plugin bucket geometry,
viewport quads, or RasterDEM full/masked tile geometry into host RGBA8/RGBA16F
targets and composite later passes. Inputs can be DEM data, earlier targets, or
host-generated color-ramp textures. Geometry graphs use viewport/per-layer
targets; DEM graphs use source-tile/per-tile targets. Unsupported graph shapes
are rejected at registration, not silently ignored.

## Examples

- `gltf` uses point anchors and layout resource requests for static TinyGLTF GLB
  meshes. The host owns its drawables and depth state. Textures, animation,
  skinning, morph targets, and compressed geometry are not implemented.
- `rectangle` uses point quads and host dynamic bindings for size, fill, and stroke.
- `ngon` uses analytic convex-polygon coverage with thirteen simultaneously
  data-driven properties. Packed endpoints leave room for rotation, corners,
  translation, and both pitch options. Query callbacks receive the same borrowed
  tile projection and camera distance needed to test the actual rotated polygon.
- `org.maplibre.hillshade` samples DEMs into derivatives, then composites shaded
  masked tiles. Its fixtures are ported from the retained built-in hillshade.
- `org.maplibre.heatmap` accumulates point density into a half-size float target,
  then samples density and a color ramp. Its fixtures are ported from the retained
  built-in heatmap.

## Packaging and limitations

Android wrappers load MapLibre before their native library, obtain the C
registration function from `MapLibrePluginRegistry`, and invoke it through JNI.
They compile against the Java API with `compileOnly`; the application chooses
exactly one renderer. Darwin wrappers use the same shared registration code;
SwiftPM and XCFramework packages do not introduce another interface.

The interface does not yet provide source-less overlays, application-owned
mutable drawable instances, arbitrary image uploads, host primitive tessellators,
or independent per-object animation/material state. Reserved source kinds not
implemented by the host are rejected. Use built-in layers and GeoJSON/custom
sources where possible.

## Simplification decisions

The existing-layer extension host, borrowed extrusion packets, raw graphics
contexts, generated-layer property fallbacks, and shadow renderer are removed.
Factory-owned immutable type identity replaces structural type comparisons.
Shared shader generation keeps the n-gon projection and coverage algorithm in one
place while emitting backend-specific syntax. The render runner remains unaware
of individual plugin semantics and discovers their fixtures from manifests.

Keep explicit resource layouts, typed property capabilities, geometry/raster-DEM
adapters, and render graphs: the retained GLTF, rectangle, hillshade, and heatmap
plugins need them. Do not replace these with backend callbacks or plugin-specific
host branches. General layout sort keys, collision placement, arbitrary images,
and per-object animation need separate designs rather than implicit fallbacks.
