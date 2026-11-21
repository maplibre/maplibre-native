if(NOT MLN_WITH_TESTS)
    return()
endif()

# The test runner, as an executable.

add_executable(
    mbgl-test-runner
    main.cpp
)

target_compile_definitions(
    mbgl-test-runner
    PRIVATE
        WORK_DIRECTORY=${PROJECT_SOURCE_DIR}
)

if(DEFINED ENV{CI} AND NOT APPLE)
    target_compile_definitions(
        mbgl-test-runner
        PRIVATE
            CI_BUILD=1
    )
endif()

if(WIN32)
    target_compile_definitions(
        mbgl-test-runner
        PRIVATE
            MBGL_BUILDING_LIB
    )
    target_include_directories(
        mbgl-test-runner
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/windows/include
    )
endif()

target_link_libraries(
    mbgl-test-runner
    PRIVATE
        mbgl-compiler-options
        $<LINK_LIBRARY:WHOLE_ARCHIVE,mbgl-test>
)

if(MSVC)
    target_link_libraries(
        mbgl-test-runner
        PRIVATE
            $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
    )
endif()

set_target_properties(
    mbgl-test-runner
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        FOLDER Executables
        XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES"
)

if(APPLE)
    add_test(
        NAME mbgl-test-runner
        COMMAND
            node
            ${PROJECT_SOURCE_DIR}/test/storage/with-server.js
            ${PROJECT_SOURCE_DIR}/test/storage/server.js
            $<TARGET_FILE:mbgl-test-runner>
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
else()
    add_test(NAME mbgl-test-runner COMMAND mbgl-test-runner WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
