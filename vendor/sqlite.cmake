if(TARGET mbgl-vendor-sqlite)
    return()
endif()

if(MLN_WITH_QT)
    add_library(mbgl-vendor-sqlite OBJECT)
else()
    add_library(mbgl-vendor-sqlite STATIC)
endif()

target_sources(
    mbgl-vendor-sqlite PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/sqlite/src/sqlite3.c
)

include(CheckSymbolExists)
check_symbol_exists("strerror_r" "string.h" MLN_SQLITE3_HAVE_STRERROR_R)

if(MLN_SQLITE3_HAVE_STRERROR_R)
    target_compile_definitions(
        mbgl-vendor-sqlite
        PRIVATE HAVE_STRERROR_R
    )
endif()

# So we don't need to link with -ldl
target_compile_definitions(
    mbgl-vendor-sqlite
    PRIVATE SQLITE_OMIT_LOAD_EXTENSION SQLITE_THREADSAFE
)

target_include_directories(
    mbgl-vendor-sqlite SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/sqlite/include
)

export(TARGETS
    mbgl-vendor-sqlite
    APPEND FILE MapboxCoreTargets.cmake
)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set_target_properties(mbgl-vendor-sqlite PROPERTIES COMPILE_FLAGS "-pthread")
endif()
