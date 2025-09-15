#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/renderable_resource.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <mbgl/util/logging.hpp>
#include <cstring> // for strlen

// Platform-specific includes for swap
#ifdef __APPLE__
#include "../../platform/glfw/glfw_webgpu_backend.hpp"
#endif

namespace mbgl {
namespace webgpu {

CommandEncoder::CommandEncoder(Context& ctx)
    : context(ctx) {
    // Initialize WebGPU command encoder
    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());

    if (device) {
        WGPUCommandEncoderDescriptor desc = {};
        WGPUStringView label = {"MapLibre Command Encoder", strlen("MapLibre Command Encoder")};
        desc.label = label;
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

std::unique_ptr<gfx::UploadPass> CommandEncoder::createUploadPass(const char* name, gfx::Renderable& renderable) {
    Log::Info(Event::Render, std::string("WebGPU CommandEncoder: Creating upload pass: ") + (name ? name : "unnamed"));
    return std::make_unique<UploadPass>(renderable, *this, name);
}

void CommandEncoder::present(gfx::Renderable& renderable) {
    // Submit the command buffer first
    submitCommandBuffer();

    // Then call swap on the RenderableResource to present the surface
    // This matches Metal's pattern where swap handles surface presentation
    try {
        renderable.getResource<webgpu::RenderableResource>().swap();
    } catch (...) {
        // No RenderableResource available - surface presentation handled elsewhere
    }
}

void CommandEncoder::submitCommandBuffer() {
    // Submit command buffer directly (for cases without RenderableResource)
    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());

    if (!device || !queue || !encoder) {
        return;
    }

    // Finish and submit the command buffer
    WGPUCommandBufferDescriptor cmdBufferDesc = {};
    WGPUStringView label = {"Command Buffer", strlen("Command Buffer")};
    cmdBufferDesc.label = label;
    WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);

    if (cmdBuffer) {
        wgpuQueueSubmit(queue, 1, &cmdBuffer);
        wgpuCommandBufferRelease(cmdBuffer);
        Log::Info(Event::Render, "WebGPU: Command buffer submitted to queue");
    } else {
        Log::Error(Event::Render, "WebGPU: Failed to finish command encoder");
    }

    // Create a new command encoder for the next frame
    encoder = nullptr;
    WGPUCommandEncoderDescriptor newDesc = {};
    WGPUStringView newLabel = {"MapLibre Command Encoder", strlen("MapLibre Command Encoder")};
    newDesc.label = newLabel;
    encoder = wgpuDeviceCreateCommandEncoder(device, &newDesc);
}



void CommandEncoder::pushDebugGroup(const char* name) {
    if (encoder) {
        WGPUStringView label = {name, name ? strlen(name) : 0};
        wgpuCommandEncoderPushDebugGroup(encoder, label);
    } else {
    }
}

void CommandEncoder::popDebugGroup() {
    if (encoder) {
        wgpuCommandEncoderPopDebugGroup(encoder);
    } else {
    }
}

void CommandEncoder::trackUploadPass(UploadPass* uploadPass) {
    // Track the upload pass for debug group management
    currentUploadPass = uploadPass;
}

void CommandEncoder::forgetUploadPass(UploadPass* uploadPass) {
    // Forget the upload pass when it's destroyed
    if (currentUploadPass == uploadPass) {
        currentUploadPass = nullptr;
    }
}

} // namespace webgpu
} // namespace mbgl
