#ifndef MLN_PLUGIN_API_H
#define MLN_PLUGIN_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#define MLN_PLUGIN_EXPORT __declspec(dllexport)
#else
#define MLN_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

#define MLN_PLUGIN_ABI_VERSION_1 1u

/* Maximum vertex attributes per plugin shader. Attribute IDs and locations
 * must be below this limit, matching the portable OpenGL/Vulkan minimum. */
enum {
    MLN_PLUGIN_MAX_VERTEX_ATTRIBUTES = 16
};

/*
 * Ownership and call contract
 * ---------------------------
 * Set struct_size to sizeof the corresponding v1 struct. Receivers must check
 * it before accessing other fields. Arrays use the declared v1 element stride;
 * struct_size is not an array stride. Unless explicitly optional, a nonzero
 * count/byte size requires a non-null pointer to that many accessible elements/
 * bytes. Validation cannot make arbitrary or dangling pointers safe; callers must
 * supply valid storage.
 *
 * Except for the layout-instance transfer described below, pointers are borrowed:
 * each side allocates and frees its own storage. No STL objects, C++ exceptions,
 * or allocator ownership cross this interface. All callbacks, including destroy,
 * must return normally without throwing. C++ plugins must catch exceptions inside
 * their callbacks and translate failures to status codes where available.
 *
 * Host-provided callback inputs (including nested arrays/strings) and writable
 * outputs are valid only for the duration of the callback. Do not retain them;
 * copy anything needed later. Callback functions must remain callable for the
 * process lifetime: plugin unloading/unregistration is not supported.
 */

typedef enum mln_plugin_status {
    MLN_PLUGIN_STATUS_OK = 0,
    MLN_PLUGIN_STATUS_ALREADY_REGISTERED = 1,
    MLN_PLUGIN_STATUS_INVALID_ARGUMENT = 2,
    MLN_PLUGIN_STATUS_UNSUPPORTED_ABI = 3,
    MLN_PLUGIN_STATUS_CONFLICT = 4,
    MLN_PLUGIN_STATUS_NOT_FOUND = 5,
    MLN_PLUGIN_STATUS_CALLBACK_ERROR = 6
} mln_plugin_status;

typedef enum mln_plugin_backend {
    MLN_PLUGIN_BACKEND_OPENGL = 1u << 0u,
    MLN_PLUGIN_BACKEND_VULKAN = 1u << 1u,
    MLN_PLUGIN_BACKEND_METAL = 1u << 2u
} mln_plugin_backend;

typedef enum mln_plugin_value_type {
    MLN_PLUGIN_VALUE_FLOAT = 1,
    MLN_PLUGIN_VALUE_FLOAT2 = 2,
    MLN_PLUGIN_VALUE_COLOR = 3,
    MLN_PLUGIN_VALUE_STRING = 4,
    MLN_PLUGIN_VALUE_BOOLEAN = 5,
    /* Sprite image identifier, represented by data.string_value. */
    MLN_PLUGIN_VALUE_IMAGE = 6,
} mln_plugin_value_type;

/* Expression dependencies accepted by a plugin property. */
typedef enum mln_plugin_expression_capability {
    MLN_PLUGIN_EXPRESSION_NONE = 0,
    MLN_PLUGIN_EXPRESSION_CAMERA = 1u << 0u,
    MLN_PLUGIN_EXPRESSION_FEATURE = 1u << 1u,
    MLN_PLUGIN_EXPRESSION_COMPOSITE = 1u << 2u,
    MLN_PLUGIN_EXPRESSION_FEATURE_STATE = 1u << 3u
} mln_plugin_expression_capability;

typedef enum mln_plugin_geometry_type {
    MLN_PLUGIN_GEOMETRY_POINT = 1u << 0u,
    MLN_PLUGIN_GEOMETRY_LINESTRING = 1u << 1u,
    MLN_PLUGIN_GEOMETRY_POLYGON = 1u << 2u
} mln_plugin_geometry_type;

typedef struct mln_plugin_string {
    /* UTF-8 bytes, not necessarily NUL-terminated; data may be null only if size
     * is zero. Optional strings may be empty, identifiers must not be empty. */
    const char* data;
    size_t size;
} mln_plugin_string;

typedef struct mln_plugin_float2 {
    float x;
    float y;
} mln_plugin_float2;

/* Registration defaults use straight-alpha colors. The host premultiplies
 * them when it adopts them into the style system; evaluated paint values
 * passed to callbacks use premultiplied colors. */
