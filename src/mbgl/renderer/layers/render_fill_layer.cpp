#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/programs/fill_program.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/renderer/buckets/fill_bucket.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/fill_layer_tweaker.hpp>
#include <mbgl/renderer/layers/render_fill_layer.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/style/layers/fill_layer_impl.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/intersection_tests.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/std.hpp>

namespace mbgl {

using namespace style;

namespace {

constexpr std::string_view FillShaderName = "FillShader";
constexpr std::string_view FillOutlineShaderName = "FillOutlineShader";
constexpr std::string_view FillPatternShaderName = "FillPatternShader";
constexpr std::string_view FillOutlinePatternShaderName = "FillOutlinePatternShader";

inline const FillLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == FillLayer::Impl::staticTypeInfo());
    return static_cast<const FillLayer::Impl&>(*impl);
}

} // namespace

RenderFillLayer::RenderFillLayer(Immutable<style::FillLayer::Impl> _impl)
    : RenderLayer(makeMutable<FillLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {}

RenderFillLayer::~RenderFillLayer() = default;

void RenderFillLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
}

void RenderFillLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto properties = makeMutable<FillLayerProperties>(staticImmutableCast<FillLayer::Impl>(baseImpl),
                                                       parameters.getCrossfadeParameters(),
                                                       unevaluated.evaluate(parameters));
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
    if (tileLayerGroup) {
        tileLayerGroup->setLayerTweaker(std::make_shared<FillLayerTweaker>(evaluatedProperties));
    }
}

bool RenderFillLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderFillLayer::hasCrossfade() const {
    return getCrossfade<FillLayerProperties>(evaluatedProperties).t != 1;
}

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
                     bucket.lineSegments,
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
                     bucket.lineSegments,
                     FillOutlinePatternProgram::TextureBindings{
                         tile.getIconAtlasTextureBinding(gfx::TextureFilterType::Linear),
                     });
            }
        }
    }
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

void RenderFillLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    // Remove everything
    if (tileLayerGroup) {
        changes.emplace_back(std::make_unique<RemoveLayerGroupRequest>(tileLayerGroup->getLayerIndex()));
        tileLayerGroup.reset();
    }
}

void RenderFillLayer::removeTile(RenderPass renderPass, const OverscaledTileID& tileID) {
    stats.tileDrawablesRemoved += tileLayerGroup->removeDrawables(renderPass, tileID).size();
}

