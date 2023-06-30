#pragma once

#include <mbgl/gfx/drawable_tweaker.hpp>

namespace mbgl {

namespace gfx {
class Drawable;
}

namespace gl {

/**
    OpenGL-specific drawable tweaker
 */
class DrawableGLTweaker : public gfx::DrawableTweaker {
public:
    DrawableGLTweaker() = default;
    ~DrawableGLTweaker() override = default;
};

} // namespace gl
} // namespace mbgl
