#pragma once

#include <memory>

namespace NS {
template <class T> class SharedPtr;
} // namespace NS

namespace MTL {
class CommandQueue;
class Device;
class Function;
class RenderCommandEncoder;
class RenderPipelineState;
class RenderPassDescriptor;
}

namespace mbgl {
namespace mtl {

using MTLCommandQueuePtr = NS::SharedPtr<MTL::CommandQueue>;
using MTLDevicePtr = NS::SharedPtr<MTL::Device>;
using MTLFunctionPtr = NS::SharedPtr<MTL::Function>;
using MTLRenderCommandEncoderPtr = NS::SharedPtr<MTL::RenderCommandEncoder>;
using MTLRenderPassDescriptorPtr = NS::SharedPtr<MTL::RenderPassDescriptor>;
using MTLRenderPipelineStatePtr = NS::SharedPtr<MTL::RenderPipelineState>;

} // namespace mtl
} // namespace mbgl
