#include <mln/gl/render_pass.hpp>
#include <mln/gl/command_encoder.hpp>
#include <mln/gl/renderable_resource.hpp>
#include <mln/gl/context.hpp>
#include <mln/gl/defines.hpp>
#include <mln/platform/gl_functions.hpp>

#include <array>

namespace mln {
namespace gl {

RenderPass::RenderPass(gl::CommandEncoder& commandEncoder_,
                       const char* name,
                       const gfx::RenderPassDescriptor& descriptor)
    : commandEncoder(commandEncoder_),
      debugGroup(commandEncoder.createDebugGroup(name)),
      hasDepth(descriptor.clearDepth.has_value()),
      hasStencil(descriptor.clearStencil.has_value()) {
    descriptor.renderable.getResource<gl::RenderableResource>().bind();
    const auto clearDebugGroup(commandEncoder.createDebugGroup("clear"));
    commandEncoder.context.setScissorTest({.x = 0, .y = 0, .width = 0, .height = 0});
    commandEncoder.context.clear(descriptor.clearColor, descriptor.clearDepth, descriptor.clearStencil);
}

RenderPass::~RenderPass() {
    // Tiled-GPU optimization: once a pass finishes, tell the driver its depth/stencil
    // contents are no longer needed so it can skip storing them from tile memory back
    // to main memory. This is only safe for offscreen targets (terrain drape / depth /
    // prepare framebuffers), which are rendered in a single pass and afterwards have
    // only their *color* sampled - their depth/stencil is always transient. The default
    // framebuffer (id 0) is left alone: its depth is shared across the frame's passes
    // (opaque -> translucent -> terrain surface) and must persist between them.
    auto& context = commandEncoder.context;
    if (context.bindFramebuffer.getCurrentValue() == 0) {
        return;
    }
    std::array<platform::GLenum, 2> attachments{};
    platform::GLsizei count = 0;
    if (hasDepth) {
        attachments[count++] = GL_DEPTH_ATTACHMENT;
    }
    if (hasStencil) {
        attachments[count++] = GL_STENCIL_ATTACHMENT;
    }
    if (count > 0) {
        MBGL_CHECK_ERROR(platform::glInvalidateFramebuffer(GL_FRAMEBUFFER, count, attachments.data()));
    }
}

void RenderPass::pushDebugGroup(const char* name) {
    commandEncoder.pushDebugGroup(name);
}

void RenderPass::popDebugGroup() {
    commandEncoder.popDebugGroup();
}

} // namespace gl
} // namespace mln
