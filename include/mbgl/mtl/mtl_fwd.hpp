#pragma once

// Forward-declarations for metal-cpp types, allowing
// references without including any third-party headers

#include <cstdint>

namespace CA {
class MetalDrawable;
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

template <typename T>
class SafeSharedPtr;

using CAMetalDrawablePtr = SafeSharedPtr<CA::MetalDrawable>;

using MTLBlitCommandEncoderPtr = SafeSharedPtr<MTL::BlitCommandEncoder>;
using MTLBlitPassDescriptorPtr = SafeSharedPtr<MTL::BlitPassDescriptor>;
using MTLBufferPtr = SafeSharedPtr<MTL::Buffer>;
using MTLCaptureScopePtr = SafeSharedPtr<MTL::CaptureScope>;
using MTLCommandBufferPtr = SafeSharedPtr<MTL::CommandBuffer>;
using MTLCommandQueuePtr = SafeSharedPtr<MTL::CommandQueue>;
using MTLDevicePtr = SafeSharedPtr<MTL::Device>;
using MTLDepthStencilStatePtr = SafeSharedPtr<MTL::DepthStencilState>;
using MTLFunctionPtr = SafeSharedPtr<MTL::Function>;
using MTLRenderCommandEncoderPtr = SafeSharedPtr<MTL::RenderCommandEncoder>;
using MTLRenderPassDescriptorPtr = SafeSharedPtr<MTL::RenderPassDescriptor>;
using MTLRenderPipelineStatePtr = SafeSharedPtr<MTL::RenderPipelineState>;
using MTLTexturePtr = SafeSharedPtr<MTL::Texture>;
using MTLTextureDescriptorPtr = SafeSharedPtr<MTL::TextureDescriptor>;
using MTLSamplerStatePtr = SafeSharedPtr<MTL::SamplerState>;
using MTLSamplerDescriptorPtr = SafeSharedPtr<MTL::SamplerDescriptor>;
using MTLVertexAttributeDescriptorPtr = SafeSharedPtr<MTL::VertexAttributeDescriptor>;
using MTLVertexBufferLayoutDescriptorPtr = SafeSharedPtr<MTL::VertexBufferLayoutDescriptor>;
using MTLVertexDescriptorPtr = SafeSharedPtr<MTL::VertexDescriptor>;
} // namespace mtl
} // namespace mbgl
