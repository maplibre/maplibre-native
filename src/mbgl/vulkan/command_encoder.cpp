#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/upload_pass.hpp>
#include <mbgl/vulkan/render_pass.hpp>

#include <cstring>

namespace mbgl {
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

void CommandEncoder::present(gfx::Renderable&) {}

void CommandEncoder::pushDebugGroup(const char* name) {
    pushDebugGroup(name, {});
}

void CommandEncoder::pushDebugGroup([[maybe_unused]] const char* name,
                                    [[maybe_unused]] const std::array<float, 4>& color) {
#ifdef ELABLE_VULKAN_VALIDATION
    commandBuffer->beginDebugUtilsLabelEXT(vk::DebugUtilsLabelEXT().setPLabelName(name).setColor(color));
#endif
}

void CommandEncoder::popDebugGroup() {
#ifdef ELABLE_VULKAN_VALIDATION
    commandBuffer->endDebugUtilsLabelEXT();
#endif
}

} // namespace vulkan
} // namespace mbgl
