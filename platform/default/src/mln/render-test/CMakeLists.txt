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

if(NOT APPLE)
    install(TARGETS mbgl-render-test-runner RUNTIME DESTINATION bin)
endif()
