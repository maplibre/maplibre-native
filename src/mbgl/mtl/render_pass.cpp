#include <mbgl/mtl/render_pass.hpp>

#include <mbgl/mtl/command_encoder.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/util/logging.hpp>

#include <Metal/Metal.hpp>

namespace mbgl {
namespace mtl {

struct RenderPass::Impl {
    MTLRenderCommandEncoderPtr encoder;
};

RenderPass::RenderPass(CommandEncoder& commandEncoder_, const char* name, const gfx::RenderPassDescriptor& descriptor)
    : descriptor(descriptor),
      commandEncoder(commandEncoder_),
      impl(std::make_unique<Impl>())
//, debugGroup(commandEncoder.createDebugGroup(name))
{
    auto& resource = descriptor.renderable.getResource<RenderableResource>();

    resource.bind();

    if (const auto& buffer = resource.getCommandBuffer()) {
        if (const auto& rpd = resource.getRenderPassDescriptor()) {
            impl->encoder = NS::RetainPtr(buffer->renderCommandEncoder(rpd.get()));
        }
    }

    // const auto clearDebugGroup(commandEncoder.createDebugGroup("clear"));
    // commandEncoder.context.clear(descriptor.clearColor, descriptor.clearDepth, descriptor.clearStencil);
}

RenderPass::~RenderPass() {
    endEncoding();
}

void RenderPass::endEncoding() {
    if (impl->encoder) {
        impl->encoder->endEncoding();
        impl->encoder.reset();
    }
}

const MTLRenderCommandEncoderPtr& RenderPass::getMetalEncoder() const {
    return impl->encoder;
}

void RenderPass::pushDebugGroup(const char* name) {
    // commandEncoder.pushDebugGroup(name);
}

void RenderPass::popDebugGroup() {
    // commandEncoder.popDebugGroup();
}

} // namespace mtl
} // namespace mbgl
