#pragma once

#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>

namespace mbgl {
namespace gfx {
class Renderable;
} // namespace gfx

namespace vulkan {

class Context;
class RenderPass;
class UploadPass;

class CommandEncoder final : public gfx::CommandEncoder {
public:
    explicit CommandEncoder(Context& context_, const vk::UniqueCommandBuffer& buffer_);
    ~CommandEncoder() override;

    vulkan::Context& getContext() { return context; }
    const vulkan::Context& getContext() const { return context; }
    const vk::UniqueCommandBuffer& getCommandBuffer() const { return commandBuffer; }

    std::unique_ptr<gfx::UploadPass> createUploadPass(const char* name, gfx::Renderable&) override;
    std::unique_ptr<gfx::RenderPass> createRenderPass(const char* name, const gfx::RenderPassDescriptor&) override;

    void present(gfx::Renderable&) override;

private:
    void pushDebugGroup(const char* name) override;
    void pushDebugGroup(const char* name, const std::array<float, 4>& color);
    void popDebugGroup() override;

private:
    friend class RenderPass;
    friend class UploadPass;

    vulkan::Context& context;
    const vk::UniqueCommandBuffer& commandBuffer;
};

} // namespace vulkan
} // namespace mbgl
