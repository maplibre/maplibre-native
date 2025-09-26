#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/renderable_resource.hpp>
#include <webgpu/webgpu.h>
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
    } else {
        Log::Error(Event::Render, "WebGPU: Failed to finish command encoder");
    }

    // Create a new command encoder so subsequent passes in the same frame can continue recording
    WGPUCommandEncoderDescriptor desc = {};
    WGPUStringView encoderLabel = {"MapLibre Command Encoder", strlen("MapLibre Command Encoder")};
    desc.label = encoderLabel;
    encoder = wgpuDeviceCreateCommandEncoder(device, &desc);

    if (encoder) {
        context.setCurrentCommandEncoder(this);
    } else {
        context.setCurrentCommandEncoder(nullptr);
    }
}

void CommandEncoder::pushDebugGroup(const char*) {
    // Debug markers are currently handled at the render pass level.
}

void CommandEncoder::popDebugGroup() {
    // Debug markers are currently handled at the render pass level.
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
