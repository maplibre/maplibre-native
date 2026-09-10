# Dynamic property binders for plugin layers

Status: implemented in the unpublished v1 C API.

## Objective

Give source-bound plugin layers the same paint-property behavior as generated MapLibre Native layers without exposing C++ templates or renderer objects through the plugin boundary. A plugin property must be able to use constants, camera expressions, feature expressions, composite feature-and-zoom expressions, feature state, runtime paint updates, and transitions. MapLibre must choose uniforms or vertex attributes, own mutable paint buffers, interpolate composite values, and update feature ranges. Plugins continue to own geometry generation and shader semantics.

The implementation modifies ABI v1 in place. The API has not been published, so no compatibility path or legacy descriptor interpretation is required.

## Previous behavior and problem

`supports_expressions` causes core to parse a plugin property as a normal typed `PropertyValue<T>`. Geometry layout evaluates each property once for each feature at the bucket's tile zoom and provides the result through `mln_plugin_feature_v1.evaluated_properties`. Plugins such as rectangle and heatmap copy those values into their own static vertex streams. The render-thread uniform callback separately receives camera-evaluated property values.

This supports simple source expressions such as `rectangle-color: ["get", "color"]` and camera expressions used exclusively as uniforms. It does not implement the binder behavior of built-in layers:

- a feature-and-zoom expression is sampled once at bucket layout zoom instead of storing the two endpoints needed for smooth interpolation;
- feature-state expressions are initially evaluated with empty state and the plugin bucket has no update path;
- all plugin property changes are classified as layout changes, even for paint-only values;
- plugin shaders have one fixed attribute layout rather than uniform/attribute variants;
- existing-layer extension snapshots fall back to defaults for expressions;
- `supports_transitions` is registered but plugin render layers do not transition values.

The present rectangle bucket also demonstrates why segments cannot serve as the feature-to-vertex index: a segment can contain geometry from multiple source features while its single `feature_index` field can name only one.

## Design principles

1. MapLibre owns expression parsing, dependency validation, evaluation, transitions, binder selection, paint buffers, uploads, and feature-state updates.
2. Plugins own geometry topology, property meaning, shader code, and the declaration that maps a property to shader resources.
3. No `PropertyExpression`, `PaintPropertyBinder`, STL type, graphics object, or callback vtable crosses the C boundary.
4. Geometry streams and paint streams are separate. A plugin must not rebuild geometry because a paint value changes.
5. Property behavior is backend-independent. OpenGL, Vulkan, and Metal consume the same host-selected uniform/attribute representation.
6. Array, string, and color-ramp properties remain non-data-driven unless the API defines an explicit GPU encoding for them.

## Property capabilities

The ABI replaces the ambiguous `supports_expressions` byte with dependency capabilities:

```c
typedef enum mln_plugin_expression_capability {
    MLN_PLUGIN_EXPRESSION_CAMERA       = 1u << 0u,
    MLN_PLUGIN_EXPRESSION_FEATURE      = 1u << 1u,
    MLN_PLUGIN_EXPRESSION_COMPOSITE    = 1u << 2u,
    MLN_PLUGIN_EXPRESSION_FEATURE_STATE = 1u << 3u
} mln_plugin_expression_capability;

typedef struct mln_plugin_property_descriptor_v1 {
    /* existing schema fields */
    uint32_t expression_capabilities;
    uint8_t supports_transitions;
} mln_plugin_property_descriptor_v1;
```

Core inspects expression dependencies after parsing and rejects a value whose dependencies exceed the descriptor. `COMPOSITE` means simultaneous camera/zoom and feature dependence. `FEATURE_STATE` requires a feature ID in the source data.

Host-bound data-driven values support boolean, float, float2, color, and declared enum strings. Other strings and arrays may use constants or camera expressions in plugin-managed uniform blocks. Color ramps remain host-generated textures.

## Declarative shader property bindings

A shader declares how a paint property can be presented as both a uniform and an attribute:

```c
typedef enum mln_plugin_property_encoding_v1 {
    MLN_PLUGIN_PROPERTY_ENCODING_FLOAT,
    MLN_PLUGIN_PROPERTY_ENCODING_FLOAT2,
    MLN_PLUGIN_PROPERTY_ENCODING_COLOR,
    MLN_PLUGIN_PROPERTY_ENCODING_BOOLEAN_FLOAT,
    MLN_PLUGIN_PROPERTY_ENCODING_ENUM_FLOAT
} mln_plugin_property_encoding_v1;

typedef struct mln_plugin_shader_property_binding_v1 {
    uint32_t struct_size;
    mln_plugin_string property_name;
    mln_plugin_property_encoding_v1 encoding;

    /* Destination within a declared host-populated uniform block. */
    uint32_t uniform_id;
    uint32_t uniform_byte_offset;

    /* Source/composite attribute locations declared by this shader. */
    uint32_t minimum_attribute_id;
    uint32_t maximum_attribute_id;

    /* Per-drawable interpolation factor destination. */
    uint32_t interpolation_uniform_id;
    uint32_t interpolation_uniform_byte_offset;
} mln_plugin_shader_property_binding_v1;
```

