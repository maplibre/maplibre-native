if(NOT MLN_WITH_TOOLS)
    return()
endif()

add_executable(
    mbgl-cache
    ${CMAKE_CURRENT_LIST_DIR}/cache.cpp
)

target_link_libraries(
    mbgl-cache
    PRIVATE
        MapLibreNative::Base
        mbgl-vendor-args
        mbgl-compiler-options
        mbgl-core
)

add_executable(
    mbgl-offline
    ${CMAKE_CURRENT_LIST_DIR}/offline.cpp
)

target_link_libraries(
    mbgl-offline
    PRIVATE mbgl-vendor-args mbgl-compiler-options mbgl-core
)

add_executable(
    mbgl-render
    ${CMAKE_CURRENT_LIST_DIR}/render.cpp
)

target_link_libraries(
    mbgl-render
    PRIVATE mbgl-vendor-args mbgl-compiler-options mbgl-core
)

target_link_libraries(mbgl-cache PRIVATE ${MLN_LIBUV_LIBRARIES})
target_link_libraries(mbgl-offline PRIVATE ${MLN_LIBUV_LIBRARIES})
target_link_libraries(mbgl-render PRIVATE ${MLN_LIBUV_LIBRARIES})

install(TARGETS mbgl-offline mbgl-render RUNTIME DESTINATION bin)

# FIXME: CI must have a valid token
#
# add_test(NAME mbgl-offline-tool-test COMMAND mbgl-offline -s maptiler://maps/hybrid --maxZoom=0 WORKING_DIRECTORY
# ${PROJECT_SOURCE_DIR} )
#
# add_test(NAME mbgl-render-tool-test COMMAND mbgl-render WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} )
