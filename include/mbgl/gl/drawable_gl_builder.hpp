#pragma once

#include <mbgl/gfx/drawable_builder.hpp>

namespace mbgl {
namespace gl {

/**
    Base class for OpenGL-specific drawable builders.
 */
class DrawableGLBuilder : public gfx::DrawableBuilder {
public:
    DrawableGLBuilder() {
    }
    virtual ~DrawableGLBuilder() = default;

protected:
    virtual gfx::DrawablePtr createDrawable() const override;
};

} // namespace gl
} // namespace mbgl
