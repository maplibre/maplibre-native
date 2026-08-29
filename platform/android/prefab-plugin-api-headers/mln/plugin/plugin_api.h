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
    MLN_PLUGIN_VALUE_BOOLEAN = 1,
    MLN_PLUGIN_VALUE_FLOAT = 2,
    MLN_PLUGIN_VALUE_FLOAT2 = 3,
    MLN_PLUGIN_VALUE_COLOR = 4,
    MLN_PLUGIN_VALUE_STRING = 5
} mln_plugin_value_type;

typedef enum mln_plugin_property_scope {
    MLN_PLUGIN_PROPERTY_PAINT = 1,
    MLN_PLUGIN_PROPERTY_LAYOUT = 2
} mln_plugin_property_scope;

/*
 * Custom layer source contract. Geometry sources cover GeoJSON and vector
 * tile sources. The remaining values are reserved so adding raster and DEM
 * adapters does not change the shape of the layer descriptor.
 */
typedef enum mln_plugin_source_kind {
    MLN_PLUGIN_SOURCE_NONE = 0,
    MLN_PLUGIN_SOURCE_GEOMETRY = 1,
    MLN_PLUGIN_SOURCE_RASTER = 2,
    MLN_PLUGIN_SOURCE_RASTER_DEM = 3
} mln_plugin_source_kind;

typedef enum mln_plugin_geometry_type {
    MLN_PLUGIN_GEOMETRY_POINT = 1u << 0u,
    MLN_PLUGIN_GEOMETRY_LINESTRING = 1u << 1u,
    MLN_PLUGIN_GEOMETRY_POLYGON = 1u << 2u
} mln_plugin_geometry_type;

typedef struct mln_plugin_string {
    const char* data;
    size_t size;
} mln_plugin_string;

typedef struct mln_plugin_float2 {
    float x;
    float y;
} mln_plugin_float2;

typedef struct mln_plugin_color {
    float r;
    float g;
    float b;
    float a;
} mln_plugin_color;

typedef union mln_plugin_value_data {
    uint8_t boolean_value;
    float float_value;
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
    mln_plugin_property_scope scope;
    mln_plugin_value default_value;
    /* Expressions are parsed and evaluated by the host. */
    uint8_t supports_expressions;
    /* Paint properties may opt into the normal MapLibre transition system. */
    uint8_t supports_transitions;
} mln_plugin_property_descriptor_v1;

typedef struct mln_plugin_property_value_v1 {
    uint32_t struct_size;
    mln_plugin_string name;
    mln_plugin_value value;
    uint8_t explicitly_set;
} mln_plugin_property_value_v1;

typedef enum mln_plugin_draw_packet_kind {
    MLN_PLUGIN_DRAW_PACKET_TRIANGLES = 1,
    MLN_PLUGIN_DRAW_PACKET_INSTANCED_WALLS = 2
} mln_plugin_draw_packet_kind;

typedef enum mln_plugin_attribute_type {
    MLN_PLUGIN_ATTRIBUTE_NONE = 0,
    MLN_PLUGIN_ATTRIBUTE_INT16_X2 = 1,
    MLN_PLUGIN_ATTRIBUTE_UINT16_X2 = 2,
    MLN_PLUGIN_ATTRIBUTE_FLOAT = 3,
    MLN_PLUGIN_ATTRIBUTE_FLOAT_X2 = 4
} mln_plugin_attribute_type;

typedef struct mln_plugin_buffer_binding_v1 {
    uint32_t struct_size;
    uint64_t buffer;
    uint64_t offset;
    uint32_t stride;
    mln_plugin_attribute_type type;
} mln_plugin_buffer_binding_v1;

typedef enum mln_plugin_render_stage {
    MLN_PLUGIN_RENDER_STAGE_PREPARE = 1,
    MLN_PLUGIN_RENDER_STAGE_PASS_3D = 2,
    MLN_PLUGIN_RENDER_STAGE_OPAQUE = 3,
    MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT = 4
} mln_plugin_render_stage;

typedef void (*mln_plugin_proc)(void);

/*
 * Metal objects are Objective-C objects expressed as opaque pointers so this
 * header remains valid C. They are borrowed by the host and valid only until
 * the callback returns. A plugin may send messages to them from Objective-C
 * or Objective-C++ after casting to the matching id<MTL...> protocol.
 */
typedef struct mln_plugin_metal_context_v1 {
    uint32_t struct_size;
    void* device;
    void* command_queue;
    void* command_buffer;
    void* render_command_encoder;
    uint32_t color_pixel_format;
    uint32_t depth_pixel_format;
    uint32_t stencil_pixel_format;
    uint32_t sample_count;
} mln_plugin_metal_context_v1;

