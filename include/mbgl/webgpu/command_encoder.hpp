#pragma once

#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <memory>

namespace mbgl {
namespace webgpu {

class Context;
class RenderPass;

class CommandEncoder final : public gfx::CommandEncoder {
public:
    explicit CommandEncoder(Context& context);
    ~CommandEncoder() override;

    std::unique_ptr<gfx::RenderPass> createRenderPass(const char* name, const gfx::RenderPassDescriptor&) override;
    std::unique_ptr<gfx::UploadPass> createUploadPass(const char* name, gfx::UploadPassDescriptor&&) override;
    
    void present() override;
    void clearStencilBuffer(int32_t) override;
    
    // WebGPU specific methods
    WGPUCommandEncoder getEncoder() const { return encoder; }
    void finish();
    
private:
    Context& context;
    WGPUCommandEncoder encoder = nullptr;
    WGPUCommandBuffer commandBuffer = nullptr;
    bool finished = false;
};

} // namespace webgpu
} // namespace mbgl