The shader descriptor owns an array of these bindings. Registration validates property names, value encodings, attribute IDs, std140/Metal alignment, uniform bounds, and non-overlapping byte ranges.

Equal minimum/maximum attribute IDs explicitly request packed endpoints. Scalar,
boolean and enum pairs occupy one float2 attribute; float2 pairs occupy one
float4. Color endpoints still require two float4 attributes. The encoding does
not change the property value or uniform representation.

`ENUM_FLOAT` encodes the ordinal in the property's copied `enum_values` list.
Shaders select an enum endpoint rather than linearly blending ordinals. Enum
statistics return strings owned by the descriptor, never temporary evaluation
storage. Unknown feature values fall back to the declared default. Together with
packed endpoints this lets all thirteen n-gon properties be feature-driven within
the shared sixteen-attribute limit. Registration rejects out-of-range attribute
IDs/locations and incompatible packed types before shader compilation.

Core compiles and caches a shader permutation based on the set of properties represented as uniforms. It injects a numeric macro for each binding:

```glsl
#if MLN_PLUGIN_PROPERTY_RECTANGLE_COLOR_IS_UNIFORM
    vec4 color = u_properties.rectangle_color;
#else
    vec4 color = mix(a_rectangle_color_min,
                     a_rectangle_color_max,
                     u_properties.rectangle_color_t);
#endif
```

Metal function constants or source macros and Vulkan/OpenGL preprocessor macros express the same choice. Both attributes may be physically bound for a source-only expression; their values are identical and the interpolation factor is zero. Unused declared attributes are not required in the constant shader permutation.

Plugin `update_uniform_block` runs first for plugin-owned fields. Core then writes and validates its declared property ranges, preventing plugins from accidentally overriding evaluated values.

## Feature-to-vertex mapping

The bucket result adds an explicit mapping from source features to the geometry vertices used by each drawable:

```c
typedef struct mln_plugin_feature_vertex_range_v1 {
    uint32_t struct_size;
    uint64_t feature_index;
    uint64_t drawable_key;
    uint32_t first_vertex;
    uint32_t vertex_count;
} mln_plugin_feature_vertex_range_v1;

typedef struct mln_plugin_bucket_v1 {
    /* existing geometry output */
    const mln_plugin_feature_vertex_range_v1* feature_vertex_ranges;
    size_t feature_vertex_range_count;
} mln_plugin_bucket_v1;
```

Ranges address the drawable's logical vertex sequence, independent of the plugin's geometry stream IDs. Multiple disjoint ranges for one feature and ranges for several drawables are allowed. Core rejects out-of-bounds ranges and ranges naming unknown drawable keys. A data-driven drawable must cover every vertex for which it requests a bound paint property.

The host creates separate interleaved paint streams. Plugin geometry remains immutable and can continue to be shared between layers and drawables.

## Runtime binder

Core adds a type-erased `PluginPaintPropertyBinder` whose concrete storage is selected from the registered value type. It reuses or extracts the algorithms from `PaintPropertyBinder` but is driven by runtime descriptors rather than generated property type lists.

For each layer, property, bucket, and drawable, the binder classifies the possibly evaluated value:

- **Constant or camera-only:** write the current value to the host-owned property UBO; allocate no paint attribute stream.
- **Source expression:** evaluate once per feature and duplicate one encoded value across all of the feature's vertex ranges.
- **Composite expression:** evaluate each feature at the bucket zoom and bucket zoom plus one; write minimum and maximum attributes; update only the interpolation factor every frame.
- **Feature-state expression:** retain feature-ID-to-range mappings and reevaluate affected ranges when source feature state changes.

The binder maintains min/max statistics. These statistics support conservative rendered-feature query radii and other plugin callbacks without rescanning GPU buffers.

`PluginBucket` implements `Bucket::update`. It receives feature-state changes, obtains the corresponding source feature, reevaluates only affected binder ranges, marks the paint stream modified, and causes the normal upload pass to update the buffer. Geometry and indices remain untouched.

## Layout and paint invalidation

Plugin property scope becomes meaningful to style diffing:

- layout property, source, source-layer, filter, or plugin geometry schema change: relayout tiles and replace buckets;
- paint expression change: rebuild or refill binders and possibly select another shader permutation, but retain geometry;
- constant/camera paint change with the same representation: update only property UBOs;
- zoom change: evaluate camera uniforms and composite interpolation factors only;
- feature-state change: update affected paint ranges only;
- visibility or zoom-range change: use normal layer scheduling without relayout.

`PluginStyleLayer::Impl::hasLayoutDifference` compares only layout-scoped properties plus ordinary layout inputs. A separate paint-difference path tells the render layer which binders need replacement.

