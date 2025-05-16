#include <mbgl/renderer/layers/render_fill_extrusion_layer.hpp>

#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/style/layers/fill_extrusion_layer_impl.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/intersection_tests.hpp>
#include <mbgl/util/math.hpp>

#include <mbgl/gfx/drawable_atlases_tweaker.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/fill_extrusion_layer_tweaker.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>
#include <mbgl/shaders/shader_program_base.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {

inline const FillExtrusionLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == FillExtrusionLayer::Impl::staticTypeInfo());
    return static_cast<const FillExtrusionLayer::Impl&>(*impl);
}

} // namespace

RenderFillExtrusionLayer::RenderFillExtrusionLayer(Immutable<style::FillExtrusionLayer::Impl> _impl)
    : RenderLayer(makeMutable<FillExtrusionLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {
    styleDependencies = unevaluated.getDependencies();
}

RenderFillExtrusionLayer::~RenderFillExtrusionLayer() = default;

void RenderFillExtrusionLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    styleDependencies = unevaluated.getDependencies();
}

void RenderFillExtrusionLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    const auto previousProperties = staticImmutableCast<FillExtrusionLayerProperties>(evaluatedProperties);
    auto properties = makeMutable<FillExtrusionLayerProperties>(
        staticImmutableCast<FillExtrusionLayer::Impl>(baseImpl),
        parameters.getCrossfadeParameters(),
        unevaluated.evaluate(parameters, previousProperties->evaluated));

    passes = (properties->evaluated.get<style::FillExtrusionOpacity>() > 0) ? RenderPass::Translucent
                                                                            : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);

    if (layerTweaker) {
        layerTweaker->updateProperties(evaluatedProperties);
    }
}

bool RenderFillExtrusionLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderFillExtrusionLayer::hasCrossfade() const {
    return getCrossfade<FillExtrusionLayerProperties>(evaluatedProperties).t != 1;
}

bool RenderFillExtrusionLayer::is3D() const {
    return true;
}

bool RenderFillExtrusionLayer::queryIntersectsFeature(const GeometryCoordinates& queryGeometry,
                                                      const GeometryTileFeature& feature,
                                                      const float,
                                                      const TransformState& transformState,
                                                      const float pixelsToTileUnits,
                                                      const mat4&,
                                                      const FeatureState&) const {
    const auto& evaluated = static_cast<const FillExtrusionLayerProperties&>(*evaluatedProperties).evaluated;
    auto translatedQueryGeometry = FeatureIndex::translateQueryGeometry(
        queryGeometry,
        evaluated.get<style::FillExtrusionTranslate>(),
        evaluated.get<style::FillExtrusionTranslateAnchor>(),
        static_cast<float>(transformState.getBearing()),
        pixelsToTileUnits);

    return util::polygonIntersectsMultiPolygon(translatedQueryGeometry.value_or(queryGeometry),
                                               feature.getGeometries());
}

