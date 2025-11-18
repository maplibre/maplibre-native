// Include WebGPU headers first to define WEBGPU_H_
#if MLN_WEBGPU_IMPL_DAWN
#include <webgpu/webgpu.h>
#elif MLN_WEBGPU_IMPL_WGPU
#include <webgpu.h>
#endif
#include <mbgl/webgpu/wgpu_cpp_compat.hpp>

#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/renderable_resource.hpp>
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
    const gfx::UniformBufferArray* globalUniformBuffers = nullptr;
    wgpu::TextureView colorView;
    wgpu::TextureView depthStencilView;
    wgpu::TextureFormat previousColorFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureFormat previousDepthStencilFormat = wgpu::TextureFormat::Undefined;
    bool colorFormatUpdated = false;
    bool depthFormatUpdated = false;
};

RenderPass::RenderPass(CommandEncoder& commandEncoder_, const char* name, const gfx::RenderPassDescriptor& descriptor_)
    : descriptor(descriptor_),
      commandEncoder(commandEncoder_),
      debugGroup(commandEncoder_.createDebugGroup(name)),
      impl(std::make_unique<Impl>()) {
    // Get the WebGPU command encoder from our CommandEncoder
    impl->commandEncoder = commandEncoder_.getEncoder();

    if (!impl->commandEncoder) {
        mbgl::Log::Error(mbgl::Event::Render, "WebGPU: Command encoder unavailable");
        return;
    }

    // Access the renderable resource associated with this pass
    // Check if resource exists before accessing it
    if (!descriptor.renderable.hasResource()) {
        mbgl::Log::Error(mbgl::Event::Render, "WebGPU: No renderable resource available");
        return;
    }

    auto& renderableResource = descriptor.renderable.getResource<RenderableResource>();
    renderableResource.bind();

    // Get the backend to access device and surface fallbacks
    auto& context = commandEncoder_.getContext();
    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    impl->previousColorFormat = backend.getColorFormat();
    impl->previousDepthStencilFormat = backend.getDepthStencilFormat();

    // Resolve the color attachment view, preferring the renderable's texture view
    WGPUTextureView colorViewHandle = renderableResource.getColorTextureView();
    if (!colorViewHandle) {
        if (void* textureViewPtr = backend.getCurrentTextureView()) {
            colorViewHandle = reinterpret_cast<WGPUTextureView>(textureViewPtr);
        }
    }

    if (!colorViewHandle) {
        mbgl::Log::Error(mbgl::Event::Render, "WebGPU: Failed to acquire color attachment view");
        return;
    }

    wgpuTextureViewAddRef(colorViewHandle);

#if MLN_WEBGPU_IMPL_DAWN
    impl->colorView = wgpu::TextureView::Acquire(colorViewHandle);
#elif MLN_WEBGPU_IMPL_WGPU
    impl->colorView = wgpu::TextureView(colorViewHandle);
#endif

    if (auto colorFormat = renderableResource.getColorTextureFormat()) {
        backend.setColorFormat(*colorFormat);
        impl->colorFormatUpdated = true;
    }

    WGPURenderPassColorAttachment colorAttachment = {};

#if MLN_WEBGPU_IMPL_DAWN
    colorAttachment.view = impl->colorView.Get();
#elif MLN_WEBGPU_IMPL_WGPU
    colorAttachment.view = static_cast<WGPUTextureView>(impl->colorView);

#endif
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.storeOp = WGPUStoreOp_Store;

    if (descriptor.clearColor) {
        const auto& clearColor = descriptor.clearColor.value();
        const WGPUColor value{static_cast<double>(clearColor.r),
                              static_cast<double>(clearColor.g),
                              static_cast<double>(clearColor.b),
                              static_cast<double>(clearColor.a)};
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.clearValue = value;
    } else {
        colorAttachment.loadOp = WGPULoadOp_Load;
    }

    WGPURenderPassDepthStencilAttachment depthAttachment = {};
    depthAttachment.depthLoadOp = descriptor.clearDepth ? WGPULoadOp_Clear : WGPULoadOp_Load;
    depthAttachment.depthStoreOp = WGPUStoreOp_Store;
    depthAttachment.depthClearValue = descriptor.clearDepth ? descriptor.clearDepth.value() : 1.0f;
    depthAttachment.depthReadOnly = false;
    if (descriptor.clearStencil) {
        depthAttachment.stencilLoadOp = WGPULoadOp_Clear;
        depthAttachment.stencilStoreOp = WGPUStoreOp_Store;
        depthAttachment.stencilClearValue = static_cast<uint32_t>(descriptor.clearStencil.value());
    } else {
        depthAttachment.stencilLoadOp = WGPULoadOp_Undefined;
        depthAttachment.stencilStoreOp = WGPUStoreOp_Undefined;
        depthAttachment.stencilClearValue = 0u;
    }
    depthAttachment.stencilReadOnly = false;
    depthAttachment.view = nullptr;

    WGPURenderPassDepthStencilAttachment* depthAttachmentPtr = nullptr;

    WGPUTextureView depthViewHandle = renderableResource.getDepthStencilTextureView();
    if (!depthViewHandle) {
        if (void* depthStencilViewPtr = backend.getDepthStencilView()) {
            depthViewHandle = reinterpret_cast<WGPUTextureView>(depthStencilViewPtr);
        }
    }

    if (depthViewHandle) {
        wgpuTextureViewAddRef(depthViewHandle);
#if MLN_WEBGPU_IMPL_DAWN
        impl->depthStencilView = wgpu::TextureView::Acquire(depthViewHandle);
#elif MLN_WEBGPU_IMPL_WGPU
        impl->depthStencilView = wgpu::TextureView(depthViewHandle);
#endif
        if (impl->depthStencilView) {
#if MLN_WEBGPU_IMPL_DAWN
            depthAttachment.view = impl->depthStencilView.Get();
#elif MLN_WEBGPU_IMPL_WGPU
            depthAttachment.view = static_cast<WGPUTextureView>(impl->depthStencilView);
#endif
            depthAttachmentPtr = &depthAttachment;
        }
    }

    if (depthAttachmentPtr) {
        if (auto depthFormat = renderableResource.getDepthStencilTextureFormat()) {
            backend.setDepthStencilFormat(*depthFormat);
            impl->depthFormatUpdated = true;
        }
    }

    WGPURenderPassDescriptor renderPassDesc = {};
    const WGPUStringView passLabel{name, name ? strlen(name) : 0};
    renderPassDesc.label = passLabel;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.depthStencilAttachment = depthAttachmentPtr;

    impl->encoder = wgpuCommandEncoderBeginRenderPass(impl->commandEncoder, &renderPassDesc);

    if (impl->encoder) {
        auto size = descriptor.renderable.getSize();
        wgpuRenderPassEncoderSetViewport(
            impl->encoder, 0.0f, 0.0f, static_cast<float>(size.width), static_cast<float>(size.height), 0.0f, 1.0f);
        wgpuRenderPassEncoderSetScissorRect(impl->encoder, 0, 0, size.width, size.height);
    } else {
        mbgl::Log::Error(mbgl::Event::Render, "WebGPU: Failed to begin render pass");
        impl->colorView = nullptr;
        impl->depthStencilView = nullptr;
    }
}

RenderPass::~RenderPass() {
    if (impl->encoder) {
        // End the render pass
        try {
            wgpuRenderPassEncoderEnd(impl->encoder);
        } catch (...) {
            mbgl::Log::Error(mbgl::Event::Render, "WebGPU: Failed to end render pass");
        }
        wgpuRenderPassEncoderRelease(impl->encoder);
    }

    auto& backend = static_cast<RendererBackend&>(commandEncoder.getContext().getBackend());
    if (impl->colorFormatUpdated) {
        backend.setColorFormat(impl->previousColorFormat);
    }
    if (impl->depthFormatUpdated) {
        backend.setDepthStencilFormat(impl->previousDepthStencilFormat);
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

void RenderPass::setGlobalUniformBuffers(const gfx::UniformBufferArray* buffers) {
    impl->globalUniformBuffers = buffers;
}

const gfx::UniformBufferArray* RenderPass::getGlobalUniformBuffers() const {
    return impl->globalUniformBuffers;
}

} // namespace webgpu
} // namespace mbgl
