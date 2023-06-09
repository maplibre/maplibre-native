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
#include <mbgl/style/expression/image.hpp>
#include <mbgl/style/layers/fill_layer_impl.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/intersection_tests.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/math.hpp>

namespace mbgl {

using namespace style;

namespace {

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
                             const TransformState& /*state*/,
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

    if (!fillShader) {
        fillShader = context.getGenericShader(shaders, "FillShader");
    }
    if (!outlineShader) {
        outlineShader = context.getGenericShader(shaders, "FillOutlineShader");
    }
    if (!patternShader) {
        patternShader = context.getGenericShader(shaders, "FillPatternShader");
    }
    if (!outlinePatternShader) {
        outlinePatternShader = context.getGenericShader(shaders, "FillOutlinePatternShader");
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

    const auto finish = [&](gfx::DrawableBuilder& builder, RenderPass pass, const OverscaledTileID& tileID) {
        builder.flush();

        for (auto& drawable : builder.clearDrawables()) {
            drawable->setTileID(tileID);
            tileLayerGroup->addDrawable(pass, tileID, std::move(drawable));
            ++stats.tileDrawablesAdded;
        }
    };

    const auto commonInit = [](gfx::DrawableBuilder& builder) {
        builder.setColorAttrMode(gfx::DrawableBuilder::ColorAttrMode::None);
        builder.setCullFaceMode(gfx::CullFaceMode::disabled());
        builder.setNeedsStencil(true);
    };

    for (const auto renderPass : {RenderPass::Opaque, RenderPass::Translucent}) {
        if (!(mbgl::underlying_type(renderPass) & evaluatedProperties->renderPasses)) {
            continue;
        }

        tileLayerGroup->observeDrawables([&](gfx::UniqueDrawable& drawable) {
            // Has this tile dropped out of the cover set?
            const auto tileID = drawable->getTileID();
            if (tileID && newTileIDs.find(*tileID) == newTileIDs.end()) {
                // remove it
                drawable.reset();
                ++stats.tileDrawablesRemoved;
            }
        });

        constexpr auto samplerLocation = 0;
        const auto layerPrefix = getID() + "/";

        for (const RenderTile& tile : *renderTiles) {
            const auto& tileID = tile.getOverscaledTileID();

            // If we already have drawables for this tile, skip.
            // If a drawable needs to be updated, that's handled in the layer tweaker.
            if (tileLayerGroup->getDrawableCount(renderPass, tileID) > 0) {
                continue;
            }

            const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
            if (!renderData) {
                removeTile(renderPass, tileID);
                continue;
            }

            auto& bucket = static_cast<FillBucket&>(*renderData->bucket);
            const auto& evaluated = getEvaluated<FillLayerProperties>(renderData->layerProperties);
            const auto& crossfade = getCrossfade<FillLayerProperties>(renderData->layerProperties);

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

            const auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());

            if (unevaluated.get<FillPattern>().isUndefined()) {
                const auto evalColor = evaluated.get<FillColor>().constantOr(Color());
                const auto fillOpacity = evaluated.get<FillOpacity>().constantOr(0);
                const auto fillColor = evalColor * (fillOpacity > 0.0f ? fillOpacity : 1.0f);
                const auto fillRenderPass = (fillColor.a >= 1.0f
                                             /* && parameters.currentLayer >= parameters.opaquePassCutoff*/)
                                                ? RenderPass::Opaque
                                                : RenderPass::Translucent;
                const auto doFill = fillRenderPass == renderPass;
                const auto doOutline = evaluated.get<FillAntialias>() && renderPass == RenderPass::Translucent;

                if (!doFill && !doOutline) {
                    removeTile(renderPass, tileID);
                    continue;
                }

                if (doFill) {
                    if (auto& binder = paintPropertyBinders.get<FillColor>()) {
                        const auto count = binder->getVertexCount();
                        // check that vertexVector.elements() == sum(segments.vertexLength)
                        if (auto& attr = fillVertexAttrs.getOrAdd("a_color")) {
                            for (std::size_t i = 0; i < count; ++i) {
                                const auto& packed = std::get<0>(binder->getVertexValue(i)).a1;
                                attr->set(i, packed);
                            }
                        }
                    }
                }
                if (doOutline) {
                    if (auto& binder = paintPropertyBinders.get<FillOutlineColor>()) {
                        const auto count = binder->getVertexCount();
                        if (auto& attr = outlineVertexAttrs.getOrAdd("a_outline_color")) {
                            for (std::size_t i = 0; i < count; ++i) {
                                const auto& packed = std::get<0>(binder->getVertexValue(i)).a1;
                                attr->set(i, packed);
                            }
                        }
                    }
                }
                if (auto& binder = paintPropertyBinders.get<FillOpacity>()) {
                    const auto count = binder->getVertexCount();
                    for (auto& attrs :
                         {std::reference_wrapper(fillVertexAttrs), std::reference_wrapper(outlineVertexAttrs)}) {
                        if (auto& attr = attrs.get().getOrAdd("a_opacity")) {
                            for (std::size_t i = 0; i < count; ++i) {
                                const auto& opacity = std::get<0>(binder->getVertexValue(i)).a1;
                                attr->set(i, opacity);
                            }
                        }
                    }
                }

                if (doFill && !fillBuilder && fillShader) {
                    if (auto builder = context.createDrawableBuilder(layerPrefix + "fill")) {
                        commonInit(*builder);
                        builder->setShader(fillShader);
                        builder->setDepthType((renderPass == RenderPass::Opaque) ? gfx::DepthMaskType::ReadWrite
                                                                                 : gfx::DepthMaskType::ReadOnly);
                        builder->setSubLayerIndex(0);
                        fillBuilder = std::move(builder);
                    }
                }
                if (doOutline && !outlineBuilder && outlineShader) {
                    if (auto builder = context.createDrawableBuilder(layerPrefix + "fill-outline")) {
                        commonInit(*builder);
                        builder->setShader(outlineShader);
                        builder->setLineWidth(2.0f);
                        builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                        builder->setSubLayerIndex(unevaluated.get<FillOutlineColor>().isUndefined() ? 2 : 0);
                        outlineBuilder = std::move(builder);
                    }
                }

                if (fillBuilder) {
                    buildVertices();
                    fillBuilder->setRenderPass(renderPass);
                    fillBuilder->setVertexAttributes(fillVertexAttrs);
                    fillBuilder->addVertices(rawVerts, 0, rawVerts.size());
                    fillBuilder->setSegments(
                        gfx::Triangles(),
                        bucket.triangles.vector(),
                        reinterpret_cast<const std::vector<Segment<void>>&>(bucket.triangleSegments));
                    finish(*fillBuilder, renderPass, tileID);
                }
                if (outlineBuilder) {
                    buildVertices();
                    outlineBuilder->setRenderPass(renderPass);
                    outlineBuilder->setVertexAttributes(outlineVertexAttrs);
                    outlineBuilder->addVertices(rawVerts, 0, rawVerts.size());
                    outlineBuilder->setSegments(
                        gfx::Lines(2),
                        bucket.lines.vector(),
                        reinterpret_cast<const std::vector<Segment<void>>&>(bucket.lineSegments));
                    finish(*outlineBuilder, renderPass, tileID);
                }
            } else { // FillPattern is defined
                if (renderPass != RenderPass::Translucent) {
                    continue;
                }

                const auto doOutline = evaluated.get<FillAntialias>() &&
                                       unevaluated.get<FillOutlineColor>().isUndefined();
                const auto& fillPatternValue = evaluated.get<FillPattern>().constantOr(
                    Faded<expression::Image>{"", ""});
                const auto patternPosA = tile.getPattern(fillPatternValue.from.id());
                const auto patternPosB = tile.getPattern(fillPatternValue.to.id());
                paintPropertyBinders.setPatternParameters(patternPosA, patternPosB, crossfade);

                // if (samplerLocation < 0) curShader->getSamplerLocation("u_image");

                if (auto& binder = paintPropertyBinders.get<FillOpacity>()) {
                    const auto count = binder->getVertexCount();
                    for (auto& attrs : {std::reference_wrapper(patternVertexAttrs),
                                        std::reference_wrapper(patternOutlineVertexAttrs)}) {
                        if (auto& attr = attrs.get().getOrAdd("a_opacity")) {
                            for (std::size_t i = 0; i < count; ++i) {
                                const auto& opacity = std::get<0>(binder->getVertexValue(i)).a1;
                                attr->set(i, opacity);
                            }
                        }
                    }
                }
                //                if (auto& binder = paintPropertyBinders.get<FillOpacity>()) {
                //                    const auto count = binder->getVertexCount();
                //                    for (auto& attrs :
                //                         {std::reference_wrapper(patternVertexAttrs),
                //                         std::reference_wrapper(patternOutlineVertexAttrs)}) {
                //                        if (auto& attr = attrs.get().getOrAdd("a_pattern_from")) {
                //                            for (std::size_t i = 0; i < count; ++i) {
                //                                const auto& opacity =
                //                                    static_cast<const
                //                                    gfx::detail::VertexType<gfx::AttributeType<float, 4>>*>(
                //                                        binder->getVertexValue(i))
                //                                        ->a1;
                //                                attr->set<gfx::VertexAttribute::float2>(i, {});
                //                            }
                //                        }
                //                    }
                //                }

                if (!patternBuilder && patternShader) {
                    if (auto builder = context.createDrawableBuilder(layerPrefix + "fill-pattern")) {
                        commonInit(*builder);
                        builder->setShader(fillShader);
                        builder->setDepthType(gfx::DepthMaskType::ReadWrite);
                        builder->setSubLayerIndex(1);
                        if (auto& tex = tile.getIconAtlasTexture()) {
                            builder->setTexture(tex, samplerLocation);
                        }
                        patternBuilder = std::move(builder);
                    }
                }
                if (doOutline && !outlinePatternBuilder && outlinePatternShader) {
                    if (auto builder = context.createDrawableBuilder(layerPrefix + "fill-outline-pattern")) {
                        commonInit(*builder);
                        builder->setShader(outlineShader);
                        builder->setLineWidth(2.0f);
                        builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                        builder->setSubLayerIndex(2);
                        if (auto& tex = tile.getIconAtlasTexture()) {
                            builder->setTexture(tex, samplerLocation);
                        }
                        outlinePatternBuilder = std::move(builder);
                    }
                }

                if (patternBuilder) {
                    buildVertices();
                    patternBuilder->setRenderPass(renderPass);
                    patternBuilder->setVertexAttributes(fillVertexAttrs);
                    patternBuilder->addVertices(rawVerts, 0, rawVerts.size());
                    patternBuilder->setSegments(
                        gfx::Triangles(),
                        bucket.triangles.vector(),
                        reinterpret_cast<const std::vector<Segment<void>>&>(bucket.triangleSegments));
                    finish(*patternBuilder, renderPass, tileID);
                }
                if (outlinePatternBuilder) {
                    buildVertices();
                    outlinePatternBuilder->setRenderPass(renderPass);
                    outlinePatternBuilder->setVertexAttributes(outlineVertexAttrs);
                    outlinePatternBuilder->addVertices(rawVerts, 0, rawVerts.size());
                    outlinePatternBuilder->setSegments(
                        gfx::Lines(2),
                        bucket.lines.vector(),
                        reinterpret_cast<const std::vector<Segment<void>>&>(bucket.lineSegments));
                    finish(*outlinePatternBuilder, renderPass, tileID);
                }
            }
        }
    }
}

} // namespace mbgl
