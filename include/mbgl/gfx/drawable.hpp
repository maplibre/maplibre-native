#pragma once

#include <mbgl/util/identity.hpp>

#include <memory>
#include <vector>

namespace mbgl {

class PaintParameters;

namespace gfx {

class DrawableTweaker;

using DrawPriority = int64_t;
using DrawableTweakerPtr = std::shared_ptr<DrawableTweaker>;

class Drawable {
protected:
    Drawable() {
    }

public:
    virtual ~Drawable() = default;

    const util::SimpleIdentity& getId() const { return uniqueID; }

    /// Draw the drawable
    virtual void draw(const PaintParameters &) const = 0;

    /// DrawPriority determines the drawing order
    DrawPriority getDrawPriority() const { return drawPriority; }
    void setDrawPriority(DrawPriority value) { drawPriority = value; }

    /// Attach a tweaker to be run on this drawable for each frame
    void addTweaker(DrawableTweakerPtr tweaker) { tweakers.emplace_back(std::move(tweaker)); }
    template <typename TIter>
    void addTweakers(TIter beg, TIter end) { tweakers.insert(tweakers.end(), beg, end); }

    /// Get the tweakers attached to this drawable
    std::vector<DrawableTweakerPtr> getTweakers() const { return tweakers; }

protected:
    util::SimpleIdentity uniqueID;
    DrawPriority drawPriority = 0;
    std::vector<DrawableTweakerPtr> tweakers;
};

using DrawablePtr = std::shared_ptr<Drawable>;

} // namespace gfx
} // namespace mbgl
