# WebGPU implementation configuration
# This file handles the integration of either Dawn or wgpu

set(MLN_WEBGPU_IMPL "dawn" CACHE STRING "WebGPU backend implementation (dawn or wgpu)")
set_property(CACHE MLN_WEBGPU_IMPL PROPERTY STRINGS dawn wgpu stub)

if(MLN_WITH_WEBGPU)
    if(MLN_WEBGPU_IMPL STREQUAL "dawn")
        message(STATUS "Using Dawn as WebGPU backend")

    elseif(MLN_WEBGPU_IMPL STREQUAL "wgpu")
        message(STATUS "Using wgpu as WebGPU backend")

    else()
        message(FATAL_ERROR "MLN_WEBGPU_IMPL can take values \"dawn\" or \"wgpu\"")

    endif()
endif()
