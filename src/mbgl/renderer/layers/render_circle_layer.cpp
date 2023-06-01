#include <mbgl/renderer/layers/render_circle_layer.hpp>
#include <mbgl/renderer/layers/circle_layer_tweaker.hpp>
#include <mbgl/renderer/buckets/circle_bucket.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/programs/circle_program.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/style/layers/circle_layer_impl.hpp>
#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/intersection_tests.hpp>

namespace mbgl {

using namespace style;

namespace {

struct RenderableSegment {
    RenderableSegment(const Segment<CircleAttributes>& segment_,
                      const RenderTile& tile_,
                      const LayerRenderData* renderData_,
                      float sortKey_)
        : segment(segment_),
          tile(tile_),
          renderData(renderData_),
          sortKey(sortKey_) {}

    const Segment<CircleAttributes>& segment;
    const RenderTile& tile;
    const LayerRenderData* renderData;
    const float sortKey;

    friend bool operator<(const RenderableSegment& lhs, const RenderableSegment& rhs) {
        if (lhs.sortKey == rhs.sortKey) return lhs.tile.id < rhs.tile.id;
        return lhs.sortKey < rhs.sortKey;
    }
};

inline const style::CircleLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == CircleLayer::Impl::staticTypeInfo());
    return static_cast<const style::CircleLayer::Impl&>(*impl);
}

} // namespace

RenderCircleLayer::RenderCircleLayer(Immutable<style::CircleLayer::Impl> _impl)
    : RenderLayer(makeMutable<CircleLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {}

void RenderCircleLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
}

void RenderCircleLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto properties = makeMutable<CircleLayerProperties>(staticImmutableCast<CircleLayer::Impl>(baseImpl),
                                                         unevaluated.evaluate(parameters));
    const auto& evaluated = properties->evaluated;

    passes = ((evaluated.get<style::CircleRadius>().constantOr(1) > 0 ||
               evaluated.get<style::CircleStrokeWidth>().constantOr(1) > 0) &&
              (evaluated.get<style::CircleColor>().constantOr(Color::black()).a > 0 ||
               evaluated.get<style::CircleStrokeColor>().constantOr(Color::black()).a > 0) &&
              (evaluated.get<style::CircleOpacity>().constantOr(1) > 0 ||
               evaluated.get<style::CircleStrokeOpacity>().constantOr(1) > 0))
                 ? RenderPass::Translucent
                 : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);
    if (tileLayerGroup) {
        tileLayerGroup->setLayerTweaker(std::make_shared<CircleLayerTweaker>(evaluatedProperties));
    }
}

bool RenderCircleLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderCircleLayer::hasCrossfade() const {
    return false;
}

void RenderCircleLayer::render(PaintParameters& parameters) {
    assert(renderTiles);

    if (parameters.pass == RenderPass::Opaque) {
        return;
    }

    if (!parameters.shaders.populate(circleProgram)) return;

    const auto drawTile = [&](const RenderTile& tile, const LayerRenderData* data, const auto& segments) {
        auto& circleBucket = static_cast<CircleBucket&>(*data->bucket);
        const auto& evaluated = getEvaluated<CircleLayerProperties>(data->layerProperties);
        const bool scaleWithMap = evaluated.template get<CirclePitchScale>() == CirclePitchScaleType::Map;
        const bool pitchWithMap = evaluated.template get<CirclePitchAlignment>() == AlignmentType::Map;
        const auto& paintPropertyBinders = circleBucket.paintPropertyBinders.at(getID());

        using LayoutUniformValues = CircleProgram::LayoutUniformValues;
        const auto& allUniformValues = CircleProgram::computeAllUniformValues(
            LayoutUniformValues(
                uniforms::matrix::Value(tile.translatedMatrix(evaluated.template get<CircleTranslate>(),
                                                              evaluated.template get<CircleTranslateAnchor>(),
                                                              parameters.state)),
                uniforms::scale_with_map::Value(scaleWithMap),
                uniforms::extrude_scale::Value(
                    pitchWithMap ? std::array<float, 2>{{tile.id.pixelsToTileUnits(
                                                             1.0f, static_cast<float>(parameters.state.getZoom())),
                                                         tile.id.pixelsToTileUnits(
                                                             1.0f, static_cast<float>(parameters.state.getZoom()))}}
                                 : parameters.pixelsToGLUnits),
                uniforms::device_pixel_ratio::Value(parameters.pixelRatio),
                uniforms::camera_to_center_distance::Value(parameters.state.getCameraToCenterDistance()),
                uniforms::pitch_with_map::Value(pitchWithMap)),
            paintPropertyBinders,
            evaluated,
            static_cast<float>(parameters.state.getZoom()));
        const auto& allAttributeBindings = CircleProgram::computeAllAttributeBindings(
            *circleBucket.vertexBuffer, paintPropertyBinders, evaluated);

        checkRenderability(parameters, CircleProgram::activeBindingCount(allAttributeBindings));

        circleProgram->draw(parameters.context,
                            *parameters.renderPass,
                            gfx::Triangles(),
                            parameters.depthModeForSublayer(0, gfx::DepthMaskType::ReadOnly),
                            gfx::StencilMode::disabled(),
                            parameters.colorModeForRenderPass(),
                            gfx::CullFaceMode::disabled(),
                            *circleBucket.indexBuffer,
                            segments,
                            allUniformValues,
                            allAttributeBindings,
                            CircleProgram::TextureBindings{},
                            getID());
    };

    const bool sortFeaturesByKey = !impl_cast(baseImpl).layout.get<CircleSortKey>().isUndefined();
    std::multiset<RenderableSegment> renderableSegments;

    for (const RenderTile& renderTile : *renderTiles) {
        const LayerRenderData* renderData = getRenderDataForPass(renderTile, parameters.pass);
        if (!renderData) {
            continue;
        }
        auto& bucket = static_cast<CircleBucket&>(*renderData->bucket);
        if (!sortFeaturesByKey) {
            drawTile(renderTile, renderData, bucket.segments);
            continue;
        }
        for (auto& segment : bucket.segments) {
            renderableSegments.emplace(segment, renderTile, renderData, segment.sortKey);
        }
    }

    if (sortFeaturesByKey) {
        for (const auto& renderable : renderableSegments) {
            drawTile(renderable.tile, renderable.renderData, renderable.segment);
        }
    }
}

