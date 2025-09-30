# WebGPU implementation configuration
# This file handles the integration of either Dawn or wgpu

set(MLN_WEBGPU_IMPL "dawn" CACHE STRING "WebGPU backend implementation (dawn or wgpu)")
set_property(CACHE MLN_WEBGPU_IMPL PROPERTY STRINGS dawn wgpu stub)

if(MLN_WITH_WEBGPU)
    if(MLN_WEBGPU_IMPL STREQUAL "dawn")
        message(STATUS "Using Dawn as WebGPU backend")

    elseif(MLN_WEBGPU_IMPL STREQUAL "wgpu")
        message(STATUS "Using wgpu as WebGPU backend")

        # wgpu integration
        # wgpu-native provides a Rust-based implementation of WebGPU

        # Option 1: Use wgpu-native as a submodule
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/wgpu-native)
            # wgpu-native typically provides prebuilt binaries
            find_library(WGPU_LIBRARY
                NAMES wgpu_native
                PATHS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/wgpu-native/lib
            )
            if(WGPU_LIBRARY)
                target_link_libraries(mbgl-core PRIVATE ${WGPU_LIBRARY})
                target_include_directories(mbgl-core PRIVATE
                    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/wgpu-native/include
                )
                target_compile_definitions(mbgl-core PRIVATE USE_WGPU=1)
            endif()
        else()
            # Option 2: Find system wgpu
            find_package(wgpu)
            if(wgpu_FOUND)
                target_link_libraries(mbgl-core PRIVATE wgpu::wgpu_native)
                target_compile_definitions(mbgl-core PRIVATE USE_WGPU=1)
            else()
                message(WARNING "wgpu not found. WebGPU backend will use stub implementation.")
            endif()
        endif()

    else()
        message(STATUS "Using generic WebGPU headers (no implementation)")
        # Use webgpu-headers for development/testing without a specific backend
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/webgpu-headers)
            target_include_directories(mbgl-core PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/vendor/webgpu-headers/include
            )
        endif()
    endif()
endif()
