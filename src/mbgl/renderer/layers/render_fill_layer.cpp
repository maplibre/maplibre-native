#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/renderer/buckets/fill_bucket.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/renderer/layers/render_fill_layer.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/style/layers/fill_layer_impl.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/intersection_tests.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/std.hpp>

#include <mbgl/gfx/drawable_atlases_tweaker.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/renderer/layers/fill_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>
#include <mbgl/shaders/shader_program_base.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {

constexpr auto FillShaderName = "FillShader";
constexpr auto FillOutlineShaderName = "FillOutlineShader";
constexpr auto FillPatternShaderName = "FillPatternShader";
constexpr auto FillOutlinePatternShaderName = "FillOutlinePatternShader";
#if MLN_TRIANGULATE_FILL_OUTLINES
constexpr auto FillOutlineTriangulatedShaderName = "FillOutlineTriangulatedShader";
#endif

inline const FillLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == FillLayer::Impl::staticTypeInfo());
    return static_cast<const FillLayer::Impl&>(*impl);
}

} // namespace

RenderFillLayer::RenderFillLayer(Immutable<style::FillLayer::Impl> _impl)
    : RenderLayer(makeMutable<FillLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {
    styleDependencies = unevaluated.getDependencies();
}

RenderFillLayer::~RenderFillLayer() = default;

void RenderFillLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    styleDependencies = unevaluated.getDependencies();
}

void RenderFillLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    const auto previousProperties = staticImmutableCast<FillLayerProperties>(evaluatedProperties);
    auto properties = makeMutable<FillLayerProperties>(staticImmutableCast<FillLayer::Impl>(baseImpl),
                                                       parameters.getCrossfadeParameters(),
                                                       unevaluated.evaluate(parameters, previousProperties->evaluated));
    auto& evaluated = properties->evaluated;

    if (unevaluated.get<style::FillOutlineColor>().isUndefined()) {
        evaluated.get<style::FillOutlineColor>() = evaluated.get<style::FillColor>();
    }

    passes = RenderPass::Translucent;

    if (!(!unevaluated.get<style::FillPattern>().isUndefined() ||
          evaluated.get<style::FillColor>().constantOr(Color()).a < 1.0f ||
          evaluated.get<style::FillOpacity>().constantOr(0) < 1.0f)) {
        // Supply both - evaluated based on opaquePassCutoff in render().
        passes |= RenderPass::Opaque;
    }
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);

    if (layerTweaker) {
        layerTweaker->updateProperties(evaluatedProperties);
    }
}

bool RenderFillLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderFillLayer::hasCrossfade() const {
    return getCrossfade<FillLayerProperties>(evaluatedProperties).t != 1;
}

bool RenderFillLayer::queryIntersectsFeature(const GeometryCoordinates& queryGeometry,
                                             const GeometryTileFeature& feature,
                                             const float,
                                             const TransformState& transformState,
                                             const float pixelsToTileUnits,
                                             const mat4&,
                                             const FeatureState&) const {
    const auto& evaluated = getEvaluated<FillLayerProperties>(evaluatedProperties);
    auto translatedQueryGeometry = FeatureIndex::translateQueryGeometry(queryGeometry,
                                                                        evaluated.get<style::FillTranslate>(),
                                                                        evaluated.get<style::FillTranslateAnchor>(),
                                                                        static_cast<float>(transformState.getBearing()),
                                                                        pixelsToTileUnits);

    return util::polygonIntersectsMultiPolygon(translatedQueryGeometry.value_or(queryGeometry),
                                               feature.getGeometries());
}

void RenderFillLayer::update(gfx::ShaderRegistry& shaders,
                             gfx::Context& context,
                             const TransformState&,
                             const std::shared_ptr<UpdateParameters>&,
                             const RenderTree&,
                             UniqueChangeRequestVec& changes) {
    if (!renderTiles || renderTiles->empty()) {
        removeAllDrawables();
        return;
    }

    // Set up a layer group for fill
    if (!layerGroup) {
        if (auto layerGroup_ = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID())) {
            setLayerGroup(std::move(layerGroup_), changes);
        } else {
            return;
        }
    }
    auto* fillTileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());

#if MLN_TRIANGULATE_FILL_OUTLINES
    if (!outlineTriangulatedShaderGroup) {
        outlineTriangulatedShaderGroup = shaders.getShaderGroup(std::string(FillOutlineTriangulatedShaderName));
    }
