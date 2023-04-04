#pragma once

//#include <mbgl/gfx/paint_parameters.hpp>  // this is private currently

#include <memory>

namespace mbgl {
namespace gfx {

class Drawable {
protected:
    Drawable() {
    }

public:
    virtual ~Drawable() = default;

    virtual void draw(/*const PaintParameters &*/) const = 0;

protected:
};

using DrawablePtr = std::shared_ptr<Drawable>;

} // namespace gfx
} // namespace mbgl
