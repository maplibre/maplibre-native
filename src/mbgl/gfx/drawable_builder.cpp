#include <mbgl/gfx/drawable_builder.hpp>

#include <mbgl/gfx/drawable_builder_impl.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/gfx/gfx_types.hpp>

namespace mbgl {
namespace gfx {

DrawableBuilder::DrawableBuilder(std::string name_)
    : name(std::move(name_)),
      vertexAttrId(0),
      renderPass(mbgl::RenderPass::Opaque),
      impl(std::make_unique<Impl>()) {}

DrawableBuilder::~DrawableBuilder() = default;

const gfx::ColorMode& DrawableBuilder::getColorMode() const {
    return impl->colorMode;
}
void DrawableBuilder::setColorMode(const gfx::ColorMode& value) {
    impl->colorMode = value;
}

const gfx::CullFaceMode& DrawableBuilder::getCullFaceMode() const {
    return impl->cullFaceMode;
}
void DrawableBuilder::setCullFaceMode(const gfx::CullFaceMode& value) {
    impl->cullFaceMode = value;
}

UniqueDrawable& DrawableBuilder::getCurrentDrawable(bool createIfNone) {
    if (!currentDrawable && createIfNone) {
        currentDrawable = createDrawable();
    }
    return currentDrawable;
}

void DrawableBuilder::flush(gfx::Context& context) {
    Log::Info(Event::General, "DrawableBuilder::flush called for: " + name + ", vertex count: " + std::to_string(curVertexCount()));
    if (curVertexCount()) {
        if (Impl::Mode::Polylines == impl->getMode()) {
            // setup for polylines
            impl->setupForPolylines(context, *this);
        } else if (Impl::Mode::WideVectorLocal == impl->getMode() || Impl::Mode::WideVectorGlobal == impl->getMode()) {
            // setup for wide vectors
            impl->setupForWideVectors(context, *this);
        }

        const auto& draw = getCurrentDrawable(/*createIfNone=*/true);
        draw->setEnabled(enabled);
        draw->setLineWidth(static_cast<int32_t>(lineWidth));
        draw->setEnableColor(enableColor);
        draw->setEnableStencil(enableStencil);
        draw->setRenderPass(renderPass);
        draw->setDrawPriority(drawPriority);
        draw->setSubLayerIndex(subLayerIndex);
        draw->setEnableDepth(enableDepth);
        draw->setDepthType(depthType);
        draw->setIs3D(is3D);
        draw->setColorMode(impl->colorMode);
        draw->setCullFaceMode(impl->cullFaceMode);
        draw->setShader(shader);
        draw->setTextures(textures);
        draw->setTweakers(tweakers);
        draw->setOrigin(origin);

        if (vertexAttrs) {
            draw->setVertexAttributes(vertexAttrs);
        } else {
            draw->setVertexAttributes(context.createVertexAttributeArray());
        }

        if (instanceAttrs) {
            draw->setInstanceAttributes(instanceAttrs);
        }

        init();

        // reset mode
        impl->setMode(Impl::Mode::Custom);

        // reset origin
        origin.reset();
    }
    if (currentDrawable) {
        Log::Info(Event::General, "DrawableBuilder::flush - Adding drawable to drawables vector");
        drawables.emplace_back(std::move(currentDrawable));
        Log::Info(Event::General, "DrawableBuilder::flush - Drawables vector now has " + std::to_string(drawables.size()) + " items");
    } else {
        Log::Warning(Event::General, "DrawableBuilder::flush - currentDrawable is null after init(), not adding to vector");
    }
}

util::SimpleIdentity DrawableBuilder::getDrawableId() {
    return currentDrawable ? currentDrawable->getID() : util::SimpleIdentity::Empty;
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

static const gfx::Texture2DPtr noTexture;

const gfx::Texture2DPtr& DrawableBuilder::getTexture(size_t id) const {
    return (id < textures.size()) ? textures[id] : noTexture;
}

void DrawableBuilder::setTexture(const std::shared_ptr<gfx::Texture2D>& texture, size_t id) {
    assert(id < textures.size());
    if (id >= textures.size()) {
        return;
    }
    textures[id] = std::move(texture);
}

void DrawableBuilder::addTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    // check and set the current mode
    if (!impl->checkAndSetMode(Impl::Mode::Primitives)) return;

    const auto n = static_cast<uint16_t>(impl->vertices.elements());
    impl->vertices.emplace_back(Impl::VT({{{x0, y0}}}));
    impl->vertices.emplace_back(Impl::VT({{{x1, y1}}}));
    impl->vertices.emplace_back(Impl::VT({{{x2, y2}}}));
    impl->buildIndexes.insert(impl->buildIndexes.end(),
                              {n, static_cast<uint16_t>(n + 1), static_cast<uint16_t>(n + 2)});

    if (impl->segments.empty()) {
        impl->segments.emplace_back(createSegment(gfx::Triangles(), {0, 0}));
    }
    auto& segment = impl->segments.back();
    segment->getSegment().vertexLength += 3;
    segment->getSegment().indexLength += 3;
}

void DrawableBuilder::appendTriangle(int16_t x0, int16_t y0) {
    // check and set the current mode
    if (!impl->checkAndSetMode(Impl::Mode::Primitives)) return;

    const auto n = (uint16_t)impl->vertices.elements();
    impl->vertices.emplace_back(Impl::VT({{{x0, y0}}}));
    impl->buildIndexes.insert(impl->buildIndexes.end(),
                              {static_cast<uint16_t>(n - 2), static_cast<uint16_t>(n - 1), n});

    assert(!impl->segments.empty());
    auto& segment = impl->segments.back();
    segment->getSegment().vertexLength += 1;
    segment->getSegment().indexLength += 3;
}

void DrawableBuilder::addQuad(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    // check and set the current mode
    if (!impl->checkAndSetMode(Impl::Mode::Primitives)) return;

    addTriangle(x0, y0, x1, y0, x0, y1);
    appendTriangle(x1, y1);
}

std::size_t DrawableBuilder::addVertices(const std::vector<std::array<int16_t, 2>>& vertices,
                                         std::size_t vertexOffset,
                                         std::size_t vertexLength) {
    Log::Info(Event::General, "DrawableBuilder::addVertices called for: " + name + ", length: " + std::to_string(vertexLength));
    const auto baseIndex = impl->vertices.elements();
    std::for_each(std::next(vertices.begin(), vertexOffset),
                  std::next(vertices.begin(), vertexOffset + vertexLength),
                  [&](const std::array<int16_t, 2>& x) { impl->vertices.emplace_back(Impl::VT({{x}})); });
    return baseIndex;
}

void DrawableBuilder::setRawVertices(std::vector<uint8_t>&& data, std::size_t count, AttributeDataType type) {
    impl->rawVertices = std::move(data);
    impl->rawVerticesCount = count;
    impl->rawVerticesType = type;
}

void DrawableBuilder::setSegments(const gfx::DrawMode mode,
                                  std::vector<uint16_t> indexes,
                                  const SegmentBase* segments,
                                  const std::size_t segmentCount) {
    setSegments(mode, std::make_shared<gfx::IndexVectorBase>(std::move(indexes)), segments, segmentCount);
}

void DrawableBuilder::setSegments(const gfx::DrawMode mode,
                                  gfx::IndexVectorBasePtr indexes,
                                  const SegmentBase* segments,
                                  const std::size_t segmentCount) {
    if (!indexes || !indexes->elements()) {
        assert("Missing segment indexes");
        return;
    }
    impl->sharedIndexes = std::move(indexes);
    for (std::size_t i = 0; i < segmentCount; ++i) {
        const auto& seg = segments[i];
#if !defined(NDEBUG)
        if (mode.type == DrawModeType::Triangles) {
            assert(seg.indexLength % 3 == 0);
        } else if (mode.type == DrawModeType::Lines) {
            assert(seg.indexLength % 2 == 0);
        }
        const auto vertexCount = curVertexCount();
        assert(!vertexCount || seg.vertexOffset + seg.vertexLength <= curVertexCount());
        assert(seg.indexOffset + seg.indexLength <= impl->sharedIndexes->elements());
        if (vertexCount) {
            for (decltype(seg.indexLength) j = 0; j < seg.indexLength; ++j) {
                assert(impl->sharedIndexes->vector()[seg.indexOffset + j] < vertexCount);
            }
        }
#endif

        auto segCopy = SegmentBase{
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
                                              SegmentBase{/*.vertexOffset = */ 0,
                                                          /*.indexOffset = */ indexes.size(),
                                                          /*.vertexLength = */ 0,
                                                          /*.indexLength = */ indexLength}));

    if (impl->buildIndexes.empty()) {
        impl->buildIndexes.reserve(indexLength);
    }

    for (auto i = std::next(indexes.begin(), indexOffset); i != std::next(indexes.begin(), indexOffset + indexLength);
         ++i) {
        impl->buildIndexes.emplace_back(static_cast<uint16_t>(*i + baseIndex));
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
                                              SegmentBase{/*.vertexOffset = */ 0,
                                                          /*.indexOffset = */ impl->buildIndexes.size(),
                                                          /*.vertexLength = */ impl->vertices.elements(),
                                                          /*.indexLength = */ indexLength}));