#endif

    if (!layerTweaker) {
        layerTweaker = std::make_shared<FillLayerTweaker>(getID(), evaluatedProperties);
        layerGroup->addLayerTweaker(layerTweaker);
    }

    if (!fillShaderGroup) {
        fillShaderGroup = shaders.getShaderGroup(std::string(FillShaderName));
    }
    if (!outlineShaderGroup) {
        outlineShaderGroup = shaders.getShaderGroup(std::string(FillOutlineShaderName));
    }
    if (!patternShaderGroup) {
        patternShaderGroup = shaders.getShaderGroup(std::string(FillPatternShaderName));
    }
    if (!outlinePatternShaderGroup) {
        outlinePatternShaderGroup = shaders.getShaderGroup(std::string(FillOutlinePatternShaderName));
    }
    if (!fillShaderGroup || !outlineShaderGroup || !patternShaderGroup || !outlinePatternShaderGroup) {
        removeAllDrawables();
        return;
    }

    std::unique_ptr<gfx::DrawableBuilder> fillBuilder;
    std::unique_ptr<gfx::DrawableBuilder> outlineBuilder;
    std::unique_ptr<gfx::DrawableBuilder> patternBuilder;
    std::unique_ptr<gfx::DrawableBuilder> outlinePatternBuilder;

    const auto layerPrefix = getID() + "/";
    constexpr auto renderPass = RenderPass::Translucent;
    constexpr auto lineWidth = 2.0f;

    const auto commonInit = [&](gfx::DrawableBuilder& builder) {
        builder.setCullFaceMode(gfx::CullFaceMode::disabled());
        builder.setEnableStencil(true);
    };

    stats.drawablesRemoved += fillTileLayerGroup->removeDrawablesIf([&](gfx::Drawable& drawable) {
        // If the render pass has changed or the tile has dropped out of the cover set, remove it.
        const auto& tileID = drawable.getTileID();
        return tileID && !hasRenderTile(*tileID);
    });

    fillTileLayerGroup->setStencilTiles(renderTiles);

    StringIDSetsPair propertiesAsUniforms;
    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
        if (!renderData || !renderData->bucket || !renderData->bucket->hasData()) {
            removeTile(renderPass, tileID);
            continue;
        }

        auto& bucket = static_cast<FillBucket&>(*renderData->bucket);
        auto& binders = bucket.paintPropertyBinders.at(getID());

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
            removeTile(renderPass, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());

        const auto& evaluated = getEvaluated<FillLayerProperties>(renderData->layerProperties);

        gfx::DrawableTweakerPtr atlasTweaker;
        auto getAtlasTweaker = [&]() {
            if (!atlasTweaker) {
                if (const auto& atlases = tile.getAtlasTextures(); atlases && atlases->icon) {
                    atlasTweaker = std::make_shared<gfx::DrawableAtlasesTweaker>(
                        atlases,
                        std::nullopt,
                        idFillImageTexture,
                        /*isText*/ false,
                        /*sdfIcons*/ true, // to force linear filter
                        /*rotationAlignment_*/ AlignmentType::Auto,
                        /*iconScaled*/ false,
                        /*textSizeIsZoomConstant_*/ false);
                }
            }
            return atlasTweaker;
        };

        propertiesAsUniforms.first.clear();
        propertiesAsUniforms.second.clear();

        // `Fill*Program` all use `style::FillPaintProperties`
        // TODO: Only rebuild the vertex attributes when something has changed.
        // TODO: Can we update them in-place instead of replacing?
        auto vertexAttrs = context.createVertexAttributeArray();
        vertexAttrs->readDataDrivenPaintProperties<FillColor, FillOpacity, FillOutlineColor, FillPattern>(
            binders, evaluated, propertiesAsUniforms, idFillColorVertexAttribute);

        const auto fillVertexCount = bucket.vertices.elements();
        if (const auto& attr = vertexAttrs->set(idFillPosVertexAttribute)) {
            attr->setSharedRawData(bucket.sharedVertices,
                                   offsetof(FillLayoutVertex, a1),
                                   /*vertexOffset=*/0,
                                   sizeof(FillLayoutVertex),
                                   gfx::AttributeDataType::Short2);
        }