void RenderFillExtrusionLayer::update(gfx::ShaderRegistry& shaders,
                                      gfx::Context& context,
                                      const TransformState&,
                                      const std::shared_ptr<UpdateParameters>&,
                                      const RenderTree&,
                                      UniqueChangeRequestVec& changes) {
    if (!renderTiles || renderTiles->empty() || passes == RenderPass::None) {
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

    if (!layerTweaker) {
        layerTweaker = std::make_shared<FillExtrusionLayerTweaker>(getID(), evaluatedProperties);
        layerGroup->addLayerTweaker(layerTweaker);
    }

    if (!fillExtrusionGroup) {
        fillExtrusionGroup = shaders.getShaderGroup("FillExtrusionShader");
    }
    if (!fillExtrusionPatternGroup) {
        fillExtrusionPatternGroup = shaders.getShaderGroup("FillExtrusionPatternShader");
    }

    auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());

    const auto& evaluated = static_cast<const FillExtrusionLayerProperties&>(*evaluatedProperties).evaluated;

    constexpr auto drawPass = RenderPass::Translucent;

    stats.drawablesRemoved += tileLayerGroup->removeDrawablesIf([&](gfx::Drawable& drawable) {
        // If the render pass has changed or the tile has  dropped out of the cover set, remove it.
        const auto& tileID = drawable.getTileID();
        if (!(drawable.getRenderPass() & drawPass) || (tileID && !hasRenderTile(*tileID))) {
            return true;
        }
        return false;
    });

    const auto layerPrefix = getID() + "/";
    const auto hasPattern = !unevaluated.get<FillExtrusionPattern>().isUndefined();
    const auto opaque = evaluated.get<FillExtrusionOpacity>() >= 1;

    std::unique_ptr<gfx::DrawableBuilder> depthBuilder;
    std::unique_ptr<gfx::DrawableBuilder> colorBuilder;

    const auto& shaderGroup = hasPattern ? fillExtrusionPatternGroup : fillExtrusionGroup;
    if (!shaderGroup) {
        removeAllDrawables();
        return;
    }

    tileLayerGroup->setStencilTiles(renderTiles);

    StringIDSetsPair propertiesAsUniforms;
    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const auto* optRenderData = getRenderDataForPass(tile, drawPass);
        if (!optRenderData || !optRenderData->bucket || !optRenderData->bucket->hasData()) {
            removeTile(drawPass, tileID);
            continue;
        }

        const auto& renderData = *optRenderData;
        auto& bucket = static_cast<FillExtrusionBucket&>(*renderData.bucket);

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
            removeTile(drawPass, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());

        gfx::DrawableTweakerPtr tweaker;
        if (depthBuilder) {
            depthBuilder->clearTweakers();
        }
        if (colorBuilder) {
            colorBuilder->clearTweakers();
        }

        const auto vertexCount = bucket.vertices.elements();
        auto& binders = bucket.paintPropertyBinders.at(getID());

        // If we already have drawables for this tile, update them.
        auto updateExisting = [&](gfx::Drawable& drawable) {
            if (drawable.getLayerTweaker() != layerTweaker) {
                // This drawable was produced on a previous style/bucket, and should not be updated.
                return false;
            }
            return true;
        };
        if (updateTile(drawPass, tileID, std::move(updateExisting))) {
            continue;
        }

        propertiesAsUniforms.first.clear();
        propertiesAsUniforms.second.clear();

        auto vertexAttrs = context.createVertexAttributeArray();
        vertexAttrs->readDataDrivenPaintProperties<FillExtrusionBase,
                                                   FillExtrusionColor,
                                                   FillExtrusionHeight,
                                                   FillExtrusionPattern>(
            binders, evaluated, propertiesAsUniforms, idFillExtrusionBaseVertexAttribute);

        const auto shader = std::static_pointer_cast<gfx::ShaderProgramBase>(
            shaderGroup->getOrCreateShader(context, propertiesAsUniforms));
        if (!shader) {
            continue;
        }

        // The non-pattern path in `render()` only uses two-pass rendering if there's translucency.
        // The pattern path always uses two passes.
        const auto doDepthPass = (!opaque || hasPattern);

        if (doDepthPass && !depthBuilder) {
            if (auto builder = context.createDrawableBuilder(layerPrefix + "depth")) {
                builder->setShader(shader);
                builder->setIs3D(true);
                builder->setEnableColor(false);
                builder->setRenderPass(drawPass);
                builder->setCullFaceMode(gfx::CullFaceMode::backCCW());
                builder->setDrawPriority(0);
                if (tweaker) {
                    builder->addTweaker(tweaker);
                }
                depthBuilder = std::move(builder);
            }
        }
        if (!colorBuilder) {
            if (auto builder = context.createDrawableBuilder(layerPrefix + "color")) {
                builder->setShader(shader);
                builder->setIs3D(true);
                builder->setEnableColor(true);
                builder->setColorMode(gfx::ColorMode::alphaBlended());
                builder->setRenderPass(drawPass);
                builder->setCullFaceMode(gfx::CullFaceMode::backCCW());
                builder->setDrawPriority(1);
                if (tweaker) {
                    builder->addTweaker(tweaker);
                }
                colorBuilder = std::move(builder);
            }
        }

        if (hasPattern && !tweaker) {
            if (const auto& atlases = tile.getAtlasTextures()) {
                tweaker = std::make_shared<gfx::DrawableAtlasesTweaker>(atlases,
                                                                        std::nullopt,
                                                                        idFillExtrusionImageTexture,
                                                                        /*isText=*/false,
                                                                        false,
                                                                        style::AlignmentType::Auto,
                                                                        false,
                                                                        false);
                if (depthBuilder) {
                    depthBuilder->addTweaker(tweaker);
                }
                if (colorBuilder) {
                    colorBuilder->addTweaker(tweaker);
                }
            }
        }

        if (const auto& attr = vertexAttrs->set(idFillExtrusionPosVertexAttribute)) {
            attr->setSharedRawData(bucket.sharedVertices,
                                   offsetof(FillExtrusionLayoutVertex, a1),
                                   /*vertexOffset=*/0,
                                   sizeof(FillExtrusionLayoutVertex),
                                   gfx::AttributeDataType::Short2);
        }
        if (const auto& attr = vertexAttrs->set(idFillExtrusionNormalEdVertexAttribute)) {
            attr->setSharedRawData(bucket.sharedVertices,
                                   offsetof(FillExtrusionLayoutVertex, a2),
                                   /*vertexOffset=*/0,
                                   sizeof(FillExtrusionLayoutVertex),
                                   gfx::AttributeDataType::Short4);
        }

        if (doDepthPass) {
            depthBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
            depthBuilder->setVertexAttributes(vertexAttrs);
        }

        colorBuilder->setEnableStencil(doDepthPass);
        colorBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
        colorBuilder->setVertexAttributes(std::move(vertexAttrs));

        const auto finish = [&](gfx::DrawableBuilder& builder) {
            if (!bucket.sharedTriangles->elements()) {
                return;
            }
            builder.setSegments(gfx::Triangles(),
                                bucket.sharedTriangles,
                                bucket.triangleSegments.data(),
                                bucket.triangleSegments.size());

            builder.flush(context);

            for (auto& drawable : builder.clearDrawables()) {
                drawable->setTileID(tileID);
                drawable->setType(static_cast<std::size_t>(hasPattern));
                drawable->setLayerTweaker(layerTweaker);
                drawable->setBinders(renderData.bucket, &binders);
                drawable->setRenderTile(renderTilesOwner, &tile);

                tileLayerGroup->addDrawable(drawPass, tileID, std::move(drawable));
                ++stats.drawablesAdded;
            }
        };

        if (doDepthPass) {
            finish(*depthBuilder);
        }
        finish(*colorBuilder);
    }
}

} // namespace mbgl
