#include <mbgl/vulkan/render_pass.hpp>

#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace vulkan {

RenderPass::RenderPass(CommandEncoder& commandEncoder_, const char* name, const gfx::RenderPassDescriptor& descriptor)
    : descriptor(descriptor),
      commandEncoder(commandEncoder_) {
    auto& resource = descriptor.renderable.getResource<RenderableResource>();

    resource.bind();

    std::array<vk::ClearValue, 2> clearValues;
    
    if (descriptor.clearColor.has_value())
        clearValues[0].setColor(descriptor.clearColor.value().operator std::array<float, 4Ui64>());
    clearValues[1].depthStencil.setDepth(descriptor.clearDepth.value_or(1.0f));
    clearValues[1].depthStencil.setStencil(descriptor.clearStencil.value_or(0));

    const auto& renderPassBeginInfo = vk::RenderPassBeginInfo()
        .setRenderPass(resource.renderPass.get())
        .setFramebuffer(resource.swapchainFramebuffers[commandEncoder.context.getCurrentFrameResourceIndex()].get())
        .setRenderArea({ {0, 0}, resource.extent })
        .setClearValues(clearValues);

    commandEncoder.getCommandBuffer()->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandEncoder.context.performCleanup();
}

RenderPass::~RenderPass() {
    endEncoding();
}

void RenderPass::endEncoding() {

    commandEncoder.getCommandBuffer()->endRenderPass();

    debugGroups.clear();
}

void RenderPass::pushDebugGroup(const char* name) {

}

void RenderPass::popDebugGroup() {

}

void RenderPass::addDebugSignpost(const char* name) {

}

void RenderPass::bindVertex(const BufferResource& buf, std::size_t offset, std::size_t index, std::size_t size) {
    const auto actualSize = size ? size : buf.getSizeInBytes() - offset;
    assert(actualSize <= buf.getSizeInBytes());
    
}

void RenderPass::bindFragment(const BufferResource& buf, std::size_t offset, std::size_t index, std::size_t size) {
    const auto actualSize = size ? size : buf.getSizeInBytes() - offset;
    assert(actualSize <= buf.getSizeInBytes());
    
}

} // namespace vulkan
} // namespace mbgl