    if (impl->buildIndexes.empty()) {
        impl->buildIndexes.reserve(indexLength);
    }

    for (auto i = std::next(indexes.begin(), indexOffset); i != std::next(indexes.begin(), indexOffset + indexLength);
         ++i) {
        impl->buildIndexes.emplace_back(static_cast<uint16_t>(*i + baseIndex));
    }
}

std::size_t DrawableBuilder::curVertexCount() const {
    return impl->vertexCount();
}

void DrawableBuilder::addPolyline(const GeometryCoordinates& coordinates,
                                  const gfx::PolylineGeneratorOptions& options) {
    // mark the current mode
    if (!impl->checkAndSetMode(Impl::Mode::Polylines)) return;

    // append polyline
    impl->addPolyline(*this, coordinates, options);
}

void DrawableBuilder::addWideVectorPolylineLocal(const GeometryCoordinates& coordinates,
                                                 const gfx::PolylineGeneratorOptions& options) {
    // mark the current mode
    if (!impl->checkAndSetMode(Impl::Mode::WideVectorLocal)) return;

    // append polyline
    impl->addWideVectorPolylineLocal(*this, coordinates, options);
}

void DrawableBuilder::addWideVectorPolylineGlobal(const LineString<double>& coordinates,
                                                  const gfx::PolylineGeneratorOptions& options) {
    // mark the current mode
    if (!impl->checkAndSetMode(Impl::Mode::WideVectorGlobal)) return;

    // append polyline
    origin = impl->addWideVectorPolylineGlobal(*this, coordinates, options);
}

} // namespace gfx
} // namespace mbgl
