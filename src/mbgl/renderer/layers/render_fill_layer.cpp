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
#include <mbgl/renderer/layers/render_fill_layer.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/style/layers/fill_layer_impl.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/intersection_tests.hpp>
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
}

bool RenderFillLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderFillLayer::hasCrossfade() const {
    return getCrossfade<FillLayerProperties>(evaluatedProperties).t != 1;
}

void RenderFillLayer::render(PaintParameters& parameters) {
    assert(renderTiles);

    if (!parameters.shaders.populate(fillProgram)) return;
    if (!parameters.shaders.populate(fillPatternProgram)) return;
    if (!parameters.shaders.populate(fillOutlineProgram)) return;
    if (!parameters.shaders.populate(fillOutlinePatternProgram)) return;

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
                        tile.getIconAtlasTexture().size,
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
                         textures::image::Value{tile.getIconAtlasTexture().getResource(),
                                                gfx::TextureFilterType::Linear},
                     });
            }
            if (evaluated.get<FillAntialias>() && unevaluated.get<FillOutlineColor>().isUndefined()) {
                draw(*fillOutlinePatternProgram,
                     gfx::Lines{2.0f},
                     parameters.depthModeForSublayer(2, gfx::DepthMaskType::ReadOnly),
                     *bucket.lineIndexBuffer,
                     bucket.lineSegments,
                     FillOutlinePatternProgram::TextureBindings{
                         textures::image::Value{tile.getIconAtlasTexture().getResource(),
                                                gfx::TextureFilterType::Linear},
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

constexpr auto shaderName = "background_generic";

void RenderFillLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    // TODO: This isn't happening on style change, so old tile drawables are being left active

    // Remove everything
    decltype(tileDrawables) localDrawables;
    {
        std::unique_lock<std::mutex> guard(mutex);
        localDrawables = std::move(tileDrawables);
    }

    removeDrawables<decltype(localDrawables)::const_iterator>(
        localDrawables.cbegin(), localDrawables.cend(), changes, [](auto& ii) { return ii->second->getId(); });
}

void RenderFillLayer::update(const int32_t layerIndex,
                             gfx::ShaderRegistry& shaders,
                             gfx::Context& context,
                             const TransformState& /*state*/,
                             UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

    if (!shader) {
        shader = context.getGenericShader(shaders, shaderName);
    }

    // const auto& evaluated = getEvaluated<FillLayerProperties>(evaluatedProperties);
    //    std::optional<Color> color;
    //    if (evaluated.get<FillPattern>().from.empty()) {
    //        const auto opacity = evaluated.get<style::FillOpacity>();
    //        if (opacity > 0.0f) {
    //            color = evaluated.get<FillColor>() * evaluated.get<FillOpacity>();
    //        }
    //    }
    //
    //    // If the result is transparent or missing, just remove any existing drawables and stop
    //    if (!color) {
    //        removeDrawables<decltype(tileDrawables)::const_iterator>(
    //            tileDrawables.cbegin(), tileDrawables.cend(), changes, [](auto& ii) { return ii->second->getId(); });
    //        tileDrawables.clear();
    //        return;
    //    }
    //
    //    const bool colorChange = (color != lastColor);
    //    const bool layerChange = (layerIndex != lastLayerIndex);
    //    lastColor = color;
    //    lastLayerIndex = layerIndex;
    //
    //    const auto zoom = state.getIntegerZoom();
    //    const auto tileCover = util::tileCover(state, zoom);
    //
    //    // Drawables per overscaled or canonical tile?
    //    // const UnwrappedTileID unwrappedTileID = tileID.toUnwrapped();
    //
    //    // Put the tile cover into a searchable form.
    //    // TODO: Likely better to sort and `std::binary_search` the vector.
    //    // If it's returned in a well-defined order, we might not even need to sort.
    //    const std::unordered_set<OverscaledTileID> newTileIDs(tileCover.begin(), tileCover.end());
    //
    //    // For each existing tile drawable...
    //    for (auto iter = tileDrawables.begin(); iter != tileDrawables.end();) {
    //        const auto& drawable = iter->second;
    //
    //        // Has this tile dropped out of the cover set?
    //        if (newTileIDs.find(iter->first) == newTileIDs.end()) {
    //            // remove it
    //            changes.emplace_back(std::make_unique<RemoveDrawableRequest>(drawable->getId()));
    //            // Log::Warning(Event::General, "Removing drawable for " + util::toString(iter->first) + " total " +
    //            // std::to_string(stats.tileDrawablesRemoved+1));
    //            iter = tileDrawables.erase(iter);
    //            ++stats.tileDrawablesRemoved;
    //            continue;
    //        }
    //        ++iter;
    //
    //        // If the color evaluated to a new value, update all vertexes of the drawable to the new color
    //        if (colorChange) {
    //            drawable->resetColor(*color);
    //        }
    //        if (layerChange) {
    //            drawable->setLayerIndex(layerIndex);
    //        }
    //    }
    //
    //    std::unique_ptr<gfx::DrawableBuilder> builder;
    //
    //    // For each tile in the cover set, add a tile drawable if one doesn't already exist.
    //    // We currently assume only one drawable per tile.
    //    for (const auto& tileID : tileCover) {
    //        const auto result = tileDrawables.insert(std::make_pair(tileID, gfx::DrawablePtr()));
    //        if (!result.second) {
    //            // Already present
    //            // TODO: Update matrix here or in the tweaker?
    //            continue;
    //        }
    //
    //        // We actually need to build things, so set up a builder if we haven't already
    //        if (!builder) {
    //            builder = context.createDrawableBuilder("background");
    //            builder->setShader(shader);
    //            builder->addTweaker(context.createDrawableTweaker());
    //            builder->setColor(*color);
    //            builder->setColorMode(gfx::DrawableBuilder::ColorMode::PerDrawable);
    //            builder->setDepthType(gfx::DepthMaskType::ReadWrite);
    //            builder->setLayerIndex(layerIndex);
    //        }
    //
    //        // Tile coordinates are fixed...
    //        builder->addQuad(0, 0, util::EXTENT, util::EXTENT);
    //
    //        // ... they're placed with the matrix in the uniforms, which changes with the view
    //        builder->setMatrix(/*parameters.matrixForTile(tileID.toUnwrapped())*/ matrix::identity4());
    //
    //        builder->flush();
    //
    //        auto drawables = builder->clearDrawables();
    //        if (!drawables.empty()) {
    //            auto& drawable = drawables[0];
    //            drawable->setTileID(tileID);
    //            result.first->second = drawable;
    //            changes.emplace_back(std::make_unique<AddDrawableRequest>(std::move(drawable)));
    //            ++stats.tileDrawablesAdded;
    //            // Log::Warning(Event::General, "Adding drawable for " + util::toString(tileID) + " total " +
    //            // std::to_string(stats.tileDrawablesAdded+1));
    //        }
    //    }

    std::unordered_set<OverscaledTileID> newTileIDs(renderTiles->size());
    std::transform(renderTiles->begin(),
                   renderTiles->end(),
                   std::inserter(newTileIDs, newTileIDs.begin()),
                   [](const auto& renderTile) -> OverscaledTileID { return renderTile.get().getOverscaledTileID(); });

    // For each existing tile drawable...
    for (auto iter = tileDrawables.begin(); iter != tileDrawables.end();) {
        const auto& drawable = iter->second;

        // Has this tile dropped out of the cover set?
        if (newTileIDs.find(iter->first) == newTileIDs.end()) {
            // remove it
            if (drawable) {
                changes.emplace_back(std::make_unique<RemoveDrawableRequest>(drawable->getId()));
            }
            // Log::Warning(Event::General, "Removing drawable for " + util::toString(iter->first) + " total " +
            // std::to_string(stats.tileDrawablesRemoved+1));
            iter = tileDrawables.erase(iter);
            ++stats.tileDrawablesRemoved;
            continue;
        }
        ++iter;

        // update...
    }

    std::unique_ptr<gfx::DrawableBuilder> builder;

    if (unevaluated.get<FillPattern>().isUndefined()) {
        //        parameters.renderTileClippingMasks(renderTiles);
        for (const RenderTile& tile : *renderTiles) {
            const auto& tileID = tile.getOverscaledTileID();

            const auto hit = tileDrawables.find(tileID);

            const auto renderPass = RenderPass::Translucent;
            const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
            if (!renderData) {
                // Remove the tile if it was previously present
                if (hit != tileDrawables.end()) {
                    if (hit->second) {
                        changes.emplace_back(std::make_unique<RemoveDrawableRequest>(hit->second->getId()));
                    }
                    tileDrawables.erase(hit);
                    ++stats.tileDrawablesRemoved;
                }
                continue;
            }

            if (hit != tileDrawables.end()) {
                // already present
                continue;
            }

            auto& bucket = static_cast<FillBucket&>(*renderData->bucket);
            const auto& evaluated = getEvaluated<FillLayerProperties>(renderData->layerProperties);

            if (!builder) {
                builder = context.createDrawableBuilder("fill");
                builder->setShader(shader);
                builder->addTweaker(context.createDrawableTweaker());
                builder->setColorMode(gfx::DrawableBuilder::ColorMode::PerDrawable);
                builder->setDepthType(gfx::DepthMaskType::ReadWrite);
                builder->setLayerIndex(layerIndex);
            }

            const auto fillRenderPass = (evaluated.get<FillColor>().constantOr(Color()).a >= 1.0f &&
                                         evaluated.get<FillOpacity>().constantOr(0) >= 1.0f
                                         /* && parameters.currentLayer >= parameters.opaquePassCutoff*/)
                                            ? RenderPass::Opaque
                                            : RenderPass::Translucent;
            builder->setRenderPass(fillRenderPass);

            const std::vector<gfx::VertexVector<gfx::detail::VertexType<gfx::AttributeType<int16_t, 2>>>::Vertex>&
                verts = bucket.vertices.vector();
            std::vector<std::array<int16_t, 2>> rawVerts(verts.size());
            std::transform(verts.begin(), verts.end(), rawVerts.begin(), [](const auto& x) { return x.a1; });

            for (const auto& seg : bucket.triangleSegments) {
                builder->addTriangles(rawVerts,
                                      seg.vertexOffset,
                                      seg.vertexLength,
                                      bucket.triangles.vector(),
                                      seg.indexOffset,
                                      seg.indexLength);
            }

            //            evaluated.get<FillTranslate>(),
            //            evaluated.get<FillTranslateAnchor>(),
            //            parameters.stencilModeForClipping(tile.id),
            //            parameters.colorModeForRenderPass(),
            //            gfx::CullFaceMode::disabled(),
            //
            //            const auto isOpaque = (evaluated.get<FillColor>().constantOr(Color()).a >= 1.0f &&
            //                                   evaluated.get<FillOpacity>().constantOr(0) >= 1.0f
            //                                   );// && layerIndex >= parameters.opaquePassCutoff);
            //            const auto fillRenderPass = isOpaque ? RenderPass::Opaque : RenderPass::Translucent;
            //
            //            if (renderPass == fillRenderPass) {
            //            }

            // const auto depthMask = (renderPass == RenderPass::Opaque) ? gfx::DepthMaskType::ReadWrite
            //                                                           : gfx::DepthMaskType::ReadOnly;

            //            const auto depthMode = parameters.depthModeForSublayer(1, depthMask);

            //            if (evaluated.get<FillAntialias>() && parameters.pass == RenderPass::Translucent) {
            //                draw(*fillOutlineProgram,
            //                     gfx::Lines{2.0f},
            //                     parameters.depthModeForSublayer(unevaluated.get<FillOutlineColor>().isUndefined() ? 2
            //                     : 0,
            //                                                     gfx::DepthMaskType::ReadOnly),
            //                     *bucket.lineIndexBuffer,
            //                     bucket.lineSegments,
            //                     FillOutlineProgram::TextureBindings{});
            //            }

            builder->flush();

            auto newDrawables = builder->clearDrawables();
            if (!newDrawables.empty()) {
                auto& drawable = newDrawables[0];
                drawable->setTileID(tileID);

                // Track it.
#if !defined(NDEBUG)
                const auto result =
#endif
                tileDrawables.insert(std::make_pair(tileID, drawable));
                // This should always insert because we checked previously.
                assert(result.second);

                changes.emplace_back(std::make_unique<AddDrawableRequest>(std::move(drawable)));
                ++stats.tileDrawablesAdded;
                // Log::Warning(Event::General, "Adding drawable for " + util::toString(tileID) + " total " +
                // std::to_string(stats.tileDrawablesAdded+1));
            }
        }
    } else {
        //        if (parameters.pass != RenderPass::Translucent) {
        //            return;
        //        }
        //
        //        parameters.renderTileClippingMasks(renderTiles);
        //
        //        for (const RenderTile& tile : *renderTiles) {
        //            const LayerRenderData* renderData = getRenderDataForPass(tile, parameters.pass);
        //            if (!renderData) {
        //                continue;
        //            }
        //            auto& bucket = static_cast<FillBucket&>(*renderData->bucket);
        //            const auto& evaluated = getEvaluated<FillLayerProperties>(renderData->layerProperties);
        //            const auto& crossfade = getCrossfade<FillLayerProperties>(renderData->layerProperties);
        //
        //            const auto& fillPatternValue =
        //            evaluated.get<FillPattern>().constantOr(Faded<expression::Image>{"", ""});
        //            std::optional<ImagePosition> patternPosA = tile.getPattern(fillPatternValue.from.id());
        //            std::optional<ImagePosition> patternPosB = tile.getPattern(fillPatternValue.to.id());
        //
        //            auto draw = [&](auto& programInstance,
        //                            const auto& drawMode,
        //                            const auto& depthMode,
        //                            const auto& indexBuffer,
        //                            const auto& segments,
        //                            auto&& textureBindings) {
        //                const auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());
        //                paintPropertyBinders.setPatternParameters(patternPosA, patternPosB, crossfade);
        //
        //                const auto allUniformValues = programInstance.computeAllUniformValues(
        //                    FillPatternProgram::layoutUniformValues(
        //                        tile.translatedMatrix(
        //                            evaluated.get<FillTranslate>(), evaluated.get<FillTranslateAnchor>(),
        //                            parameters.state),
        //                        parameters.backend.getDefaultRenderable().getSize(),
        //                        tile.getIconAtlasTexture().size,
        //                        crossfade,
        //                        tile.id,
        //                        parameters.state,
        //                        parameters.pixelRatio),
        //                    paintPropertyBinders,
        //                    evaluated,
        //                    static_cast<float>(parameters.state.getZoom()));
        //                const auto allAttributeBindings = programInstance.computeAllAttributeBindings(
        //                    *bucket.vertexBuffer, paintPropertyBinders, evaluated);
        //
        //                checkRenderability(parameters, programInstance.activeBindingCount(allAttributeBindings));
        //
        //                programInstance.draw(parameters.context,
        //                                     *parameters.renderPass,
        //                                     drawMode,
        //                                     depthMode,
        //                                     parameters.stencilModeForClipping(tile.id),
        //                                     parameters.colorModeForRenderPass(),
        //                                     gfx::CullFaceMode::disabled(),
        //                                     indexBuffer,
        //                                     segments,
        //                                     allUniformValues,
        //                                     allAttributeBindings,
        //                                     std::forward<decltype(textureBindings)>(textureBindings),
        //                                     getID());
        //            };
        //
        //            if (bucket.triangleIndexBuffer) {
        //                draw(*fillPatternProgram,
        //                     gfx::Triangles(),
        //                     parameters.depthModeForSublayer(1, gfx::DepthMaskType::ReadWrite),
        //                     *bucket.triangleIndexBuffer,
        //                     bucket.triangleSegments,
        //                     FillPatternProgram::TextureBindings{
        //                         textures::image::Value{tile.getIconAtlasTexture().getResource(),
        //                                                gfx::TextureFilterType::Linear},
        //                     });
        //            }
        //            if (evaluated.get<FillAntialias>() && unevaluated.get<FillOutlineColor>().isUndefined()) {
        //                draw(*fillOutlinePatternProgram,
        //                     gfx::Lines{2.0f},
        //                     parameters.depthModeForSublayer(2, gfx::DepthMaskType::ReadOnly),
        //                     *bucket.lineIndexBuffer,
        //                     bucket.lineSegments,
        //                     FillOutlinePatternProgram::TextureBindings{
        //                         textures::image::Value{tile.getIconAtlasTexture().getResource(),
        //                                                gfx::TextureFilterType::Linear},
        //                     });
        //            }
        //        }
    }
}

} // namespace mbgl
