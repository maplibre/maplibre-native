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

    virtual void execute(Drawable&, const PaintParameters&) = 0;
};

using DrawableTweakerPtr = std::shared_ptr<DrawableTweaker>;

} // namespace gfx
} // namespace mbgl
