#pragma once

#include <mbgl/gfx/render_pass.hpp>

namespace mbgl {
namespace gfx {
class CommandEncoder;
} // namespace gfx

namespace webgpu {

class CommandEncoder;

class RenderPass final : public gfx::RenderPass {
public:
    RenderPass(CommandEncoder& commandEncoder, const char* name, const gfx::RenderPassDescriptor& descriptor);
    ~RenderPass() override;

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;
    void addDebugSignpost(const char* name) override;

private:
    [[maybe_unused]] CommandEncoder& commandEncoder;
    const gfx::DebugGroup<gfx::CommandEncoder> debugGroup;
};

} // namespace webgpu
} // namespace mbgl