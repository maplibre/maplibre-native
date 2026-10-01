#include <mln/gl/render_pass.hpp>
#include <mln/gl/command_encoder.hpp>
#include <mln/gl/renderable_resource.hpp>
#include <mln/gl/context.hpp>

namespace mln {
namespace gl {

RenderPass::RenderPass(gl::CommandEncoder& commandEncoder_,
                       const char* name,
                       const gfx::RenderPassDescriptor& descriptor)
    : commandEncoder(commandEncoder_),
      debugGroup(commandEncoder.createDebugGroup(name)) {
    descriptor.renderable.getResource<gl::RenderableResource>().bind();
    const auto clearDebugGroup(commandEncoder.createDebugGroup("clear"));
    commandEncoder.context.setScissorTest({.x = 0, .y = 0, .width = 0, .height = 0});
    commandEncoder.context.clear(descriptor.clearColor, descriptor.clearDepth, descriptor.clearStencil);
}

void RenderPass::pushDebugGroup(const char* name) {
    commandEncoder.pushDebugGroup(name);
}

void RenderPass::popDebugGroup() {
    commandEncoder.popDebugGroup();
}

} // namespace gl
} // namespace mln
