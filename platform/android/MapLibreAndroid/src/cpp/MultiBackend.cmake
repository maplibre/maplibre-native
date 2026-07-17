# Android multi-backend build (Gradle `multiBackend` flavor only).
#
# Builds libmaplibre.so (OpenGL) and libmaplibre-vulkan.so (Vulkan) by invoking this directory's
# stock single-backend build twice via ExternalProject, and surfaces both to AGP as SHARED
# targets so both .so land in the multiBackend AAR.
#
# Reached via include() from src/cpp/CMakeLists.txt while project(MapLibreAndroid) is in effect,
# so the add_library targets below land in the AGP-invoked MapLibreAndroid project (AGP's CMake
# file-API query only sees targets declared there) and CMAKE_CURRENT_SOURCE_DIR stays src/cpp.
include(ExternalProject)

set(_src ${CMAKE_CURRENT_SOURCE_DIR})            # src/cpp -- the single-backend entry

# Forward the exact Android toolchain AGP handed this configure to each sub-build. Each sub-build
# takes the single-backend path (MLN_ANDROID_MULTI_BACKEND=OFF) and builds core into its own
# BINARY_DIR (see src/cpp/CMakeLists.txt edit (b)), so the two sub-builds isolate automatically.
set(_fwd
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DANDROID_ABI=${ANDROID_ABI}
    -DANDROID_PLATFORM=${ANDROID_PLATFORM}
    -DANDROID_NDK=${ANDROID_NDK}
    -DANDROID_TOOLCHAIN=${ANDROID_TOOLCHAIN}
    -DANDROID_STL=${ANDROID_STL}
    -DANDROID_CPP_FEATURES=${ANDROID_CPP_FEATURES}
    -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=${ANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DMLN_ANDROID_MULTI_BACKEND=OFF)

# Distinct BINARY_DIR per backend => distinct core build dir => no shared static-lib / LTO dir.
set(_gl ${CMAKE_BINARY_DIR}/multi-opengl)
set(_vk ${CMAKE_BINARY_DIR}/multi-vulkan)

ExternalProject_Add(maplibre-opengl-build
    SOURCE_DIR ${_src}
    BINARY_DIR ${_gl}
    CMAKE_ARGS
        ${_fwd}
        -DMLN_WITH_OPENGL=ON
        -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${_gl}/lib
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target maplibre
    INSTALL_COMMAND ""
    BUILD_ALWAYS 1)

ExternalProject_Add(maplibre-vulkan-build
    SOURCE_DIR ${_src}
    BINARY_DIR ${_vk}
    CMAKE_ARGS
        ${_fwd}
        -DMLN_WITH_VULKAN=ON
        -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${_vk}/lib
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target maplibre
    INSTALL_COMMAND ""
    BUILD_ALWAYS 1)

# Surface both externally-built .so to AGP via stub SHARED targets whose output file is
# overwritten with the real backend .so. AGP packages targets it builds; ExternalProject outputs
# are opaque to it and IMPORTED targets aren't packaged -- hence this bridge.
set(_stub ${CMAKE_BINARY_DIR}/maplibre_multi_stub.cpp)
file(WRITE ${_stub} "extern \"C\" void maplibre_multi_backend_stub() {}\n")

add_library(maplibre-opengl SHARED ${_stub})
# Produce libmaplibre.so (not libmaplibre-opengl.so) so the OpenGL output matches the
# single-backend OpenGL AAR's library name.
set_target_properties(maplibre-opengl PROPERTIES OUTPUT_NAME maplibre)
add_dependencies(maplibre-opengl maplibre-opengl-build)
add_custom_command(TARGET maplibre-opengl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${_gl}/lib/libmaplibre.so
        $<TARGET_FILE:maplibre-opengl>
    VERBATIM)

add_library(maplibre-vulkan SHARED ${_stub})    # -> libmaplibre-vulkan.so
add_dependencies(maplibre-vulkan maplibre-vulkan-build)
add_custom_command(TARGET maplibre-vulkan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${_vk}/lib/libmaplibre.so
        $<TARGET_FILE:maplibre-vulkan>
    VERBATIM)

# AGP's defaultConfig nativeTargets always lists "maplibre"; provide a custom target so
# `cmake --build . --target maplibre` triggers both .so builds.
add_custom_target(maplibre DEPENDS maplibre-opengl maplibre-vulkan)

install(TARGETS maplibre-opengl maplibre-vulkan LIBRARY DESTINATION lib)