typedef struct mln_plugin_color {
    float r;
    float g;
    float b;
    float a;
} mln_plugin_color;

typedef union mln_plugin_value_data {
    float float_value;
    /* Must be 0 or 1. */
    uint8_t boolean_value;
    mln_plugin_float2 float2_value;
    mln_plugin_color color_value;
    mln_plugin_string string_value;
} mln_plugin_value_data;

typedef struct mln_plugin_value {
    uint32_t struct_size;
    mln_plugin_value_type type;
    mln_plugin_value_data data;
} mln_plugin_value;

typedef struct mln_plugin_property_descriptor_v1 {
    uint32_t struct_size;
    mln_plugin_string name;
    mln_plugin_value_type type;
    mln_plugin_value default_value;
    /* Bitmask of mln_plugin_expression_capability values. */
    uint32_t expression_capabilities;
    /* Paint properties may opt into the normal MapLibre transition system. */
    uint8_t supports_transitions;
    uint8_t has_minimum;
    uint8_t has_maximum;
    float minimum;
    float maximum;
    /* Optional allowed string values. */
    const mln_plugin_string* enum_values;
    size_t enum_value_count;
    /* 0 = paint, 1 = layout. Layout values may use camera expressions only,
     * never transition, and are evaluated at tile zoom before create_layout. */
    uint8_t is_layout;
} mln_plugin_property_descriptor_v1;

typedef struct mln_plugin_property_value_v1 {
    uint32_t struct_size;
    mln_plugin_string name;
    mln_plugin_value value;
    uint8_t explicitly_set;
} mln_plugin_property_value_v1;

/* ------------------------------------------------------------------------- */
/* Source layout and host-owned drawable API                                 */
/* ------------------------------------------------------------------------- */

typedef enum mln_plugin_vertex_attribute_type {
    MLN_PLUGIN_VERTEX_INT16 = 1,
    MLN_PLUGIN_VERTEX_INT16_X2 = 2,
    MLN_PLUGIN_VERTEX_UINT16 = 3,
    MLN_PLUGIN_VERTEX_UINT16_X2 = 4,
    MLN_PLUGIN_VERTEX_FLOAT = 5,
    MLN_PLUGIN_VERTEX_FLOAT_X2 = 6,
    MLN_PLUGIN_VERTEX_FLOAT_X3 = 7,
    MLN_PLUGIN_VERTEX_FLOAT_X4 = 8,
    MLN_PLUGIN_VERTEX_UINT8_X4_NORMALIZED = 9
} mln_plugin_vertex_attribute_type;

/* Stable IDs are local to one registered plugin layer type. */
typedef struct mln_plugin_shader_attribute_v1 {
    uint32_t struct_size;
    uint32_t attribute_id;
    uint32_t location;
    mln_plugin_string name;
    mln_plugin_vertex_attribute_type type;
} mln_plugin_shader_attribute_v1;

typedef enum mln_plugin_shader_stage {
    MLN_PLUGIN_SHADER_STAGE_VERTEX = 1u << 0u,
    MLN_PLUGIN_SHADER_STAGE_FRAGMENT = 1u << 1u
} mln_plugin_shader_stage;

typedef enum mln_plugin_uniform_scope_v1 {
    /* Callback and storage per drawable, on every frame. */
    MLN_PLUGIN_UNIFORM_DRAWABLE = 0,
    /*
     * Callback once per layer/shader per frame. All drawables share the result.
     * The context has an identity tile_matrix and zero pixels_to_tile_units.
     * Only layer-wide values belong here, never tile-dependent interpolation.
     */
    MLN_PLUGIN_UNIFORM_LAYER = 1,
    /*
     * Callback per drawable per frame. Metal batches the results in one array,
     * indexed by MLN_PLUGIN_DRAWABLE_INDEX_BINDING. OpenGL/Vulkan currently
     * supply an individual block, like DRAWABLE. Shader sources can inspect
     * MLN_PLUGIN_UNIFORM_<uniform_id>_IS_ARRAY (1 on Metal, 0 otherwise).
     */
    MLN_PLUGIN_UNIFORM_DRAWABLE_ARRAY = 2
} mln_plugin_uniform_scope_v1;

