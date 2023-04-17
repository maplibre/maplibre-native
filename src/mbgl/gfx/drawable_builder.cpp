#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/drawable_builder_impl.hpp>

namespace mbgl {
namespace gfx {

DrawableBuilder::DrawableBuilder()
    : impl(std::make_unique<Impl>()) {
}

DrawableBuilder::~DrawableBuilder() = default;

const Color& DrawableBuilder::getColor() const {
    return impl->currentColor;
}
void DrawableBuilder::setColor(const Color& value) {
    impl->currentColor = value;
}

DrawablePtr DrawableBuilder::getCurrentDrawable(bool createIfNone) {
    if (!currentDrawable && createIfNone) {
        currentDrawable = createDrawable();
    }
    return currentDrawable;
}

void DrawableBuilder::flush() {
    if (!impl->vertices.empty()) {
        auto draw = getCurrentDrawable(/*create=*/true);
        currentDrawable->setDrawPriority(drawPriority);
        currentDrawable->setShaderID(shaderID);
        currentDrawable->setVertexAttributes(getVertexAttributes());
        currentDrawable->addTweakers(tweakers.begin(), tweakers.end());
        init();
    }
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

void DrawableBuilder::addTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    const auto n = (uint16_t)impl->vertices.elements();
    impl->vertices.emplace_back(Impl::VT({{{ x0, y0 }}}));
    impl->vertices.emplace_back(Impl::VT({{{ x1, y1 }}}));
    impl->vertices.emplace_back(Impl::VT({{{ x2, y2 }}}));
    impl->indexes.emplace_back(n, n+1, n+2);
    impl->colors.insert(impl->colors.end(), 3, impl->currentColor);
}

void DrawableBuilder::appendTriangle(int16_t x0, int16_t y0) {
    const auto n = (uint16_t)impl->vertices.elements();
    impl->vertices.emplace_back(Impl::VT({{{ x0, y0 }}}));
    impl->indexes.emplace_back(n-1, n, n+1);
    impl->colors.emplace_back(impl->currentColor);
}

void DrawableBuilder::addQuad(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    addTriangle(x0, y0, x1, y0, x0, y1);
    appendTriangle(x1, y1);
}

} // namespace gfx
} // namespace mbgl
