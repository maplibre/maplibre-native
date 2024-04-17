#pragma once

// Forward-declarations for metal-cpp types, allowing
// references without including any third-party headers

#include <cstdint>

namespace CA {
class MetalDrawable;
class MetalLayer;
} // namespace CA

namespace NS {
template <class T>
class SharedPtr;
using UInteger = std::uintptr_t;
} // namespace NS

namespace MTL {
class BlitCommandEncoder;
class BlitPassDescriptor;
class Buffer;
class CaptureScope;
class CommandBuffer;
class CommandQueue;
class Device;
class DepthStencilState;
class Function;
class RenderCommandEncoder;
class RenderPipelineState;
class RenderPassDescriptor;
class Texture;
class TextureDescriptor;
class SamplerState;
class SamplerDescriptor;
class VertexAttributeDescriptor;
class VertexBufferLayoutDescriptor;
class VertexDescriptor;
} // namespace MTL

namespace mbgl {
namespace mtl {

// When the undefined behavior sanitizer is active, use a wrapper that prevents
// methods calls through null pointers in the mtlcpp smart pointer class.
#if __has_feature(undefined_behavior_sanitizer)
template <typename T>
class SafeSharedPtr;
#define MTLCPP_SHARED_PTR SafeSharedPtr
#else
#define MTLCPP_SHARED_PTR NS::SharedPtr
#endif // __has_feature(undefined_behavior_sanitizer)

using CAMetalDrawablePtr = MTLCPP_SHARED_PTR<CA::MetalDrawable>;
using CAMetalLayerPtr = MTLCPP_SHARED_PTR<CA::MetalLayer>;

using MTLBlitCommandEncoderPtr = MTLCPP_SHARED_PTR<MTL::BlitCommandEncoder>;
using MTLBlitPassDescriptorPtr = MTLCPP_SHARED_PTR<MTL::BlitPassDescriptor>;
using MTLBufferPtr = MTLCPP_SHARED_PTR<MTL::Buffer>;
using MTLCaptureScopePtr = MTLCPP_SHARED_PTR<MTL::CaptureScope>;
using MTLCommandBufferPtr = MTLCPP_SHARED_PTR<MTL::CommandBuffer>;
using MTLCommandQueuePtr = MTLCPP_SHARED_PTR<MTL::CommandQueue>;
using MTLDevicePtr = MTLCPP_SHARED_PTR<MTL::Device>;
using MTLDepthStencilStatePtr = MTLCPP_SHARED_PTR<MTL::DepthStencilState>;
using MTLFunctionPtr = MTLCPP_SHARED_PTR<MTL::Function>;
using MTLRenderCommandEncoderPtr = MTLCPP_SHARED_PTR<MTL::RenderCommandEncoder>;
using MTLRenderPassDescriptorPtr = MTLCPP_SHARED_PTR<MTL::RenderPassDescriptor>;
using MTLRenderPipelineStatePtr = MTLCPP_SHARED_PTR<MTL::RenderPipelineState>;
using MTLTexturePtr = MTLCPP_SHARED_PTR<MTL::Texture>;
using MTLTextureDescriptorPtr = MTLCPP_SHARED_PTR<MTL::TextureDescriptor>;
using MTLSamplerStatePtr = MTLCPP_SHARED_PTR<MTL::SamplerState>;
using MTLSamplerDescriptorPtr = MTLCPP_SHARED_PTR<MTL::SamplerDescriptor>;
using MTLVertexAttributeDescriptorPtr = MTLCPP_SHARED_PTR<MTL::VertexAttributeDescriptor>;
using MTLVertexBufferLayoutDescriptorPtr = MTLCPP_SHARED_PTR<MTL::VertexBufferLayoutDescriptor>;
using MTLVertexDescriptorPtr = MTLCPP_SHARED_PTR<MTL::VertexDescriptor>;
} // namespace mtl
} // namespace mbgl
