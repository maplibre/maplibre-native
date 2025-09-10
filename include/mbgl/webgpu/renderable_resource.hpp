#pragma once

#include <mbgl/gfx/renderable.hpp>

namespace mbgl {
namespace webgpu {

class RendererBackend;

class RenderableResource : public gfx::RenderableResource {
protected:
    explicit RenderableResource() = default;

public:
    virtual void swap() {
        // Renderable resources that require a swap function to be called
        // explicitly can override this method.
    }

    virtual const mbgl::webgpu::RendererBackend& getBackend() const = 0;
};

} // namespace webgpu
} // namespace mbgl