#include <mbgl/gfx/drawable_builder.hpp>

#include <mbgl/gfx/drawable_builder_impl.hpp>
#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace gfx {

DrawableBuilder::DrawableBuilder(std::string name_)
    : name(std::move(name_)),
      vertexAttrName("a_pos"),
      colorAttrName("a_color"),
      renderPass(RenderPass::Opaque),
      impl(std::make_unique<Impl>()) {}

DrawableBuilder::~DrawableBuilder() = default;

const Color& DrawableBuilder::getColor() const {
    return impl->currentColor;
}
void DrawableBuilder::setColor(const Color& value) {
    impl->currentColor = value;
}

const UniqueDrawable& DrawableBuilder::getCurrentDrawable(bool createIfNone) {
    if (!currentDrawable && createIfNone) {
        currentDrawable = createDrawable();
    }
    return currentDrawable;
}

void DrawableBuilder::flush() {
    if (!impl->vertices.empty()) {
        const auto& draw = getCurrentDrawable(/*createIfNone=*/true);
        draw->setRenderPass(renderPass);
        draw->setDrawPriority(drawPriority);
        draw->setLayerIndex(layerIndex);
        draw->setDepthType(depthType);
        draw->setShader(shader);
        draw->setMatrix(matrix);
        draw->addTweakers(tweakers.begin(), tweakers.end());

        if (auto drawAttrs = getVertexAttributes().clone()) {
            vertexAttrs.observeAttributes([&](const std::string& iName, const VertexAttribute& iAttr) {
                if (auto& drawAttr = drawAttrs->getOrAdd(iName)) {
                    if (iAttr.getCount() == 1) {
                        // Apply the value to all vertexes
                        for (std::size_t i = 0; i < impl->vertices.elements(); ++i) {
                            drawAttr->setVariant(i, iAttr.get(0));
                        }
                    } else if (iAttr.getCount() == impl->vertices.elements()) {
                        for (std::size_t i = 0; i < impl->vertices.elements(); ++i) {
                            drawAttr->setVariant(i, iAttr.get(i));
                        }
                    } else {
                        // throw?
                        Log::Warning(Event::General, "Invalid attribute count");
                    }
                }
            });

            draw->setVertexAttributes(std::move(*drawAttrs));
        }

        init();
    }
    if (currentDrawable) {
        drawables.emplace_back(std::move(currentDrawable));
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
    impl->triangleIndexes.emplace_back(n, n + 1, n + 2);
    if (colorMode == ColorMode::PerVertex) {
        impl->colors.insert(impl->colors.end(), 3, impl->currentColor);
    }
}

void DrawableBuilder::appendTriangle(int16_t x0, int16_t y0) {
    const auto n = (uint16_t)impl->vertices.elements();
    impl->vertices.emplace_back(Impl::VT({{{x0, y0}}}));
    impl->triangleIndexes.emplace_back(n - 2, n - 1, n);
    if (colorMode == ColorMode::PerVertex) {
        impl->colors.emplace_back(impl->currentColor);
    }
}

void DrawableBuilder::addQuad(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    addTriangle(x0, y0, x1, y0, x0, y1);
    appendTriangle(x1, y1);
}

void DrawableBuilder::setVertexAttributes(const VertexAttributeArray& attrs) {
    vertexAttrs = attrs;
}

void DrawableBuilder::setVertexAttributes(VertexAttributeArray&& attrs) {
    vertexAttrs = attrs;
}

std::size_t DrawableBuilder::addVertices(const std::vector<std::array<int16_t, 2>>& vertices,
                                         std::size_t vertexOffset,
                                         std::size_t vertexLength) {
    const auto baseIndex = impl->vertices.elements();
    std::for_each(std::next(vertices.begin(), vertexOffset),
                  std::next(vertices.begin(), vertexOffset + vertexLength),
                  [&](const std::array<int16_t, 2>& x) { impl->vertices.emplace_back(Impl::VT({{x}})); });
    if (colorMode == ColorMode::PerVertex) {
        for (size_t i = 0; i < vertexLength; ++i) {
            impl->colors.emplace_back(impl->currentColor);
        }
    }
    return baseIndex;
}

void DrawableBuilder::addLines(const std::vector<uint16_t>& indexes,
                               std::size_t indexOffset,
                               std::size_t indexLength,
                               std::size_t baseIndex) {
    for (auto i = std::next(indexes.begin(), indexOffset);
         i != std::next(indexes.begin(), indexOffset + indexLength);) {
        impl->lineIndexes.emplace_back(static_cast<uint16_t>(*i++ + baseIndex),
                                       static_cast<uint16_t>(*i++ + baseIndex));
    }
}

void DrawableBuilder::addTriangles(const std::vector<uint16_t>& indexes,
                                   std::size_t indexOffset,
                                   std::size_t indexLength,
                                   std::size_t baseIndex) {
    for (auto i = std::next(indexes.begin(), indexOffset);
         i != std::next(indexes.begin(), indexOffset + indexLength);) {
        impl->triangleIndexes.emplace_back(static_cast<uint16_t>(*i++ + baseIndex),
                                           static_cast<uint16_t>(*i++ + baseIndex),
                                           static_cast<uint16_t>(*i++ + baseIndex));
    }
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

    const auto baseIndex = addVertices(vertices, vertexOffset, vertexLength);
    addTriangles(indexes, indexOffset, indexLength, baseIndex);
}

} // namespace gfx
} // namespace mbgl
