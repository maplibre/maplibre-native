#ifndef plugin_layer_debug_h
#define plugin_layer_debug_h

// All of these parameters should only be available in a debug build
#ifdef DEBUG
// All logging in the plugin_layer_* classes is wrapped with this #if.
// Set to 0 to disable all logging in plugin layers
#define MLN_PLUGIN_LAYER_LOGGING_ENABLED 1
#endif

#endif /* plugin_layer_debug_h */
