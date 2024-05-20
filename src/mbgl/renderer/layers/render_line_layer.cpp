#include <mbgl/renderer/layers/render_line_layer.hpp>

#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/geometry/line_atlas.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/programs/line_program.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/renderer/buckets/line_bucket.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/renderer/upload_parameters.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/style/layers/line_layer_impl.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/intersection_tests.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/math.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gfx/drawable_atlases_tweaker.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/line_drawable_data.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/line_layer_tweaker.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#endif

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {

inline const LineLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == LineLayer::Impl::staticTypeInfo());
    return static_cast<const LineLayer::Impl&>(*impl);
}

#if MLN_DRAWABLE_RENDERER

const auto posNormalAttribName = "a_pos_normal";

#endif // MLN_DRAWABLE_RENDERER

} // namespace

RenderLineLayer::RenderLineLayer(Immutable<style::LineLayer::Impl> _impl)
    : RenderLayer(makeMutable<LineLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()),
      colorRamp(std::make_shared<PremultipliedImage>(Size(256, 1))) {
    styleDependencies = unevaluated.getDependencies();
}

RenderLineLayer::~RenderLineLayer() = default;

void RenderLineLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    updateColorRamp();

#if MLN_RENDER_BACKEND_METAL
    if (auto* tweaker = static_cast<LineLayerTweaker*>(layerTweaker.get())) {
        tweaker->updateGPUExpressions(unevaluated, parameters.now);
    }
#endif // MLN_RENDER_BACKEND_METAL
}

void RenderLineLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    const auto previousProperties = staticImmutableCast<LineLayerProperties>(evaluatedProperties);
    auto properties = makeMutable<LineLayerProperties>(staticImmutableCast<LineLayer::Impl>(baseImpl),
                                                       parameters.getCrossfadeParameters(),
                                                       unevaluated.evaluate(parameters, previousProperties->evaluated));
    auto& evaluated = properties->evaluated;

    passes = (evaluated.get<style::LineOpacity>().constantOr(1.0) > 0 &&
              evaluated.get<style::LineColor>().constantOr(Color::black()).a > 0 &&
              evaluated.get<style::LineWidth>().constantOr(1.0) > 0)
                 ? RenderPass::Translucent
                 : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);

#if MLN_DRAWABLE_RENDERER
    if (auto* tweaker = static_cast<LineLayerTweaker*>(layerTweaker.get())) {
        tweaker->updateProperties(evaluatedProperties);
#if MLN_RENDER_BACKEND_METAL
        tweaker->updateGPUExpressions(unevaluated, parameters.now);
#endif // MLN_RENDER_BACKEND_METAL
    }
#endif
}

bool RenderLineLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderLineLayer::hasCrossfade() const {
    return getCrossfade<LineLayerProperties>(evaluatedProperties).t != 1;
}

void RenderLineLayer::prepare(const LayerPrepareParameters& params) {
    RenderLayer::prepare(params);
    for (const RenderTile& tile : *renderTiles) {
        const LayerRenderData* renderData = tile.getLayerRenderData(*baseImpl);
        if (!renderData) continue;

        const auto& evaluated = getEvaluated<LineLayerProperties>(renderData->layerProperties);
        if (evaluated.get<LineDasharray>().from.empty()) continue;

        const auto& bucket = static_cast<const LineBucket&>(*renderData->bucket);
        const LinePatternCap cap = bucket.layout.get<LineCap>() == LineCapType::Round ? LinePatternCap::Round
                                                                                      : LinePatternCap::Square;
        // Ensures that the dash data gets added to the atlas.
        params.lineAtlas.getDashPatternTexture(
            evaluated.get<LineDasharray>().from, evaluated.get<LineDasharray>().to, cap);
    }
}

#if MLN_LEGACY_RENDERER
void RenderLineLayer::upload(gfx::UploadPass& uploadPass) {
    if (!unevaluated.get<LineGradient>().getValue().isUndefined() && !colorRampTexture) {
        colorRampTexture = uploadPass.createTexture(*colorRamp);
    }
}

