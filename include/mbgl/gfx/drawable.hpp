#pragma once

#include <mbgl/util/identity.hpp>

#include <memory>

namespace mbgl {

class PaintParameters;

namespace gfx {

using DrawPriority = int64_t;

class Drawable {
protected:
    Drawable() {
    }

public:
    virtual ~Drawable() = default;

    virtual void draw(const PaintParameters &) const = 0;

    const util::SimpleIdentity& getId() const { return uniqueID; }

    DrawPriority getDrawPriority() const { return drawPriority; }
    void setDrawPriority(DrawPriority value) { drawPriority = value; }

protected:
    util::SimpleIdentity uniqueID;
    DrawPriority drawPriority = 0;
};

using DrawablePtr = std::shared_ptr<Drawable>;

} // namespace gfx
} // namespace mbgl
