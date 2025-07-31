# ARM64 Windows fixup
# This file ensures proper configuration for ARM64 builds

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64" OR CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
    message(STATUS "Applying ARM64 fixups for Windows build")
    
    set(VCPKG_ARM64_ROOT "${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/installed/arm64-windows")
    
    # Simplify library variables to avoid complex generator expressions
    if(TARGET JPEG::JPEG)
        set(JPEG_LIBRARIES JPEG::JPEG)
    endif()
    
    if(TARGET PNG::PNG)
        set(PNG_LIBRARIES PNG::PNG)
    endif()
    
    # All dependencies are now available via vcpkg for ARM64
    # No special handling needed for libuv anymore
    
    # Fix ICU::data target which might not exist for ARM64
    if(NOT TARGET ICU::data AND TARGET ICU::uc)
        add_library(ICU::data INTERFACE IMPORTED)
    endif()
    
    # All dependencies are available for ARM64, no need to remove any source files
    
    # Add include directories for missing headers
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/installed/x86-windows/include/GLES3")
        include_directories("${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/installed/x86-windows/include")
        message(STATUS "Using x86 OpenGL headers for ARM64 build")
    endif()
    
    # Link libuv to mbgl-core if it was found
    if(LIBUV_FOUND AND (TARGET libuv::uv OR TARGET libuv::uv_a))
        target_link_libraries(mbgl-core PRIVATE $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>)
        message(STATUS "Linked libuv to mbgl-core for ARM64")
    endif()
endif()