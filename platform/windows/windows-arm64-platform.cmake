# Windows ARM64 platform-specific configuration
# This file should be included after the main windows.cmake

# Override library detection for ARM64 when packages are not available
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64")
    message(STATUS "Configuring for Windows ARM64")
    
    # Try to find ARM64 packages first
    set(VCPKG_ARM64_ROOT "${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/installed/arm64-windows")
    
    # Check which packages are actually available for ARM64
    if(EXISTS "${VCPKG_ARM64_ROOT}/lib/jpeg.lib")
        set(JPEG_LIBRARIES "${VCPKG_ARM64_ROOT}/lib/jpeg.lib")
        set(JPEG_INCLUDE_DIRS "${VCPKG_ARM64_ROOT}/include")
    else()
        message(WARNING "JPEG library not found for ARM64, image loading will be disabled")
        set(JPEG_FOUND FALSE)
    endif()
    
    if(EXISTS "${VCPKG_ARM64_ROOT}/lib/libpng16.lib")
        set(PNG_LIBRARIES "${VCPKG_ARM64_ROOT}/lib/libpng16.lib")
        set(PNG_INCLUDE_DIRS "${VCPKG_ARM64_ROOT}/include")
    else()
        message(WARNING "PNG library not found for ARM64, PNG support will be disabled")
        set(PNG_FOUND FALSE)
    endif()
    
    if(EXISTS "${VCPKG_ARM64_ROOT}/lib/libwebp.lib")
        set(WEBP_LIBRARIES "${VCPKG_ARM64_ROOT}/lib/libwebp.lib")
        set(WEBP_INCLUDE_DIRS "${VCPKG_ARM64_ROOT}/include")
    else()
        message(WARNING "WebP library not found for ARM64, WebP support will be disabled")
        set(WEBP_FOUND FALSE)
    endif()
    
    if(EXISTS "${VCPKG_ARM64_ROOT}/lib/libuv.lib")
        set(LIBUV_LIBRARIES "${VCPKG_ARM64_ROOT}/lib/libuv.lib")
        set(LIBUV_INCLUDE_DIRS "${VCPKG_ARM64_ROOT}/include")
    else()
        message(WARNING "libuv not found for ARM64, async operations may be limited")
        set(LIBUV_FOUND FALSE)
    endif()
    
    # Remove source files that depend on unavailable libraries
    if(NOT JPEG_FOUND)
        list(REMOVE_ITEM mbgl_core_SOURCES 
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/jpeg_reader.cpp)
    endif()
    
    if(NOT PNG_FOUND)
        list(REMOVE_ITEM mbgl_core_SOURCES
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/png_reader.cpp
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/png_writer.cpp)
    endif()
    
    if(NOT WEBP_FOUND)
        list(REMOVE_ITEM mbgl_core_SOURCES
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/webp_reader.cpp)
    endif()
endif()