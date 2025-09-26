# WebGPU implementation configuration
# This file handles the integration of either Dawn or wgpu

option(MLN_WEBGPU_IMPL "WebGPU backend implementation (dawn or wgpu)" "dawn")

if(MLN_WITH_WEBGPU)
    if(MLN_WEBGPU_IMPL STREQUAL "dawn")
        message(STATUS "Using Dawn as WebGPU backend")

        # Dawn integration
        # Dawn provides a native implementation of WebGPU
        # It can be built from source or linked as a prebuilt library

        # Option 1: Use Dawn as a submodule or prebuilt
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/dawn)
            # Check if Dawn is already built
            if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/dawn/build/src/dawn/native/libwebgpu_dawn.a)
                message(STATUS "Using prebuilt Dawn libraries")
                target_link_libraries(mbgl-core PRIVATE 
                    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/dawn/build/src/dawn/native/libwebgpu_dawn.a
                    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/dawn/build/src/dawn/libdawn_proc.a
                    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/dawn/build/src/dawn/platform/libdawn_platform.a
                    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/dawn/build/src/dawn/common/libdawn_common.a
                )
                target_include_directories(mbgl-core SYSTEM PRIVATE
                    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/dawn/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/dawn/build/gen/include
                )
            else()
                message(FATAL_ERROR "Dawn is not built. Please build Dawn first in vendor/dawn/build")
            endif()
            target_compile_definitions(mbgl-core PRIVATE MLN_WITH_DAWN=1)
        else()
            # Option 2: Find prebuilt Dawn
            find_package(Dawn)
            if(Dawn_FOUND)
                target_link_libraries(mbgl-core PRIVATE Dawn::dawn_native Dawn::dawn_proc)
                target_compile_definitions(mbgl-core PRIVATE MLN_WITH_DAWN=1)
            else()
                message(WARNING "Dawn not found. WebGPU backend will use stub implementation.")
            endif()
        endif()

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
