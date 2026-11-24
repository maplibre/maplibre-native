# The test runner, as an executable.

add_executable(
    mbgl-test-runner
    main.cpp
)

target_include_directories(
    mbgl-test-runner
    PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/test/include
)

target_compile_definitions(
    mbgl-test-runner
    PRIVATE
        WORK_DIRECTORY=${PROJECT_SOURCE_DIR}
        $<$<PLATFORM_ID:Windows>:MBGL_BUILDING_LIB>
)

target_link_libraries(
    mbgl-test-runner
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Gui
        mbgl-compiler-options
)

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    target_link_libraries(
        mbgl-test-runner
        PRIVATE
            -Wl,-force_load mbgl-test
    )
else()
    target_link_libraries(
        mbgl-test-runner
        PRIVATE
            $<LINK_LIBRARY:WHOLE_ARCHIVE,mbgl-test>
    )
endif()

set_target_properties(
    mbgl-test-runner
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

add_test(NAME mbgl-test-runner COMMAND mbgl-test-runner WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
