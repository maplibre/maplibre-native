#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/programs/fill_program.hpp>
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

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gfx/drawable_atlases_tweaker.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/renderer/layers/fill_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#endif

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {

#if MLN_DRAWABLE_RENDERER
constexpr auto FillShaderName = "FillShader";
constexpr auto FillOutlineShaderName = "FillOutlineShader";
constexpr auto FillPatternShaderName = "FillPatternShader";
constexpr auto FillOutlinePatternShaderName = "FillOutlinePatternShader";
#if MLN_TRIANGULATE_FILL_OUTLINES
constexpr auto FillOutlineTriangulatedShaderName = "FillOutlineTriangulatedShader";
#endif
#endif // MLN_DRAWABLE_RENDERER

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

#if MLN_DRAWABLE_RENDERER
    if (layerTweaker) {
        layerTweaker->updateProperties(evaluatedProperties);
    }
#endif
}

bool RenderFillLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderFillLayer::hasCrossfade() const {
    return getCrossfade<FillLayerProperties>(evaluatedProperties).t != 1;
}

#if MLN_LEGACY_RENDERER
void RenderFillLayer::render(PaintParameters& parameters) {
    assert(renderTiles);

    if (!parameters.shaders.getLegacyGroup().populate(fillProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(fillPatternProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(fillOutlineProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(fillOutlinePatternProgram)) return;

    if (unevaluated.get<FillPattern>().isUndefined()) {
        parameters.renderTileClippingMasks(renderTiles);
        for (const RenderTile& tile : *renderTiles) {
            const LayerRenderData* renderData = getRenderDataForPass(tile, parameters.pass);
            if (!renderData) {
                continue;
            }
            auto& bucket = static_cast<FillBucket&>(*renderData->bucket);
            const auto& evaluated = getEvaluated<FillLayerProperties>(renderData->layerProperties);

            auto draw = [&](auto& programInstance,
                            const auto& drawMode,
                            const auto& depthMode,
                            const auto& indexBuffer,
                            const auto& segments,
                            auto&& textureBindings) {
                const auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());

                const auto allUniformValues = programInstance.computeAllUniformValues(
                    FillProgram::LayoutUniformValues{
                        uniforms::matrix::Value(tile.translatedMatrix(
                            evaluated.get<FillTranslate>(), evaluated.get<FillTranslateAnchor>(), parameters.state)),
                        uniforms::world::Value(parameters.backend.getDefaultRenderable().getSize()),
                    },
                    paintPropertyBinders,
                    evaluated,
                    static_cast<float>(parameters.state.getZoom()));
                const auto allAttributeBindings = programInstance.computeAllAttributeBindings(
                    *bucket.vertexBuffer, paintPropertyBinders, evaluated);

                checkRenderability(parameters, programInstance.activeBindingCount(allAttributeBindings));

                programInstance.draw(parameters.context,
                                     *parameters.renderPass,
                                     drawMode,
                                     depthMode,
                                     parameters.stencilModeForClipping(tile.id),
                                     parameters.colorModeForRenderPass(),
                                     gfx::CullFaceMode::disabled(),
                                     indexBuffer,
                                     segments,
                                     allUniformValues,
                                     allAttributeBindings,
                                     std::forward<decltype(textureBindings)>(textureBindings),
                                     getID());
            };

            auto fillRenderPass = (evaluated.get<FillColor>().constantOr(Color()).a >= 1.0f &&
                                   evaluated.get<FillOpacity>().constantOr(0) >= 1.0f &&
                                   parameters.currentLayer >= parameters.opaquePassCutoff)
                                      ? RenderPass::Opaque
                                      : RenderPass::Translucent;
            if (bucket.triangleIndexBuffer && parameters.pass == fillRenderPass) {
                draw(*fillProgram,
                     gfx::Triangles(),
                     parameters.depthModeForSublayer(1,
                                                     parameters.pass == RenderPass::Opaque
                                                         ? gfx::DepthMaskType::ReadWrite
                                                         : gfx::DepthMaskType::ReadOnly),
                     *bucket.triangleIndexBuffer,
                     bucket.triangleSegments,
                     FillProgram::TextureBindings{});
            }

            if (evaluated.get<FillAntialias>() && parameters.pass == RenderPass::Translucent) {
                draw(*fillOutlineProgram,
                     gfx::Lines{2.0f},
                     parameters.depthModeForSublayer(unevaluated.get<FillOutlineColor>().isUndefined() ? 2 : 0,
                                                     gfx::DepthMaskType::ReadOnly),
                     *bucket.lineIndexBuffer,
                     bucket.basicLineSegments,
                     FillOutlineProgram::TextureBindings{});
            }
        }
    } else {
        if (parameters.pass != RenderPass::Translucent) {
            return;
        }

        parameters.renderTileClippingMasks(renderTiles);

        for (const RenderTile& tile : *renderTiles) {
            const LayerRenderData* renderData = getRenderDataForPass(tile, parameters.pass);
            if (!renderData) {
                continue;
            }
            auto& bucket = static_cast<FillBucket&>(*renderData->bucket);
            const auto& evaluated = getEvaluated<FillLayerProperties>(renderData->layerProperties);
            const auto& crossfade = getCrossfade<FillLayerProperties>(renderData->layerProperties);

            const auto& fillPatternValue = evaluated.get<FillPattern>().constantOr(Faded<expression::Image>{"", ""});
            std::optional<ImagePosition> patternPosA = tile.getPattern(fillPatternValue.from.id());
            std::optional<ImagePosition> patternPosB = tile.getPattern(fillPatternValue.to.id());

            auto draw = [&](auto& programInstance,
                            const auto& drawMode,
                            const auto& depthMode,
                            const auto& indexBuffer,
                            const auto& segments,
                            auto&& textureBindings) {
                const auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());
                paintPropertyBinders.setPatternParameters(patternPosA, patternPosB, crossfade);

                const auto allUniformValues = programInstance.computeAllUniformValues(
                    FillPatternProgram::layoutUniformValues(
                        tile.translatedMatrix(
                            evaluated.get<FillTranslate>(), evaluated.get<FillTranslateAnchor>(), parameters.state),
                        parameters.backend.getDefaultRenderable().getSize(),
                        tile.getIconAtlasTexture()->getSize(),
                        crossfade,
                        tile.id,
                        parameters.state,
                        parameters.pixelRatio),
                    paintPropertyBinders,
                    evaluated,
                    static_cast<float>(parameters.state.getZoom()));
                const auto allAttributeBindings = programInstance.computeAllAttributeBindings(
                    *bucket.vertexBuffer, paintPropertyBinders, evaluated);

                checkRenderability(parameters, programInstance.activeBindingCount(allAttributeBindings));

                programInstance.draw(parameters.context,
                                     *parameters.renderPass,
                                     drawMode,
                                     depthMode,
                                     parameters.stencilModeForClipping(tile.id),
                                     parameters.colorModeForRenderPass(),
                                     gfx::CullFaceMode::disabled(),
                                     indexBuffer,
                                     segments,
                                     allUniformValues,
                                     allAttributeBindings,
                                     std::forward<decltype(textureBindings)>(textureBindings),
                                     getID());
            };

            if (bucket.triangleIndexBuffer) {
                draw(*fillPatternProgram,
                     gfx::Triangles(),
                     parameters.depthModeForSublayer(1, gfx::DepthMaskType::ReadWrite),
                     *bucket.triangleIndexBuffer,
                     bucket.triangleSegments,
                     FillPatternProgram::TextureBindings{
                         tile.getIconAtlasTextureBinding(gfx::TextureFilterType::Linear),
                     });
            }
            if (evaluated.get<FillAntialias>() && unevaluated.get<FillOutlineColor>().isUndefined()) {
                draw(*fillOutlinePatternProgram,
                     gfx::Lines{2.0f},
                     parameters.depthModeForSublayer(2, gfx::DepthMaskType::ReadOnly),
                     *bucket.lineIndexBuffer,
                     bucket.basicLineSegments,
                     FillOutlinePatternProgram::TextureBindings{
                         tile.getIconAtlasTextureBinding(gfx::TextureFilterType::Linear),
                     });
            }
        }
    }
}
#endif // MLN_LEGACY_RENDERER

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

#if MLN_DRAWABLE_RENDERER

void RenderFillLayer::update(gfx::ShaderRegistry& shaders,
                             gfx::Context& context,
                             const TransformState& state,
                             const std::shared_ptr<UpdateParameters>&,
                             [[maybe_unused]] const RenderTree& renderTree,
                             [[maybe_unused]] UniqueChangeRequestVec& changes) {
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
        const auto& binders = bucket.paintPropertyBinders.at(getID());

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
            removeTile(renderPass, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());

        const auto& evaluated = getEvaluated<FillLayerProperties>(renderData->layerProperties);
        const auto& crossfade = getCrossfade<FillLayerProperties>(renderData->layerProperties);

        const auto& fillPatternValue = evaluated.get<FillPattern>().constantOr(Faded<expression::Image>{"", ""});
        const auto patternPosA = tile.getPattern(fillPatternValue.from.id());
        const auto patternPosB = tile.getPattern(fillPatternValue.to.id());
        binders.setPatternParameters(patternPosA, patternPosB, crossfade);

        const auto zoom = static_cast<float>(state.getZoom());

        std::optional<FillInterpolateUBO> fillInterpolateUBO = std::nullopt;
        std::optional<FillOutlineInterpolateUBO> fillOutlineInterpolateUBO = std::nullopt;
        std::optional<FillPatternInterpolateUBO> fillPatternInterpolateUBO = std::nullopt;
        std::optional<FillOutlinePatternInterpolateUBO> fillOutlinePatternInterpolateUBO = std::nullopt;

        auto getFillInterpolateUBO = [&]() -> const FillInterpolateUBO& {
            if (!fillInterpolateUBO) {
                fillInterpolateUBO = {
                    /* .color_t = */ std::get<0>(binders.get<FillColor>()->interpolationFactor(zoom)),
                    /* .opacity_t = */ std::get<0>(binders.get<FillOpacity>()->interpolationFactor(zoom)),
                    0,
                    0,
                };
            }

            return *fillInterpolateUBO;
        };

        auto getFillOutlineInterpolateUBO = [&]() -> const FillOutlineInterpolateUBO& {
            if (!fillOutlineInterpolateUBO) {
                fillOutlineInterpolateUBO = {
                    /* .color_t = */ std::get<0>(binders.get<FillOutlineColor>()->interpolationFactor(zoom)),
                    /* .opacity_t = */ std::get<0>(binders.get<FillOpacity>()->interpolationFactor(zoom)),
                    0,
                    0,
                };
            }

            return *fillOutlineInterpolateUBO;
        };

        auto getFillPatternInterpolateUBO = [&]() -> const FillPatternInterpolateUBO& {
            if (!fillPatternInterpolateUBO) {
                fillPatternInterpolateUBO = {
                    /* .pattern_from_t = */ std::get<0>(binders.get<FillPattern>()->interpolationFactor(zoom)),
                    /* .pattern_to_t = */ std::get<0>(binders.get<FillPattern>()->interpolationFactor(zoom)),
                    /* .opacity_t = */ std::get<0>(binders.get<FillOpacity>()->interpolationFactor(zoom)),
                    0,
                };
            }

            return *fillPatternInterpolateUBO;
        };

        auto getFillOutlinePatternInterpolateUBO = [&]() -> const FillOutlinePatternInterpolateUBO& {
            if (!fillOutlinePatternInterpolateUBO) {
                fillOutlinePatternInterpolateUBO = {
                    /* .pattern_from_t = */ std::get<0>(binders.get<FillPattern>()->interpolationFactor(zoom)),
                    /* .pattern_to_t = */ std::get<0>(binders.get<FillPattern>()->interpolationFactor(zoom)),
                    /* .opacity_t = */ std::get<0>(binders.get<FillOpacity>()->interpolationFactor(zoom)),
                    0,
                };
            }

            return *fillOutlinePatternInterpolateUBO;
        };

        std::optional<FillPatternTilePropsUBO> fillPatternTilePropsUBO = std::nullopt;
        auto getFillPatternTilePropsUBO = [&]() -> const FillPatternTilePropsUBO& {
            if (!fillPatternTilePropsUBO) {
                fillPatternTilePropsUBO = {
                    /* pattern_from = */ patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},
                    /* pattern_to = */ patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},
                };
            }

            return *fillPatternTilePropsUBO;
        };

        std::optional<FillOutlinePatternTilePropsUBO> fillOutlinePatternTilePropsUBO = std::nullopt;
        auto getFillOutlinePatternTilePropsUBO = [&]() -> const FillOutlinePatternTilePropsUBO& {
            if (!fillOutlinePatternTilePropsUBO) {
                fillOutlinePatternTilePropsUBO = {
                    /* pattern_from = */ patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},
                    /* pattern_to = */ patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},
                };
            }

            return *fillOutlinePatternTilePropsUBO;
        };

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
        auto vertexAttrs = context.createVertexAttributeArray();
        vertexAttrs->readDataDrivenPaintProperties<FillColor, FillOpacity, FillOutlineColor, FillPattern>(
            binders, evaluated, propertiesAsUniforms, idFillColorVertexAttribute);

        const auto vertexCount = bucket.vertices.elements();
        if (const auto& attr = vertexAttrs->set(idFillPosVertexAttribute)) {
            attr->setSharedRawData(bucket.sharedVertices,
                                   offsetof(FillLayoutVertex, a1),
                                   /*vertexOffset=*/0,
                                   sizeof(FillLayoutVertex),
                                   gfx::AttributeDataType::Short2);
        }

        // If we already have drawables for this tile, update them.
        auto updateExisting = [&](gfx::Drawable& drawable) {
            auto& drawableUniforms = drawable.mutableUniformBuffers();
            switch (static_cast<FillVariant>(drawable.getType())) {
                case FillVariant::Fill: {
                    drawableUniforms.createOrUpdate(idFillInterpolateUBO, &getFillInterpolateUBO(), context);
                    break;
                }
                case FillVariant::FillOutline: {
                    drawableUniforms.createOrUpdate(idFillInterpolateUBO, &getFillOutlineInterpolateUBO(), context);
                    break;
                }
                case FillVariant::FillPattern: {
                    drawableUniforms.createOrUpdate(idFillInterpolateUBO, &getFillPatternInterpolateUBO(), context);
                    drawableUniforms.createOrUpdate(idFillTilePropsUBO, &getFillPatternTilePropsUBO(), context);
                    break;
                }
                case FillVariant::FillOutlinePattern: {
                    drawableUniforms.createOrUpdate(
                        idFillInterpolateUBO, &getFillOutlinePatternInterpolateUBO(), context);
                    drawableUniforms.createOrUpdate(idFillTilePropsUBO, &getFillOutlinePatternTilePropsUBO(), context);
                    break;
                }
                case FillVariant::FillOutlineTriangulated: {
                    break;
                }
                default: {
#ifndef NDEBUG
                    mbgl::Log::Error(mbgl::Event::Render, "Invalid fill variant type supplied during drawable update!");
#endif
                    break;
                }
            }

            drawable.setVertexAttributes(vertexAttrs);
            return true;
        };
        if (updateTile(renderPass, tileID, std::move(updateExisting))) {
            continue;
        }

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
                if (doOutline && builder && bucket.sharedLineIndexes->elements()) {
                    builder->setShader(outlineTriangulatedShader);
                    builder->setRawVertices({}, bucket.lineVertices.elements(), gfx::AttributeDataType::Short2);

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
                    builder->setVertexAttributes(std::move(attrs));

                    builder->setSegments(gfx::Triangles(),
                                         bucket.sharedLineIndexes,
                                         bucket.lineSegments.data(),
                                         bucket.lineSegments.size());

                    // finish
                    builder->flush(context);
                    for (auto& drawable : builder->clearDrawables()) {
                        drawable->setTileID(tileID);
                        drawable->setType(static_cast<size_t>(FillVariant::FillOutlineTriangulated));
                        fillTileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                        ++stats.drawablesAdded;
                    }
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
                    builder->setLineWidth(2.0f);
                    builder->setSubLayerIndex(unevaluated.get<FillOutlineColor>().isUndefined() ? 2 : 0);
                    builder->setColorMode(gfx::ColorMode::alphaBlended());
                    builder->setRenderPass(RenderPass::Translucent);
                    outlineBuilder = std::move(builder);
                }
            }

            const auto finish = [&](gfx::DrawableBuilder& builder,
                                    const size_t interpolateUBOId,
                                    const auto& interpolateUBO,
                                    FillVariant type) {
                builder.flush(context);

                for (auto& drawable : builder.clearDrawables()) {
                    drawable->setTileID(tileID);
                    drawable->setLayerTweaker(layerTweaker);
                    drawable->setType(static_cast<size_t>(type));

                    auto& drawableUniforms = drawable->mutableUniformBuffers();
                    drawableUniforms.createOrUpdate(interpolateUBOId, &interpolateUBO, context);
                    fillTileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                    ++stats.drawablesAdded;
                }
            };

            if (fillBuilder && bucket.sharedTriangles->elements()) {
                fillBuilder->setShader(fillShader);
#if MLN_TRIANGULATE_FILL_OUTLINES
                if (doOutline && dataDrivenOutline && outlineBuilder) {
                    fillBuilder->setVertexAttributes(vertexAttrs);
                    outlineBuilder->setVertexAttributes(std::move(vertexAttrs));
                } else {
                    fillBuilder->setVertexAttributes(std::move(vertexAttrs));
                }
#else
                if (doOutline && outlineBuilder) {
                    fillBuilder->setVertexAttributes(vertexAttrs);
                    outlineBuilder->setVertexAttributes(std::move(vertexAttrs));
                } else {
                    fillBuilder->setVertexAttributes(std::move(vertexAttrs));
                }
#endif
                fillBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
                fillBuilder->setSegments(gfx::Triangles(),
                                         bucket.sharedTriangles,
                                         bucket.triangleSegments.data(),
                                         bucket.triangleSegments.size());
                finish(*fillBuilder, idFillInterpolateUBO, getFillInterpolateUBO(), FillVariant::Fill);
            }

