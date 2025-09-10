#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/backend_impl.hpp>

namespace mbgl {
namespace webgpu {

CommandEncoder::CommandEncoder(Context& ctx) 
    : context(ctx) {
    // Initialize WebGPU command encoder
    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    
    if (device) {
        WGPUCommandEncoderDescriptor desc = {};
        desc.label = "MapLibre Command Encoder";
        encoder = wgpuDeviceCreateCommandEncoder(device, &desc);
    }
}

CommandEncoder::~CommandEncoder() {
    if (encoder) {
        wgpuCommandEncoderRelease(encoder);
    }
}

std::unique_ptr<gfx::RenderPass> CommandEncoder::createRenderPass(const char* name,
                                                                   const gfx::RenderPassDescriptor& descriptor) {
    return std::make_unique<RenderPass>(*this, name, descriptor);
}

std::unique_ptr<gfx::UploadPass> CommandEncoder::createUploadPass(const char* name, gfx::Renderable&) {
    return std::make_unique<UploadPass>(*this, name);
}

void CommandEncoder::present(gfx::Renderable& renderable) {
    // Submit command buffer and present the surface
    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());
    
    if (!device || !queue || !encoder) {
        return;
    }
    
    // Finish and submit the command buffer
    WGPUCommandBufferDescriptor cmdBufferDesc = {};
    cmdBufferDesc.label = "Command Buffer";
    WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);
    
    if (cmdBuffer) {
        wgpuQueueSubmit(queue, 1, &cmdBuffer);
        wgpuCommandBufferRelease(cmdBuffer);
    }
    
    // Present the surface if applicable
    // Note: The actual surface presentation depends on the platform integration
    (void)renderable;
}

void CommandEncoder::clearStencilBuffer(const gfx::ClearValue& value) {
    // WebGPU doesn't have a direct clear stencil command outside of render pass
    // This would typically be done within a render pass
    (void)value;
}

void CommandEncoder::clearDepthBuffer(const gfx::ClearValue& value) {
    // WebGPU doesn't have a direct clear depth command outside of render pass
    // This would typically be done within a render pass
    (void)value;
}

void CommandEncoder::setStencilMode(const gfx::StencilMode& mode) {
    // Stencil mode is set per pipeline in WebGPU
    currentStencilMode = mode;
}

void CommandEncoder::setDepthMode(const gfx::DepthMode& mode) {
    // Depth mode is set per pipeline in WebGPU
    currentDepthMode = mode;
}

void CommandEncoder::setColorMode(const gfx::ColorMode& mode) {
    // Color mode is set per pipeline in WebGPU
    currentColorMode = mode;
}

void CommandEncoder::setCullFaceMode(const gfx::CullFaceMode& mode) {
    // Cull face mode is set per pipeline in WebGPU
    currentCullFaceMode = mode;
}

void CommandEncoder::draw(const gfx::DrawMode& drawMode,
                          std::size_t vertexOffset,
                          std::size_t vertexCount) {
    // Drawing commands must be issued within a render pass
    // This will be handled by the RenderPass class
    (void)drawMode;
    (void)vertexOffset;
    (void)vertexCount;
}

void CommandEncoder::pushDebugGroup(const char* name) {
    if (encoder) {
        wgpuCommandEncoderPushDebugGroup(encoder, name);
    }
}

void CommandEncoder::popDebugGroup() {
    if (encoder) {
        wgpuCommandEncoderPopDebugGroup(encoder);
    }
}

WGPUCommandEncoder CommandEncoder::getWGPUEncoder() const {
    return encoder;
}

} // namespace webgpu
} // namespace mbgl