#pragma once

#include <mbgl/gfx/render_pass.hpp>
#include <memory>

// Forward declare WebGPU types
typedef struct WGPURenderPassEncoderImpl* WGPURenderPassEncoder;

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
    
    // Get the WebGPU render pass encoder for drawing
    WGPURenderPassEncoder getEncoder() const;

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;
    void addDebugSignpost(const char* name) override;

private:
    const gfx::DebugGroup<gfx::CommandEncoder> debugGroup;
    
    // Implementation details
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace webgpu
} // namespace mbgl