## Transitions

Paint values are stored in a dynamic equivalent of generated transitionable/evaluated property sets. The runtime property record contains the typed unevaluated value, transition options, transitioning state, and evaluated value. `RenderPluginStyleLayer::transition`, `evaluate`, and `hasTransition` use the normal frame clock and easing behavior.

Transitions are supported only where both endpoints can be interpolated. Boolean, string, array, and color-ramp transitions are rejected unless an explicit interpolation rule is added. A transition does not change whether a property is uniform or data-driven; transitions between incompatible dependency classes complete by switching binder representation at the defined boundary.

## Existing-layer extensions

Existing-layer extension properties without a feature binding may support constants and camera expressions. `PluginLayerHost::makePropertySnapshot` evaluates those values at the current camera instead of replacing expressions with defaults.

Feature/source/composite expressions are rejected for an extension property unless the target packet adapter declares a semantic feature-property channel. This avoids implying that a layer-wide callback value can represent many feature values. Fill-extrusion geometry packets may later expose such channels through the same host binder representation.

## Queries

Precise query callbacks already receive properties evaluated with the queried feature and current feature state. Broad-phase query radius must also respond to dynamic properties. Add an optional callback receiving binder statistics and current camera properties:

```c
typedef struct mln_plugin_property_statistics_v1 {
    mln_plugin_string property_name;
    mln_plugin_value minimum;
    mln_plugin_value maximum;
} mln_plugin_property_statistics_v1;

typedef float (*mln_plugin_query_radius_fn)(
    const mln_plugin_property_statistics_v1* statistics,
    size_t statistics_count,
    const mln_plugin_property_value_v1* camera_properties,
    size_t camera_property_count);
```

The result is cached per bucket/layer and invalidated by binder or feature-state changes. Core clamps invalid values to zero and logs a diagnostic.

## Threading and ownership

- Expression parsing and style mutation remain on style/frontend threads.
- Source and composite evaluation plus initial paint-stream population run on tile layout workers.
- Feature-state reevaluation follows the existing tile bucket update path.
- UBO values and interpolation factors are produced on the render thread.
- Core copies all plugin range and binding descriptors during registration or bucket completion.
- Callback arguments and evaluated C values remain borrowed for the duration of the callback.
- Core owns all dynamic paint CPU/GPU buffers and their uploads.

No plugin callback is invoked once merely to copy standard numeric paint values. Optional plugin callbacks remain for custom uniform fields, query-radius calculation, layout, and precise hit testing.

## Implemented changes

1. Replaced `supports_expressions` with dependency capabilities and validation of parsed expression dependencies.
2. Added shader property-binding descriptors and backend-independent registration validation.
3. Added feature vertex ranges to bucket output and updated rectangle/heatmap layouts to emit them.
4. Added a type-erased runtime binder for float, float2, color, and boolean values.
5. Stored host-owned plugin paint streams in `PluginBucket` and bound them alongside plugin geometry streams.
6. Added shader permutation keys and injected uniform/attribute macros to all three shader backends.
7. Implemented composite interpolation UBO updates in `PluginLayerTweaker`.
8. Implemented `PluginBucket::update` and partial feature-state buffer updates.
9. Split plugin layout and paint invalidation, removing blanket plugin-property relayout.
10. Implemented plugin transitionable/evaluated paint storage.
11. Evaluated camera expressions for existing-layer extension snapshots and rejected unsupported dependencies.
12. Added per-layer dynamic query-radius statistics and callback support.
13. Converted rectangle and heatmap plugins to declarative host bindings, removing their manually baked paint fields.

## Validation

Core tests cover descriptor validation, dependency restrictions, encoding and buffer ranges, and transition parsing/interpolation. Plugin render fixtures exercise the host binder paths end to end.

Plugin render tests cover:

- rectangle width, height, and color as source expressions, with constant stroke properties;
- width and height as feature-and-zoom expressions at fractional zoom;
- color, width, and height updates driven by feature state;
- runtime replacement of color, width, height, and stroke width without relayout;
- shared expectations used by the OpenGL, Vulkan, and Metal plugin runners;
- heatmap camera/source-expression parity with the retained built-in heatmap fixtures.

The rectangle suite includes runtime paint replacement, feature-state mutation, and a fractional composite-zoom fixture. The shared runner also executes the retained heatmap fixtures, including feature-driven weight and radius cases, against the same host binder implementation.

## Acceptance criteria

- Plugin composite expressions interpolate continuously and match built-in layer semantics at fractional zooms.
- Feature-state changes update visible plugin features without tile relayout.
- Paint-only changes do not recreate plugin geometry or indices.
- Constant properties use uniforms; source/composite properties use host-owned attributes.
- Plugin transitions use the ordinary MapLibre timing model.
- No plugin-specific property or shader code is added to core.
- No C++ or renderer object is exposed by the C API.
