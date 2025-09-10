#include <mbgl/webgpu/uniform_buffer_array.hpp>
#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/render_pass.hpp>

namespace mbgl {
namespace webgpu {

UniformBufferArray::UniformBufferArray() = default;

UniformBufferArray::~UniformBufferArray() = default;

void UniformBufferArray::bind(gfx::RenderPass&) {
    // TODO: Bind uniform buffers to WebGPU render pass
    // auto& webgpuRenderPass = static_cast<webgpu::RenderPass&>(renderPass);
    // Iterate through uniform buffers and bind them
    for (const auto& buffer : uniformBufferVector) {
        if (buffer) {
            // TODO: Bind this uniform buffer to the pipeline
        }
    }
}

std::unique_ptr<gfx::UniformBuffer> UniformBufferArray::copy(const gfx::UniformBuffer& other) {
    // Create a copy of the uniform buffer
    const auto& webgpuBuffer = static_cast<const webgpu::UniformBuffer&>(other);
    return std::make_unique<UniformBuffer>(nullptr, webgpuBuffer.getSize(), false);
}

} // namespace webgpu
} // namespace mbgl