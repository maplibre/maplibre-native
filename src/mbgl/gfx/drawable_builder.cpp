#include <mbgl/gfx/drawable_builder.hpp>

#include <mbgl/gfx/drawable_builder_impl.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
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

const gfx::CullFaceMode& DrawableBuilder::getCullFaceMode() const {
    return impl->cullFaceMode;
}
void DrawableBuilder::setCullFaceMode(const gfx::CullFaceMode& value) {
    impl->cullFaceMode = value;
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
        draw->setLineWidth(static_cast<int32_t>(lineWidth));
        draw->setNeedsStencil(needsStencil);
        draw->setRenderPass(renderPass);
        draw->setDrawPriority(drawPriority);
        draw->setSubLayerIndex(subLayerIndex);
        draw->setDepthType(depthType);
        draw->setCullFaceMode(impl->cullFaceMode);
        draw->setShader(shader);
        draw->setTextures(textures);
        draw->setTextureSources(textureSources);

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

void DrawableBuilder::setTexture(const std::shared_ptr<gfx::Texture2D>& texture, int32_t location) {
    for (auto& tex : textures) {
        if (tex.location == location) {
            tex.texture = texture;
            return;
        }
    }
    textures.emplace_back(texture, location);
}

void DrawableBuilder::setTextureSource(int32_t location, TexSourceFunc source) {
    textureSources.resize(std::max(textureSources.size(), static_cast<size_t>(location + 1)));
    textureSources[location] = std::move(source);
}

void DrawableBuilder::addTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    const auto n = static_cast<uint16_t>(impl->vertices.elements());
    impl->vertices.emplace_back(Impl::VT({{{x0, y0}}}));
    impl->vertices.emplace_back(Impl::VT({{{x1, y1}}}));
    impl->vertices.emplace_back(Impl::VT({{{x2, y2}}}));
    impl->indexes.insert(impl->indexes.end(), {n, static_cast<uint16_t>(n + 1), static_cast<uint16_t>(n + 2)});
    if (colorAttrMode == ColorAttrMode::PerVertex) {
        impl->colors.insert(impl->colors.end(), 3, impl->currentColor);
    }

    if (impl->segments.empty()) {
        impl->segments.emplace_back(createSegment(gfx::Triangles(), {0, 0}));
    }
    auto& segment = impl->segments.back();
    segment->getSegment().vertexLength += 3;
    segment->getSegment().indexLength += 3;
}

void DrawableBuilder::appendTriangle(int16_t x0, int16_t y0) {
    const auto n = (uint16_t)impl->vertices.elements();
    impl->vertices.emplace_back(Impl::VT({{{x0, y0}}}));
    impl->indexes.insert(impl->indexes.end(), {static_cast<uint16_t>(n - 2), static_cast<uint16_t>(n - 1), n});
    if (colorAttrMode == ColorAttrMode::PerVertex) {
        impl->colors.emplace_back(impl->currentColor);
    }

    assert(!impl->segments.empty());
    auto& segment = impl->segments.back();
    segment->getSegment().vertexLength += 1;
    segment->getSegment().indexLength += 3;
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
    if (colorAttrMode == ColorAttrMode::PerVertex) {
        for (size_t i = 0; i < vertexLength; ++i) {
            impl->colors.emplace_back(impl->currentColor);
        }
    }
    return baseIndex;
}

void DrawableBuilder::setSegments(gfx::DrawMode mode,
                                  std::vector<uint16_t> indexes,
                                  const std::vector<Segment<void>>& segments) {
    impl->indexes = std::move(indexes);
    for (const auto& seg : segments) {
#if !defined(NDEBUG)
        if (mode.type == DrawModeType::Triangles) {
            assert(seg.indexLength % 3 == 0);
        } else if (mode.type == DrawModeType::Lines) {
            assert(seg.indexLength % 2 == 0);
        }
        assert(seg.vertexOffset + seg.vertexLength <= impl->vertices.elements());
        assert(seg.indexOffset + seg.indexLength <= impl->indexes.size());
        for (decltype(seg.indexLength) i = 0; i < seg.indexLength; ++i) {
            assert(impl->indexes[seg.indexOffset + i] < impl->vertices.elements());
        }
#endif

        auto segCopy = Segment<void>{
            // no copy constructor
            seg.vertexOffset,
            seg.indexOffset,
            seg.vertexLength,
            seg.indexLength,
            seg.sortKey,
        };
        impl->segments.emplace_back(createSegment(mode, std::move(segCopy)));
    }
}

void DrawableBuilder::addLines(const std::vector<uint16_t>& indexes,
                               std::size_t indexOffset,
                               std::size_t indexLength,
                               std::size_t baseIndex) {
    if (!indexLength) {
        return;
    }

    impl->segments.emplace_back(createSegment(Lines(lineWidth),
                                              Segment<void>{/*.vertexOffset = */ 0,
                                                            /*.indexOffset = */ indexes.size(),
                                                            /*.vertexLength = */ 0,
                                                            /*.indexLength = */ indexLength}));

    if (impl->indexes.empty()) {
        impl->indexes.reserve(indexLength);
    }

    for (auto i = std::next(indexes.begin(), indexOffset); i != std::next(indexes.begin(), indexOffset + indexLength);
         ++i) {
        impl->indexes.emplace_back(static_cast<uint16_t>(*i + baseIndex));
    }
}

void DrawableBuilder::addTriangles(const std::vector<uint16_t>& indexes,
                                   std::size_t indexOffset,
                                   std::size_t indexLength,
                                   std::size_t baseIndex) {
    if (!indexLength) {
        return;
    }

    impl->segments.emplace_back(createSegment(Triangles(),
                                              Segment<void>{/*.vertexOffset = */ 0,
                                                            /*.indexOffset = */ impl->indexes.size(),
                                                            /*.vertexLength = */ impl->vertices.elements(),
                                                            /*.indexLength = */ indexLength}));

    if (impl->indexes.empty()) {
        impl->indexes.reserve(indexLength);
    }

    for (auto i = std::next(indexes.begin(), indexOffset); i != std::next(indexes.begin(), indexOffset + indexLength);
         ++i) {
        impl->indexes.emplace_back(static_cast<uint16_t>(*i + baseIndex));
    }
}

} // namespace gfx
} // namespace mbgl
