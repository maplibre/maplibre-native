if(NOT EXISTS ${PROJECT_SOURCE_DIR}/node_modules/@mapbox/cmake-node-module/module.cmake)
    message(WARNING "Not building node bindings, dependencies not found. Run 'npm update'.")
    return()
endif()

set(NODE_MODULE_CACHE_DIR ${CMAKE_SOURCE_DIR}/build/headers)
include(${PROJECT_SOURCE_DIR}/node_modules/@mapbox/cmake-node-module/module.cmake)

add_node_module(
    mbgl-node
    INSTALL_PATH ${PROJECT_SOURCE_DIR}/lib/{node_abi}/mbgl.node
    NAN_VERSION 2.14.2
    EXCLUDE_NODE_ABIS
        46
        47
        48
        51
)
find_package(PkgConfig REQUIRED)
pkg_search_module(LIBUV libuv REQUIRED)

add_library(mbgl-node-loop STATIC)

target_sources(
    mbgl-node-loop
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/async_task.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/run_loop.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/timer.cpp
)

target_include_directories(
    mbgl-node-loop
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/platform/default/include
        ${PROJECT_SOURCE_DIR}/src
    PRIVATE ${LIBUV_INCLUDE_DIRS}
)

target_link_libraries(
    mbgl-node-loop
    PUBLIC
        Mapbox::Base
)

target_sources(
    mbgl-node
    INTERFACE
        ${PROJECT_SOURCE_DIR}/platform/node/src/node_conversion.hpp
        ${PROJECT_SOURCE_DIR}/platform/node/src/node_expression.cpp
        ${PROJECT_SOURCE_DIR}/platform/node/src/node_expression.hpp
        ${PROJECT_SOURCE_DIR}/platform/node/src/node_feature.cpp
        ${PROJECT_SOURCE_DIR}/platform/node/src/node_feature.hpp
        ${PROJECT_SOURCE_DIR}/platform/node/src/node_logging.cpp
        ${PROJECT_SOURCE_DIR}/platform/node/src/node_logging.hpp
        ${PROJECT_SOURCE_DIR}/platform/node/src/node_map.cpp
        ${PROJECT_SOURCE_DIR}/platform/node/src/node_map.hpp
        ${PROJECT_SOURCE_DIR}/platform/node/src/node_mapbox_gl_native.cpp
        ${PROJECT_SOURCE_DIR}/platform/node/src/node_request.cpp
        ${PROJECT_SOURCE_DIR}/platform/node/src/node_request.hpp
        ${PROJECT_SOURCE_DIR}/platform/node/src/util/async_queue.hpp
)

target_link_libraries(
    mbgl-node
    INTERFACE mbgl-node-loop
    INTERFACE Mapbox::Map
)

# FIXME: Node bindings only run fully on Linux now because it requires libuv RunLoop (which is the default on Linux). Also, Sanitizer will
# not work here because node will do a dlopen(), which is not currently supported.
if(CMAKE_SYSTEM_NAME STREQUAL Linux AND NOT MBGL_WITH_SANITIZER)
    add_test(NAME mbgl-node-memory COMMAND npm run test-memory WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    add_test(NAME mbgl-node-test COMMAND npm run test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
