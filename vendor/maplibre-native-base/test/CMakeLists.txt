include(${PROJECT_SOURCE_DIR}/extras/googletest.cmake)

set(TEST_BINARY_PATH ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${PROJECT_SOURCE_DIR}/test/test_defines.hpp.in ${PROJECT_BINARY_DIR}/test_defines.hpp)

function(create_test folder_name)
    set (target_name "test_${folder_name}")
    set (curr_folder "${PROJECT_SOURCE_DIR}/test/${folder_name}")

    file (GLOB_RECURSE test_files "${curr_folder}/*.cpp" "${curr_folder}/*.hpp")
    message (STATUS "Adding test ${target_name}")

    add_executable(${target_name} ${test_files})
    target_link_libraries(${target_name} PRIVATE
        MapLibreNative::Base
        gtest_all
        pthread
    )

    target_include_directories(${target_name} PRIVATE
            ${curr_folder}
            ${PROJECT_BINARY_DIR}
    )

    add_test(NAME ${target_name} COMMAND ${target_name} ${TEST_ARGUMENTS})
endfunction()

create_test("compatibility")
create_test("io")
create_test("std")
create_test("util")
