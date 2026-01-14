option(MLN_WITH_ARROW_POLYLINE_EXAMPLE "Build ArrowPolylineExample into maplibre" ON)

if(NOT MLN_WITH_ARROW_POLYLINE_EXAMPLE)
  return()
endif()

set(ARROW_PLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(ARROW_GENERATED_DIR ${ARROW_PLUGIN_DIR}/generated)

# Core plugin implementation
set(ARROW_EXAMPLE_SOURCES
  ${ARROW_PLUGIN_DIR}/src/ArrowPolylineExampleImpl.cpp
)

# Generated C++ sources - package-specific code
file(GLOB ARROW_GENERATED_CPP_SOURCES
  ${ARROW_GENERATED_DIR}/cpp/src/GluecodiumPlugin/*.cpp
)
list(APPEND ARROW_EXAMPLE_SOURCES ${ARROW_GENERATED_CPP_SOURCES})

# Generated C++ sources - internal helpers (Locale, TypeRepository, etc.)
# These are in the glue_internal subdirectory to avoid shadowing system headers
file(GLOB ARROW_GENERATED_INTERNAL_SOURCES
  ${ARROW_GENERATED_DIR}/cpp/src/glue_internal/*.cpp
)
list(APPEND ARROW_EXAMPLE_SOURCES ${ARROW_GENERATED_INTERNAL_SOURCES})

# Platform-specific generated sources
if(ANDROID)
  file(GLOB ARROW_GENERATED_JNI_SOURCES
    ${ARROW_GENERATED_DIR}/android/jni/*.cpp
  )
  list(APPEND ARROW_EXAMPLE_SOURCES ${ARROW_GENERATED_JNI_SOURCES})

  # Plugin initializer - uses MLN_PLUGIN_REGISTER_JNI_NATIVES to auto-init Gluecodium
  list(APPEND ARROW_EXAMPLE_SOURCES ${ARROW_PLUGIN_DIR}/android/arrow_polyline_initializer_jni.cpp)

  # Rename Gluecodium's JNI_OnLoad to avoid conflict with MapLibre's main.cpp
  target_compile_definitions(maplibre PRIVATE GLUECODIUM_JNI_ONLOAD=Gluecodium_JNI_OnLoad)
endif()

# Add sources to maplibre target
target_sources(maplibre PRIVATE ${ARROW_EXAMPLE_SOURCES})

# Include directories - use SYSTEM to avoid header conflicts
target_include_directories(maplibre PRIVATE
  ${ARROW_PLUGIN_DIR}/src
)

# Add Gluecodium includes as SYSTEM to lower priority vs NDK headers
# The internal headers (Locale.h, etc.) are in glue_internal/ subdirectory
# to avoid shadowing system <locale> headers on NDK 28+
# NOTE: Do NOT add glue_internal as a direct include path - macOS case-insensitive
# filesystem would match Locale.h when looking for <locale.h>
target_include_directories(maplibre SYSTEM PRIVATE
  ${ARROW_GENERATED_DIR}/cpp/include
  ${ARROW_GENERATED_DIR}/cpp/include/GluecodiumPlugin
)

if(ANDROID)
  target_include_directories(maplibre SYSTEM PRIVATE
    ${ARROW_GENERATED_DIR}/android/jni
  )
endif()