typedef struct mln_plugin_uniform_block_descriptor_v1 {
    uint32_t struct_size;
    uint32_t uniform_id;
    mln_plugin_string name;
    uint32_t byte_size;
    uint32_t stage_mask;
    mln_plugin_uniform_scope_v1 scope;
} mln_plugin_uniform_block_descriptor_v1;

typedef struct mln_plugin_shader_source_v1 {
    uint32_t struct_size;
    mln_plugin_backend backend;
    /*
     * GLSL vertex source for OpenGL/Vulkan; complete MSL source for Metal.
     * The host prepends one numeric binding macro for every declared resource:
     *   MLN_PLUGIN_UNIFORM_<uniform_id>_BINDING
     * This keeps backend binding allocation owned by the host. A shader should
     * use those macros in Vulkan set/binding declarations and Metal buffer,
     * attributes. OpenGL resolves uniforms by name.
     */
    mln_plugin_string vertex_source;
    /* GLSL fragment source. Empty for Metal, where vertex_source is complete. */
    mln_plugin_string fragment_source;
    /* Metal entry points. Empty for OpenGL/Vulkan. */
    mln_plugin_string vertex_entry_point;
    mln_plugin_string fragment_entry_point;
} mln_plugin_shader_source_v1;

typedef enum mln_plugin_property_encoding_v1 {
    MLN_PLUGIN_PROPERTY_ENCODING_FLOAT = 1,
    MLN_PLUGIN_PROPERTY_ENCODING_FLOAT2 = 2,
    MLN_PLUGIN_PROPERTY_ENCODING_COLOR = 3,
    /* Zero-based index into the property descriptor enum_values. */
    MLN_PLUGIN_PROPERTY_ENCODING_ENUM_FLOAT = 4,
    MLN_PLUGIN_PROPERTY_ENCODING_BOOLEAN_FLOAT = 5,
    /* Atlas rectangles (float4). FROM minimum/maximum are the zoom-in/out
     * endpoints; TO has equal endpoints. Composite images use native
     * Faded<Image>::to sampling (one zoom higher). The host resolves sprite
     * dependencies; undefined and explicitly empty images stay distinct. */
    MLN_PLUGIN_PROPERTY_ENCODING_IMAGE_FROM = 6,
    MLN_PLUGIN_PROPERTY_ENCODING_IMAGE_TO = 7,
    /* Color samples use floor(clamp(component, 0, 1) * 255) in two distinct
     * UINT8_X4_NORMALIZED attributes. Uniform colors remain float4. */
    MLN_PLUGIN_PROPERTY_ENCODING_COLOR_RGBA8 = 8
} mln_plugin_property_encoding_v1;

/*
 * Declares a host-owned dynamic paint binding. The named property is supplied
 * either through the uniform block range or through the minimum/maximum vertex
 * attributes. Composite expressions use both attributes plus the interpolation
 * factor. Source-only expressions write equal minimum and maximum values.
 * Equal minimum/maximum attribute IDs pack both endpoints into one float2
 * (scalar/enum) or float4 (float2) attribute. Color needs two float4
 * attributes. Enum endpoints must be selected, never linearly interpolated.
 */
typedef struct mln_plugin_shader_property_binding_v1 {
    uint32_t struct_size;
    mln_plugin_string property_name;
    mln_plugin_property_encoding_v1 encoding;
    uint32_t uniform_id;
    uint32_t uniform_byte_offset;
    uint32_t minimum_attribute_id;
    uint32_t maximum_attribute_id;
    uint32_t interpolation_uniform_id;
    uint32_t interpolation_uniform_byte_offset;
} mln_plugin_shader_property_binding_v1;

typedef struct mln_plugin_shader_descriptor_v1 {
    uint32_t struct_size;
    mln_plugin_string shader_id;
    const mln_plugin_shader_source_v1* sources;
    size_t source_count;
    const mln_plugin_shader_attribute_v1* attributes;
    size_t attribute_count;
    const mln_plugin_uniform_block_descriptor_v1* uniform_blocks;
    size_t uniform_block_count;
    const mln_plugin_shader_property_binding_v1* property_bindings;
    size_t property_binding_count;
    /* Tile sprite/pattern atlas, at sampler binding 0. Vulkan only. */
    uint8_t tile_pattern_texture;
    /* Vulkan only: all attributes, including paint bindings, advance per
     * instance. The vertex shader generates primitive corners using
     * gl_VertexIndex. Zero retains ordinary indexed vertex attributes. */
    uint8_t instanced;
} mln_plugin_shader_descriptor_v1;