#if MLN_TRIANGULATE_FILL_OUTLINES
            if (doOutline && outlineBuilder) {
                if (!dataDrivenOutline) {
                    outlineBuilder->setSubLayerIndex(unevaluated.get<FillOutlineColor>().isUndefined() ? 2 : 0);
                    createOutlineTriangulated(outlineBuilder);
                } else {
                    if (bucket.sharedBasicLineIndexes->elements()) {
                        outlineBuilder->setShader(outlineShader);
                        outlineBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
                        outlineBuilder->setSegments(gfx::Lines(2),
                                                    bucket.sharedBasicLineIndexes,
                                                    bucket.basicLineSegments.data(),
                                                    bucket.basicLineSegments.size());
                        finish(
                            *outlineBuilder, idFillInterpolateUBO, getFillInterpolateUBO(), FillVariant::FillOutline);
                    }
                }
            }
#else
            if (doOutline && outlineBuilder && bucket.sharedBasicLineIndexes->elements()) {
                outlineBuilder->setShader(outlineShader);
                outlineBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
                outlineBuilder->setSegments(gfx::Lines(2),
                                            bucket.sharedBasicLineIndexes,
                                            bucket.basicLineSegments.data(),
                                            bucket.basicLineSegments.size());
                finish(*outlineBuilder, idFillInterpolateUBO, getFillOutlineInterpolateUBO(), FillVariant::FillOutline);
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
                    builder->setLineWidth(2.0f);
                    builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                    builder->setColorMode(gfx::ColorMode::alphaBlended());
                    builder->setSubLayerIndex(2);
                    builder->setRenderPass(RenderPass::Translucent);
                    outlinePatternBuilder = std::move(builder);
                }
            }

            if (patternBuilder) {
                patternBuilder->clearTweakers();
                patternBuilder->addTweaker(getAtlasTweaker());
            }
            if (doOutline && outlinePatternBuilder) {
                outlinePatternBuilder->clearTweakers();
                outlinePatternBuilder->addTweaker(getAtlasTweaker());
            }

            const auto finish = [&](gfx::DrawableBuilder& builder,
                                    const size_t interpolateNameId,
                                    const auto& interpolateUBO,
                                    const size_t tileUBOId,
                                    const auto& tileUBO,
                                    FillVariant type) {
                builder.flush(context);

                for (auto& drawable : builder.clearDrawables()) {
                    drawable->setTileID(tileID);
                    drawable->setLayerTweaker(layerTweaker);
                    drawable->setType(static_cast<size_t>(type));

                    auto& drawableUniforms = drawable->mutableUniformBuffers();
                    drawableUniforms.createOrUpdate(interpolateNameId, &interpolateUBO, context);
                    drawableUniforms.createOrUpdate(tileUBOId, &tileUBO, context);
                    fillTileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                    ++stats.drawablesAdded;
                }
            };

            if (patternBuilder && bucket.sharedTriangles->elements()) {
                patternBuilder->setShader(fillShader);
                patternBuilder->setRenderPass(renderPass);
                if (doOutline && outlinePatternBuilder) {
                    patternBuilder->setVertexAttributes(vertexAttrs);
                    outlinePatternBuilder->setVertexAttributes(std::move(vertexAttrs));
                } else {
                    patternBuilder->setVertexAttributes(std::move(vertexAttrs));
                }
                patternBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
                patternBuilder->setSegments(gfx::Triangles(),
                                            bucket.sharedTriangles,
                                            bucket.triangleSegments.data(),
                                            bucket.triangleSegments.size());

                finish(*patternBuilder,
                       idFillInterpolateUBO,
                       getFillPatternInterpolateUBO(),
                       idFillTilePropsUBO,
                       getFillPatternTilePropsUBO(),
                       FillVariant::FillPattern);
            }

            if (doOutline && outlinePatternBuilder && bucket.sharedBasicLineIndexes->elements()) {
                outlinePatternBuilder->setShader(outlineShader);
                outlinePatternBuilder->setRenderPass(renderPass);
                outlinePatternBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
                outlinePatternBuilder->setSegments(gfx::Lines(2),
                                                   bucket.sharedBasicLineIndexes,
                                                   bucket.basicLineSegments.data(),
                                                   bucket.basicLineSegments.size());

                finish(*outlinePatternBuilder,
                       idFillInterpolateUBO,
                       getFillOutlinePatternInterpolateUBO(),
                       idFillTilePropsUBO,
                       getFillOutlinePatternTilePropsUBO(),
                       FillVariant::FillOutlinePattern);
            }
        }
    }
}
#endif

} // namespace mbgl
