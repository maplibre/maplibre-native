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
    gfx::DrawablePtr createDrawable() const override;
    
    /// Setup the SDK-specific aspects after all the values are present
    void init() override;
};

} // namespace gl
} // namespace mbgl
