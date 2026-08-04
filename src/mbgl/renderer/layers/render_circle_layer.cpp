#include <mbgl/renderer/layers/render_circle_layer.hpp>

#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/renderer/buckets/circle_bucket.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/circle_layer_tweaker.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layers/circle_layer_impl.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/util/intersection_tests.hpp>
#include <mbgl/util/math.hpp>

namespace mbgl {

using namespace style;

namespace {

inline const style::CircleLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == CircleLayer::Impl::staticTypeInfo());
    return static_cast<const style::CircleLayer::Impl&>(*impl);
}

} // namespace

RenderCircleLayer::RenderCircleLayer(Immutable<style::CircleLayer::Impl> _impl)
    : RenderLayer(makeMutable<CircleLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {
    styleDependencies = unevaluated.getDependencies();
}

void RenderCircleLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    styleDependencies = unevaluated.getDependencies();
}

void RenderCircleLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    const auto previousProperties = staticImmutableCast<CircleLayerProperties>(evaluatedProperties);
    auto properties = makeMutable<CircleLayerProperties>(
        staticImmutableCast<CircleLayer::Impl>(baseImpl),
        unevaluated.evaluate(parameters, previousProperties->evaluated));
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

    if (layerTweaker) {
        layerTweaker->updateProperties(evaluatedProperties);
    }
}

