#pragma once
#include <mln/plugin/plugin_api.h>

#if defined(_WIN32)
#define MLN_GLTF_EXPORT __declspec(dllexport)
#else
#define MLN_GLTF_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 3D models drawn at the points of a GeoJSON or vector source: `"type": "gltf"`. The layer's `models` layout
 * property maps names to .glb files, and `model-id` (a name, usually a feature expression) picks the model
 * each feature is drawn as.
 */
MLN_GLTF_EXPORT mln_plugin_status mln_gltf_layer_register(mln_plugin_register_function_v1 register_plugin,
                                                          char* error_message,
                                                          size_t error_message_capacity);

#ifdef __cplusplus
}
#endif
