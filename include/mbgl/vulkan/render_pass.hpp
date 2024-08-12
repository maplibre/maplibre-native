#pragma once

#include <mbgl/gfx/render_pass.hpp>

#include <memory>
#include <optional>

namespace mbgl {
namespace vulkan {

class BufferResource;
class CommandEncoder;
class Context;

class RenderPass final : public gfx::RenderPass {
public:
    RenderPass(CommandEncoder&, const char* name, const gfx::RenderPassDescriptor&);
    ~RenderPass() override;

    vulkan::CommandEncoder& getEncoder() { return commandEncoder; }
    const gfx::RenderPassDescriptor& getDescriptor() { return descriptor; }
    void endEncoding();

    void clearStencil(uint32_t value = 0) const;

    void addDebugSignpost(const char* name) override;

    void bindVertex(const BufferResource&, std::size_t offset, std::size_t index, std::size_t size = 0);
    void bindFragment(const BufferResource&, std::size_t offset, std::size_t index, std::size_t size = 0);

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

private:
    gfx::RenderPassDescriptor descriptor;
    vulkan::CommandEncoder& commandEncoder;
};

} // namespace vulkan
} // namespace mbgl
