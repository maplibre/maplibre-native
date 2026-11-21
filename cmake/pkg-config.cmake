# A pkg-config file for linking against the installed core, mbgl-core.pc.
#
# The core is only ever a static library, so whatever links it also links its
# dependencies: those built from vendored code, installed beside it, and those
# found installed, through their own pkg-config files where they have them. None
# is part of its interface, so they would be `.private`, but those are only
# given with `--static`, which a consumer would have to know to pass, and which
# also gives every dependency's own private dependencies, though they are shared
# libraries linking those themselves. So they are in `Libs` and `Requires`,
# which a plain `pkg-config --libs` gives.
#
# Where the core links an installed library, the pkg-config module it comes
# with, or the linker flags for it if it has none, are recorded with these, so
# linking another one into the core needs such a line next to it too.

function(mln_pkg_config_requires)
    set_property(GLOBAL APPEND PROPERTY MLN_CORE_PC_REQUIRES ${ARGN})
endfunction()

function(mln_pkg_config_libs)
    set_property(GLOBAL APPEND PROPERTY MLN_CORE_PC_LIBS ${ARGN})
endfunction()

# Write and install the file, once everything has been recorded. Like the rest
# of installing, only when the core is a static library.
function(mln_install_pkg_config)
    get_target_property(type mbgl-core TYPE)
    if(NOT type STREQUAL "STATIC_LIBRARY")
        return()
    endif()

    set(libs "")
    get_property(installed GLOBAL PROPERTY MLN_CORE_INSTALLED_LIBRARIES)
    foreach(library IN LISTS installed)
        string(APPEND libs " -l$<TARGET_FILE_BASE_NAME:${library}>")
    endforeach()
    get_property(flags GLOBAL PROPERTY MLN_CORE_PC_LIBS)
    foreach(flag IN LISTS flags)
        string(APPEND libs " ${flag}")
    endforeach()

    get_property(requires GLOBAL PROPERTY MLN_CORE_PC_REQUIRES)
    list(REMOVE_DUPLICATES requires)
    list(JOIN requires " " requires)

    file(
        GENERATE
        OUTPUT ${CMAKE_BINARY_DIR}/mbgl-core.pc
        CONTENT "prefix=${CMAKE_INSTALL_PREFIX}
libdir=${CMAKE_INSTALL_FULL_LIBDIR}
includedir=${CMAKE_INSTALL_FULL_INCLUDEDIR}

Name: mbgl-core
Description: The core library of MapLibre Native
Version: 0
Cflags: -I\${includedir}
Libs: -L\${libdir}${libs}
Requires: ${requires}
"
    )

    install(
        FILES ${CMAKE_BINARY_DIR}/mbgl-core.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        COMPONENT core
    )
endfunction()