#if MLN_TRIANGULATE_FILL_OUTLINES
        const auto lineVertexCount = bucket.lineVertices.elements();
        const auto getTriangulatedAttributes = [&]() {
            auto attrs = context.createVertexAttributeArray();
            if (const auto& attr = attrs->set(idLinePosNormalVertexAttribute)) {
                attr->setSharedRawData(bucket.sharedLineVertices,
                                       offsetof(LineLayoutVertex, a1),
                                       /*vertexOffset=*/0,
                                       sizeof(LineLayoutVertex),
                                       gfx::AttributeDataType::Short2);
            }
            if (const auto& attr = attrs->set(idLineDataVertexAttribute)) {
                attr->setSharedRawData(bucket.sharedLineVertices,
                                       offsetof(LineLayoutVertex, a2),
                                       /*vertexOffset=*/0,
                                       sizeof(LineLayoutVertex),
                                       gfx::AttributeDataType::UByte4);
            }
            return attrs;
        };
#endif

        // If we already have drawables for this tile, update them.
        auto updateExisting = [&](gfx::Drawable& drawable) {
            if (drawable.getLayerTweaker() != layerTweaker) {
                // This drawable was produced on a previous style/bucket, and should not be updated.
                return false;
            }

            switch (static_cast<FillVariant>(drawable.getType())) {
                case FillVariant::Fill:
                case FillVariant::FillPattern:
                    drawable.updateVertexAttributes(vertexAttrs,
                                                    fillVertexCount,
                                                    gfx::Triangles(),
                                                    bucket.sharedTriangles,
                                                    bucket.triangleSegments.data(),
                                                    bucket.triangleSegments.size());
                    break;
                case FillVariant::FillOutline:
                case FillVariant::FillOutlinePattern:
                    drawable.updateVertexAttributes(vertexAttrs,
                                                    fillVertexCount,
                                                    gfx::Lines(lineWidth),
                                                    bucket.sharedBasicLineIndexes,
                                                    bucket.basicLineSegments.data(),
                                                    bucket.basicLineSegments.size());
                    break;
#if MLN_TRIANGULATE_FILL_OUTLINES
                case FillVariant::FillOutlineTriangulated:
                    if (const auto updated = drawable.getAttributeUpdateTime();
                        !updated || bucket.lineVertices.getLastModified() > *updated) {
                        drawable.updateVertexAttributes(getTriangulatedAttributes(),
                                                        lineVertexCount,
                                                        gfx::Triangles(),
                                                        bucket.sharedLineIndexes,
                                                        bucket.lineSegments.data(),
                                                        bucket.lineSegments.size());
                    }
                    break;
#endif
                default:
                    Log::Error(Event::General, "Invalid fill type " + util::toString(drawable.getType()));
                    assert(false);
                    return false;
            }

            return true;
        };
        if (updateTile(renderPass, tileID, std::move(updateExisting))) {
            continue;
        }

        const auto finish = [&](gfx::DrawableBuilder& builder, FillVariant type) {
            builder.flush(context);

            for (auto& drawable : builder.clearDrawables()) {
                drawable->setTileID(tileID);
                drawable->setType(static_cast<size_t>(type));
                drawable->setLayerTweaker(layerTweaker);
                drawable->setBinders(renderData->bucket, &binders);
                drawable->setRenderTile(renderTilesOwner, &tile);
                fillTileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                ++stats.drawablesAdded;
            }
        };

        // Outline always occurs in translucent pass, defaults to fill color
        // Outline does not default to fill in the pattern case
        const auto doOutline = evaluated.get<FillAntialias>() && (unevaluated.get<FillPattern>().isUndefined() ||
                                                                  unevaluated.get<FillOutlineColor>().isUndefined());
#if MLN_TRIANGULATE_FILL_OUTLINES
        const bool dataDrivenOutline = !evaluated.get<FillOutlineColor>().isConstant() ||
                                       !evaluated.get<FillOpacity>().isConstant();
#endif

        if (unevaluated.get<FillPattern>().isUndefined()) {
            // Simple fill
            if (!fillShaderGroup || (doOutline && !outlineShaderGroup)) {
                continue;
            }
            const auto fillShader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                fillShaderGroup->getOrCreateShader(context, propertiesAsUniforms));

