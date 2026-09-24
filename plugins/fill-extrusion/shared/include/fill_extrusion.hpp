#pragma once
#include <mln/plugin/plugin_api.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Call before loading styles to replace the fill-extrusion style type. */
MLN_PLUGIN_EXPORT mln_plugin_status mln_fill_extrusion_register(mln_plugin_register_function_v1 register_plugin,
                                                                char* error,
                                                                size_t capacity);
#ifdef __cplusplus
}
#endif
