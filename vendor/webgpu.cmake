# WebGPU implementation configuration
# This file handles the integration of either Dawn or wgpu

set(MLN_WEBGPU_IMPL_DAWN OFF CACHE STRING "WebGPU with Dawn implementation")
set(MLN_WEBGPU_IMPL_WGPU OFF CACHE STRING "WebGPU with wgpu implementation")
set(MLN_WEBGPU_IMPL_FFI OFF CACHE STRING "WebGPU with ffi interface")
set(MLN_WEBGPU_IMPL_WEBGPU_HEADER_DIR "" CACHE STRING "If not empty use this path to the webgpu.h file")

if(MLN_WITH_WEBGPU)
    if(MLN_WEBGPU_IMPL_DAWN)
        message(STATUS "Using Dawn as WebGPU backend")
        add_compile_definitions(MLN_WEBGPU_IMPL_DAWN)

    elseif(MLN_WEBGPU_IMPL_WGPU)
        message(STATUS "Using wgpu as WebGPU backend")
        add_compile_definitions(MLN_WEBGPU_IMPL_WGPU)
    else()
        message(FATAL_ERROR "MLN_WEBGPU_IMPL_DAWN or MLN_WEBGPU_IMPL_WGPU must be set")
    endif()

	if (MLN_WEBGPU_IMPL_FFI)
        add_compile_definitions(MLN_WEBGPU_IMPL_FFI)
	endif()
endif()
