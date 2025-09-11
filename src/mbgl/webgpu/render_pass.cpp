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

// Platform specific - for getting the current texture view
#ifdef __APPLE__
#include "../../platform/glfw/glfw_webgpu_backend.hpp"
#endif

namespace mbgl {
namespace webgpu {

class RenderPass::Impl {
public:
    WGPURenderPassEncoder encoder = nullptr;
    WGPUCommandEncoder commandEncoder = nullptr;
};

RenderPass::RenderPass(CommandEncoder& commandEncoder_, 
                      const char* name,
                      const gfx::RenderPassDescriptor& descriptor)
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
    
    // This is a bit hacky - we need to get the texture view from the GLFW backend
    // In a proper implementation, this would be passed through the descriptor
    // Using static_cast since we know the backend type in this context
    #ifdef __APPLE__
    auto* glfw_backend = static_cast<GLFWWebGPUBackend*>(&backend);
    if (glfw_backend) {
        textureView = glfw_backend->getCurrentTextureView();
    }
    #endif
    
    // Create a default render pass if we have a texture view
    if (textureView) {
        Log::Info(Event::General, "Got texture view for render pass");
        
        // Track if this is the first render pass in the frame
        static int renderPassCount = 0;
        static bool newFrame = true;
        
        wgpu::RenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = wgpu::TextureView(textureView);
        
        // Only clear on the first render pass of each frame
        if (descriptor.clearColor && newFrame) {
            colorAttachment.loadOp = wgpu::LoadOp::Clear;
            // Use the actual requested clear color
            colorAttachment.clearValue = {
                static_cast<float>(descriptor.clearColor->r),
                static_cast<float>(descriptor.clearColor->g),
                static_cast<float>(descriptor.clearColor->b),
                static_cast<float>(descriptor.clearColor->a)
            };
            Log::Info(Event::General, "RenderPass #" + std::to_string(renderPassCount) + 
                      ": CLEAR (first pass)");
            newFrame = false;
        } else {
            colorAttachment.loadOp = wgpu::LoadOp::Load;
            Log::Info(Event::General, "RenderPass #" + std::to_string(renderPassCount) + 
                      ": LOAD existing content");
        }
        colorAttachment.storeOp = wgpu::StoreOp::Store;
        
        renderPassCount++;
        
        // Reset for next frame (heuristic: if we've had many passes, likely new frame)
        if (renderPassCount > 10) {
            renderPassCount = 0;
            newFrame = true;
        }
        
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
            Log::Info(Event::General, "Render pass encoder created successfully");
            
            // Set viewport to full framebuffer
            // Get the framebuffer size from the backend
            #ifdef __APPLE__
            if (glfw_backend) {
                auto size = glfw_backend->getSize();
                Log::Info(Event::General, "Setting viewport: " + std::to_string(size.width) + "x" + std::to_string(size.height));
                wgpuRenderPassEncoderSetViewport(impl->encoder, 
                    0, 0,  // x, y
                    static_cast<float>(size.width), static_cast<float>(size.height),  // width, height
                    0.0f, 1.0f);  // minDepth, maxDepth
                    
                // Also set scissor rect to full viewport
                wgpuRenderPassEncoderSetScissorRect(impl->encoder, 
                    0, 0,  // x, y
                    size.width, size.height);  // width, height
                Log::Info(Event::General, "Set scissor rect to full viewport");
            }
            #endif
        }
    } else {
        Log::Warning(Event::General, "No texture view available for render pass");
    }
}

RenderPass::~RenderPass() {
    if (impl->encoder) {
        Log::Info(Event::General, "Ending WebGPU render pass");
        // End the render pass
        try {
            wgpuRenderPassEncoderEnd(impl->encoder);
            Log::Info(Event::General, "wgpuRenderPassEncoderEnd called");
        } catch (...) {
            Log::Warning(Event::General, "Failed to end render pass encoder");
        }
        wgpuRenderPassEncoderRelease(impl->encoder);
        Log::Info(Event::General, "WebGPU render pass ended");
    }
}

void RenderPass::pushDebugGroup(const char* name) {
    Log::Info(Event::General, std::string("RenderPass::pushDebugGroup called: ") + (name ? name : "null"));
    if (impl->encoder) {
        // TODO: Debug groups on render pass encoders seem to cause a freeze in Dawn
        // Commenting out for now to allow rendering to proceed
        // WGPUStringView label = {name, name ? strlen(name) : 0};
        // wgpuRenderPassEncoderPushDebugGroup(impl->encoder, label);
        Log::Info(Event::General, "RenderPass::pushDebugGroup skipped (Dawn issue)");
    } else {
        Log::Warning(Event::General, "RenderPass::pushDebugGroup called but encoder is null");
    }
}

void RenderPass::popDebugGroup() {
    Log::Info(Event::General, "RenderPass::popDebugGroup called");
    if (impl->encoder) {
        // TODO: Debug groups on render pass encoders seem to cause a freeze in Dawn
        // Commenting out for now to allow rendering to proceed
        // wgpuRenderPassEncoderPopDebugGroup(impl->encoder);
        Log::Info(Event::General, "RenderPass::popDebugGroup skipped (Dawn issue)");
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