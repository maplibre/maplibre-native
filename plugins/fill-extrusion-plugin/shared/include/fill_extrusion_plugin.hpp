#pragma once
#include <mln/plugin/plugin_api.h>

#if defined(_WIN32)
#define MLN_FILL_EXTRUSION_PLUGIN_EXPORT __declspec(dllexport)
#else
#define MLN_FILL_EXTRUSION_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

MLN_FILL_EXTRUSION_PLUGIN_EXPORT mln_plugin_status mln_fill_extrusion_plugin_register(
    mln_plugin_register_function_v1 register_plugin, char* error_message, size_t error_message_capacity);

#ifdef __cplusplus
}
#endif
