#include <mbgl/mtl/render_pass.hpp>

#include <mbgl/mtl/command_encoder.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/context.hpp>

namespace mbgl {
namespace mtl {

RenderPass::RenderPass(CommandEncoder& commandEncoder_,
                       const char* name,
                       const gfx::RenderPassDescriptor& descriptor)
    : commandEncoder(commandEncoder_)
      //, debugGroup(commandEncoder.createDebugGroup(name))
{
    //descriptor.renderable.getResource<gl::RenderableResource>().bind();
    //const auto clearDebugGroup(commandEncoder.createDebugGroup("clear"));
    //commandEncoder.context.clear(descriptor.clearColor, descriptor.clearDepth, descriptor.clearStencil);
}

void RenderPass::pushDebugGroup(const char* name) {
    //commandEncoder.pushDebugGroup(name);
}

void RenderPass::popDebugGroup() {
    //commandEncoder.popDebugGroup();
}

} // namespace mtl
} // namespace mbgl