typedef struct mln_plugin_backend_context_v1 {
    uint32_t struct_size;
    mln_plugin_backend backend;
    uint64_t instance;
    uint64_t physical_device;
    uint64_t device;
    uint64_t graphics_queue;
    uint32_t graphics_queue_family;
    uint64_t command_buffer;
    uint64_t render_pass;
    uint64_t framebuffer;
    void* resolver_context;
    mln_plugin_proc (*get_proc_address)(void* resolver_context, const char* name);
    const mln_plugin_metal_context_v1* metal;
    float screen_pre_rotation_radians_clockwise;
} mln_plugin_backend_context_v1;

/* Backend objects are opaque and valid only for the duration of a callback. */
typedef struct mln_plugin_draw_packet_v1 {
    uint32_t struct_size;
    mln_plugin_draw_packet_kind kind;
    uint64_t index_buffer;
    uint64_t index_offset;
    uint32_t index_count;
    uint32_t instance_count;
    int32_t base_vertex;
    mln_plugin_buffer_binding_v1 wall_vertex;
    mln_plugin_buffer_binding_v1 position;
    mln_plugin_buffer_binding_v1 decimals_edge;
    mln_plugin_buffer_binding_v1 normal;
    mln_plugin_buffer_binding_v1 base;
    mln_plugin_buffer_binding_v1 height;
    float tile_matrix[16];
    float constant_base;
    float constant_height;
    float base_interpolation;
    float height_interpolation;
    float height_factor;
    float layer_opacity;
    uint8_t base_is_attribute;
    uint8_t height_is_attribute;
} mln_plugin_draw_packet_v1;

typedef struct mln_plugin_frame_context_v1 {
    uint32_t struct_size;
    mln_plugin_render_stage stage;
    uint32_t width;
    uint32_t height;
    uint64_t frame_number;
    const mln_plugin_backend_context_v1* backend;
    const mln_plugin_property_value_v1* properties;
    size_t property_count;
    const mln_plugin_draw_packet_v1* fill_extrusion_packets;
    size_t fill_extrusion_packet_count;
    const struct mln_plugin_camera_context_v1* camera;
} mln_plugin_frame_context_v1;

/*
 * Camera values and matrices are immutable and valid only for the duration of
 * the render callback. Matrices are column-major. Geographic positions can be
 * converted to normalized Web Mercator coordinates by the plugin; the
 * projection matrices accept those coordinates directly.
 */
typedef struct mln_plugin_camera_context_v1 {
    uint32_t struct_size;
    double latitude;
    double longitude;
    double zoom;
    double bearing;
    double pitch;
    double field_of_view;
    float pixel_ratio;
    double projection_matrix[16];
    double near_clipped_projection_matrix[16];
} mln_plugin_camera_context_v1;

typedef struct mln_plugin_resource_response_v1 {
    uint32_t struct_size;
    uint64_t request_id;
    const uint8_t* data;
    size_t data_size;
    mln_plugin_string error_message;
} mln_plugin_resource_response_v1;

/* Response bytes and strings are borrowed and valid only during the callback. */
typedef void (*mln_plugin_resource_callback_fn)(void* callback_context,
                                                const mln_plugin_resource_response_v1* response);

typedef struct mln_plugin_host_api_v1 {
    uint32_t struct_size;
    uint32_t abi_version;
    void (*log)(int32_t severity, mln_plugin_string message);
    void* context;
    mln_plugin_status (*request_resource)(void* context,
                                          mln_plugin_string url,
                                          mln_plugin_resource_callback_fn callback,
                                          void* callback_context,
                                          uint64_t* request_id);
    void (*cancel_resource_request)(void* context, uint64_t request_id);
    void (*request_repaint)(void* context);
} mln_plugin_host_api_v1;

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

typedef enum mln_plugin_draw_mode {
    MLN_PLUGIN_DRAW_MODE_TRIANGLES = 1,
    MLN_PLUGIN_DRAW_MODE_LINES = 2,
    MLN_PLUGIN_DRAW_MODE_POINTS = 3
} mln_plugin_draw_mode;

typedef enum mln_plugin_depth_mode {
    MLN_PLUGIN_DEPTH_DISABLED = 0,
    MLN_PLUGIN_DEPTH_READ_ONLY = 1,
    MLN_PLUGIN_DEPTH_READ_WRITE = 2
} mln_plugin_depth_mode;

typedef enum mln_plugin_blend_mode {
    MLN_PLUGIN_BLEND_REPLACE = 0,
    MLN_PLUGIN_BLEND_ALPHA = 1,
    MLN_PLUGIN_BLEND_PREMULTIPLIED_ALPHA = 2,
    MLN_PLUGIN_BLEND_MULTIPLY = 3
} mln_plugin_blend_mode;

