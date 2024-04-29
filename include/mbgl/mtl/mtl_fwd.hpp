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

using CAMetalDrawablePtr = NS::SharedPtr<CA::MetalDrawable>;
using CAMetalLayerPtr = NS::SharedPtr<CA::MetalLayer>;

using MTLBlitCommandEncoderPtr = NS::SharedPtr<MTL::BlitCommandEncoder>;
using MTLBlitPassDescriptorPtr = NS::SharedPtr<MTL::BlitPassDescriptor>;
using MTLBufferPtr = NS::SharedPtr<MTL::Buffer>;
using MTLCaptureScopePtr = NS::SharedPtr<MTL::CaptureScope>;
using MTLCommandBufferPtr = NS::SharedPtr<MTL::CommandBuffer>;
using MTLCommandQueuePtr = NS::SharedPtr<MTL::CommandQueue>;
using MTLDevicePtr = NS::SharedPtr<MTL::Device>;
using MTLDepthStencilStatePtr = NS::SharedPtr<MTL::DepthStencilState>;
using MTLFunctionPtr = NS::SharedPtr<MTL::Function>;
using MTLRenderCommandEncoderPtr = NS::SharedPtr<MTL::RenderCommandEncoder>;
using MTLRenderPassDescriptorPtr = NS::SharedPtr<MTL::RenderPassDescriptor>;
using MTLRenderPipelineStatePtr = NS::SharedPtr<MTL::RenderPipelineState>;
using MTLTexturePtr = NS::SharedPtr<MTL::Texture>;
using MTLTextureDescriptorPtr = NS::SharedPtr<MTL::TextureDescriptor>;
using MTLSamplerStatePtr = NS::SharedPtr<MTL::SamplerState>;
using MTLSamplerDescriptorPtr = NS::SharedPtr<MTL::SamplerDescriptor>;
using MTLVertexAttributeDescriptorPtr = NS::SharedPtr<MTL::VertexAttributeDescriptor>;
using MTLVertexBufferLayoutDescriptorPtr = NS::SharedPtr<MTL::VertexBufferLayoutDescriptor>;
using MTLVertexDescriptorPtr = NS::SharedPtr<MTL::VertexDescriptor>;
} // namespace mtl
} // namespace mbgl
