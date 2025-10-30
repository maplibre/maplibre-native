if(WIN32)
    find_package(glfw3 REQUIRED)
    find_package(libuv REQUIRED)
else()
    find_package(PkgConfig REQUIRED)

    pkg_search_module(GLFW glfw3 REQUIRED)
endif()

add_executable(
    mbgl-glfw
    ${PROJECT_SOURCE_DIR}/platform/glfw/main.cpp
    ${PROJECT_SOURCE_DIR}/platform/glfw/glfw_view.cpp
    ${PROJECT_SOURCE_DIR}/platform/glfw/glfw_renderer_frontend.cpp
    ${PROJECT_SOURCE_DIR}/platform/glfw/settings_json.cpp
    ${PROJECT_SOURCE_DIR}/platform/glfw/test_writer.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/map/map_snapshotter.cpp
    ${PROJECT_SOURCE_DIR}/platform/glfw/example_custom_drawable_style_layer.cpp
)


include(${PROJECT_SOURCE_DIR}/vendor/tinyobjloader.cmake)

target_link_libraries(
    mbgl-glfw
    PRIVATE tinyobjloader
)

set_property(
    SOURCE ${PROJECT_SOURCE_DIR}/platform/glfw/glfw_view.cpp
    PROPERTY COMPILE_DEFINITIONS MLN_ASSETS_PATH=\"${PROJECT_SOURCE_DIR}/platform/glfw/assets/\"
)

if(MLN_WITH_OPENGL)
    find_package(OpenGL REQUIRED)
    target_sources(
        mbgl-glfw
        PRIVATE ${PROJECT_SOURCE_DIR}/platform/glfw/glfw_gl_backend.cpp
    )
endif()

if(MLN_WITH_EGL)
    target_compile_definitions(
        mbgl-glfw
        PRIVATE MBGL_WITH_EGL=1
    )
endif()

if(MLN_WITH_VULKAN)
    target_sources(
        mbgl-glfw
        PRIVATE ${PROJECT_SOURCE_DIR}/platform/glfw/glfw_vulkan_backend.cpp
    )
endif()

if(MLN_WITH_METAL)
    target_sources(
        mbgl-glfw
        PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/glfw/glfw_metal_backend.mm
        ${PROJECT_SOURCE_DIR}/platform/glfw/metal_backend.mm
        ${PROJECT_SOURCE_DIR}/platform/glfw/glfw_metal_backend.h
        ${PROJECT_SOURCE_DIR}/platform/glfw/metal_backend.h
    )
    target_link_libraries(
        mbgl-glfw
        PRIVATE
        mbgl-vendor-metal-cpp
    )
endif()

if(MLN_WITH_WEBGPU)
    if(APPLE)
        target_sources(
            mbgl-glfw
            PRIVATE ${PROJECT_SOURCE_DIR}/platform/glfw/glfw_webgpu_backend.mm
        )
    else()
        target_sources(
            mbgl-glfw
            PRIVATE ${PROJECT_SOURCE_DIR}/platform/glfw/glfw_webgpu_backend.cpp
        )
    endif()
    if(TARGET mbgl-vendor-dawn)
        target_link_libraries(mbgl-glfw PRIVATE mbgl-vendor-dawn)
    endif()
    if(TARGET mbgl-vendor-wgpu)
        target_link_libraries(mbgl-glfw PRIVATE mbgl-vendor-wgpu)
    endif()
endif()

if(DEFINED MLN_WITH_X11 AND MLN_WITH_X11)
    target_compile_definitions(
        mbgl-glfw
        PRIVATE MLN_WITH_X11=1
    )
endif()

if(DEFINED MLN_WITH_WAYLAND AND MLN_WITH_WAYLAND)
    target_compile_definitions(
        mbgl-glfw
        PRIVATE MLN_WITH_WAYLAND=1
    )
endif()

target_include_directories(
    mbgl-glfw
    PRIVATE
        ${GLFW_INCLUDE_DIRS}
        # For /platform/default/src/mbgl/map/map_snapshotter.hpp
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

include(${PROJECT_SOURCE_DIR}/vendor/tinyobjloader.cmake)

# Use target_link_directories when we move away from CMake 3.10.
target_link_libraries(
    mbgl-glfw
    PRIVATE $<$<BOOL:${GLFW_LIBRARY_DIRS}>:-L${GLFW_LIBRARY_DIRS}> tinyobjloader
)

if(WIN32)
    target_compile_definitions(
        mbgl-glfw
        PRIVATE
            NOMINMAX
            CURL_STATICLIB
    )

    target_link_libraries(
        mbgl-glfw
        PRIVATE
            glfw
            $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
    )
else()
    target_link_libraries(
        mbgl-glfw
        PRIVATE
            ${GLFW_LIBRARIES}
    )
endif()

target_link_libraries(
    mbgl-glfw
    PRIVATE
        mbgl-vendor-args
        mbgl-vendor-rapidjson
        Mapbox::Map
        mbgl-compiler-options
        MapLibreNative::Base::cheap-ruler-cpp
)

if(NOT WIN32 AND MLN_WITH_OPENGL)
    target_link_libraries(
        mbgl-glfw
        PRIVATE OpenGL::GL
    )
endif()



set_property(TARGET mbgl-glfw PROPERTY FOLDER MapLibre)

install(TARGETS mbgl-glfw RUNTIME DESTINATION bin)
