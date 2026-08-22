#include <mln/vulkan/command_encoder.hpp>
#include <mln/vulkan/context.hpp>
#include <mln/vulkan/renderable_resource.hpp>
#include <mln/vulkan/upload_pass.hpp>
#include <mln/vulkan/render_pass.hpp>

#include <cstring>

namespace mln {
namespace vulkan {

CommandEncoder::CommandEncoder(Context& context_, const vk::UniqueCommandBuffer& buffer_)
    : context(context_),
      commandBuffer(buffer_) {}

CommandEncoder::~CommandEncoder() {}

std::unique_ptr<gfx::UploadPass> CommandEncoder::createUploadPass(const char* name, gfx::Renderable& renderable) {
    return std::make_unique<UploadPass>(renderable, *this, name);
}

std::unique_ptr<gfx::RenderPass> CommandEncoder::createRenderPass(const char* name,
                                                                  const gfx::RenderPassDescriptor& descriptor) {
    return std::make_unique<RenderPass>(*this, name, descriptor);
}

void CommandEncoder::present(gfx::Renderable& renderable) {
    renderable.getResource<RenderableResource>().swap();
}

void CommandEncoder::pushDebugGroup(const char* name) {
    pushDebugGroup(name, {});
}

void CommandEncoder::pushDebugGroup(const char* name, const std::array<float, 4>& color) {
    context.getBackend().beginDebugLabel(commandBuffer.get(), name, color);
}

void CommandEncoder::popDebugGroup() {
    context.getBackend().endDebugLabel(commandBuffer.get());
}

} // namespace vulkan
} // namespace mln
