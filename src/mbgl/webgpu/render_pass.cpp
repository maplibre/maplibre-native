// Include Dawn headers first to define WEBGPU_H_
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/util/logging.hpp>

#include <cstring> // for strlen

namespace mbgl {
namespace webgpu {

class RenderPass::Impl {
public:
    WGPURenderPassEncoder encoder = nullptr;
    WGPUCommandEncoder commandEncoder = nullptr;
};

RenderPass::RenderPass(CommandEncoder& commandEncoder_, const char* name, const gfx::RenderPassDescriptor& descriptor)
    : debugGroup(commandEncoder_.createDebugGroup(name)),
      impl(std::make_unique<Impl>()) {
    // Get the WebGPU command encoder from our CommandEncoder
    impl->commandEncoder = commandEncoder_.getEncoder();

    if (!impl->commandEncoder) {
        Log::Error(Event::General, "Failed to get WebGPU command encoder for render pass");
        return;
    }

    // Get the backend to access device and surface
    auto& context = commandEncoder_.getContext();
    auto& backend = static_cast<RendererBackend&>(context.getBackend());

    // Try to get the current texture view from the surface
    wgpu::TextureView textureView;

    // Get the texture view through the virtual method
    void* textureViewPtr = backend.getCurrentTextureView();
    if (textureViewPtr) {
        textureView = wgpu::TextureView::Acquire(reinterpret_cast<WGPUTextureView>(textureViewPtr));
    }

    // Create a default render pass if we have a texture view
    if (textureView) {

        wgpu::RenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = wgpu::TextureView(textureView);

        // Use the descriptor to determine if we should clear
        // MapLibre sets clearColor for the first pass that needs clearing
        if (descriptor.clearColor) {
            colorAttachment.loadOp = wgpu::LoadOp::Clear;
            colorAttachment.clearValue = {static_cast<float>(descriptor.clearColor->r),
                                          static_cast<float>(descriptor.clearColor->g),
                                          static_cast<float>(descriptor.clearColor->b),
                                          static_cast<float>(descriptor.clearColor->a)};

        } else {
            colorAttachment.loadOp = wgpu::LoadOp::Load;

        }
        colorAttachment.storeOp = wgpu::StoreOp::Store;

        wgpu::RenderPassDescriptor renderPassDesc = {};
        renderPassDesc.label = name;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;

        // TODO: Add depth/stencil attachment if needed
        if (descriptor.clearDepth || descriptor.clearStencil) {
            // Would need to create/get depth texture and set up depth/stencil attachment
        }

        // Create the render pass encoder
        wgpu::CommandEncoder wgpuEncoder(impl->commandEncoder);
        wgpu::RenderPassEncoder passEncoder = wgpuEncoder.BeginRenderPass(&renderPassDesc);
        impl->encoder = passEncoder.MoveToCHandle();

        if (impl->encoder) {


            // Set viewport to full framebuffer
            // Get the framebuffer size from the backend
            auto size = backend.getFramebufferSize();
            wgpuRenderPassEncoderSetViewport(impl->encoder,
                                             0,
                                             0, // x, y
                                             static_cast<float>(size.width),
                                             static_cast<float>(size.height), // width, height
                                             0.0f,
                                             1.0f); // minDepth, maxDepth

            // Also set scissor rect to full viewport
            wgpuRenderPassEncoderSetScissorRect(impl->encoder,
                                                0,
                                                0, // x, y
                                                size.width,
                                                size.height); // width, height
        }
    } else {
        Log::Warning(Event::General, "No texture view available for render pass");
    }
}

RenderPass::~RenderPass() {
    if (impl->encoder) {

        // End the render pass
        try {
            wgpuRenderPassEncoderEnd(impl->encoder);
        } catch (...) {
            Log::Warning(Event::General, "Failed to end render pass encoder");
        }
        wgpuRenderPassEncoderRelease(impl->encoder);
    }
}

void RenderPass::pushDebugGroup(const char* name) {
    if (impl->encoder) {
        // TODO: Debug groups on render pass encoders seem to cause a freeze in Dawn
        // Commenting out for now to allow rendering to proceed
        // WGPUStringView label = {name, name ? strlen(name) : 0};
        // wgpuRenderPassEncoderPushDebugGroup(impl->encoder, label);
    } else {
        Log::Warning(Event::General, "RenderPass::pushDebugGroup called but encoder is null");
    }
}

void RenderPass::popDebugGroup() {
    if (impl->encoder) {
        // TODO: Debug groups on render pass encoders seem to cause a freeze in Dawn
        // Commenting out for now to allow rendering to proceed
        // wgpuRenderPassEncoderPopDebugGroup(impl->encoder);
    } else {
        Log::Warning(Event::General, "RenderPass::popDebugGroup called but encoder is null");
    }
}

void RenderPass::addDebugSignpost(const char* name) {
    if (impl->encoder) {
        WGPUStringView label = {name, name ? strlen(name) : 0};
        wgpuRenderPassEncoderInsertDebugMarker(impl->encoder, label);
    }
}

WGPURenderPassEncoder RenderPass::getEncoder() const {
    return impl->encoder;
}

} // namespace webgpu
} // namespace mbgl
