#pragma once

#include <memory>

namespace mbgl {

class PaintParameters;

namespace gfx {

class Drawable;

/**
    Base class for drawable tweakers, which manipulate drawables per frame
 */
class DrawableTweaker {
protected:
    DrawableTweaker() = default;

public:
    virtual ~DrawableTweaker() = default;

    /// Called as the drawable gets added to the layer
    virtual void init(Drawable&) = 0;

    /// Called just before rendering
    virtual void execute(Drawable&, PaintParameters&) = 0;
};

using DrawableTweakerPtr = std::shared_ptr<DrawableTweaker>;

} // namespace gfx
} // namespace mbgl
