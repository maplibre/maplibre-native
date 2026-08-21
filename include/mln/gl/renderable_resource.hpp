#pragma once

#include <mln/gfx/renderable.hpp>

#include <cstdlib>

namespace mln {
namespace gl {

class RenderableResource : public gfx::RenderableResource {
protected:
    explicit RenderableResource() = default;

public:
    virtual void swap() {
        // Renderable resources that require a swap function to be called
        // explicitly can override this method.
    }
};

} // namespace gl
} // namespace mln