/* Stable IDs are local to one registered plugin layer type. */
typedef struct mln_plugin_shader_attribute_v1 {
    uint32_t struct_size;
    uint32_t attribute_id;
    uint32_t location;
    mln_plugin_string name;
    mln_plugin_vertex_attribute_type type;
} mln_plugin_shader_attribute_v1;

typedef struct mln_plugin_shader_source_v1 {
    uint32_t struct_size;
    mln_plugin_backend backend;
    /* GLSL vertex source for OpenGL/Vulkan; complete MSL source for Metal. */
    mln_plugin_string vertex_source;
    /* GLSL fragment source. Empty for Metal, where vertex_source is complete. */
    mln_plugin_string fragment_source;
    /* Metal entry points. Empty for OpenGL/Vulkan. */
    mln_plugin_string vertex_entry_point;
    mln_plugin_string fragment_entry_point;
} mln_plugin_shader_source_v1;

typedef struct mln_plugin_shader_descriptor_v1 {
    uint32_t struct_size;
    mln_plugin_string shader_id;
    const mln_plugin_shader_source_v1* sources;
    size_t source_count;
    const mln_plugin_shader_attribute_v1* attributes;
    size_t attribute_count;
} mln_plugin_shader_descriptor_v1;

/* A geometry is represented as paths into a flat tile-coordinate point list. */
typedef struct mln_plugin_tile_point_v1 {
    int16_t x;
    int16_t y;
} mln_plugin_tile_point_v1;

typedef struct mln_plugin_feature_v1 {
    uint32_t struct_size;
    mln_plugin_geometry_type geometry_type;
    uint64_t feature_index;
    mln_plugin_string feature_id;
    const mln_plugin_tile_point_v1* points;
    size_t point_count;
    /* path_offsets has path_count + 1 entries and ends at point_count. */
    const uint32_t* path_offsets;
    size_t path_count;
    /* UTF-8 JSON object. Borrowed and valid only during layout_feature. */
    mln_plugin_string properties_json;
    /* Host-evaluated plugin properties for this feature and tile zoom. */
    const mln_plugin_property_value_v1* evaluated_properties;
    size_t evaluated_property_count;
} mln_plugin_feature_v1;

typedef struct mln_plugin_layout_context_v1 {
    uint32_t struct_size;
    float zoom;
    int32_t canonical_z;
    int32_t canonical_x;
    int32_t canonical_y;
    uint32_t extent;
    mln_plugin_string layer_id;
    mln_plugin_string source_layer_id;
    /* Serialized style values. Expressions are deliberately not evaluated here. */
    const mln_plugin_property_value_v1* properties;
    size_t property_count;
    /* Resource requests use MapLibre's configured file source and cache. */
    const mln_plugin_host_api_v1* host;
} mln_plugin_layout_context_v1;

/* Bytes are copied by the host before finish_layout returns. */
typedef struct mln_plugin_vertex_stream_v1 {
    uint32_t struct_size;
    uint32_t stream_id;
    const uint8_t* data;
    size_t data_size;
    uint32_t vertex_count;
    uint32_t stride;
} mln_plugin_vertex_stream_v1;

typedef struct mln_plugin_attribute_binding_v1 {
    uint32_t struct_size;
    uint32_t attribute_id;
    uint32_t stream_id;
    uint32_t byte_offset;
    mln_plugin_vertex_attribute_type type;
} mln_plugin_attribute_binding_v1;

typedef struct mln_plugin_segment_v1 {
    uint32_t struct_size;
    uint32_t vertex_offset;
    uint32_t index_offset;
    uint32_t vertex_length;
    uint32_t index_length;
    uint64_t feature_index;
} mln_plugin_segment_v1;

typedef struct mln_plugin_drawable_descriptor_v1 {
    uint32_t struct_size;
    uint64_t drawable_key;
    mln_plugin_string shader_id;
    mln_plugin_draw_mode draw_mode;
    mln_plugin_render_stage render_stage;
    mln_plugin_depth_mode depth_mode;
    mln_plugin_blend_mode blend_mode;
    uint8_t enable_stencil;
    uint8_t enable_cull_face;
    const mln_plugin_attribute_binding_v1* attributes;
    size_t attribute_count;
    const mln_plugin_segment_v1* segments;
    size_t segment_count;
} mln_plugin_drawable_descriptor_v1;

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
} mln_plugin_bucket_v1;

typedef mln_plugin_status (*mln_plugin_create_layout_fn)(const mln_plugin_layout_context_v1* context,
                                                         void** layout_instance);