GeometryCoordinate projectPoint(const GeometryCoordinate& p, const mat4& posMatrix, const Size& size) {
    vec4 pos = {{static_cast<double>(p.x), static_cast<double>(p.y), 0, 1}};
    matrix::transformMat4(pos, pos, posMatrix);
    return {static_cast<int16_t>((static_cast<float>(pos[0] / pos[3]) + 1) * size.width * 0.5),
            static_cast<int16_t>((static_cast<float>(pos[1] / pos[3]) + 1) * size.height * 0.5)};
}

GeometryCoordinates projectQueryGeometry(const GeometryCoordinates& queryGeometry,
                                         const mat4& posMatrix,
                                         const Size& size) {
    GeometryCoordinates projectedGeometry;
    for (auto& p : queryGeometry) {
        projectedGeometry.push_back(projectPoint(p, posMatrix, size));
    }
    return projectedGeometry;
}

bool RenderCircleLayer::queryIntersectsFeature(const GeometryCoordinates& queryGeometry,
                                               const GeometryTileFeature& feature,
                                               const float zoom,
                                               const TransformState& transformState,
                                               const float pixelsToTileUnits,
                                               const mat4& posMatrix,
                                               const FeatureState& featureState) const {
    const auto& evaluated = static_cast<const CircleLayerProperties&>(*evaluatedProperties).evaluated;
    // Translate query geometry
    const GeometryCoordinates& translatedQueryGeometry = FeatureIndex::translateQueryGeometry(
                                                             queryGeometry,
                                                             evaluated.get<style::CircleTranslate>(),
                                                             evaluated.get<style::CircleTranslateAnchor>(),
                                                             static_cast<float>(transformState.getBearing()),
                                                             pixelsToTileUnits)
                                                             .value_or(queryGeometry);

    // Evaluate functions
    auto radius = evaluated.evaluate<style::CircleRadius>(zoom, feature, featureState);
    auto stroke = evaluated.evaluate<style::CircleStrokeWidth>(zoom, feature, featureState);
    auto size = radius + stroke;

    // For pitch-alignment: map, compare feature geometry to query geometry in
    // the plane of the tile Otherwise, compare geometry in the plane of the
    // viewport A circle with fixed scaling relative to the viewport gets larger
    // in tile space as it moves into the distance A circle with fixed scaling
    // relative to the map gets smaller in viewport space as it moves into the
    // distance
    bool alignWithMap = evaluated.evaluate<style::CirclePitchAlignment>(zoom, feature) == AlignmentType::Map;
    const GeometryCoordinates& transformedQueryGeometry = alignWithMap ? translatedQueryGeometry
                                                                       : projectQueryGeometry(translatedQueryGeometry,
                                                                                              posMatrix,
                                                                                              transformState.getSize());
    auto transformedSize = alignWithMap ? size * pixelsToTileUnits : size;

    const auto& geometry = feature.getGeometries();
    for (auto& ring : geometry) {
        for (auto& point : ring) {
            const GeometryCoordinate& transformedPoint = alignWithMap
                                                             ? point
                                                             : projectPoint(point, posMatrix, transformState.getSize());

            float adjustedSize = transformedSize;
            vec4 center = {{static_cast<double>(point.x), static_cast<double>(point.y), 0, 1}};
            matrix::transformMat4(center, center, posMatrix);
            auto pitchScale = evaluated.evaluate<style::CirclePitchScale>(zoom, feature);
            auto pitchAlignment = evaluated.evaluate<style::CirclePitchAlignment>(zoom, feature);
            if (pitchScale == CirclePitchScaleType::Viewport && pitchAlignment == AlignmentType::Map) {
                adjustedSize *= static_cast<float>(center[3] / transformState.getCameraToCenterDistance());
            } else if (pitchScale == CirclePitchScaleType::Map && pitchAlignment == AlignmentType::Viewport) {
                adjustedSize *= static_cast<float>(transformState.getCameraToCenterDistance() / center[3]);
            }

            if (util::polygonIntersectsBufferedPoint(transformedQueryGeometry, transformedPoint, adjustedSize))
                return true;
        }
    }

    return false;
}

void RenderCircleLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    // Remove everything
    if (tileLayerGroup) {
        changes.emplace_back(std::make_unique<RemoveLayerGroupRequest>(tileLayerGroup->getLayerIndex()));
        tileLayerGroup.reset();
    }
}

void RenderCircleLayer::removeTile(RenderPass renderPass, const OverscaledTileID& tileID) {
    stats.tileDrawablesRemoved += tileLayerGroup->removeDrawables(renderPass, tileID).size();
}

void RenderCircleLayer::update(gfx::ShaderRegistry& shaders,
                               gfx::Context& context,
                               const TransformState& /*state*/,
                               [[maybe_unused]] const RenderTree& renderTree,
                               UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

    if (!renderTiles || renderTiles->empty()) {
        if (tileLayerGroup) {
            stats.tileDrawablesRemoved += tileLayerGroup->getDrawableCount();
            tileLayerGroup->clearDrawables();
        }
        return;
    }

    // Set up a layer group
    if (!tileLayerGroup) {
        tileLayerGroup = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64);
        if (!tileLayerGroup) {
            return;
        }
        tileLayerGroup->setLayerTweaker(std::make_shared<CircleLayerTweaker>(evaluatedProperties));
        changes.emplace_back(std::make_unique<AddLayerGroupRequest>(tileLayerGroup, /*canReplace=*/true));
    }

    if (!circleShader) {
        circleShader = context.getGenericShader(shaders, "CircleShader");
    }

    std::unordered_set<OverscaledTileID> newTileIDs(renderTiles->size());
    std::transform(renderTiles->begin(),
                   renderTiles->end(),
                   std::inserter(newTileIDs, newTileIDs.begin()),
                   [](const auto& renderTile) -> OverscaledTileID { return renderTile.get().getOverscaledTileID(); });

    std::unique_ptr<gfx::DrawableBuilder> circleBuilder;
    std::vector<gfx::DrawablePtr> newTiles;
    gfx::VertexAttributeArray circleVertexAttrs;
    auto renderPass = RenderPass::Translucent;

    if (!(mbgl::underlying_type(renderPass) & evaluatedProperties->renderPasses)) {
        return;
    }

    tileLayerGroup->observeDrawables([&](gfx::UniqueDrawable& drawable) {
        const auto tileID = drawable->getTileID();
        if (tileID && newTileIDs.find(*tileID) == newTileIDs.end()) {
            // remove it
            drawable.reset();
            ++stats.tileDrawablesRemoved;
        }
    });

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        if (tileLayerGroup->getDrawableCount(renderPass, tileID) > 0) {
            continue;
        }

        const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
        if (!renderData) {
            removeTile(renderPass, tileID);
            continue;
        }

        auto& bucket = static_cast<CircleBucket&>(*renderData->bucket);

        std::vector<std::array<int16_t, 2>> rawVerts;
        const auto buildVertices = [&]() {
            const std::vector<gfx::VertexVector<gfx::detail::VertexType<gfx::AttributeType<int16_t, 2>>>::Vertex>&
                verts = bucket.vertices.vector();
            if (rawVerts.size() < verts.size()) {
                rawVerts.resize(verts.size());
                std::transform(verts.begin(), verts.end(), rawVerts.begin(), [](const auto& x) { return x.a1; });
            }
        };

        circleVertexAttrs.clear();

        const auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());

        if (auto& binder = paintPropertyBinders.get<CircleColor>()) {
            const auto count = binder->getVertexCount();
            if (auto& attr = circleVertexAttrs.getOrAdd("a_color")) {
                for (std::size_t i = 0; i < count; ++i) {
                    const auto& packedColor = static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 2>>*>(
                                                  binder->getVertexValue(i))
                                                  ->a1;
                    attr->set<gfx::VertexAttribute::float4>(
                        i, {packedColor[0], packedColor[1], packedColor[0], packedColor[1]});
                }
            }
        }
        if (auto& binder = paintPropertyBinders.get<CircleRadius>()) {
            const auto count = binder->getVertexCount();
            if (auto& attr = circleVertexAttrs.getOrAdd("a_radius")) {
                for (std::size_t i = 0; i < count; ++i) {
                    const auto& radius = static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 1>>*>(
                                             binder->getVertexValue(i))
                                             ->a1;
                    attr->set<gfx::VertexAttribute::float2>(i, {radius[0], radius[0]});
                }
            }
        }
        if (auto& binder = paintPropertyBinders.get<CircleBlur>()) {
            const auto count = binder->getVertexCount();
            if (auto& attr = circleVertexAttrs.getOrAdd("a_blur")) {
                for (std::size_t i = 0; i < count; ++i) {
                    const auto& blur = static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 1>>*>(
                                           binder->getVertexValue(i))
                                           ->a1;
                    attr->set<gfx::VertexAttribute::float2>(i, {blur[0], blur[0]});
                }
            }
        }
        if (auto& binder = paintPropertyBinders.get<CircleOpacity>()) {
            const auto count = binder->getVertexCount();
            if (auto& attr = circleVertexAttrs.getOrAdd("a_opacity")) {
                for (std::size_t i = 0; i < count; ++i) {
                    const auto& opacity = static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 1>>*>(
                                              binder->getVertexValue(i))
                                              ->a1;
                    attr->set<gfx::VertexAttribute::float2>(i, {opacity[0], opacity[0]});
                }
            }
        }
        if (auto& binder = paintPropertyBinders.get<CircleStrokeColor>()) {
            const auto count = binder->getVertexCount();
            if (auto& attr = circleVertexAttrs.getOrAdd("a_stroke_color")) {
                for (std::size_t i = 0; i < count; ++i) {
                    const auto& packedStrokeColor =
                        static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 2>>*>(
                            binder->getVertexValue(i))
                            ->a1;
                    attr->set<gfx::VertexAttribute::float4>(
                        i, {packedStrokeColor[0], packedStrokeColor[1], packedStrokeColor[0], packedStrokeColor[1]});
                }
            }
        }
        if (auto& binder = paintPropertyBinders.get<CircleStrokeWidth>()) {
            const auto count = binder->getVertexCount();
            if (auto& attr = circleVertexAttrs.getOrAdd("a_stroke_width")) {
                for (std::size_t i = 0; i < count; ++i) {
                    const auto& strokeWidth = static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 1>>*>(
                                                  binder->getVertexValue(i))
                                                  ->a1;
                    attr->set<gfx::VertexAttribute::float2>(i, {strokeWidth[0], strokeWidth[0]});
                }
            }
        }
        if (auto& binder = paintPropertyBinders.get<CircleStrokeOpacity>()) {
            const auto count = binder->getVertexCount();
            if (auto& attr = circleVertexAttrs.getOrAdd("a_stroke_opacity")) {
                for (std::size_t i = 0; i < count; ++i) {
                    const auto& strokeOpacity =
                        static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 1>>*>(
                            binder->getVertexValue(i))
                            ->a1;
                    attr->set<gfx::VertexAttribute::float2>(i, {strokeOpacity[0], strokeOpacity[0]});
                }
            }
        }

        circleBuilder = context.createDrawableBuilder("circle");
        circleBuilder->setShader(circleShader);
        circleBuilder->setColorAttrMode(gfx::DrawableBuilder::ColorAttrMode::None);
        circleBuilder->setDepthType((renderPass == RenderPass::Opaque) ? gfx::DepthMaskType::ReadWrite
                                                                       : gfx::DepthMaskType::ReadOnly);
        circleBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());

        circleBuilder->setRenderPass(renderPass);
        circleBuilder->setVertexAttributes(circleVertexAttrs);

        buildVertices();
        circleBuilder->addVertices(rawVerts, 0, rawVerts.size());

        for (const auto& seg : bucket.segments) {
            circleBuilder->addTriangles(bucket.triangles.vector(), seg.indexOffset, seg.indexLength);
        }

        circleBuilder->flush();

        for (auto& drawable : circleBuilder->clearDrawables()) {
            drawable->setTileID(tileID);

            tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
            ++stats.tileDrawablesAdded;
        }
    }
}

} // namespace mbgl
