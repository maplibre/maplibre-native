set(NODE_MODULE_MINIMUM_ABI 127) # Minimum supported Node.js version: 22
set(NODE_MODULE_CACHE_DIR ${CMAKE_BINARY_DIR}/headers)

if(DEFINED ENV{MSYSTEM})
    message(FATAL "MSYS2 is not supported, Node.js in Windows only has binary compatibility with Microsoft Visual C++.")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmake/module.cmake)


# For node versions, see: https://raw.githubusercontent.com/nodejs/node/main/doc/abi_version_registry.json
add_node_module(
    mbgl-node
    INSTALL_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/{node_abi}/mbgl.node
    NAN_VERSION 2.27.0
    EXCLUDE_NODE_ABIS
        46
        47
        48
        51
)

include(${PROJECT_SOURCE_DIR}/cmake/dependency/libuv.cmake)

add_library(mbgl-node-loop STATIC)

target_sources(
    mbgl-node-loop
    PRIVATE
        ${MLN_DEFAULT_LIBUV_RUN_LOOP_SOURCES}
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
        MapLibreNative::Base
)

target_sources(
    mbgl-node
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/node_conversion.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/node_expression.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/node_expression.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/node_feature.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/node_feature.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/node_logging.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/node_logging.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/node_map.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/node_map.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/node_mapbox_gl_native.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/node_request.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/node_request.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/util/async_queue.hpp
)

target_link_libraries(
    mbgl-node
    INTERFACE mbgl-node-loop
    INTERFACE Mapbox::Map
)

# FIXME: Node bindings only run fully on Linux now because it requires libuv RunLoop (which is the default on Linux). Also, Sanitizer will
# not work here because node will do a dlopen(), which is not currently supported.
if(CMAKE_SYSTEM_NAME STREQUAL Linux AND NOT MLN_WITH_SANITIZER)
    add_test(NAME mbgl-node-memory COMMAND npm run test-memory WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    add_test(NAME mbgl-node-test COMMAND npm run test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

# needed because v8config.h uses __cplusplus
# https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-170
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
endif()
