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
#include <cstdlib> // for std::getenv

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

        // Add depth/stencil attachment
        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment = {};
        void* depthStencilViewPtr = backend.getDepthStencilView();
        if (depthStencilViewPtr) {
            // Don't use Acquire - the backend owns the view
            wgpu::TextureView depthStencilView(reinterpret_cast<WGPUTextureView>(depthStencilViewPtr));

            depthStencilAttachment.view = depthStencilView;

            // Configure depth operations
            if (descriptor.clearDepth) {
                depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
                depthStencilAttachment.depthClearValue = descriptor.clearDepth.value();
            } else {
                depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Load;
            }
            depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;

            // Configure stencil operations
            if (descriptor.clearStencil) {
                depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
                depthStencilAttachment.stencilClearValue = descriptor.clearStencil.value();
            } else {
                depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Load;
            }
            depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;

            renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
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
    }
}

RenderPass::~RenderPass() {
    if (impl->encoder) {

        // End the render pass
        try {
            wgpuRenderPassEncoderEnd(impl->encoder);
        } catch (...) {
        }
        wgpuRenderPassEncoderRelease(impl->encoder);
    }
}

void RenderPass::pushDebugGroup(const char* name) {
    if (impl->encoder) {
        // Debug groups on render pass encoders can cause issues in some Dawn versions
        // Enable with environment variable if needed for debugging
        static bool enableDebugGroups = std::getenv("WEBGPU_ENABLE_DEBUG_GROUPS") != nullptr;
        if (enableDebugGroups) {
            WGPUStringView label = {name, name ? strlen(name) : 0};
            wgpuRenderPassEncoderPushDebugGroup(impl->encoder, label);
        }
    }
}

void RenderPass::popDebugGroup() {
    if (impl->encoder) {
        // Debug groups on render pass encoders can cause issues in some Dawn versions
        // Enable with environment variable if needed for debugging
        static bool enableDebugGroups = std::getenv("WEBGPU_ENABLE_DEBUG_GROUPS") != nullptr;
        if (enableDebugGroups) {
            wgpuRenderPassEncoderPopDebugGroup(impl->encoder);
        }
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
