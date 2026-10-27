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

/* A 3D model drawn at every point of a GeoJSON or vector source: `"type": "gltf"`. */
MLN_GLTF_EXPORT mln_plugin_status mln_gltf_layer_register(mln_plugin_register_function_v1 register_plugin,
                                                          char* error_message,
                                                          size_t error_message_capacity);

/*
 * Sets the mesh a `gltf` layer draws when it has no `model-uri` layout property, from the bytes of a binary
 * glTF (.glb). The bytes are parsed during the call and not retained.
 *
 * Call it before a style containing such a layer is loaded: a tile that was already laid out keeps the mesh
 * it was built with. Passing null or zero bytes clears the mesh, so those layers draw nothing.
 * Safe to call from any thread.
 *
 * error_message is optional caller-owned storage for a NUL-terminated diagnostic.
 */
MLN_GLTF_EXPORT mln_plugin_status mln_gltf_layer_set_model(const uint8_t* glb_bytes,
                                                           size_t glb_size,
                                                           char* error_message,
                                                           size_t error_message_capacity);

#ifdef __cplusplus
}
#endif
