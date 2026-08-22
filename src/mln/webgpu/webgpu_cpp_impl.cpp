// Implementation file for WebGPU-Cpp wrapper
// This file must be compiled exactly once to provide implementations
// for the WebGPU C++ wrapper functions

#define WEBGPU_CPP_IMPLEMENTATION
// TODO: remove this workaround as soon as https://github.com/eliemichel/WebGPU-Cpp/issues/35 is fixed
#ifndef _NULLABLE
#define _NULLABLE WGPU_NULLABLE
#endif
#include <webgpu.hpp>

#undef _NULLABLE
