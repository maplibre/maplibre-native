if(WIN32)
    find_package(glfw3 REQUIRED)
else()
    find_package(PkgConfig REQUIRED)

    pkg_search_module(GLFW glfw3 REQUIRED)
endif()

add_executable(
    mbgl-glfw
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/glfw_view.cpp
    ${CMAKE_CURRENT_LIST_DIR}/glfw_renderer_frontend.cpp
    ${CMAKE_CURRENT_LIST_DIR}/settings_json.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_writer.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mln/map/map_snapshotter.cpp
)


set_property(
    SOURCE ${CMAKE_CURRENT_LIST_DIR}/glfw_view.cpp
    PROPERTY COMPILE_DEFINITIONS MLN_ASSETS_PATH=\"${CMAKE_CURRENT_LIST_DIR}/assets/\"
)

if(APPLE)
    target_sources(
        mbgl-glfw
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/glfw_apple_init.mm
    )
endif()

if(MLN_WITH_OPENGL)
    find_package(OpenGL REQUIRED)
    target_sources(
        mbgl-glfw
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/glfw_gl_backend.cpp
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
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/glfw_vulkan_backend.cpp
    )
endif()

if(MLN_WITH_METAL)
    target_sources(
        mbgl-glfw
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/glfw_metal_backend.mm
        ${CMAKE_CURRENT_LIST_DIR}/metal_backend.mm
        ${CMAKE_CURRENT_LIST_DIR}/glfw_metal_backend.h
        ${CMAKE_CURRENT_LIST_DIR}/metal_backend.h
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
            PRIVATE ${CMAKE_CURRENT_LIST_DIR}/glfw_webgpu_backend.mm
        )
    else()
        target_sources(
            mbgl-glfw
            PRIVATE ${CMAKE_CURRENT_LIST_DIR}/glfw_webgpu_backend.cpp
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
        # For /platform/default/src/mln/map/map_snapshotter.hpp
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

# Use target_link_directories when we move away from CMake 3.10.
target_link_libraries(
    mbgl-glfw
    PRIVATE $<$<BOOL:${GLFW_LIBRARY_DIRS}>:-L${GLFW_LIBRARY_DIRS}>
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
            ${MLN_LIBUV_LIBRARIES}
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

if(MLN_WITH_PLUGINS)
    target_compile_definitions(mbgl-glfw PRIVATE MLN_GLFW_PLUGINS=1)
    target_link_libraries(mbgl-glfw PRIVATE mln-ngon-layer mln-fill-extrusion)
endif()

set_property(TARGET mbgl-glfw PROPERTY FOLDER MapLibre)

install(TARGETS mbgl-glfw RUNTIME DESTINATION bin)