typedef mln_plugin_status (*mln_plugin_layout_feature_fn)(void* layout_instance, const mln_plugin_feature_v1* feature);
typedef mln_plugin_status (*mln_plugin_finish_layout_fn)(void* layout_instance, mln_plugin_bucket_v1* bucket);
typedef void (*mln_plugin_destroy_layout_fn)(void* layout_instance);

/*
 * Optional exact hit test. query_x/query_y and feature geometry are in tile
 * coordinates; pixels_to_tile_units converts the bucket query radius.
 */
typedef uint8_t (*mln_plugin_query_feature_fn)(const mln_plugin_feature_v1* feature,
                                               const mln_plugin_tile_point_v1* query_geometry,
                                               size_t query_geometry_count,
                                               double pixels_to_tile_units,
                                               const mln_plugin_property_value_v1* properties,
                                               size_t property_count);

typedef mln_plugin_status (*mln_plugin_create_instance_fn)(const mln_plugin_host_api_v1* host,
                                                           mln_plugin_string layer_id,
                                                           void** instance);
typedef void (*mln_plugin_destroy_instance_fn)(void* instance);
typedef mln_plugin_status (*mln_plugin_prepare_frame_fn)(void* instance, const mln_plugin_frame_context_v1* frame);
typedef mln_plugin_status (*mln_plugin_render_before_layer_fn)(void* instance,
                                                               const mln_plugin_frame_context_v1* frame);
typedef void (*mln_plugin_context_lost_fn)(void* instance);

typedef struct mln_plugin_layer_extension_v1 {
    uint32_t struct_size;
    mln_plugin_string target_layer_type;
    int32_t render_priority;
    uint32_t backend_mask;
    const mln_plugin_property_descriptor_v1* properties;
    size_t property_count;
    mln_plugin_create_instance_fn create_instance;
    mln_plugin_destroy_instance_fn destroy_instance;
    mln_plugin_prepare_frame_fn prepare_frame;
    mln_plugin_render_before_layer_fn render_before_layer;
    mln_plugin_context_lost_fn context_lost;
} mln_plugin_layer_extension_v1;

/*
 * A custom style layer type. The host owns parsing, source/tile scheduling,
 * expression evaluation, bucket lifetime, shaders, GPU resources, drawables,
 * ordering, visibility, queries and zoom ranges. The plugin supplies immutable
 * metadata and CPU layout callbacks only.
 *
 * Custom layers are source-bound and cannot issue backend commands directly.
 * Existing-layer extensions use mln_plugin_layer_extension_v1 when they need
 * access to a target layer's short-lived render packets.
 */
typedef struct mln_plugin_layer_type_v1 {
    uint32_t struct_size;
    mln_plugin_string layer_type;
    uint32_t backend_mask;
    mln_plugin_render_stage render_stage;
    uint8_t requires_3d;
    const mln_plugin_property_descriptor_v1* properties;
    size_t property_count;
    mln_plugin_source_kind source_kind;
    uint32_t geometry_type_mask;
    const mln_plugin_shader_descriptor_v1* shaders;
    size_t shader_count;
    mln_plugin_create_layout_fn create_layout;
    mln_plugin_layout_feature_fn layout_feature;
    mln_plugin_finish_layout_fn finish_layout;
    mln_plugin_destroy_layout_fn destroy_layout;
    mln_plugin_query_feature_fn query_feature;
} mln_plugin_layer_type_v1;

typedef struct mln_plugin_descriptor_v1 {
    uint32_t struct_size;
    uint32_t abi_version;
    mln_plugin_string plugin_id;
    mln_plugin_string plugin_version;
    uint32_t minimum_host_abi;
    uint32_t maximum_host_abi;
    const mln_plugin_layer_extension_v1* layer_extensions;
    size_t layer_extension_count;
    const mln_plugin_layer_type_v1* layer_types;
    size_t layer_type_count;
} mln_plugin_descriptor_v1;

typedef mln_plugin_status (*mln_plugin_register_function_v1)(const mln_plugin_descriptor_v1* descriptor,
                                                             char* error_message,
                                                             size_t error_message_capacity);

MLN_PLUGIN_EXPORT mln_plugin_status mln_plugin_register_v1(const mln_plugin_descriptor_v1* descriptor,
                                                           char* error_message,
                                                           size_t error_message_capacity);
MLN_PLUGIN_EXPORT uint8_t mln_plugin_is_registered_v1(const char* plugin_id);
MLN_PLUGIN_EXPORT size_t mln_plugin_count_v1(void);
MLN_PLUGIN_EXPORT size_t mln_plugin_id_at_v1(size_t index, char* output, size_t output_capacity);

#ifdef __cplusplus
}
#endif

#endif /* MLN_PLUGIN_API_H */
