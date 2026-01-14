# BasicPluginExample - Cross-platform MapLibre plugin example
#
# This plugin demonstrates the XPlatformPlugin architecture.
# Plugins are built into maplibre.so and can optionally use
# MLN_PLUGIN_REGISTER_JNI_NATIVES to hook into MapLibre's JNI system.
#
# Plugins with their own binding systems (e.g., Gluecodium) can skip
# the JNI registration macro entirely.

option(MLN_WITH_BASIC_EXAMPLE_PLUGIN "Build BasicPluginExample into maplibre" ON)

if(NOT MLN_WITH_BASIC_EXAMPLE_PLUGIN)
  return()
endif()

# Core plugin sources (cross-platform)
set(BASIC_PLUGIN_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/BasicPluginExample.cpp
)

set(BASIC_PLUGIN_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Platform-specific sources
if(ANDROID)
  list(APPEND BASIC_PLUGIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/android/basic_plugin_example_jni.cpp
  )
elseif(APPLE)
  list(APPEND BASIC_PLUGIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/darwin/MLNBasicPluginExample.mm
  )
  list(APPEND BASIC_PLUGIN_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/darwin
  )
endif()

# Add sources directly to maplibre target
target_sources(maplibre PRIVATE ${BASIC_PLUGIN_SOURCES})
target_include_directories(maplibre PRIVATE ${BASIC_PLUGIN_INCLUDE_DIRS})
