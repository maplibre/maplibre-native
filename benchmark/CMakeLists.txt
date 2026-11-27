add_library(
    mbgl-benchmark STATIC EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_LIST_DIR}/api/query.benchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/render.benchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/function/camera_function.benchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/function/composite_function.benchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/function/source_function.benchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/parse/filter.benchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/parse/tile_mask.benchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/parse/vector_tile.benchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderer/group_layers.benchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/benchmark/benchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/offline_database.benchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/tilecover.benchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/color.benchmark.cpp
)

target_include_directories(
    mbgl-benchmark
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src ${PROJECT_SOURCE_DIR}/platform/default/include ${PROJECT_SOURCE_DIR}/src
)

target_include_directories(
    mbgl-benchmark
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include ${PROJECT_SOURCE_DIR}/include
)

include(${PROJECT_SOURCE_DIR}/vendor/benchmark.cmake)

mln_test_library_properties(mbgl-benchmark mbgl-vendor-benchmark)

# Needed for testing private classes
get_target_property(MLN_CORE_PRIVATE_LIBRARIES mbgl-core LINK_LIBRARIES)

target_link_libraries(
    mbgl-benchmark
    PRIVATE ${MLN_CORE_PRIVATE_LIBRARIES} mbgl-vendor-benchmark mbgl-compiler-options
    PUBLIC mbgl-core
)

set_property(TARGET mbgl-benchmark PROPERTY FOLDER MapLibre)
