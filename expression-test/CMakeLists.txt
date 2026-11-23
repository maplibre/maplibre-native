add_executable(
    mbgl-expression-test
    ${CMAKE_CURRENT_LIST_DIR}/expression_test_logger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/expression_test_logger.hpp
    ${CMAKE_CURRENT_LIST_DIR}/expression_test_parser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/expression_test_parser.hpp
    ${CMAKE_CURRENT_LIST_DIR}/expression_test_runner.cpp
    ${CMAKE_CURRENT_LIST_DIR}/expression_test_runner.hpp
    ${CMAKE_CURRENT_LIST_DIR}/test_runner_common.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_runner_common.hpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

# FIXME: Should not use core private interface
target_include_directories(
    mbgl-expression-test
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    mbgl-expression-test
    PRIVATE
        MapLibreNative::Base
        mbgl-vendor-args
        mbgl-core
        mbgl-compiler-options
)

set_property(TARGET mbgl-expression-test PROPERTY FOLDER MapLibre)

string(RANDOM LENGTH 5 ALPHABET 0123456789 MLN_EXPRESSION_TEST_SEED)

add_test(
    NAME mbgl-expression-test
    COMMAND mbgl-expression-test -s --seed=${MLN_EXPRESSION_TEST_SEED}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

target_link_libraries(mbgl-expression-test PRIVATE ${MLN_LIBUV_LIBRARIES})
