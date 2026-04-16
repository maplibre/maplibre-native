# Vendored static dependencies for portable Linux shared library builds.
#
# When MLN_VENDORED_DEPS is ON (used with MLN_CREATE_AMALGAMATION), this file
# downloads and builds system-level dependencies from source with -fPIC so
# they can be amalgamated into mbgl-core without runtime dependency on
# distro-specific library versions.
#
# System curl is still linked dynamically (stable ABI, complex dep tree).

include(FetchContent)

# All vendored deps inherit CMAKE_POSITION_INDEPENDENT_CODE=ON from the
# root CMakeLists.txt, ensuring every object is compiled with -fPIC.

# ── zlib (required by libpng and others) ─────────────────────────────────────

FetchContent_Declare(
    vendored_zlib
    URL https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz
    URL_HASH SHA256=9a93b2b7dfdac77ceba5a558a580e74667dd6fede4585b91eefb60f03b72df23
)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(vendored_zlib)

# Make zlib findable by libpng's find_package(ZLIB)
set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)
set(ZLIB_INCLUDE_DIR "${vendored_zlib_SOURCE_DIR};${vendored_zlib_BINARY_DIR}" CACHE PATH "" FORCE)
set(ZLIB_LIBRARY zlibstatic CACHE STRING "" FORCE)
set(ZLIB_LIBRARIES zlibstatic CACHE STRING "" FORCE)
set(ZLIB_ROOT "${vendored_zlib_SOURCE_DIR};${vendored_zlib_BINARY_DIR}" CACHE PATH "" FORCE)
add_library(ZLIB::ZLIB ALIAS zlibstatic)

# ── libpng ───────────────────────────────────────────────────────────────────

