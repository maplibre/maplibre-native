#pragma once

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <cstdlib>

namespace mbgl {
namespace mtl {

class RendererBackend;

class RenderableResource : public gfx::RenderableResource {
protected:
    explicit RenderableResource() = default;

public:
    virtual void swap() {
        // Renderable resources that require a swap function to be called
        // explicitly can override this method.
    }

    virtual const mbgl::mtl::RendererBackend& getBackend() const = 0;
    virtual const mbgl::mtl::MTLCommandBufferPtr& getCommandBuffer() const = 0;
    virtual mbgl::mtl::MTLBlitPassDescriptorPtr getUploadPassDescriptor() const = 0;
    virtual mbgl::mtl::MTLRenderPassDescriptorPtr getRenderPassDescriptor() const = 0;
};

} // namespace mtl
} // namespace mbgl
