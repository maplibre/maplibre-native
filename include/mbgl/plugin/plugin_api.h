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
 * A source-less style layer type implemented entirely by a plugin. The host
 * owns style parsing, property storage, ordering, visibility and zoom ranges;
 * the plugin owns its renderer and GPU resources.
 */
typedef struct mln_plugin_layer_type_v1 {
    uint32_t struct_size;
    mln_plugin_string layer_type;
    uint32_t backend_mask;
    mln_plugin_render_stage render_stage;
    uint8_t requires_3d;
    const mln_plugin_property_descriptor_v1* properties;
    size_t property_count;
    mln_plugin_create_instance_fn create_instance;
    mln_plugin_destroy_instance_fn destroy_instance;
    mln_plugin_prepare_frame_fn prepare_frame;
    mln_plugin_render_before_layer_fn render_layer;
    mln_plugin_context_lost_fn context_lost;
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