FetchContent_Declare(
    vendored_png
    URL https://github.com/pnggroup/libpng/archive/refs/tags/v1.6.44.tar.gz
    URL_HASH SHA256=0ef5b633d0c65f780c4fced27ff832998e71478c13b45dfb6e94f23a82f64f7c
)
set(PNG_SHARED OFF CACHE BOOL "" FORCE)
set(PNG_STATIC ON CACHE BOOL "" FORCE)
set(PNG_TESTS OFF CACHE BOOL "" FORCE)
set(PNG_TOOLS OFF CACHE BOOL "" FORCE)
set(SKIP_INSTALL_EXPORT TRUE CACHE BOOL "" FORCE)
set(SKIP_INSTALL_CONFIG_FILE TRUE CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(vendored_png)
unset(SKIP_INSTALL_EXPORT CACHE)
unset(SKIP_INSTALL_CONFIG_FILE CACHE)

# ── libjpeg-turbo (ExternalProject — can't use add_subdirectory) ──────────────

include(ExternalProject)
ExternalProject_Add(
    vendored_jpeg_ext
    URL https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/3.1.0/libjpeg-turbo-3.1.0.tar.gz
    URL_HASH SHA256=9564c72b1dfd1d6fe6274c5f95a8d989b59854575d4bbee44ade7bc17aa9bc93
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DENABLE_SHARED=OFF
        -DENABLE_STATIC=ON
        -DWITH_TURBOJPEG=OFF
    BUILD_BYPRODUCTS <INSTALL_DIR>/lib/libjpeg.a
)
ExternalProject_Get_Property(vendored_jpeg_ext INSTALL_DIR)
set(VENDORED_JPEG_INSTALL_DIR "${INSTALL_DIR}")

# Pre-create the include directory so CMake doesn't complain at configure time
file(MAKE_DIRECTORY "${INSTALL_DIR}/include")

# Create an imported target for the built libjpeg
add_library(vendored_jpeg_imported STATIC IMPORTED GLOBAL)
set_target_properties(vendored_jpeg_imported PROPERTIES
    IMPORTED_LOCATION "${INSTALL_DIR}/lib/libjpeg.a"
)
add_dependencies(vendored_jpeg_imported vendored_jpeg_ext)

# ── libwebp ──────────────────────────────────────────────────────────────────

FetchContent_Declare(
    vendored_webp
    URL https://github.com/webmproject/libwebp/archive/refs/tags/v1.5.0.tar.gz
    URL_HASH SHA256=668c9aba45565e24c27e17f7aaf7060a399f7f31dba6c97a044e1feacb930f37
)
set(WEBP_BUILD_ANIM_UTILS OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_CWEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_DWEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_GIF2WEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_IMG2WEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_VWEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_WEBPINFO OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_WEBPMUX OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(vendored_webp)

# ── libuv ────────────────────────────────────────────────────────────────────

FetchContent_Declare(
    vendored_uv
    URL https://github.com/libuv/libuv/archive/refs/tags/v1.50.0.tar.gz
    URL_HASH SHA256=b1ec56444ee3f1e10c8bd3eed16ba47016ed0b94fe42137435aaf2e0bd574579
)
set(LIBUV_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(LIBUV_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LIBUV_BUILD_BENCH OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(vendored_uv)

# ── bzip2 ────────────────────────────────────────────────────────────────────

FetchContent_Declare(
    vendored_bz2
    URL https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz
    URL_HASH SHA256=ab5a03176ee106d3f0fa90e381da478ddae405918153cca248e682cd0c4a2269
)
# bzip2 doesn't ship a CMakeLists.txt, so we use FetchContent_Populate
# (not MakeAvailable) to download + extract without calling add_subdirectory.
FetchContent_GetProperties(vendored_bz2)
if(NOT vendored_bz2_POPULATED)
    FetchContent_Populate(vendored_bz2)
endif()

# Build bzip2 manually from its source files
add_library(vendored_bzip2 STATIC
    ${vendored_bz2_SOURCE_DIR}/blocksort.c
    ${vendored_bz2_SOURCE_DIR}/huffman.c
    ${vendored_bz2_SOURCE_DIR}/crctable.c
    ${vendored_bz2_SOURCE_DIR}/randtable.c
    ${vendored_bz2_SOURCE_DIR}/compress.c
    ${vendored_bz2_SOURCE_DIR}/decompress.c
    ${vendored_bz2_SOURCE_DIR}/bzlib.c
)
target_include_directories(vendored_bzip2 PUBLIC ${vendored_bz2_SOURCE_DIR})
target_compile_definitions(vendored_bzip2 PRIVATE _GNU_SOURCE)

# NOTE: OpenSSL is NOT vendored or linked explicitly. System curl links
# against its own TLS backend dynamically.

# ── License metadata ─────────────────────────────────────────────────────────
# Required by scripts/license.cmake for notice generation.

set_target_properties(zlibstatic PROPERTIES
    INTERFACE_MAPLIBRE_NAME "zlib"
    INTERFACE_MAPLIBRE_URL "https://github.com/madler/zlib"
    INTERFACE_MAPLIBRE_AUTHOR "Jean-loup Gailly and Mark Adler"
    INTERFACE_MAPLIBRE_LICENSE "${vendored_zlib_SOURCE_DIR}/LICENSE"
)

set_target_properties(png_static PROPERTIES
    INTERFACE_MAPLIBRE_NAME "libpng"
    INTERFACE_MAPLIBRE_URL "https://github.com/pnggroup/libpng"
    INTERFACE_MAPLIBRE_AUTHOR "Contributing Authors and Group 42, Inc."
    INTERFACE_MAPLIBRE_LICENSE "${vendored_png_SOURCE_DIR}/LICENSE"
)

set_target_properties(vendored_jpeg_imported PROPERTIES
    INTERFACE_MAPLIBRE_NAME "libjpeg-turbo"
    INTERFACE_MAPLIBRE_URL "https://github.com/libjpeg-turbo/libjpeg-turbo"
    INTERFACE_MAPLIBRE_AUTHOR "libjpeg-turbo contributors"
    INTERFACE_MAPLIBRE_LICENSE "${VENDORED_JPEG_INSTALL_DIR}/src/vendored_jpeg_ext/LICENSE.md"
)

set_target_properties(webp PROPERTIES
    INTERFACE_MAPLIBRE_NAME "libwebp"
    INTERFACE_MAPLIBRE_URL "https://github.com/webmproject/libwebp"
    INTERFACE_MAPLIBRE_AUTHOR "Google LLC"
    INTERFACE_MAPLIBRE_LICENSE "${vendored_webp_SOURCE_DIR}/COPYING"
)

set_target_properties(uv_a PROPERTIES
    INTERFACE_MAPLIBRE_NAME "libuv"
    INTERFACE_MAPLIBRE_URL "https://github.com/libuv/libuv"
    INTERFACE_MAPLIBRE_AUTHOR "libuv project contributors"
    INTERFACE_MAPLIBRE_LICENSE "${vendored_uv_SOURCE_DIR}/LICENSE"
)

set_target_properties(vendored_bzip2 PROPERTIES
    INTERFACE_MAPLIBRE_NAME "bzip2"
    INTERFACE_MAPLIBRE_URL "https://sourceware.org/bzip2/"
    INTERFACE_MAPLIBRE_AUTHOR "Julian Seward"
    INTERFACE_MAPLIBRE_LICENSE "${vendored_bz2_SOURCE_DIR}/LICENSE"
)

# ── Export variables for linux.cmake to consume ──────────────────────────────

set(VENDORED_PNG_TARGET png_static)
set(VENDORED_ZLIB_TARGET zlibstatic)
set(VENDORED_JPEG_TARGET vendored_jpeg_imported)
set(VENDORED_WEBP_TARGET webp)
set(VENDORED_UV_TARGET uv_a)
set(VENDORED_BZ2_TARGET vendored_bzip2)

set(VENDORED_JPEG_INCLUDE_DIR "${VENDORED_JPEG_INSTALL_DIR}/include")
set(VENDORED_UV_INCLUDE_DIR "${vendored_uv_SOURCE_DIR}/include")
set(VENDORED_WEBP_INCLUDE_DIR "${vendored_webp_SOURCE_DIR}/src")

# Export vendored targets so maplibre-native's export() calls don't fail
export(TARGETS
    zlibstatic png_static webp sharpyuv uv_a vendored_bzip2
    APPEND FILE MapboxCoreTargets.cmake
)

message(STATUS "Vendored static dependencies configured (all built with -fPIC)")
