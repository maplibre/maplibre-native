#include <mbgl/gfx/drawable_builder.hpp>

namespace mbgl {
namespace gfx {

DrawablePtr DrawableBuilder::getCurrentDrawable(bool createIfNone) {
    if (!currentDrawable && createIfNone) {
        currentDrawable = createDrawable();
        currentDrawable->setDrawPriority(drawPriority);
    }
    return currentDrawable;
}

void DrawableBuilder::flush() {
    if (currentDrawable) {
        drawables.push_back(currentDrawable);
        currentDrawable.reset();
    }
}

util::SimpleIdentity DrawableBuilder::getDrawableId() {
    return currentDrawable ? currentDrawable->getId() : util::SimpleIdentity::Empty;
}

DrawPriority DrawableBuilder::getDrawPriority() const {
    return drawPriority;
}

void DrawableBuilder::setDrawPriority(DrawPriority value) {
    drawPriority = value;
    if (currentDrawable) {
        currentDrawable->setDrawPriority(value);
    }
}

void DrawableBuilder::resetDrawPriority(DrawPriority value) {
    setDrawPriority(value);
    for (auto &drawble : drawables) {
        drawble->setDrawPriority(value);
    }
}

} // namespace gfx
} // namespace mbgl
