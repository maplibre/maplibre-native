add_library(
    mbgl-render-test STATIC EXCLUDE_FROM_ALL
    ${PROJECT_SOURCE_DIR}/expression-test/test_runner_common.cpp
    ${PROJECT_SOURCE_DIR}/expression-test/test_runner_common.hpp
    ${CMAKE_CURRENT_LIST_DIR}/allocation_index.cpp
    ${CMAKE_CURRENT_LIST_DIR}/allocation_index.hpp
    ${CMAKE_CURRENT_LIST_DIR}/file_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/file_source.hpp
    ${CMAKE_CURRENT_LIST_DIR}/filesystem.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/mln/render_test.hpp
    ${CMAKE_CURRENT_LIST_DIR}/manifest_parser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/manifest_parser.hpp
    ${CMAKE_CURRENT_LIST_DIR}/metadata.hpp
    ${CMAKE_CURRENT_LIST_DIR}/parser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/parser.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/runner.cpp
    ${CMAKE_CURRENT_LIST_DIR}/runner.hpp
)

mln_test_library_properties(mbgl-render-test)

# FIXME: Should not use core private interface
target_include_directories(
    mbgl-render-test
    PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/platform/default/include
)

target_include_directories(
    mbgl-render-test
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include ${PROJECT_SOURCE_DIR}/include
)

# needed for test_runner_common.hpp
target_include_directories(
    mbgl-render-test
    PRIVATE ${PROJECT_SOURCE_DIR}/expression-test
)

include(${PROJECT_SOURCE_DIR}/vendor/boost.cmake)

target_link_libraries(
    mbgl-render-test
    PRIVATE
        mbgl-vendor-args
        MapLibreNative::Base::pixelmatch-cpp
        mbgl-compiler-options
        mbgl-vendor-boost
    PUBLIC mbgl-core
)

set_property(TARGET mbgl-render-test PROPERTY FOLDER MapLibre)