/* Borrowed render-thread inputs for a host-owned plugin uniform block. */
typedef struct mln_plugin_uniform_context_v1 {
    uint32_t struct_size;
    double bearing;
    float pixels_to_gl_units[2];
    float tile_matrix[16];
    uint32_t viewport_width;
    uint32_t viewport_height;
    float pixels_to_tile_units;
    float camera_to_center_distance;
    float pixel_ratio;
    float light_color[3];
    float light_intensity;
    /* Cartesian style-light position, rotated for a viewport anchor. */
    float light_direction[3];
    float zoom;
    uint32_t tile_x, tile_y;
    uint8_t tile_z;
    int32_t tile_wrap;
    float crossfade_from_scale, crossfade_to_scale, crossfade_t;
    float pattern_texture_size[2];
} mln_plugin_uniform_context_v1;

/* output is host-owned writable storage of output_size bytes, borrowed only
 * during this call. Never free/retain it or write beyond output_size. */
typedef mln_plugin_status (*mln_plugin_update_uniform_block_fn)(const mln_plugin_uniform_context_v1* context,
                                                                uint32_t uniform_id,
                                                                uint8_t* output,
                                                                size_t output_size);

/* A geometry is represented as paths into a flat tile-coordinate point list. */
typedef struct mln_plugin_tile_point_v1 {
    int16_t x;
    int16_t y;
} mln_plugin_tile_point_v1;

typedef struct mln_plugin_feature_v1 {
    uint32_t struct_size;
    mln_plugin_geometry_type geometry_type;
    uint64_t feature_index;
    const mln_plugin_tile_point_v1* points;
    size_t point_count;
    /* path_offsets has path_count + 1 entries and ends at point_count. */
    const uint32_t* path_offsets;
    size_t path_count;
} mln_plugin_feature_v1;

/* Worker-thread input. Paint evaluation is owned by the host binders. */
typedef struct mln_plugin_layout_context_v1 {
    uint32_t struct_size;
    float zoom;
    uint32_t extent;
    const mln_plugin_property_value_v1* properties;
    size_t property_count;
} mln_plugin_layout_context_v1;

/* Returned bytes remain valid until destroy_layout. The host copies them after
 * finish_layout returns and before destroying the layout instance. */
typedef struct mln_plugin_vertex_stream_v1 {
    uint32_t struct_size;
    uint32_t stream_id;
    const uint8_t* data;
    size_t data_size;
    /* Input records; instances for an instanced shader. Streams bound to one
     * drawable must have equal counts before element_offset is applied. */
    uint32_t vertex_count;
    uint32_t stride;
} mln_plugin_vertex_stream_v1;

typedef struct mln_plugin_attribute_binding_v1 {
    uint32_t struct_size;
    uint32_t attribute_id;
    uint32_t stream_id;
    uint32_t byte_offset;
    /* Type comes from the shader's attribute declaration. Starting record in
     * the stream, before vertex_offset or first_instance is applied. */
    uint32_t element_offset;
} mln_plugin_attribute_binding_v1;

typedef struct mln_plugin_segment_v1 {
    uint32_t struct_size;
    /* For instanced shaders these describe generated primitive corners,
     * independently of the instance record range below. */
    uint32_t vertex_offset;
    uint32_t index_offset;
    uint32_t vertex_length;
    uint32_t index_length;
    /* Both zero for ordinary draws. Instanced shaders require a positive count. */
    uint32_t first_instance;
    uint32_t instance_count;
} mln_plugin_segment_v1;

typedef struct mln_plugin_drawable_descriptor_v1 {
    /* Indexed triangles, drawn once for each enabled layer draw pass. */
    uint32_t struct_size;
    uint64_t drawable_key;
    mln_plugin_string shader_id;
    const mln_plugin_attribute_binding_v1* attributes;
    size_t attribute_count;
    const mln_plugin_segment_v1* segments;
    size_t segment_count;
} mln_plugin_drawable_descriptor_v1;

/* For an instanced shader these ranges address input records (instances),
 * including unused sentinel records, rather than generated primitive corners. */
typedef struct mln_plugin_feature_vertex_range_v1 {
    uint32_t struct_size;
    uint64_t feature_index;
    uint64_t drawable_key;
    uint32_t first_vertex;
    uint32_t vertex_count;
} mln_plugin_feature_vertex_range_v1;

