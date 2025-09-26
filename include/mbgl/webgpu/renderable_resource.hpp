#pragma once

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/webgpu/wgpu_cpp_compat.hpp>

#include <cstdlib>
#include <optional>

namespace mbgl {
namespace webgpu {

class RendererBackend;

class RenderableResource : public gfx::RenderableResource {
protected:
    explicit RenderableResource() = default;

public:
    void bind() override {
        // WebGPU binding happens at the command encoder level
        // This is a no-op for now
    }

    virtual void swap() {
        // Renderable resources that require a swap function to be called
        // explicitly can override this method.
    }

    virtual const mbgl::webgpu::RendererBackend& getBackend() const = 0;
    virtual const WGPUCommandEncoder& getCommandEncoder() const = 0;
    virtual WGPURenderPassEncoder getRenderPassEncoder() const = 0;

    /**
     * Obtain a texture view that should be bound as the color attachment for
     * the render pass targeting this resource. Implementations should return a
     * valid `WGPUTextureView` or `nullptr` if the resource relies on the
     * backend to supply the swapchain view (e.g. default framebuffer).
     */
    virtual WGPUTextureView getColorTextureView() { return nullptr; }

    virtual std::optional<wgpu::TextureFormat> getColorTextureFormat() const { return std::nullopt; }

    /**
     * Obtain a depth/stencil texture view for the render pass if the resource
     * manages one. Returning `nullptr` indicates that the backend-provided
     * depth/stencil view (if any) should be used instead.
     */
    virtual WGPUTextureView getDepthStencilTextureView() { return nullptr; }

    virtual std::optional<wgpu::TextureFormat> getDepthStencilTextureFormat() const { return std::nullopt; }
};

} // namespace webgpu
} // namespace mbgl
