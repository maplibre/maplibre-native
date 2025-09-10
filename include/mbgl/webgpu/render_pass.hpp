#pragma once

#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <memory>

namespace mbgl {
namespace webgpu {

class CommandEncoder;
class Context;

class RenderPass final : public gfx::RenderPass {
public:
    RenderPass(CommandEncoder& commandEncoder, const char* name, const gfx::RenderPassDescriptor& descriptor);
    ~RenderPass() override;

    void draw(gfx::DrawablePtr) override;
    void bindUniformBuffers(gfx::UniformBufferArrayPtr buffers, std::size_t uniformCount) override;
    void unbindUniformBuffers(std::size_t uniformCount) override;
    
    // WebGPU specific
    WGPURenderPassEncoder getEncoder() const { return encoder; }
    
private:
    void endEncoding();
    
    CommandEncoder& commandEncoder;
    WGPURenderPassEncoder encoder = nullptr;
    bool encodingEnded = false;
    
    // Current pipeline state
    WGPURenderPipeline currentPipeline = nullptr;
    WGPUBindGroup currentBindGroup = nullptr;
};

} // namespace webgpu
} // namespace mbgl