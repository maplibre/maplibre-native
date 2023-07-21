#pragma once

// Forward-declarations for metal-cpp types, allowing
// references without including any third-party headers

#include <memory>

namespace CA {
class MetalDrawable;
} // namespace CA

namespace NS {
template <class T>
class SharedPtr;
using UInteger = std::uintptr_t;
} // namespace NS

namespace MTL {
class Buffer;
class CommandBuffer;
class CommandQueue;
class Device;
class Function;
class RenderCommandEncoder;
class RenderPipelineState;
class RenderPassDescriptor;
} // namespace MTL

namespace mbgl {
namespace mtl {
using CAMetalDrawablePtr = NS::SharedPtr<CA::MetalDrawable>;

using MTLBufferPtr = NS::SharedPtr<MTL::Buffer>;
using MTLCommandBufferPtr = NS::SharedPtr<MTL::CommandBuffer>;
using MTLCommandQueuePtr = NS::SharedPtr<MTL::CommandQueue>;
using MTLDevicePtr = NS::SharedPtr<MTL::Device>;
using MTLFunctionPtr = NS::SharedPtr<MTL::Function>;
using MTLRenderCommandEncoderPtr = NS::SharedPtr<MTL::RenderCommandEncoder>;
using MTLRenderPassDescriptorPtr = NS::SharedPtr<MTL::RenderPassDescriptor>;
using MTLRenderPipelineStatePtr = NS::SharedPtr<MTL::RenderPipelineState>;
} // namespace mtl
} // namespace mbgl
