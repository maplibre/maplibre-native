#pragma once

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wstrict-aliasing"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

#if MLN_WEBGPU_IMPL_DAWN
// For Dawn backend, use Dawn's C++ wrapper
#include <webgpu/webgpu_cpp.h>
#elif MLN_WEBGPU_IMPL_WGPU
// For wgpu-native backend, use the WebGPU-Cpp wrapper
// TODO: remove this workaround as soon as https://github.com/eliemichel/WebGPU-Cpp/issues/35 is fixed
#ifndef _NULLABLE
#define _NULLABLE WGPU_NULLABLE
#endif
#include <webgpu.hpp>

#undef _NULLABLE

#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