void RenderFillLayer::update(gfx::ShaderRegistry& shaders,
                             gfx::Context& context,
                             const TransformState& state,
                             [[maybe_unused]] const RenderTree& renderTree,
                             [[maybe_unused]] UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

    if (!renderTiles || renderTiles->empty()) {
        if (tileLayerGroup) {
            stats.tileDrawablesRemoved += tileLayerGroup->clearDrawables();
        }
        return;
    }

    // Set up a layer group
    if (!tileLayerGroup) {
        tileLayerGroup = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID());
        if (!tileLayerGroup) {
            return;
        }
        tileLayerGroup->setLayerTweaker(std::make_shared<FillLayerTweaker>(evaluatedProperties));
    }

    const auto removeAll = [&]() {
        if (tileLayerGroup) {
            stats.tileDrawablesRemoved += tileLayerGroup->getDrawableCount();
            tileLayerGroup->clearDrawables();
        }
    };

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
        removeAll();
        return;
    }

    std::unordered_set<OverscaledTileID> newTileIDs(renderTiles->size());
    std::transform(renderTiles->begin(),
                   renderTiles->end(),
                   std::inserter(newTileIDs, newTileIDs.begin()),
                   [](const auto& renderTile) -> OverscaledTileID { return renderTile.get().getOverscaledTileID(); });

    std::unique_ptr<gfx::DrawableBuilder> fillBuilder;
    std::unique_ptr<gfx::DrawableBuilder> outlineBuilder;
    std::unique_ptr<gfx::DrawableBuilder> patternBuilder;
    std::unique_ptr<gfx::DrawableBuilder> outlinePatternBuilder;
    std::vector<gfx::DrawablePtr> newTiles;
    gfx::VertexAttributeArray fillVertexAttrs;
    gfx::VertexAttributeArray outlineVertexAttrs;
    gfx::VertexAttributeArray patternVertexAttrs;
    gfx::VertexAttributeArray patternOutlineVertexAttrs;

    const auto layerPrefix = getID() + "/";
    const auto renderPass = static_cast<RenderPass>(evaluatedProperties->renderPasses);

    const auto finish = [&](gfx::DrawableBuilder& builder,
                            const OverscaledTileID& tileID,
                            const FillInterpolateUBO& interpUBO,
                            const FillDrawableTilePropsUBO& tileUBO) {
        builder.flush();

        for (auto& drawable : builder.clearDrawables()) {
            drawable->setTileID(tileID);
            auto& uniforms = drawable->mutableUniformBuffers();
            uniforms.createOrUpdate(FillLayerTweaker::FillInterpolateUBOName, &interpUBO, context);
            uniforms.createOrUpdate(FillLayerTweaker::FillTilePropsUBOName, &tileUBO, context);
            tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
            ++stats.tileDrawablesAdded;
        }
    };

    const auto commonInit = [&](gfx::DrawableBuilder& builder) {
        builder.setColorAttrMode(gfx::DrawableBuilder::ColorAttrMode::None);
        builder.setCullFaceMode(gfx::CullFaceMode::disabled());
        builder.setNeedsStencil(true);
    };

    tileLayerGroup->observeDrawables([&](gfx::UniqueDrawable& drawable) {
        // If the render pass has changed or the tile has  dropped out of the cover set, remove it.
        const auto tileID = drawable->getTileID();
        if (drawable->getRenderPass() != renderPass || (tileID && newTileIDs.find(*tileID) == newTileIDs.end())) {
            drawable.reset();
            ++stats.tileDrawablesRemoved;
        }
    });

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
        if (!renderData) {
            removeTile(renderPass, tileID);
            continue;
        }

        auto& bucket = static_cast<FillBucket&>(*renderData->bucket);
        const auto& binders = bucket.paintPropertyBinders.at(getID());

        const auto& evaluated = getEvaluated<FillLayerProperties>(renderData->layerProperties);
        const auto& crossfade = getCrossfade<FillLayerProperties>(renderData->layerProperties);

        const auto& fillPatternValue = evaluated.get<FillPattern>().constantOr(Faded<expression::Image>{"", ""});
        const auto patternPosA = tile.getPattern(fillPatternValue.from.id());
        const auto patternPosB = tile.getPattern(fillPatternValue.to.id());
        binders.setPatternParameters(patternPosA, patternPosB, crossfade);

        const auto zoom = static_cast<float>(state.getZoom());
        const FillInterpolateUBO interpolateUBO = {
            /* .color_t = */ std::get<0>(binders.get<FillColor>()->interpolationFactor(zoom)),
            /* .opacity_t = */ std::get<0>(binders.get<FillOpacity>()->interpolationFactor(zoom)),
            /* .outline_color_t = */ std::get<0>(binders.get<FillOutlineColor>()->interpolationFactor(zoom)),
            /* .pattern_from_t = */ std::get<0>(binders.get<FillPattern>()->interpolationFactor(zoom)),
            /* .pattern_to_t = */ std::get<0>(binders.get<FillColor>()->interpolationFactor(zoom)),
            /* .fade = */ crossfade.t,
            /* .padding = */ {0},
        };

        // TODO: only update if properties re-evaluated?
        const FillDrawableTilePropsUBO tileProps = {
            /* pattern_from = */ patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},
            /* pattern_to = */ patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},
        };

        // If we already have drawables for this tile, update them.
        if (tileLayerGroup->getDrawableCount(renderPass, tileID) > 0) {
            // TODO: Share buffers
            tileLayerGroup->observeDrawables(renderPass, tileID, [&](gfx::Drawable& drawable) {
                drawable.mutableUniformBuffers().createOrUpdate(
                    FillLayerTweaker::FillInterpolateUBOName, &interpolateUBO, context);
                drawable.mutableUniformBuffers().createOrUpdate(
                    FillLayerTweaker::FillTilePropsUBOName, &tileProps, context);
            });

            continue;
        }

        std::vector<std::array<int16_t, 2>> rawVerts;
        const auto buildVertices = [&]() {
            const std::vector<gfx::VertexVector<gfx::detail::VertexType<gfx::AttributeType<int16_t, 2>>>::Vertex>&
                verts = bucket.vertices.vector();
            if (rawVerts.size() < verts.size()) {
                rawVerts.resize(verts.size());
                std::transform(verts.begin(), verts.end(), rawVerts.begin(), [](const auto& x) { return x.a1; });
            }
        };

        fillVertexAttrs.clear();
        outlineVertexAttrs.clear();

        // `Fill*Program` all use `style::FillPaintProperties`
        const auto fillUniformProps =
            fillVertexAttrs.readDataDrivenPaintProperties<FillColor, FillOpacity, FillOutlineColor, FillPattern>(
                binders, evaluated);
        const auto outlineUniformProps =
            outlineVertexAttrs.readDataDrivenPaintProperties<FillColor, FillOpacity, FillOutlineColor, FillPattern>(
                binders, evaluated);

        if (unevaluated.get<FillPattern>().isUndefined()) {
            // Fill will occur in opaque or translucent pass based on `opaquePassCutoff`.
            // Outline always occurs in translucent pass, defaults to fill color
            const auto doOutline = evaluated.get<FillAntialias>();

            const auto fillShader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                fillShaderGroup->getOrCreateShader(context, fillUniformProps));
            const auto outlineShader = doOutline
                                           ? std::static_pointer_cast<gfx::ShaderProgramBase>(
                                                 outlineShaderGroup->getOrCreateShader(context, outlineUniformProps))
                                           : nullptr;

            if (!fillBuilder && fillShader) {
                if (auto builder = context.createDrawableBuilder(layerPrefix + "fill")) {
                    commonInit(*builder);
                    builder->setDepthType((renderPass == RenderPass::Opaque) ? gfx::DepthMaskType::ReadWrite
                                                                             : gfx::DepthMaskType::ReadOnly);
                    builder->setSubLayerIndex(0);
                    builder->setRenderPass(renderPass);
                    fillBuilder = std::move(builder);
                }
            }
            if (doOutline && !outlineBuilder && outlineShader) {
                if (auto builder = context.createDrawableBuilder(layerPrefix + "fill-outline")) {
                    commonInit(*builder);
                    builder->setLineWidth(2.0f);
                    builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                    builder->setSubLayerIndex(unevaluated.get<FillOutlineColor>().isUndefined() ? 2 : 0);
                    builder->setRenderPass(RenderPass::Translucent);
                    outlineBuilder = std::move(builder);
                }
            }

            if (fillBuilder) {
                buildVertices();
                fillBuilder->setShader(fillShader);
                fillBuilder->setVertexAttributes(fillVertexAttrs);
                fillBuilder->addVertices(rawVerts, 0, rawVerts.size());
                fillBuilder->setSegments(gfx::Triangles(),
                                         bucket.triangles.vector(),
                                         reinterpret_cast<const std::vector<SegmentBase>&>(bucket.triangleSegments));
                finish(*fillBuilder, tileID, interpolateUBO, tileProps);
            }
            if (outlineBuilder) {
                buildVertices();
                outlineBuilder->setShader(outlineShader);
                outlineBuilder->setVertexAttributes(outlineVertexAttrs);
                outlineBuilder->addVertices(rawVerts, 0, rawVerts.size());
                outlineBuilder->setSegments(gfx::Lines(2),
                                            bucket.lines.vector(),
                                            reinterpret_cast<const std::vector<SegmentBase>&>(bucket.lineSegments));
                finish(*outlineBuilder, tileID, interpolateUBO, tileProps);
            }
        } else { // FillPattern is defined
            if ((renderPass & RenderPass::Translucent) == 0) {
                continue;
            }

            // Outline does not default to fill in the pattern case
            const auto doOutline = evaluated.get<FillAntialias>() && unevaluated.get<FillOutlineColor>().isUndefined();

            const auto fillShader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                patternShaderGroup->getOrCreateShader(context, fillUniformProps));
            const auto outlineShader = doOutline ? std::static_pointer_cast<gfx::ShaderProgramBase>(
                                                       outlinePatternShaderGroup->getOrCreateShader(
                                                           context, outlineUniformProps))
                                                 : nullptr;

            if (!patternBuilder) {
                if (auto builder = context.createDrawableBuilder(layerPrefix + "fill-pattern")) {
                    commonInit(*builder);
                    builder->setDepthType(gfx::DepthMaskType::ReadWrite);
                    builder->setSubLayerIndex(1);
                    builder->setRenderPass(RenderPass::Translucent);
                    patternBuilder = std::move(builder);
                }
            }
            if (doOutline && !outlinePatternBuilder) {
                if (auto builder = context.createDrawableBuilder(layerPrefix + "fill-outline-pattern")) {
                    commonInit(*builder);
                    builder->setLineWidth(2.0f);
                    builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                    builder->setSubLayerIndex(2);
                    builder->setRenderPass(RenderPass::Translucent);
                    outlinePatternBuilder = std::move(builder);
                }
            }

            if (patternBuilder) {
                buildVertices();
                patternBuilder->setShader(fillShader);
                patternBuilder->setRenderPass(renderPass);
                patternBuilder->setVertexAttributes(fillVertexAttrs);
                patternBuilder->addVertices(rawVerts, 0, rawVerts.size());
                patternBuilder->setSegments(
                    gfx::Triangles(),
                    bucket.triangles.vector(),
                    reinterpret_cast<const std::vector<SegmentBase>&>(bucket.triangleSegments));

                if (const auto& atlases = tile.getAtlasTextures()) {
                    if (const auto samplerLocation = fillShader->getSamplerLocation("u_image")) {
                        patternBuilder->setTextureSource(*samplerLocation, [=]() { return atlases->icon; });
                    }
                }

                finish(*patternBuilder, tileID, interpolateUBO, tileProps);
            }
            if (outlinePatternBuilder) {
                buildVertices();
                outlinePatternBuilder->setShader(outlineShader);
                outlinePatternBuilder->setRenderPass(renderPass);
                outlinePatternBuilder->setVertexAttributes(outlineVertexAttrs);
                outlinePatternBuilder->addVertices(rawVerts, 0, rawVerts.size());
                outlinePatternBuilder->setSegments(
                    gfx::Lines(2),
                    bucket.lines.vector(),
                    reinterpret_cast<const std::vector<SegmentBase>&>(bucket.lineSegments));

                if (const auto& atlases = tile.getAtlasTextures()) {
                    if (const auto samplerLocation = outlineShader->getSamplerLocation("u_image")) {
                        outlinePatternBuilder->setTextureSource(*samplerLocation, [=]() { return atlases->icon; });
                    }
                }

                finish(*outlinePatternBuilder, tileID, interpolateUBO, tileProps);
            }
        }
    }
}

} // namespace mbgl
