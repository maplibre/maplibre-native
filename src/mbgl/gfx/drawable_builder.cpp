#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/drawable_builder_impl.hpp>
#include <mbgl/renderer/render_pass.hpp>

namespace mbgl {
namespace gfx {

DrawableBuilder::DrawableBuilder(std::string name_)
    : name(std::move(name_)),
      renderPass(RenderPass::Opaque),
      impl(std::make_unique<Impl>()) {}

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
        auto draw = getCurrentDrawable(/*createIfNone=*/true);
        currentDrawable->setRenderPass(renderPass);
        currentDrawable->setDrawPriority(drawPriority);
        currentDrawable->setLayerIndex(layerIndex);
        currentDrawable->setDepthType(depthType);
        currentDrawable->setShader(shader);
        currentDrawable->setMatrix(matrix);
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
    for (auto& drawble : drawables) {
        drawble->setDrawPriority(value);
    }
}

void DrawableBuilder::addTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    const auto n = (uint16_t)impl->vertices.elements();
    impl->vertices.emplace_back(Impl::VT({{{x0, y0}}}));
    impl->vertices.emplace_back(Impl::VT({{{x1, y1}}}));
    impl->vertices.emplace_back(Impl::VT({{{x2, y2}}}));
    impl->indexes.emplace_back(n, n + 1, n + 2);
    if (colorMode == ColorMode::PerVertex) {
        impl->colors.insert(impl->colors.end(), 3, impl->currentColor);
    }
}

void DrawableBuilder::appendTriangle(int16_t x0, int16_t y0) {
    const auto n = (uint16_t)impl->vertices.elements();
    impl->vertices.emplace_back(Impl::VT({{{x0, y0}}}));
    impl->indexes.emplace_back(n - 2, n - 1, n);
    if (colorMode == ColorMode::PerVertex) {
        impl->colors.emplace_back(impl->currentColor);
    }
}

void DrawableBuilder::addQuad(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    addTriangle(x0, y0, x1, y0, x0, y1);
    appendTriangle(x1, y1);
}

void DrawableBuilder::addTriangles(const std::vector<std::array<int16_t, 2>>& vertices,
                                   std::size_t vertexOffset,
                                   std::size_t vertexLength,
                                   const std::vector<uint16_t>& indexes,
                                   std::size_t indexOffset,
                                   std::size_t indexLength) {
    // TODO: bulk add in `gfx::VertexVector`... or use vector directly?
    // impl->vertices.insert(impl->vertices.end(),
    //                      std::next(vertices.begin(), vertexOffset),
    //                      std::next(vertices.begin(), vertexOffset + vertexLength));
    // impl->indexes.insert(impl->indexes.end(),
    //                     std::next(indexes.begin(), indexOffset),
    //                     std::next(indexes.begin(), indexOffset + indexLength));

    const auto baseIndex = impl->vertices.elements();
    std::for_each(std::next(vertices.begin(), vertexOffset),
                  std::next(vertices.begin(), vertexOffset + vertexLength),
                  [&](const std::array<int16_t, 2>& x) { impl->vertices.emplace_back(Impl::VT({{x}})); });
    for (auto i = std::next(indexes.begin(), indexOffset);
         i != std::next(indexes.begin(), indexOffset + indexLength);) {
        impl->indexes.emplace_back(static_cast<uint16_t>(*i++ + baseIndex),
                                   static_cast<uint16_t>(*i++ + baseIndex),
                                   static_cast<uint16_t>(*i++ + baseIndex));
    }
    if (colorMode == ColorMode::PerVertex) {
        for (size_t i = 0; i < vertexLength; ++i) {
            impl->colors.emplace_back(impl->currentColor);
        }
    }
}

} // namespace gfx
} // namespace mbgl
