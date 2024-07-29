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
        clearValues[0].setColor(descriptor.clearColor.value().operator std::array<float, 4>());
    clearValues[1].depthStencil.setDepth(descriptor.clearDepth.value_or(1.0f));
    clearValues[1].depthStencil.setStencil(descriptor.clearStencil.value_or(0));

    const auto renderPassBeginInfo = vk::RenderPassBeginInfo()
                                         .setRenderPass(resource.getRenderPass().get())
                                         .setFramebuffer(resource.getFramebuffer().get())
                                         .setRenderArea({{0, 0}, resource.getExtent()})
                                         .setClearValues(clearValues);

    pushDebugGroup(name);

    commandEncoder.getCommandBuffer()->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandEncoder.context.performCleanup();
}

RenderPass::~RenderPass() {
    endEncoding();

    popDebugGroup();
}

void RenderPass::endEncoding() {
    commandEncoder.getCommandBuffer()->endRenderPass();
}

void RenderPass::clearStencil(uint32_t value) const {
    const auto& resource = descriptor.renderable.getResource<RenderableResource>();
    const auto& extent = resource.getExtent();

    const auto attach = vk::ClearAttachment()
                            .setAspectMask(vk::ImageAspectFlagBits::eStencil)
                            .setClearValue(vk::ClearDepthStencilValue(0.0f, value));

    const auto& rect = vk::ClearRect().setBaseArrayLayer(0).setLayerCount(1).setRect(
        {{0, 0}, {extent.width, extent.height}});

    commandEncoder.getCommandBuffer()->clearAttachments(attach, rect);
}

void RenderPass::pushDebugGroup(const char* name) {
    commandEncoder.pushDebugGroup(name);
}

void RenderPass::popDebugGroup() {
    commandEncoder.popDebugGroup();
}

void RenderPass::addDebugSignpost(const char* name) {
    commandEncoder.getContext().getBackend().insertDebugLabel(commandEncoder.getCommandBuffer().get(), name);
}

void RenderPass::bindVertex(const BufferResource& buf, std::size_t offset, std::size_t, std::size_t size) {
    [[maybe_unused]] const auto actualSize = size ? size : buf.getSizeInBytes() - offset;
    assert(actualSize <= buf.getSizeInBytes());
}

void RenderPass::bindFragment(const BufferResource& buf, std::size_t offset, std::size_t, std::size_t size) {
    [[maybe_unused]] const auto actualSize = size ? size : buf.getSizeInBytes() - offset;
    assert(actualSize <= buf.getSizeInBytes());
}

} // namespace vulkan
} // namespace mbgl