/* The bucket struct is host-owned. All views written into it, including nested
 * drawable strings/arrays, indices, vertex bytes and feature ranges, remain
 * plugin-owned and valid until destroy_layout. On success the host copies them
 * before destruction; it neither retains nor frees plugin allocations directly.
 * On failure the output is ignored and the layout instance is still destroyed. */
typedef struct mln_plugin_bucket_v1 {
    uint32_t struct_size;
    const mln_plugin_vertex_stream_v1* vertex_streams;
    size_t vertex_stream_count;
    const uint16_t* indices;
    size_t index_count;
    const mln_plugin_drawable_descriptor_v1* drawables;
    size_t drawable_count;
    /* Maximum screen-pixel distance used by precise rendered-feature query. */
    float query_radius;
    const mln_plugin_feature_vertex_range_v1* feature_vertex_ranges;
    size_t feature_vertex_range_count;
} mln_plugin_bucket_v1;

/*
 * Layout callbacks run on tile workers, serially for each layout instance.
 * Different instances may run concurrently; do not assume a single worker.
 * The host initializes *layout_instance to null before create_layout. Every
 * non-null returned handle transfers to the host regardless of the status code:
 * it must be safe to pass to destroy_layout even when creation failed. The host
 * calls destroy_layout exactly once for such a handle, on the layout worker,
 * after copying successful output or abandoning the layout on any failure.
 * A null handle is never destroyed; OK with a null handle is a creation failure.
 * The plugin must clean up allocations that it does not return through the handle.
 */
typedef mln_plugin_status (*mln_plugin_create_layout_fn)(const mln_plugin_layout_context_v1* context,
                                                         void** layout_instance);
typedef mln_plugin_status (*mln_plugin_layout_feature_fn)(void* layout_instance, const mln_plugin_feature_v1* feature);
typedef mln_plugin_status (*mln_plugin_finish_layout_fn)(void* layout_instance, mln_plugin_bucket_v1* bucket);
typedef void (*mln_plugin_destroy_layout_fn)(void* layout_instance);

/* Borrowed inputs for a rendered-feature hit test. Bearing is in radians;
 * viewport dimensions and pixel distances are logical pixels. */
typedef struct mln_plugin_query_context_v1 {
    uint32_t struct_size;
    double pixels_to_tile_units;
    double camera_to_center_distance;
    double bearing;
    double tile_matrix[16];
    uint32_t viewport_width;
    uint32_t viewport_height;
} mln_plugin_query_context_v1;

/* Optional exact hit test. Query and feature geometry use tile coordinates.
 * All arguments and nested storage are borrowed for this call only.
 * Use the projection context for pitch, translation, and viewport-aligned marks. */
typedef uint8_t (*mln_plugin_query_feature_fn)(const mln_plugin_feature_v1* feature,
                                               const mln_plugin_tile_point_v1* query_geometry,
                                               size_t query_geometry_count,
                                               const mln_plugin_query_context_v1* context,
                                               const mln_plugin_property_value_v1* properties,
                                               size_t property_count);

typedef struct mln_plugin_property_statistics_v1 {
    uint32_t struct_size;
    mln_plugin_string property_name;
    mln_plugin_value minimum;
    mln_plugin_value maximum;
} mln_plugin_property_statistics_v1;

/* Optional conservative screen-pixel radius for broad-phase feature queries.
 * Statistics/property arrays and their string values are borrowed for this call
 * only. This callback can run on workers during layout and on the render thread;
 * it must support concurrent calls for different buckets/maps. */
typedef float (*mln_plugin_query_radius_fn)(const mln_plugin_property_statistics_v1* statistics,
                                            size_t statistics_count,
                                            const mln_plugin_property_value_v1* camera_properties,
                                            size_t camera_property_count);

/* Passes run in array order across ALL tiles in the layer. Depth-only passes
 * can precede color passes. A zero pass count selects one default 2D pass:
 * depth test, read-only depth, color writes, premultiplied blending, no culling.
 * Stencil deduplication shares one reference across the layer and requires 3D.
 * All boolean flags must be 0 or 1. */
typedef enum mln_plugin_cull_mode_v1 {
    MLN_PLUGIN_CULL_NONE = 0,
    MLN_PLUGIN_CULL_BACK_CCW = 1
} mln_plugin_cull_mode_v1;

typedef struct mln_plugin_draw_pass_v1 {
    uint32_t struct_size;
    uint8_t depth_test;
    uint8_t depth_write;
    uint8_t color_write;
    uint8_t blend;
    uint8_t stencil_dedup;
    mln_plugin_cull_mode_v1 cull;
} mln_plugin_draw_pass_v1;

