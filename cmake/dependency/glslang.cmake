# glslang: an installed one if it is at least as new as the vendored copy,
# unless MLN_USE_BUILTIN_GLSLANG is set, and the vendored copy otherwise.

if(TARGET glslang)
    return()
endif()

if(NOT MLN_USE_BUILTIN_GLSLANG)
    # Without a version: glslang's version file only accepts the same major one.
    find_package(glslang CONFIG QUIET)
endif()

if(glslang_FOUND AND glslang_VERSION VERSION_GREATER_EQUAL 14)
    message(STATUS "Using installed glslang ${glslang_VERSION}.")
    add_library(glslang INTERFACE IMPORTED)
    target_link_libraries(glslang INTERFACE glslang::glslang)
    # The SPIRV headers are installed under glslang/, where the vendored ones
    # are at the root. Not every glslang has a SPIRV library of its own.
    get_target_property(glslang_include_dirs glslang::glslang INTERFACE_INCLUDE_DIRECTORIES)
    list(TRANSFORM glslang_include_dirs APPEND /glslang)
    add_library(SPIRV INTERFACE IMPORTED)
    target_include_directories(SPIRV INTERFACE ${glslang_include_dirs})
    target_link_libraries(SPIRV INTERFACE glslang::glslang $<TARGET_NAME_IF_EXISTS:glslang::SPIRV>)
    add_library(glslang-default-resource-limits INTERFACE IMPORTED)
    target_link_libraries(glslang-default-resource-limits INTERFACE glslang::glslang-default-resource-limits)
    # glslang installs no pkg-config module, but SPIRV-Tools, which it uses, does.
    mln_pkg_config_libs(-lglslang -lglslang-default-resource-limits)
    if(TARGET glslang::SPIRV)
        mln_pkg_config_libs(-lSPIRV)
    endif()
    mln_pkg_config_requires(SPIRV-Tools)
else()
    set(MLN_USE_BUILTIN_GLSLANG TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/glslang.cmake)
endif()