#if MLN_TRIANGULATE_FILL_OUTLINES
            const auto outlineTriangulatedShader = doOutline && !dataDrivenOutline ? [&]() -> auto {
                static const StringIDSetsPair outlinePropertiesAsUniforms{
                    {"a_color", "a_opacity", "a_width"},
                    {idLineColorVertexAttribute, idLineOpacityVertexAttribute, idLineWidthVertexAttribute}};
                return std::static_pointer_cast<gfx::ShaderProgramBase>(
                    outlineTriangulatedShaderGroup->getOrCreateShader(context, outlinePropertiesAsUniforms));
            }()
                : nullptr;

            auto createOutlineTriangulated = [&](auto& builder) {
                if (doOutline && builder && lineVertexCount) {
                    builder->setShader(outlineTriangulatedShader);
                    builder->setRawVertices({}, lineVertexCount, gfx::AttributeDataType::Short2);
                    builder->setVertexAttributes(getTriangulatedAttributes());
                    builder->setSegments(gfx::Triangles(),
                                         bucket.sharedLineIndexes,
                                         bucket.lineSegments.data(),
                                         bucket.lineSegments.size());
                    finish(*builder, FillVariant::FillOutlineTriangulated);
                }
            };
#endif
            const auto outlineShader = doOutline
                                           ? std::static_pointer_cast<gfx::ShaderProgramBase>(
                                                 outlineShaderGroup->getOrCreateShader(context, propertiesAsUniforms))
                                           : nullptr;

            if (!fillBuilder && fillShader) {
                if (auto builder = context.createDrawableBuilder(layerPrefix + "fill")) {
                    // Only write opaque fills to the depth buffer, matching `fillRenderPass` in legacy rendering
                    const bool opaque = (evaluated.get<FillColor>().constantOr(Color()).a >= 1.0f &&
                                         evaluated.get<FillOpacity>().constantOr(0) >= 1.0f);

                    commonInit(*builder);
                    builder->setDepthType(opaque ? gfx::DepthMaskType::ReadWrite : gfx::DepthMaskType::ReadOnly);
                    builder->setColorMode(gfx::ColorMode::alphaBlended());
                    builder->setSubLayerIndex(1);
                    builder->setRenderPass(renderPass);
                    fillBuilder = std::move(builder);
                }
            }
            if (doOutline && !outlineBuilder && outlineShader) {
                if (auto builder = context.createDrawableBuilder(layerPrefix + "fill-outline")) {
                    commonInit(*builder);
                    builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                    builder->setLineWidth(lineWidth);
                    builder->setSubLayerIndex(unevaluated.get<FillOutlineColor>().isUndefined() ? 2 : 0);
                    builder->setColorMode(gfx::ColorMode::alphaBlended());
                    builder->setRenderPass(RenderPass::Translucent);
                    outlineBuilder = std::move(builder);
                }
            }

            if (fillBuilder && bucket.sharedTriangles->elements()) {
                fillBuilder->setShader(fillShader);
#if MLN_TRIANGULATE_FILL_OUTLINES
                if (doOutline && dataDrivenOutline && outlineBuilder) {
                    outlineBuilder->setVertexAttributes(vertexAttrs);
                }
#else
                if (doOutline && outlineBuilder) {
                    outlineBuilder->setVertexAttributes(vertexAttrs);
                }
#endif
                fillBuilder->setVertexAttributes(std::move(vertexAttrs));

                fillBuilder->setRawVertices({}, fillVertexCount, gfx::AttributeDataType::Short2);
                fillBuilder->setSegments(gfx::Triangles(),
                                         bucket.sharedTriangles,
                                         bucket.triangleSegments.data(),
                                         bucket.triangleSegments.size());
                finish(*fillBuilder, FillVariant::Fill);
            }

#if MLN_TRIANGULATE_FILL_OUTLINES
            if (doOutline && outlineBuilder) {
                if (!dataDrivenOutline) {
                    outlineBuilder->setSubLayerIndex(unevaluated.get<FillOutlineColor>().isUndefined() ? 2 : 0);
                    createOutlineTriangulated(outlineBuilder);
                } else {
                    if (bucket.sharedBasicLineIndexes->elements()) {
                        outlineBuilder->setShader(outlineShader);
                        outlineBuilder->setRawVertices({}, fillVertexCount, gfx::AttributeDataType::Short2);
                        outlineBuilder->setSegments(gfx::Lines(lineWidth),
                                                    bucket.sharedBasicLineIndexes,
                                                    bucket.basicLineSegments.data(),
                                                    bucket.basicLineSegments.size());
                        finish(*outlineBuilder, FillVariant::FillOutline);
                    }
                }
            }