typedef struct mln_plugin_layer_evaluation_v1 {
    uint32_t struct_size;
    /* Bit i enables descriptor i, at most 32 passes. Initially all enabled. */
    uint32_t enabled_passes;
    mln_plugin_float2 translation;
    /* 0 = map, 1 = viewport. */
    uint8_t translation_anchor_viewport;
} mln_plugin_layer_evaluation_v1;

/* Render-thread callback after camera property evaluation, BEFORE orchestration.
 * Properties and output are borrowed. Output starts with all passes enabled and
 * zero translation. Failure or invalid output suppresses rendering for this
 * evaluation. Geometry and feature paint bindings are unaffected. */
typedef mln_plugin_status (*mln_plugin_evaluate_layer_fn)(const mln_plugin_property_value_v1* camera_properties,
                                                          size_t property_count,
                                                          mln_plugin_layer_evaluation_v1* output);

/* Optional animation signal, consulted on the render thread during layer
 * evaluation with the camera-evaluated paint property values (the same array
 * shape as query_feature; feature-dependent properties evaluate without a
 * feature). The property array and its strings are borrowed for the call only.
 * A nonzero return marks the layer as animating, and the map keeps repainting;
 * return zero while the layer's animation is disabled. A null callback means
 * the layer never animates. It must support concurrent calls for different
 * maps. */
typedef uint8_t (*mln_plugin_should_animate_fn)(const mln_plugin_property_value_v1* properties, size_t property_count);

typedef struct mln_plugin_layer_type_v1 {
    uint32_t struct_size;
    mln_plugin_string layer_type;
    uint32_t backend_mask;
    const mln_plugin_property_descriptor_v1* properties;
    size_t property_count;
    uint32_t geometry_type_mask;
    const mln_plugin_shader_descriptor_v1* shaders;
    size_t shader_count;
    mln_plugin_create_layout_fn create_layout;
    mln_plugin_layout_feature_fn layout_feature;
    mln_plugin_finish_layout_fn finish_layout;
    mln_plugin_destroy_layout_fn destroy_layout;
    mln_plugin_query_feature_fn query_feature;
    mln_plugin_update_uniform_block_fn update_uniform_block;
    mln_plugin_query_radius_fn get_query_radius;
    /* Explicitly opt into replacing a built-in STYLE name. Existing objects
     * keep their implementation identity. Does not replace another plugin. */
    uint8_t replace_builtin;
    uint8_t is_3d;
    const mln_plugin_draw_pass_v1* draw_passes;
    size_t draw_pass_count;
    mln_plugin_evaluate_layer_fn evaluate_layer;
    /* Unaligned projection with near-clip adjustment; translation is host-owned. */
    uint8_t enable_near_clipped_matrix;
    mln_plugin_should_animate_fn should_animate;
} mln_plugin_layer_type_v1;

typedef struct mln_plugin_descriptor_v1 {
    uint32_t struct_size;
    uint32_t abi_version;
    mln_plugin_string plugin_id;
    mln_plugin_string plugin_version;
    uint32_t minimum_host_abi;
    uint32_t maximum_host_abi;
    const mln_plugin_layer_type_v1* layer_types;
    size_t layer_type_count;
} mln_plugin_descriptor_v1;

typedef mln_plugin_status (*mln_plugin_register_function_v1)(const mln_plugin_descriptor_v1* descriptor,
                                                             char* error_message,
                                                             size_t error_message_capacity);

/* Thread-safe, process-wide registration, required before loading dependent
 * styles. The descriptor and all nested metadata (strings, defaults, arrays and
 * shader source text) are copied during this call and may be freed afterwards.
 * Only callback addresses are retained. Identical repeated registration succeeds
 * with ALREADY_REGISTERED; failed registration publishes no layer/property types.
 *
 * error_message is optional caller-owned writable storage. If non-null with
 * nonzero capacity, it receives a NUL-terminated diagnostic (possibly truncated),
 * or an empty string on success. The host never retains it. */
MLN_PLUGIN_EXPORT mln_plugin_status mln_plugin_register_v1(const mln_plugin_descriptor_v1* descriptor,
                                                           char* error_message,
                                                           size_t error_message_capacity);

#ifdef __cplusplus
}
#endif

#endif /* MLN_PLUGIN_API_H */
