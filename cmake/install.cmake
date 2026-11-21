# Installing the core, in the `core` component, so that it can be built and
# installed as a normal system package. Each library, and each directory of
# headers, is installed where it is defined, with these.

# All of this is only when the core is a static library. With Qt it is an
# object library, part of Qt's own library, as are its vendored libraries.

# Install the core, and the vendored libraries it links.
function(mln_install_libraries)
    get_target_property(type mbgl-core TYPE)
    if(NOT type STREQUAL "STATIC_LIBRARY")
        return()
    endif()

    install(
        TARGETS ${ARGN}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT core
    )
    # For the pkg-config file.
    set_property(GLOBAL APPEND PROPERTY MLN_CORE_INSTALLED_LIBRARIES ${ARGN})
endfunction()

# Install the headers in directories, for building against the installed core.
# Only the `.h` and `.hpp` files, as the directories also hold code generation
# templates and the like, and not those of the renderers not built, which
# include headers of APIs that are not there.
function(mln_install_headers)
    get_target_property(type mbgl-core TYPE)
    if(NOT type STREQUAL "STATIC_LIBRARY")
        return()
    endif()

    set(excludes "")
    if(NOT MLN_WITH_OPENGL)
        list(APPEND excludes PATTERN gl EXCLUDE)
    endif()
    if(NOT MLN_WITH_METAL)
        list(APPEND excludes PATTERN mtl EXCLUDE)
    endif()
    if(NOT MLN_WITH_VULKAN)
        list(APPEND excludes PATTERN vulkan EXCLUDE)
    endif()
    if(NOT MLN_WITH_WEBGPU)
        list(APPEND excludes PATTERN webgpu EXCLUDE)
    endif()

    cmake_parse_arguments(PARSE_ARGV 0 arg "" "" "DIRECTORIES")
    install(
        DIRECTORY ${arg_DIRECTORIES}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT core
        FILES_MATCHING
            PATTERN "*.h"
            PATTERN "*.hpp"
            ${excludes}
    )
endfunction()
