#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
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

std::unique_ptr<gfx::UploadPass> CommandEncoder::createUploadPass(const char* name, gfx::Renderable&) {
    return std::make_unique<UploadPass>(*this, name);
}

void CommandEncoder::present(gfx::Renderable&) {

    // Submit command buffer and present the surface
    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());

    if (!device || !queue || !encoder) {
        Log::Error(Event::General, "Missing device, queue, or encoder in present");
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
        Log::Error(Event::General, "Failed to finish command encoder");
    }

    // Note: Surface presentation (swap) is handled by GLFWView::renderSync()
    // We don't need to call swap here as it will be called by the view

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
        Log::Warning(Event::General, "pushDebugGroup called but encoder is null");
    }
}

void CommandEncoder::popDebugGroup() {
    if (encoder) {
        wgpuCommandEncoderPopDebugGroup(encoder);
    } else {
        Log::Warning(Event::General, "popDebugGroup called but encoder is null");
    }
}

} // namespace webgpu
} // namespace mbgl
