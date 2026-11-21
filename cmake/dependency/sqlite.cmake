# SQLite, as mbgl-sqlite: an installed one if it is at least as new as the
# vendored copy, unless MLN_USE_BUILTIN_SQLITE is set, and the vendored copy
# otherwise.

if(TARGET mbgl-sqlite)
    return()
endif()

if(NOT MLN_USE_BUILTIN_SQLITE)
    find_package(SQLite3 QUIET)
endif()

if(SQLite3_FOUND AND SQLite3_VERSION VERSION_GREATER_EQUAL 3.45.3)
    message(STATUS "Using installed SQLite ${SQLite3_VERSION}.")
    # SQLite3::SQLite3 since CMake 4.3, which deprecates SQLite::SQLite3.
    if(TARGET SQLite3::SQLite3)
        add_library(mbgl-sqlite ALIAS SQLite3::SQLite3)
    else()
        add_library(mbgl-sqlite ALIAS SQLite::SQLite3)
    endif()
    mln_pkg_config_requires(sqlite3)
else()
    set(MLN_USE_BUILTIN_SQLITE TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/sqlite.cmake)
    add_library(mbgl-sqlite ALIAS mbgl-vendor-sqlite)
endif()
