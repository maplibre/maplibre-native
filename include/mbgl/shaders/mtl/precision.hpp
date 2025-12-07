#pragma once

namespace mbgl {
namespace shaders {

// Metal shader precision type definitions
// Apple Silicon GPUs (GPUFamilyApple3+) have excellent half-precision performance
// Intel Mac GPUs (GPUFamilyMac2) have limited half-precision support
// This header provides conditional types that adapt based on the target GPU

constexpr auto precisionTypes = R"(

#ifdef USE_HALF_FLOAT
// Apple Silicon - use native half precision for better performance
using PrecisionFloat = half;
using PrecisionFloat2 = half2;
using PrecisionFloat3 = half3;
using PrecisionFloat4 = half4;
#define PRECISION_FLOAT(x) half(x)
#define PRECISION_FLOAT2(x, y) half2(x, y)
#define PRECISION_FLOAT3(x, y, z) half3(x, y, z)
#define PRECISION_FLOAT4(x, y, z, w) half4(x, y, z, w)
#else
// Intel Mac - use full 32-bit precision for compatibility
using PrecisionFloat = float;
using PrecisionFloat2 = float2;
using PrecisionFloat3 = float3;
using PrecisionFloat4 = float4;
#define PRECISION_FLOAT(x) float(x)
#define PRECISION_FLOAT2(x, y) float2(x, y)
#define PRECISION_FLOAT3(x, y, z) float3(x, y, z)
#define PRECISION_FLOAT4(x, y, z, w) float4(x, y, z, w)
#endif

)";

} // namespace shaders
} // namespace mbgl