bool RenderCircleLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderCircleLayer::hasCrossfade() const {
    return false;
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

namespace {
using namespace vector;
using namespace matrix;

constexpr auto CircleShaderGroupName = "CircleShader";

// The common part of the three variations on circle vertex transform
struct VertexBoundHelper {
    using BucketVertex = decltype(CircleBucket::VertexVector::Vertex::a1);

    VertexBoundHelper(const BucketVertex& vertex_,
                      const mat4& tileMatrix,
                      float radius_,
                      float strokeWidth,
                      const vec2& extrudeScale,
                      bool useProjectedCenter)
        : vertex(vertex_),
          matrix(tileMatrix),
          center{std::trunc(vertex[0] / 2.0), std::trunc(vertex[1] / 2.0)},
          extrude{gl_fmod(vec(vertex[0], vertex[1]), 2.0) * 2 - 1},
          radius{extrude * extrudeScale * (radius_ + strokeWidth)},
          projectedCenter(useProjectedCenter ? matrix * vector::vec(center, 0, 1) : vec4{}) {}

    const BucketVertex& vertex;
    const mat4& matrix;
    const vec2 center;          // circle center (view space)
    const vec2 extrude;         // corner offset (view space)
    const vec2 radius;          // scaled corner offset (view space)
    const vec4 projectedCenter; // projected center (clip space, optional)
};

} // namespace

using namespace shaders;

void RenderCircleLayer::captureRenderedFeatures(const CircleBucket& bucket,
                                                const RenderTile& tile,
                                                const CircleBinders& binders,
                                                const style::CirclePaintProperties::PossiblyEvaluated& evaluated,
                                                const TransformState& state,
                                                const TransformParameters& transformParams) {
    using namespace vector;

    const auto& tileID = tile.getOverscaledTileID();

    const bool alphaIsConstant = evaluated.get<CircleOpacity>().isConstant();
    const bool strokeAlphaIsConstant = evaluated.get<CircleStrokeOpacity>().isConstant();
    const auto& alphaBinder = binders.get<CircleOpacity>();
    const auto& strokeAlphaBinder = binders.get<CircleStrokeOpacity>();
    if (alphaIsConstant && strokeAlphaIsConstant &&
        std::get<0>(alphaBinder->uniformValue(evaluated.get<CircleOpacity>())) == 0 &&
        std::get<0>(strokeAlphaBinder->uniformValue(evaluated.get<CircleStrokeOpacity>())) == 0) {
        return;
    }

    const bool colorIsConstant = evaluated.get<CircleColor>().isConstant();
    const bool strokeColorIsConstant = evaluated.get<CircleStrokeColor>().isConstant();
    const auto& colorBinder = binders.template get<CircleColor>();
    const auto& strokeColorBinder = binders.template get<CircleStrokeColor>();
    if (colorIsConstant && strokeColorIsConstant &&
        std::get<0>(colorBinder->uniformValue(evaluated.get<CircleColor>())).a == 0 &&
        std::get<0>(strokeColorBinder->uniformValue(evaluated.get<CircleStrokeColor>())).a == 0) {
        return;
    }

    constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
    constexpr bool nearClipped = false;
    constexpr bool aligned = false;
    constexpr bool is3d = false;
    constexpr bool enableDepth = true;
    constexpr std::int32_t subLayerIndex = 0;
    constexpr std::optional<mbgl::Point<double>> origin = std::nullopt;

    const auto zoom = state.getZoom();
    const auto zoomFraction = state.getZoomFraction();
    const auto cameraToCenterDistance = state.getCameraToCenterDistance();
    const auto pixelsToTileUnits = tileID.toUnwrapped().pixelsToTileUnits(1.0f, zoom);

    const auto translation = evaluated.get<CircleTranslate>();
    const auto translationAnchor = evaluated.get<CircleTranslateAnchor>();
    const auto radius = constOrDefault<CircleRadius>(evaluated);
    const bool pitchWithMap = evaluated.get<CirclePitchAlignment>() == AlignmentType::Map;
    const bool scaleWithMap = evaluated.get<CirclePitchScale>() == CirclePitchScaleType::Map;
    const auto strokeWidth = constOrDefault<CircleStrokeWidth>(evaluated);
    const auto extrudeScale = pitchWithMap ? vec2{pixelsToTileUnits, pixelsToTileUnits}
                                           : vec2{{2.0f / state.getSize().width, -2.0f / state.getSize().height}};
    const bool preTransformedVertices = !pitchWithMap;

    // Select the appropriate vertex transform function based on the pitch and scale settings.
    // The transform is applied to each vertex of the circle geometry to compute its position
    // in clip space.
    std::function<vec3(const VertexBoundHelper&)> getVertexImpl;
    if (pitchWithMap) {
        if (scaleWithMap) {
            getVertexImpl = [](const VertexBoundHelper& v) {
                return vec(v.center + v.radius, 0);
            };
        } else {
            getVertexImpl = [=](const VertexBoundHelper& v) {
                const auto clipScale = (v.projectedCenter[3] / cameraToCenterDistance);
                return vec(v.center + v.radius * clipScale, 0);
            };
        }
    } else {
        getVertexImpl = [=](const VertexBoundHelper& v) {
            const auto clipScale = scaleWithMap ? cameraToCenterDistance : v.projectedCenter[3];
            const vec4 clipPos = v.projectedCenter + vec(v.radius * clipScale, 0, 0);
            return slice<0, 3>(clipPos) / clipPos[3];
        };
    }

    std::optional<mat4> tileMatrix;

    const auto& features = bucket.getRetainedFeatures();
    stats.renderedFeatures.reserve(features.size());

    for (std::size_t i = 0; i < features.size(); ++i) {
        const auto& featureEntry = features[i];
        const auto& featureID = featureEntry.featureId;
        assert(!featureID.empty());

        // Consider only the value for the first vertex of this feature, for now.
        const auto vertexOffset = featureEntry.vertexOffset;

        if (!alphaIsConstant || !strokeAlphaIsConstant) {
            const auto interpAlpha = alphaIsConstant ? std::array<float, 2>{1, 1}
                                                     : std::get<0>(alphaBinder->getVertexValue(vertexOffset)).a1;
            const auto interpStrokeAlpha = strokeAlphaIsConstant
                                               ? std::array<float, 2>{1, 1}
                                               : std::get<0>(strokeAlphaBinder->getVertexValue(vertexOffset)).a1;
            // const auto interpAlpha = std::get<0>(alphaBinder->getVertexValue(vertexOffset)).a1;
            if (mbgl::util::interpolate(interpAlpha[0], interpAlpha[1], zoomFraction) == 0 &&
                mbgl::util::interpolate(interpStrokeAlpha[0], interpStrokeAlpha[1], zoomFraction) == 0) {
                continue;
            }
        }
        if (!colorIsConstant) {
            const auto alpha = unpack_mix_alpha(std::get<0>(colorBinder->getVertexValue(vertexOffset)).a1,
                                                zoomFraction);
            const auto strokeAlpha = unpack_mix_alpha(std::get<0>(strokeColorBinder->getVertexValue(vertexOffset)).a1,
                                                      zoomFraction);
            if (alpha == 0 && strokeAlpha == 0) {
                continue;
            }
        }

        // Compute the tile matrix once
        if (!tileMatrix.has_value()) {
            tileMatrix = LayerTweaker::getTileMatrix(tileID.toUnwrapped(),
                                                     state,
                                                     transformParams,
                                                     layerIndex,
                                                     translation,
                                                     translationAnchor,
                                                     origin,
                                                     is3d,
                                                     enableDepth,
                                                     subLayerIndex,
                                                     nearClipped,
                                                     inViewportPixelUnits,
                                                     aligned);
        }

        const auto getVertex = [&](std::size_t vi) {
            const auto& vertex = bucket.vertices.at(vertexOffset + vi).a1;
            const bool useProjectedCenter = !pitchWithMap || !scaleWithMap;
            return getVertexImpl({vertex, *tileMatrix, radius, strokeWidth, extrudeScale, useProjectedCenter});
        };
        if (const auto bound = computeFeatureNDCBound(
                featureEntry.vertexCount, *tileMatrix, preTransformedVertices, getVertex)) {
            stats.addRenderedFeature(featureID, *bound, {tileID});
        }
    }
}

void RenderCircleLayer::update(gfx::ShaderRegistry& shaders,
                               gfx::Context& context,
                               const TransformState& transformState,
                               const std::shared_ptr<UpdateParameters>& updateParameters,
                               const RenderTree& renderTree,
                               UniqueChangeRequestVec& changes) {
    stats.renderedFeatures.clear();

    if (!renderTiles || renderTiles->empty()) {
        removeAllDrawables();
        return;
    }

    // Set up a layer group
    if (!layerGroup) {
        if (auto layerGroup_ = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID())) {
            setLayerGroup(std::move(layerGroup_), changes);
        } else {
            return;
        }
    }
    auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());
    if (!layerTweaker) {
        layerTweaker = std::make_shared<CircleLayerTweaker>(getID(), evaluatedProperties);
        layerGroup->addLayerTweaker(layerTweaker);
    }

    if (!circleShaderGroup) {
        circleShaderGroup = shaders.getShaderGroup(CircleShaderGroupName);
    }
    if (!circleShaderGroup) {
        removeAllDrawables();
        return;
    }

    std::unique_ptr<gfx::DrawableBuilder> circleBuilder;
    constexpr auto renderPass = RenderPass::Translucent;

    if (!(mbgl::underlying_type(renderPass) & evaluatedProperties->renderPasses)) {
        return;
    }

    stats.drawablesRemoved += tileLayerGroup->removeDrawablesIf(
        [&](gfx::Drawable& drawable) { return drawable.getTileID() && !hasRenderTile(*drawable.getTileID()); });

    const auto& evaluated = static_cast<const CircleLayerProperties&>(*evaluatedProperties).evaluated;
    StringIDSetsPair propertiesAsUniforms;

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
        if (!renderData || !renderData->bucket || !renderData->bucket->hasData()) {
            removeTile(renderPass, tileID);
            continue;
        }

        auto& bucket = static_cast<CircleBucket&>(*renderData->bucket);
        const auto vertexCount = bucket.vertices.elements();
        auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
            removeTile(renderPass, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());

        if (updateParameters->captureRenderedFeatures) {
            const auto& params = renderTree.getParameters().transformParams;
            captureRenderedFeatures(bucket, tile, paintPropertyBinders, evaluated, transformState, params);
        }

        // If there are already drawables for this tile, update their UBOs and move on to the next tile.
        auto updateExisting = [&](gfx::Drawable& drawable) {
            if (drawable.getLayerTweaker() != layerTweaker) {
                // This drawable was produced on a previous style/bucket, and should not be updated.
                return false;
            }
            return true;
        };
        if (updateTile(renderPass, tileID, std::move(updateExisting))) {
            continue;
        }

        propertiesAsUniforms.first.clear();
        propertiesAsUniforms.second.clear();

        auto circleVertexAttrs = context.createVertexAttributeArray();
        circleVertexAttrs->readDataDrivenPaintProperties<CircleColor,
                                                         CircleRadius,
                                                         CircleBlur,
                                                         CircleOpacity,
                                                         CircleStrokeColor,
                                                         CircleStrokeWidth,
                                                         CircleStrokeOpacity>(
            paintPropertyBinders, evaluated, propertiesAsUniforms, idCircleColorVertexAttribute);

        const auto circleShader = circleShaderGroup->getOrCreateShader(context, propertiesAsUniforms);
        if (!circleShader) {
            continue;
        }

        if (const auto& attr = circleVertexAttrs->set(idCirclePosVertexAttribute)) {
            attr->setSharedRawData(bucket.sharedVertices,
                                   offsetof(CircleLayoutVertex, a1),
                                   0,
                                   sizeof(CircleLayoutVertex),
                                   gfx::AttributeDataType::Short2);
        }

        circleBuilder = context.createDrawableBuilder("circle");
        circleBuilder->setShader(std::static_pointer_cast<gfx::ShaderProgramBase>(circleShader));
        circleBuilder->setDepthType(gfx::DepthMaskType::ReadOnly);
        circleBuilder->setColorMode(gfx::ColorMode::alphaBlended());
        circleBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());

        circleBuilder->setRenderPass(renderPass);
        circleBuilder->setVertexAttributes(std::move(circleVertexAttrs));

        circleBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
        circleBuilder->setSegments(
            gfx::Triangles(), bucket.sharedTriangles, bucket.segments.data(), bucket.segments.size());

        circleBuilder->flush(context);

        for (auto& drawable : circleBuilder->clearDrawables()) {
            drawable->setTileID(tileID);
            drawable->setLayerTweaker(layerTweaker);
            drawable->setBinders(renderData->bucket, &paintPropertyBinders);
            drawable->setRenderTile(renderTilesOwner, &tile);

            tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
            ++stats.drawablesAdded;
        }
    }
}

} // namespace mbgl
