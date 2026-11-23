# libuv, for the default run loop: MLN_LIBUV_LIBRARIES to link it, and
# LIBUV_INCLUDE_DIRS. Found as each platform provides it: as a CMake package on
# MSVC, as vcpkg does, as the OHOS SDK's library there, and with pkg-config
# everywhere else.

if(DEFINED MLN_LIBUV_LIBRARIES)
    return()
endif()

if(MSVC)
    find_package(libuv REQUIRED)
    find_path(LIBUV_INCLUDE_DIRS uv.h)
    set(MLN_LIBUV_LIBRARIES $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>)
elseif(MLN_PLATFORM STREQUAL ohos)
    set(MLN_LIBUV_LIBRARIES uv)
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(LIBUV REQUIRED IMPORTED_TARGET libuv)
    set(MLN_LIBUV_LIBRARIES PkgConfig::LIBUV)
endif()
