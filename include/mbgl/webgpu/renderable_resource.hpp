#pragma once

#include <mbgl/gfx/renderable.hpp>
#include <webgpu/webgpu_cpp.h>

#include <cstdlib>

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
};

} // namespace webgpu
} // namespace mbgl