if(NOT MLN_WITH_TESTS)
    return()
endif()

# The render test runner, as an executable.

add_executable(
    mbgl-render-test-runner
    main.cpp
)

target_link_libraries(
    mbgl-render-test-runner
    PRIVATE
        mbgl-compiler-options
        mbgl-render-test
)

if(MSVC)
    target_link_libraries(
        mbgl-render-test-runner
        PRIVATE
            $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
    )
endif()

set_target_properties(
    mbgl-render-test-runner
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        FOLDER Executables
        XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES"
)

if(NOT APPLE)
    install(TARGETS mbgl-render-test-runner RUNTIME DESTINATION bin)
endif()
