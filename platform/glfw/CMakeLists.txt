find_package(OpenGL REQUIRED)

if(WIN32)
    if(MLN_WITH_OSMESA)
        message(WARNING "mbgl-glfw does not work with -DMLN_WITH_OSMESA=ON. If you want to build this target, don't use this option.")
        return()
    endif()

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
)

set_property(
    SOURCE ${PROJECT_SOURCE_DIR}/platform/glfw/glfw_view.cpp
    PROPERTY COMPILE_DEFINITIONS MAPBOX_PUCK_ASSETS_PATH=\"${PROJECT_SOURCE_DIR}/platform/glfw/assets/\"
)

if(MLN_WITH_OPENGL)
    target_sources(
        mbgl-glfw
        PRIVATE ${PROJECT_SOURCE_DIR}/platform/glfw/glfw_gl_backend.cpp
    )
    target_compile_definitions(
        mbgl-glfw
        PRIVATE MBGL_RENDER_BACKEND_OPENGL=1
    )
endif()

if(MLN_WITH_EGL)
    target_compile_definitions(
        mbgl-glfw
        PRIVATE MBGL_WITH_EGL=1
    )
endif()

target_include_directories(
    mbgl-glfw
    PRIVATE
        ${GLFW_INCLUDE_DIRS}
        # For /platform/default/src/mbgl/map/map_snapshotter.hpp
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
        Mapbox::Base::Extras::args
        Mapbox::Base::Extras::filesystem
        Mapbox::Base::Extras::rapidjson
        Mapbox::Map
        mbgl-compiler-options
        Mapbox::Base::cheap-ruler-cpp
)

if(NOT WIN32 AND MLN_WITH_OPENGL)
    target_link_libraries(
        mbgl-glfw
        PRIVATE OpenGL::GL
    )
endif()

set_property(TARGET mbgl-glfw PROPERTY FOLDER Executables)

install(TARGETS mbgl-glfw RUNTIME DESTINATION bin)
