cmake_minimum_required(VERSION 3.24)
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.3")

# Override default CMake NATIVE_ARCH_ACTUAL
# https://gitlab.kitware.com/cmake/cmake/-/issues/20893
# https://stackoverflow.com/a/22689917/5531400
if(CMAKE_GENERATOR STREQUAL Xcode)
    set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)")
endif()
set_target_properties(mbgl-core PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES")

set_target_properties(mbgl-core PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES)

target_link_libraries(
    mbgl-core
    PRIVATE
        "-framework AppKit"
)


if(MLN_WITH_OPENGL)
    find_package(OpenGL REQUIRED)

    target_compile_definitions(
        mbgl-core
        PUBLIC GL_SILENCE_DEPRECATION
    )
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gl/headless_backend.cpp
            ${PROJECT_SOURCE_DIR}/platform/darwin/src/gl_functions.cpp ${PROJECT_SOURCE_DIR}/platform/darwin/src/headless_backend_cgl.mm
    )
    target_link_libraries(
        mbgl-core
        PRIVATE OpenGL::GL
    )
endif()

if(MLN_WITH_VULKAN)
    find_package(Vulkan REQUIRED)

    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/vulkan/headless_backend.cpp
    )

    if(Vulkan_FOUND)
        get_filename_component(Vulkan_LIBRARY_DIR ${Vulkan_LIBRARY} DIRECTORY)
        set(CMAKE_INSTALL_RPATH ${Vulkan_LIBRARY_DIR})
    else()
        set(CMAKE_INSTALL_RPATH "/usr/local/lib/")
        message(STATUS "Vulkan SDK not found! Using default homebrew install path")
    endif()

    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

target_compile_options(mbgl-core PRIVATE -fobjc-arc)

# FIXME: Should not be needed, but now needed by node because of the headless frontend.
target_include_directories(
    mbgl-core
    PUBLIC ${PROJECT_SOURCE_DIR}/platform/default/include
)

target_include_directories(
    mbgl-core
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/macos/src
)

add_subdirectory(${PROJECT_SOURCE_DIR}/bin)
add_subdirectory(${PROJECT_SOURCE_DIR}/expression-test)
if(MLN_WITH_GLFW)
add_subdirectory(${PROJECT_SOURCE_DIR}/platform/glfw)
endif()
if(MLN_WITH_NODE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/platform/node)
endif()

add_executable(
    mbgl-test-runner
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/test/main.cpp
)

target_include_directories(
    mbgl-test-runner
    PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/test/include
)

target_compile_definitions(
    mbgl-test-runner
    PRIVATE WORK_DIRECTORY=${PROJECT_SOURCE_DIR}
)

target_link_libraries(
    mbgl-test-runner
    PRIVATE mbgl-compiler-options -Wl,-force_load mbgl-test
)

add_executable(
    mbgl-benchmark-runner
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/benchmark/main.cpp
)

target_include_directories(
    mbgl-benchmark-runner
    PUBLIC ${PROJECT_SOURCE_DIR}/benchmark/include
)

target_link_libraries(
    mbgl-benchmark-runner
    PRIVATE mbgl-compiler-options -Wl,-force_load mbgl-benchmark
)

add_executable(
    mbgl-render-test-runner
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/render-test/main.cpp
)

target_link_libraries(
    mbgl-render-test-runner
    PRIVATE mbgl-compiler-options mbgl-render-test
)

set_property(TARGET mbgl-benchmark-runner PROPERTY FOLDER Executables)
set_property(TARGET mbgl-test-runner PROPERTY FOLDER Executables)
set_property(TARGET mbgl-render-test-runner PROPERTY FOLDER Executables)
set_target_properties(mbgl-benchmark-runner mbgl-test-runner mbgl-render-test-runner PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES")

# Disable benchmarks in CI as they run in VM environment
if(NOT DEFINED ENV{CI})
    add_test(NAME mbgl-benchmark-runner COMMAND mbgl-benchmark-runner WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
add_test(
    NAME mbgl-test-runner
    COMMAND
        node
        ${PROJECT_SOURCE_DIR}/test/storage/with-server.js
        ${PROJECT_SOURCE_DIR}/test/storage/server.js
        $<TARGET_FILE:mbgl-test-runner>
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

find_program(ARMERGE NAMES armerge)

if(MLN_CREATE_AMALGAMATION)
    if ("${ARMERGE}" STREQUAL "ARMERGE-NOTFOUND")
        message(FATAL_ERROR "armerge required when MLN_CREATE_AMALGAMATION=ON")
    endif()
    message(STATUS "Found armerge: ${ARMERGE}")
    include(${PROJECT_SOURCE_DIR}/cmake/find_static_library.cmake)
    set(STATIC_LIBS "")

    find_static_library(STATIC_LIBS NAMES png)
    find_static_library(STATIC_LIBS NAMES jpeg)
    find_static_library(STATIC_LIBS NAMES webp)
    find_static_library(STATIC_LIBS NAMES uv uv_a)

    add_custom_command(
        TARGET mbgl-core
        POST_BUILD
        COMMAND armerge --keep-symbols 'mbgl.*' --output libmbgl-core-amalgam.a
            $<TARGET_FILE:mbgl-core>
            $<TARGET_FILE:mbgl-freetype>
            $<TARGET_FILE:mbgl-vendor-csscolorparser>
            $<TARGET_FILE:mbgl-harfbuzz>
            $<TARGET_FILE:mbgl-vendor-parsedate>
            ${STATIC_LIBS}
    )

endif()