#else
            if (doOutline && outlineBuilder && bucket.sharedBasicLineIndexes->elements()) {
                outlineBuilder->setShader(outlineShader);
                outlineBuilder->setRawVertices({}, fillVertexCount, gfx::AttributeDataType::Short2);
                outlineBuilder->setSegments(gfx::Lines(lineWidth),
                                            bucket.sharedBasicLineIndexes,
                                            bucket.basicLineSegments.data(),
                                            bucket.basicLineSegments.size());
                finish(*outlineBuilder, FillVariant::FillOutline);
            }
#endif
        } else {
            // Fill with pattern
            if ((renderPass & RenderPass::Translucent) == 0) {
                continue;
            }

            if (!patternShaderGroup || (doOutline && !outlinePatternShaderGroup)) {
                continue;
            }

            const auto fillShader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                patternShaderGroup->getOrCreateShader(context, propertiesAsUniforms));
            const auto outlineShader = doOutline ? std::static_pointer_cast<gfx::ShaderProgramBase>(
                                                       outlinePatternShaderGroup->getOrCreateShader(
                                                           context, propertiesAsUniforms))
                                                 : nullptr;

            if (!patternBuilder) {
                if (auto builder = context.createDrawableBuilder(layerPrefix + "fill-pattern")) {
                    commonInit(*builder);
                    builder->setShader(fillShader);
                    builder->setDepthType(gfx::DepthMaskType::ReadWrite);
                    builder->setColorMode(gfx::ColorMode::alphaBlended());
                    builder->setSubLayerIndex(1);
                    builder->setRenderPass(RenderPass::Translucent);
                    patternBuilder = std::move(builder);
                }
            }
            if (doOutline && !outlinePatternBuilder) {
                if (auto builder = context.createDrawableBuilder(layerPrefix + "fill-outline-pattern")) {
                    commonInit(*builder);
                    builder->setShader(outlineShader);
                    builder->setLineWidth(lineWidth);
                    builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                    builder->setColorMode(gfx::ColorMode::alphaBlended());
                    builder->setSubLayerIndex(2);
                    builder->setRenderPass(RenderPass::Translucent);
                    outlinePatternBuilder = std::move(builder);
                }
            }

            if (patternBuilder) {
                patternBuilder->clearTweakers();
                if (const auto& tweaker = getAtlasTweaker()) {
                    patternBuilder->addTweaker(tweaker);
                }
            }
            if (doOutline && outlinePatternBuilder) {
                outlinePatternBuilder->clearTweakers();
                if (const auto& tweaker = getAtlasTweaker()) {
                    outlinePatternBuilder->addTweaker(tweaker);
                }
            }

            if (patternBuilder && bucket.sharedTriangles->elements()) {
                patternBuilder->setShader(fillShader);
                patternBuilder->setRenderPass(renderPass);
                if (doOutline && outlinePatternBuilder) {
                    outlinePatternBuilder->setVertexAttributes(vertexAttrs);
                }
                patternBuilder->setVertexAttributes(std::move(vertexAttrs));
                patternBuilder->setRawVertices({}, fillVertexCount, gfx::AttributeDataType::Short2);
                patternBuilder->setSegments(gfx::Triangles(),
                                            bucket.sharedTriangles,
                                            bucket.triangleSegments.data(),
                                            bucket.triangleSegments.size());

                finish(*patternBuilder, FillVariant::FillPattern);
            }

            if (doOutline && outlinePatternBuilder && bucket.sharedBasicLineIndexes->elements()) {
                outlinePatternBuilder->setShader(outlineShader);
                outlinePatternBuilder->setRenderPass(renderPass);
                outlinePatternBuilder->setRawVertices({}, fillVertexCount, gfx::AttributeDataType::Short2);
                outlinePatternBuilder->setSegments(gfx::Lines(lineWidth),
                                                   bucket.sharedBasicLineIndexes,
                                                   bucket.basicLineSegments.data(),
                                                   bucket.basicLineSegments.size());

                finish(*outlinePatternBuilder, FillVariant::FillOutlinePattern);
            }
        }
    }
}

} // namespace mbgl
