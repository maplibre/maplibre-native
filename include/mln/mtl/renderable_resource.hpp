#pragma once

#include <mln/gfx/renderable.hpp>
#include <mln/mtl/mtl_fwd.hpp>

#include <cstdlib>

namespace mln {
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

    virtual const mln::mtl::RendererBackend& getBackend() const = 0;
    virtual const mln::mtl::MTLCommandBufferPtr& getCommandBuffer() const = 0;
    virtual mln::mtl::MTLBlitPassDescriptorPtr getUploadPassDescriptor() const = 0;
    virtual const mln::mtl::MTLRenderPassDescriptorPtr& getRenderPassDescriptor() const = 0;
};

} // namespace mtl
} // namespace mln
