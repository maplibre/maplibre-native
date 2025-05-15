#include <mbgl/renderer/layers/render_circle_layer.hpp>

#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/renderer/buckets/circle_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/style/layers/circle_layer_impl.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/intersection_tests.hpp>
#include <mbgl/util/containers.hpp>

#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/renderer/layers/circle_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/shader_program_base.hpp>

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

constexpr auto CircleShaderGroupName = "CircleShader";

} // namespace

using namespace shaders;

void RenderCircleLayer::update(gfx::ShaderRegistry& shaders,
                               gfx::Context& context,
                               const TransformState&,
                               const std::shared_ptr<UpdateParameters>&,
                               const RenderTree&,
                               UniqueChangeRequestVec& changes) {
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