void RenderLineLayer::render(PaintParameters& parameters) {
    assert(renderTiles);
    if (parameters.pass == RenderPass::Opaque) {
        return;
    }

    if (!parameters.shaders.getLegacyGroup().populate(lineProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(lineGradientProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(lineSDFProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(linePatternProgram)) return;

    parameters.renderTileClippingMasks(renderTiles);

    for (const RenderTile& tile : *renderTiles) {
        const LayerRenderData* renderData = getRenderDataForPass(tile, parameters.pass);
        if (!renderData) {
            continue;
        }
        auto& bucket = static_cast<LineBucket&>(*renderData->bucket);
        const auto& evaluated = getEvaluated<LineLayerProperties>(renderData->layerProperties);
        const auto& crossfade = getCrossfade<LineLayerProperties>(renderData->layerProperties);

        auto draw = [&](auto& programInstance,
                        auto&& uniformValues,
                        const std::optional<ImagePosition>& patternPositionA,
                        const std::optional<ImagePosition>& patternPositionB,
                        auto&& textureBindings) {
            const auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());

            paintPropertyBinders.setPatternParameters(patternPositionA, patternPositionB, crossfade);

            const auto allUniformValues = programInstance.computeAllUniformValues(
                std::forward<decltype(uniformValues)>(uniformValues),
                paintPropertyBinders,
                evaluated,
                static_cast<float>(parameters.state.getZoom()));
            const auto allAttributeBindings = programInstance.computeAllAttributeBindings(
                *bucket.vertexBuffer, paintPropertyBinders, evaluated);

            checkRenderability(parameters, programInstance.activeBindingCount(allAttributeBindings));

            programInstance.draw(parameters.context,
                                 *parameters.renderPass,
                                 gfx::Triangles(),
                                 parameters.depthModeForSublayer(0, gfx::DepthMaskType::ReadOnly),
                                 parameters.stencilModeForClipping(tile.id),
                                 parameters.colorModeForRenderPass(),
                                 gfx::CullFaceMode::disabled(),
                                 *bucket.indexBuffer,
                                 bucket.segments,
                                 allUniformValues,
                                 allAttributeBindings,
                                 std::forward<decltype(textureBindings)>(textureBindings),
                                 getID());
        };

        if (!evaluated.get<LineDasharray>().from.empty()) {
            const LinePatternCap cap = bucket.layout.get<LineCap>() == LineCapType::Round ? LinePatternCap::Round
                                                                                          : LinePatternCap::Square;
            const auto& dashPatternTexture = parameters.lineAtlas.getDashPatternTexture(
                evaluated.get<LineDasharray>().from, evaluated.get<LineDasharray>().to, cap);

            draw(*lineSDFProgram,
                 LineSDFProgram::layoutUniformValues(evaluated,
                                                     parameters.pixelRatio,
                                                     tile,
                                                     parameters.state,
                                                     parameters.pixelsToGLUnits,
                                                     dashPatternTexture.getFrom(),
                                                     dashPatternTexture.getTo(),
                                                     crossfade,
                                                     static_cast<float>(dashPatternTexture.getSize().width)),
                 {},
                 {},
                 LineSDFProgram::TextureBindings{
                     dashPatternTexture.textureBinding(),
                 });

        } else if (!unevaluated.get<LinePattern>().isUndefined()) {
            const auto& linePatternValue = evaluated.get<LinePattern>().constantOr(Faded<expression::Image>{"", ""});
            const Size& texsize = tile.getIconAtlasTexture()->getSize();

            std::optional<ImagePosition> posA = tile.getPattern(linePatternValue.from.id());
            std::optional<ImagePosition> posB = tile.getPattern(linePatternValue.to.id());

            draw(*linePatternProgram,
                 LinePatternProgram::layoutUniformValues(evaluated,
                                                         tile,
                                                         parameters.state,
                                                         parameters.pixelsToGLUnits,
                                                         parameters.pixelRatio,
                                                         texsize,
                                                         crossfade),
                 posA,
                 posB,
                 LinePatternProgram::TextureBindings{
                     tile.getIconAtlasTextureBinding(gfx::TextureFilterType::Linear),
                 });
        } else if (!unevaluated.get<LineGradient>().getValue().isUndefined()) {
            assert(colorRampTexture);

            draw(*lineGradientProgram,
                 LineGradientProgram::layoutUniformValues(
                     evaluated, tile, parameters.state, parameters.pixelsToGLUnits, parameters.pixelRatio),
                 {},
                 {},
                 LineGradientProgram::TextureBindings{
                     textures::image::Value{colorRampTexture->getResource(), gfx::TextureFilterType::Linear},
                 });
        } else {
            draw(*lineProgram,
                 LineProgram::layoutUniformValues(
                     evaluated, tile, parameters.state, parameters.pixelsToGLUnits, parameters.pixelRatio),
                 {},
                 {},
                 LineProgram::TextureBindings{});
        }
    }
}
#endif // MLN_LEGACY_RENDERER

namespace {

GeometryCollection offsetLine(const GeometryCollection& rings, double offset) {
    assert(offset != 0.0f);
    assert(!rings.empty());

    GeometryCollection newRings;
    newRings.reserve(rings.size());

    const Point<double> zero(0, 0);
    for (const auto& ring : rings) {
        newRings.emplace_back();
        auto& newRing = newRings.back();
        newRing.reserve(ring.size());

        for (auto i = ring.begin(); i != ring.end(); ++i) {
            auto& p = *i;

            Point<double> aToB = i == ring.begin() ? zero : util::perp(util::unit(convertPoint<double>(p - *(i - 1))));
            Point<double> bToC = i + 1 == ring.end() ? zero
                                                     : util::perp(util::unit(convertPoint<double>(*(i + 1) - p)));
            Point<double> extrude = util::unit(aToB + bToC);

            const double cosHalfAngle = extrude.x * bToC.x + extrude.y * bToC.y;
            extrude *= (cosHalfAngle != 0) ? (1.0 / cosHalfAngle) : 0;

            newRing.emplace_back(convertPoint<int16_t>(extrude * offset) + p);
        }
    }

    return newRings;
}

} // namespace

bool RenderLineLayer::queryIntersectsFeature(const GeometryCoordinates& queryGeometry,
                                             const GeometryTileFeature& feature,
                                             const float zoom,
                                             const TransformState& transformState,
                                             const float pixelsToTileUnits,
                                             const mat4&,
                                             const FeatureState& featureState) const {
    const auto& evaluated = static_cast<const LineLayerProperties&>(*evaluatedProperties).evaluated;
    // Translate query geometry
    auto translatedQueryGeometry = FeatureIndex::translateQueryGeometry(queryGeometry,
                                                                        evaluated.get<style::LineTranslate>(),
                                                                        evaluated.get<style::LineTranslateAnchor>(),
                                                                        static_cast<float>(transformState.getBearing()),
                                                                        pixelsToTileUnits);

    // Evaluate function
    auto offset = evaluated.get<style::LineOffset>().evaluate(
                      feature, zoom, featureState, style::LineOffset::defaultValue()) *
                  pixelsToTileUnits;
    // Test intersection
    const auto halfWidth = static_cast<float>(getLineWidth(feature, zoom, featureState) / 2.0 * pixelsToTileUnits);

    // Apply offset to geometry
    if (offset != 0.0f && !feature.getGeometries().empty()) {
        return util::polygonIntersectsBufferedMultiLine(
            translatedQueryGeometry.value_or(queryGeometry), offsetLine(feature.getGeometries(), offset), halfWidth);
    }

    return util::polygonIntersectsBufferedMultiLine(
        translatedQueryGeometry.value_or(queryGeometry), feature.getGeometries(), halfWidth);
}

void RenderLineLayer::updateColorRamp() {
    const style::ColorRampPropertyValue colorValue = unevaluated.get<LineGradient>().getValue();
    if (!colorRamp || !applyColorRamp(colorValue, *colorRamp)) {
        return;
    }

    colorRampTexture = std::nullopt;

#if MLN_DRAWABLE_RENDERER
    if (colorRampTexture2D) {
        colorRampTexture2D.reset();

        // delete all gradient drawables
        if (layerGroup) {
            stats.drawablesRemoved += layerGroup->getDrawableCount();
            layerGroup->clearDrawables();
        }
    }
#endif
}

float RenderLineLayer::getLineWidth(const GeometryTileFeature& feature,
                                    const float zoom,
                                    const FeatureState& featureState) const {
    const auto& evaluated = static_cast<const LineLayerProperties&>(*evaluatedProperties).evaluated;
    float lineWidth = evaluated.get<style::LineWidth>().evaluate(
        feature, zoom, featureState, style::LineWidth::defaultValue());
    float gapWidth = evaluated.get<style::LineGapWidth>().evaluate(
        feature, zoom, featureState, style::LineGapWidth::defaultValue());
    if (gapWidth) {
        return gapWidth + 2 * lineWidth;
    } else {
        return lineWidth;
    }
}

#if MLN_DRAWABLE_RENDERER
void RenderLineLayer::update(gfx::ShaderRegistry& shaders,
                             gfx::Context& context,
                             const TransformState& state,
                             [[maybe_unused]] const std::shared_ptr<UpdateParameters>& parameters,
                             [[maybe_unused]] const RenderTree& renderTree,
                             [[maybe_unused]] UniqueChangeRequestVec& changes) {
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
        auto tweaker = std::make_shared<LineLayerTweaker>(getID(), evaluatedProperties);
#if MLN_RENDER_BACKEND_METAL
        tweaker->updateGPUExpressions(unevaluated, parameters->timePoint);
#endif // MLN_RENDER_BACKEND_METAL

        layerTweaker = std::move(tweaker);
        layerGroup->addLayerTweaker(layerTweaker);
    }

    const RenderPass renderPass = static_cast<RenderPass>(evaluatedProperties->renderPasses &
                                                          ~mbgl::underlying_type(RenderPass::Opaque));

    stats.drawablesRemoved += tileLayerGroup->removeDrawablesIf([&](gfx::Drawable& drawable) {
        // If the render pass has changed or the tile has  dropped out of the cover set, remove it.
        const auto& tileID = drawable.getTileID();
        if (drawable.getRenderPass() != passes || (tileID && !hasRenderTile(*tileID))) {
            return true;
        }
        return false;
    });

    auto createLineBuilder = [&](const std::string& name,
                                 gfx::ShaderPtr shader) -> std::unique_ptr<gfx::DrawableBuilder> {
        std::unique_ptr<gfx::DrawableBuilder> builder = context.createDrawableBuilder(name);
        builder->setShader(std::static_pointer_cast<gfx::ShaderProgramBase>(shader));
        builder->setRenderPass(renderPass);
        builder->setSubLayerIndex(0);
        builder->setDepthType(gfx::DepthMaskType::ReadOnly);
        builder->setColorMode(renderPass == RenderPass::Translucent ? gfx::ColorMode::alphaBlended()
                                                                    : gfx::ColorMode::unblended());
        builder->setCullFaceMode(gfx::CullFaceMode::disabled());
        builder->setEnableStencil(true);

        return builder;
    };

    auto addAttributes =
        [&](gfx::DrawableBuilder& builder, const LineBucket& bucket, gfx::VertexAttributeArrayPtr&& vertexAttrs) {
            const auto vertexCount = bucket.vertices.elements();
            builder.setRawVertices({}, vertexCount, gfx::AttributeDataType::Short4);

            if (const auto& attr = vertexAttrs->set(idLinePosNormalVertexAttribute)) {
                attr->setSharedRawData(bucket.sharedVertices,
                                       offsetof(LineLayoutVertex, a1),
                                       /*vertexOffset=*/0,
                                       sizeof(LineLayoutVertex),
                                       gfx::AttributeDataType::Short2);
            }

            if (const auto& attr = vertexAttrs->set(idLineDataVertexAttribute)) {
                attr->setSharedRawData(bucket.sharedVertices,
                                       offsetof(LineLayoutVertex, a2),
                                       /*vertexOffset=*/0,
                                       sizeof(LineLayoutVertex),
                                       gfx::AttributeDataType::UByte4);
            }

            builder.setVertexAttributes(std::move(vertexAttrs));
        };

    auto setSegments = [&](std::unique_ptr<gfx::DrawableBuilder>& builder, const LineBucket& bucket) {
        builder->setSegments(gfx::Triangles(), bucket.sharedTriangles, bucket.segments.data(), bucket.segments.size());
    };

    tileLayerGroup->setStencilTiles(renderTiles);

    StringIDSetsPair propertiesAsUniforms;
    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
        if (!renderData) {
            removeTile(renderPass, tileID);
            continue;
        }

        auto& bucket = static_cast<LineBucket&>(*renderData->bucket);
        if (!bucket.sharedTriangles->elements()) {
            removeTile(renderPass, tileID);
            continue;
        }

        const auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());
        const auto& evaluated = getEvaluated<LineLayerProperties>(renderData->layerProperties);
        const auto& crossfade = getCrossfade<LineLayerProperties>(renderData->layerProperties);

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
            removeTile(renderPass, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());

        // interpolation UBOs
        const float zoom = static_cast<float>(state.getZoom());

        std::optional<LineInterpolationUBO> lineInterpolationUBO = std::nullopt;
        std::optional<LineGradientInterpolationUBO> lineGradientInterpolationUBO = std::nullopt;
        std::optional<LinePatternInterpolationUBO> linePatternInterpolationUBO = std::nullopt;
        std::optional<LineSDFInterpolationUBO> lineSDFInterpolationUBO = std::nullopt;

        auto getLineInterpolationUBO = [&]() -> const LineInterpolationUBO& {
            if (!lineInterpolationUBO) {
                lineInterpolationUBO = {
                    /*color_t =*/std::get<0>(paintPropertyBinders.get<LineColor>()->interpolationFactor(zoom)),
                    /*blur_t =*/std::get<0>(paintPropertyBinders.get<LineBlur>()->interpolationFactor(zoom)),
                    /*opacity_t =*/std::get<0>(paintPropertyBinders.get<LineOpacity>()->interpolationFactor(zoom)),
                    /*gapwidth_t =*/std::get<0>(paintPropertyBinders.get<LineGapWidth>()->interpolationFactor(zoom)),
                    /*offset_t =*/std::get<0>(paintPropertyBinders.get<LineOffset>()->interpolationFactor(zoom)),
                    /*width_t =*/std::get<0>(paintPropertyBinders.get<LineWidth>()->interpolationFactor(zoom)),
                    0,
                    0};
            }

            return *lineInterpolationUBO;
        };

        auto getLineGradientInterpolationUBO = [&]() -> const LineGradientInterpolationUBO& {
            if (!lineGradientInterpolationUBO) {
                lineGradientInterpolationUBO = {
                    /*blur_t =*/std::get<0>(paintPropertyBinders.get<LineBlur>()->interpolationFactor(zoom)),
                    /*opacity_t =*/std::get<0>(paintPropertyBinders.get<LineOpacity>()->interpolationFactor(zoom)),
                    /*gapwidth_t =*/std::get<0>(paintPropertyBinders.get<LineGapWidth>()->interpolationFactor(zoom)),
                    /*offset_t =*/std::get<0>(paintPropertyBinders.get<LineOffset>()->interpolationFactor(zoom)),
                    /*width_t =*/std::get<0>(paintPropertyBinders.get<LineWidth>()->interpolationFactor(zoom)),
                    0,
                    0,
                    0};
            }

            return *lineGradientInterpolationUBO;
        };

        auto getLinePatternInterpolationUBO = [&]() -> const LinePatternInterpolationUBO& {
            if (!linePatternInterpolationUBO) {
                linePatternInterpolationUBO = {
                    /*blur_t =*/std::get<0>(paintPropertyBinders.get<LineBlur>()->interpolationFactor(zoom)),
                    /*opacity_t =*/std::get<0>(paintPropertyBinders.get<LineOpacity>()->interpolationFactor(zoom)),
                    /*offset_t =*/std::get<0>(paintPropertyBinders.get<LineOffset>()->interpolationFactor(zoom)),
                    /*gapwidth_t =*/std::get<0>(paintPropertyBinders.get<LineGapWidth>()->interpolationFactor(zoom)),
                    /*width_t =*/std::get<0>(paintPropertyBinders.get<LineWidth>()->interpolationFactor(zoom)),
                    /*pattern_from_t =*/std::get<0>(paintPropertyBinders.get<LinePattern>()->interpolationFactor(zoom)),
                    /*pattern_to_t =*/std::get<1>(paintPropertyBinders.get<LinePattern>()->interpolationFactor(zoom)),
                    0};
            }

            return *linePatternInterpolationUBO;
        };

        auto getLineSDFInterpolationUBO = [&]() -> const LineSDFInterpolationUBO& {
            if (!lineSDFInterpolationUBO) {
                lineSDFInterpolationUBO = {
                    /*color_t =*/std::get<0>(paintPropertyBinders.get<LineColor>()->interpolationFactor(zoom)),
                    /*blur_t =*/std::get<0>(paintPropertyBinders.get<LineBlur>()->interpolationFactor(zoom)),
                    /*opacity_t =*/std::get<0>(paintPropertyBinders.get<LineOpacity>()->interpolationFactor(zoom)),
                    /*gapwidth_t =*/std::get<0>(paintPropertyBinders.get<LineGapWidth>()->interpolationFactor(zoom)),
                    /*offset_t =*/std::get<0>(paintPropertyBinders.get<LineOffset>()->interpolationFactor(zoom)),
                    /*width_t =*/std::get<0>(paintPropertyBinders.get<LineWidth>()->interpolationFactor(zoom)),
                    /*floorwidth_t =*/
                    std::get<0>(paintPropertyBinders.get<LineFloorWidth>()->interpolationFactor(zoom)),
                    0};
            }

            return *lineSDFInterpolationUBO;
        };

        // tile dependent properties UBOs:
        std::optional<LinePatternTilePropertiesUBO> linePatternTilePropertiesUBO = std::nullopt;
        const auto& linePatternValue = evaluated.get<LinePattern>().constantOr(Faded<expression::Image>{"", ""});
        const std::optional<ImagePosition> patternPosA = tile.getPattern(linePatternValue.from.id());
        const std::optional<ImagePosition> patternPosB = tile.getPattern(linePatternValue.to.id());
        paintPropertyBinders.setPatternParameters(patternPosA, patternPosB, crossfade);

        auto getLinePatternTilePropertiesUBO = [&]() -> const LinePatternTilePropertiesUBO& {
            if (!linePatternTilePropertiesUBO) {
                linePatternTilePropertiesUBO = {
                    /*pattern_from =*/patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},
                    /*pattern_to =*/patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0}};
            }

            return *linePatternTilePropertiesUBO;
        };

        auto updateExisting = [&](gfx::Drawable& drawable) {
            if (drawable.getLayerTweaker() != layerTweaker) {
                // This drawable was produced on a previous style/bucket, and should not be updated.
                return false;
            }

            auto& drawableUniforms = drawable.mutableUniformBuffers();
            const LineLayerTweaker::LineType type = static_cast<LineLayerTweaker::LineType>(drawable.getType());
            switch (type) {
                case LineLayerTweaker::LineType::Simple: {
                    drawableUniforms.createOrUpdate(idLineInterpolationUBO, &getLineInterpolationUBO(), context);
                } break;

                case LineLayerTweaker::LineType::Gradient: {
                    drawableUniforms.createOrUpdate(
                        idLineInterpolationUBO, &getLineGradientInterpolationUBO(), context);
                } break;

                case LineLayerTweaker::LineType::Pattern: {
                    drawableUniforms.createOrUpdate(idLineInterpolationUBO, &getLinePatternInterpolationUBO(), context);

                    drawableUniforms.createOrUpdate(
                        idLineTilePropertiesUBO, &getLinePatternTilePropertiesUBO(), context);
                } break;

                case LineLayerTweaker::LineType::SDF: {
                    drawableUniforms.createOrUpdate(idLineInterpolationUBO, &getLineSDFInterpolationUBO(), context);
                } break;

                default: {
                    using namespace std::string_literals;
                    Log::Error(Event::General,
                               "RenderLineLayer: unknown line type: "s + std::to_string(mbgl::underlying_type(type)));
                } break;
            }
            return true;
        };
        if (updateTile(renderPass, tileID, std::move(updateExisting))) {
            continue;
        }

        propertiesAsUniforms.first.clear();
        propertiesAsUniforms.second.clear();

        auto vertexAttrs = context.createVertexAttributeArray();
        vertexAttrs->readDataDrivenPaintProperties<LineColor,
                                                   LineBlur,
                                                   LineOpacity,
                                                   LineGapWidth,
                                                   LineOffset,
                                                   LineWidth,
                                                   LineFloorWidth,
                                                   LinePattern>(
            paintPropertyBinders, evaluated, propertiesAsUniforms, idLineColorVertexAttribute);

        if (!evaluated.get<LineDasharray>().from.empty()) {
            // dash array line (SDF)
            if (!lineSDFShaderGroup) {
                lineSDFShaderGroup = shaders.getShaderGroup("LineSDFShader");
                if (!lineSDFShaderGroup) {
                    continue;
                }
            }

            auto shader = lineSDFShaderGroup->getOrCreateShader(context, propertiesAsUniforms, posNormalAttribName);
            if (!shader) {
                continue;
            }

            auto builder = createLineBuilder("lineSDF", std::move(shader));

            // vertices, attributes and segments
            addAttributes(*builder, bucket, std::move(vertexAttrs));
            setSegments(builder, bucket);

            // finish
            builder->flush(context);
            const LinePatternCap cap = bucket.layout.get<LineCap>() == LineCapType::Round ? LinePatternCap::Round
                                                                                          : LinePatternCap::Square;
            for (auto& drawable : builder->clearDrawables()) {
                drawable->setType(mbgl::underlying_type(LineLayerTweaker::LineType::SDF));
                drawable->setTileID(tileID);
                drawable->setLayerTweaker(layerTweaker);
                drawable->setData(std::make_unique<gfx::LineDrawableData>(cap));
                drawable->mutableUniformBuffers().createOrUpdate(
                    idLineInterpolationUBO, &getLineSDFInterpolationUBO(), context);

                tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                ++stats.drawablesAdded;
            }
        } else if (!unevaluated.get<LinePattern>().isUndefined()) {
            // pattern line
            if (!linePatternShaderGroup) {
                linePatternShaderGroup = shaders.getShaderGroup("LinePatternShader");
                if (!linePatternShaderGroup) {
                    continue;
                }
            }

            auto shader = linePatternShaderGroup->getOrCreateShader(context, propertiesAsUniforms, posNormalAttribName);
            if (!shader) {
                continue;
            }

            auto builder = createLineBuilder("linePattern", std::move(shader));

            // vertices and attributes
            addAttributes(*builder, bucket, std::move(vertexAttrs));

            // texture
            if (const auto& atlases = tile.getAtlasTextures(); atlases && atlases->icon) {
                if (!iconTweaker) {
                    iconTweaker = std::make_shared<gfx::DrawableAtlasesTweaker>(
                        atlases,
                        std::nullopt,
                        idLineImageTexture,
                        /*isText*/ false,
                        /*sdfIcons*/ true, // to force linear filter
                        /*rotationAlignment_*/ AlignmentType::Auto,
                        /*iconScaled*/ false,
                        /*textSizeIsZoomConstant_*/ false);
                }

                builder->addTweaker(iconTweaker);

                setSegments(builder, bucket);

                builder->flush(context);
                for (auto& drawable : builder->clearDrawables()) {
                    drawable->setType(mbgl::underlying_type(LineLayerTweaker::LineType::Pattern));
                    drawable->setTileID(tileID);
                    drawable->setLayerTweaker(layerTweaker);
                    drawable->mutableUniformBuffers().createOrUpdate(
                        idLineInterpolationUBO, &getLinePatternInterpolationUBO(), context);
                    drawable->mutableUniformBuffers().createOrUpdate(
                        idLineTilePropertiesUBO, &getLinePatternTilePropertiesUBO(), context);

                    tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                    ++stats.drawablesAdded;
                }
            }
        } else if (!unevaluated.get<LineGradient>().getValue().isUndefined()) {
            // gradient line
            if (!lineGradientShaderGroup) {
                lineGradientShaderGroup = shaders.getShaderGroup("LineGradientShader");
                if (!lineGradientShaderGroup) {
                    continue;
                }
            }

            auto shader = lineGradientShaderGroup->getOrCreateShader(
                context, propertiesAsUniforms, posNormalAttribName);
            if (!shader) {
                continue;
            }

            auto builder = createLineBuilder("lineGradient", std::move(shader));

            // vertices and attributes
            addAttributes(*builder, bucket, std::move(vertexAttrs));

            // texture
            if (!colorRampTexture2D && colorRamp->valid()) {
                // create texture. to be reused for all the tiles of the layer
                colorRampTexture2D = context.createTexture2D();
                colorRampTexture2D->setImage(colorRamp);
                colorRampTexture2D->setSamplerConfiguration(
                    {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
            }

            if (colorRampTexture2D) {
                builder->setTexture(colorRampTexture2D, idLineImageTexture);

                // segments
                setSegments(builder, bucket);

                // finish
                builder->flush(context);
                for (auto& drawable : builder->clearDrawables()) {
                    drawable->setType(mbgl::underlying_type(LineLayerTweaker::LineType::Gradient));
                    drawable->setTileID(tileID);
                    drawable->setLayerTweaker(layerTweaker);
                    drawable->mutableUniformBuffers().createOrUpdate(
                        idLineInterpolationUBO, &getLineGradientInterpolationUBO(), context);

                    tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                    ++stats.drawablesAdded;
                }
            }
        } else {
            // simple line
            if (!lineShaderGroup) {
                lineShaderGroup = shaders.getShaderGroup("LineShader");
                if (!lineShaderGroup) {
                    continue;
                }
            }

            auto shader = lineShaderGroup->getOrCreateShader(context, propertiesAsUniforms, posNormalAttribName);
            if (!shader) {
                continue;
            }

            auto builder = createLineBuilder("line", std::move(shader));

            // vertices, attributes and segments
            addAttributes(*builder, bucket, std::move(vertexAttrs));
            setSegments(builder, bucket);

            // finish
            builder->flush(context);
            for (auto& drawable : builder->clearDrawables()) {
                drawable->setType(mbgl::underlying_type(LineLayerTweaker::LineType::Simple));
                drawable->setTileID(tileID);
                drawable->setLayerTweaker(layerTweaker);
                drawable->mutableUniformBuffers().createOrUpdate(
                    idLineInterpolationUBO, &getLineInterpolationUBO(), context);

                tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                ++stats.drawablesAdded;
            }
        }
    }
}
#endif

} // namespace mbgl
