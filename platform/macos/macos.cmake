cmake_minimum_required(VERSION 3.24)

include(${PROJECT_SOURCE_DIR}/platform/darwin/darwin.cmake)

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
            ${PROJECT_SOURCE_DIR}/platform/default/src/mln/gl/gl_functions.cpp ${PROJECT_SOURCE_DIR}/platform/darwin/src/headless_backend_cgl.mm
    )
    target_link_libraries(
        mbgl-core
        PRIVATE OpenGL::GL
    )
endif()

if(MLN_WITH_VULKAN)
    find_package(Vulkan REQUIRED)

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

target_include_directories(
    mbgl-core
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
)

find_program(ARMERGE NAMES armerge)
find_program(
    LLVM_OBJCOPY
    NAMES llvm-objcopy
    HINTS
        /opt/homebrew/opt/llvm/bin
        /usr/local/opt/llvm/bin
)

if(MLN_CREATE_AMALGAMATION)
    if(NOT ARMERGE)
        message(FATAL_ERROR "armerge required when MLN_CREATE_AMALGAMATION=ON")
    endif()
    if(NOT LLVM_OBJCOPY)
        message(FATAL_ERROR "llvm-objcopy required when MLN_CREATE_AMALGAMATION=ON")
    endif()
    message(STATUS "Found armerge: ${ARMERGE}")
    message(STATUS "Found llvm-objcopy: ${LLVM_OBJCOPY}")
    include(${PROJECT_SOURCE_DIR}/cmake/find_static_library.cmake)
    set(STATIC_LIBS "")

    find_static_library(STATIC_LIBS NAMES png)
    find_static_library(STATIC_LIBS NAMES jpeg)
    find_static_library(STATIC_LIBS NAMES webp)
    find_static_library(STATIC_LIBS NAMES uv uv_a)

    add_custom_command(
        TARGET mbgl-core
        POST_BUILD
        COMMAND ${ARMERGE} --keep-symbols 'mln.*' --output libmbgl-core-amalgam.a
            $<TARGET_FILE:mbgl-core>
            $<TARGET_FILE:mbgl-freetype>
            $<TARGET_FILE:mbgl-vendor-csscolorparser>
            $<TARGET_FILE:mbgl-harfbuzz>
            $<TARGET_FILE:mbgl-vendor-parsedate>
            $<TARGET_FILE:mbgl-vendor-icu>
            $<TARGET_FILE:mlt-cpp>
            $<TARGET_FILE:fastpfor-lib>
            ${STATIC_LIBS}
        # In Mach-O/Itanium ABI names, ZTI/ZTS/ZTV identify typeinfo,
        # type names, and vtables, while St denotes std::.
        # Keep them global so C++ exception matching works in downstream code.
        COMMAND ${LLVM_OBJCOPY} --wildcard
            "--globalize-symbol=__ZT[ISV]St*"
            libmbgl-core-amalgam.a
    )

endif()

# The expression test and the default platform's runners work here, and are
# added at the top level.
set(MLN_DEFAULT_TEST_RUNNERS TRUE)
