# MapLibre Native Plugins
#
# Plugins extend MapLibre via the XPlatformPlugin interface.
# They are compiled into maplibre.so to avoid ABI exposure issues.
#
# JNI bindings are flexible - plugins can either:
#   - Use MLN_PLUGIN_REGISTER_JNI_NATIVES for simple cases
#   - Bring their own binding system (Gluecodium, manual JNI, etc.)

# Include available plugins
add_subdirectory(basic-example-plugin)
add_subdirectory(arrow-polyline-example-plugin)
