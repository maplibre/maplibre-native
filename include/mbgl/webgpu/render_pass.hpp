#pragma once

#include <mbgl/gfx/render_pass.hpp>
#include <memory>

// Forward declare WebGPU types
typedef struct WGPURenderPassEncoderImpl* WGPURenderPassEncoder;

namespace mbgl {
namespace gfx {
class CommandEncoder;
class UniformBufferArray;
} // namespace gfx

namespace webgpu {

class CommandEncoder;

class RenderPass final : public gfx::RenderPass {
public:
    RenderPass(CommandEncoder& commandEncoder, const char* name, const gfx::RenderPassDescriptor& descriptor);
    ~RenderPass() override;

    // Match Metal's interface
    CommandEncoder& getCommandEncoder() { return commandEncoder; }
    const CommandEncoder& getCommandEncoder() const { return commandEncoder; }

    // Get the WebGPU render pass encoder (similar to Metal's getMetalEncoder)
    WGPURenderPassEncoder getEncoder() const;
    const gfx::RenderPassDescriptor& getDescriptor() const { return descriptor; }

    // Set/get global uniform buffers for drawables to access
    void setGlobalUniformBuffers(const gfx::UniformBufferArray* buffers);
    const gfx::UniformBufferArray* getGlobalUniformBuffers() const;

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;
    void addDebugSignpost(const char* name) override;

private:
    gfx::RenderPassDescriptor descriptor;
    CommandEncoder& commandEncoder;
    const gfx::DebugGroup<gfx::CommandEncoder> debugGroup;

    // Implementation details
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace webgpu
} // namespace mbgl
