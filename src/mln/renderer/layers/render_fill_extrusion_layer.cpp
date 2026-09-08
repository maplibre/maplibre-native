#include <mln/renderer/layers/render_fill_extrusion_layer.hpp>

#include <mln/geometry/feature_index.hpp>
#include <mln/gfx/cull_face_mode.hpp>
#include <mln/gfx/render_pass.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/gfx/shader_registry.hpp>
#include <mln/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mln/renderer/image_manager.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/render_static_data.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/tile_render_data.hpp>
#include <mln/style/expression/image.hpp>
#include <mln/style/layers/fill_extrusion_layer_impl.hpp>
#include <mln/tile/geometry_tile.hpp>
#include <mln/tile/tile.hpp>
#include <mln/util/intersection_tests.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/math.hpp>

#include <chrono>
#include <cstdlib>

#include <mln/gfx/drawable_atlases_tweaker.hpp>
#include <mln/gfx/drawable_builder.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/layers/fill_extrusion_layer_tweaker.hpp>
#include <mln/renderer/update_parameters.hpp>
#include <mln/shaders/fill_extrusion_layer_ubo.hpp>
#include <mln/shaders/shader_program_base.hpp>

#if MLN_USE_FILL_EXTRUSION_INSTANCING
#include <mln/renderer/layers/fill_extrusion_shadow_layer_tweaker.hpp>
#include <mln/shaders/fill_extrusion_shadow_layer_ubo.hpp>
#endif

namespace mln {

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
    properties->renderPasses = mln::underlying_type(passes);
    evaluatedProperties = std::move(properties);

    if (layerTweaker) {
        layerTweaker->updateProperties(evaluatedProperties);
    }
#if MLN_USE_FILL_EXTRUSION_INSTANCING
    // The shadow has its own tweaker, on its own layer group. Without this it keeps whatever
    // properties it was constructed with, and changing the shadow colour, length, azimuth or
    // opacity on a layer that already has a shadow would have no effect.
    if (shadowTweaker) {
        shadowTweaker->updateProperties(evaluatedProperties);
    }
#endif
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

void RenderFillExtrusionLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
#if MLN_USE_FILL_EXTRUSION_INSTANCING
    // The shadow group has to be registered before `layerGroup`. Layer groups live in a
    // std::multimap keyed by layer index, and insert() places equivalent keys at the upper bound of
    // their range, so insertion order decides draw order within one index. That is what puts the
    // shadow underneath the buildings.
    isRenderable = willRender;
    activateLayerGroup(shadowGroup, willRender, changes);
    activateLayerGroup(layerGroup, willRender, changes);
#else
    RenderLayer::markLayerRenderable(willRender, changes);
#endif
}

void RenderFillExtrusionLayer::layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec& changes) {
#if MLN_USE_FILL_EXTRUSION_INSTANCING
    // Same ordering rule as above: the re-insert must keep the shadow ahead of the buildings.
    layerIndex = newLayerIndex;
    changeLayerIndex(shadowGroup, newLayerIndex, changes);
    changeLayerIndex(layerGroup, newLayerIndex, changes);
#else
    RenderLayer::layerIndexChanged(newLayerIndex, changes);
#endif
}

std::size_t RenderFillExtrusionLayer::removeAllDrawables() {
    auto removed = RenderLayer::removeAllDrawables();

#if MLN_USE_FILL_EXTRUSION_INSTANCING
    if (shadowGroup) {
        const auto count = shadowGroup->getDrawableCount();
        removed += count;
        stats.drawablesRemoved += count;
        shadowGroup->clearDrawables();
    }
#endif

    return removed;
}

#if MLN_USE_FILL_EXTRUSION_INSTANCING

bool RenderFillExtrusionLayer::shadowEnabled() const {
    const auto& evaluated = static_cast<const FillExtrusionLayerProperties&>(*evaluatedProperties).evaluated;
    return evaluated.get<FillExtrusionShadowOpacity>() > 0.0f && evaluated.get<FillExtrusionShadowLength>() > 0.0f;
}

void RenderFillExtrusionLayer::teardownShadow(UniqueChangeRequestVec& changes) {
    if (shadowGroup) {
        activateLayerGroup(shadowGroup, false, changes);
        shadowGroup.reset();
    }
    shadowTweaker.reset();
}

bool RenderFillExtrusionLayer::prepareShadow(gfx::ShaderRegistry& shaders,
                                             gfx::Context& context,
                                             UniqueChangeRequestVec& changes) {
    if (!shadowMaskShaderGroup) {
        shadowMaskShaderGroup = shaders.getShaderGroup("FillExtrusionShadowMaskShader");
    }
    if (!shadowMaskInstancedShaderGroup) {
        shadowMaskInstancedShaderGroup = shaders.getShaderGroup("FillExtrusionShadowMaskInstancedShader");
    }
    // Backends without shadow shader specializations land here and quietly stay disabled.
    if (!shadowMaskShaderGroup || !shadowMaskInstancedShaderGroup) {
        return false;
    }

    // Created first, so it sorts ahead of the buildings at the same layer index.
    if (!shadowGroup) {
        auto group = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID() + "-shadow");
        if (!group) {
            return false;
        }
        shadowGroup = std::move(group);
        activateLayerGroup(shadowGroup, isRenderable, changes);

        // If the buildings were already registered -- which is the case whenever the shadow is
        // switched on at runtime rather than at style load -- re-insert them so they land after the
        // shadow group again. Without this the shadow would draw over the buildings.
        if (layerGroup && isRenderable) {
            activateLayerGroup(layerGroup, false, changes);
            activateLayerGroup(layerGroup, true, changes);
        }
    }

    if (!shadowTweaker) {
        shadowTweaker = std::make_shared<FillExtrusionShadowLayerTweaker>(getID(), evaluatedProperties);
        shadowGroup->addLayerTweaker(shadowTweaker);
    }

    return true;
}

/// Averages the shadow's CPU cost over a window of frames and logs it, so a one-off hitch does not
/// look like a steady regression. Off unless MLN_SHADOW_STATS is set in the environment.
void RenderFillExtrusionLayer::reportShadowStats(double setupMs) {
    static const bool enabled = (::getenv("MLN_SHADOW_STATS") != nullptr);
    if (!enabled) {
        return;
    }

    constexpr std::uint64_t window = 60;
    shadowSetupMsAccum += setupMs;
    if (++shadowFrameCount < window) {
        return;
    }

    const auto drawables = shadowGroup ? shadowGroup->getDrawableCount() : 0;

    Log::Info(Event::Render,
              "fill-extrusion shadow: " + std::to_string(drawables) + " drawables, cpu setup " +
                  std::to_string(shadowSetupMsAccum / static_cast<double>(window)) + " ms/frame (avg over " +
                  std::to_string(window) + " frames)");

    shadowFrameCount = 0;
    shadowSetupMsAccum = 0.0;
}

#endif // MLN_USE_FILL_EXTRUSION_INSTANCING

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
                                      [[maybe_unused]] const TransformState& state,
                                      const std::shared_ptr<UpdateParameters>&,
                                      [[maybe_unused]] const PaintParameters& paintParameters,
                                      const RenderTree&,
                                      UniqueChangeRequestVec& changes) {
    if (!renderTiles || renderTiles->empty() || passes == RenderPass::None) {
        removeAllDrawables();
#if MLN_USE_FILL_EXTRUSION_INSTANCING
        teardownShadow(changes);
#endif
        return;
    }

#if MLN_USE_FILL_EXTRUSION_INSTANCING
    // Establish the shadow resources before the buildings' layer group, so the shadow group is
    // registered first and therefore draws underneath. Costs nothing when the shadow is off.
    bool drawShadow = false;
    double shadowSetupMs = 0.0;
    if (shadowEnabled()) {
        const auto t0 = std::chrono::steady_clock::now();
        drawShadow = prepareShadow(shaders, context, changes);
        shadowSetupMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    }
    if (!drawShadow) {
        teardownShadow(changes);
    }
    if (drawShadow != shadowWasEnabled) {
        // Toggling forces a full rebuild: otherwise tiles that already have building drawables are
        // short-circuited by updateTile() below and never get their shadow drawables.
        removeAllDrawables();
        shadowWasEnabled = drawShadow;
    }
#endif

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

    const auto dropStaleDrawables = [&](gfx::Drawable& drawable) {
        // If the render pass has changed or the tile has  dropped out of the cover set, remove it.
        const auto& tileID = drawable.getTileID();
        if (!(drawable.getRenderPass() & drawPass) || (tileID && !hasRenderTile(*tileID))) {
            return true;
        }
        return false;
    };
    stats.drawablesRemoved += tileLayerGroup->removeDrawablesIf(dropStaleDrawables);

#if MLN_USE_FILL_EXTRUSION_INSTANCING
    if (drawShadow) {
        if (auto* shadowTileGroup = static_cast<TileLayerGroup*>(shadowGroup.get())) {
            stats.drawablesRemoved += shadowTileGroup->removeDrawablesIf(dropStaleDrawables);
        }
    }
#endif

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

#if MLN_USE_FILL_EXTRUSION_INSTANCING
    if (drawShadow) {
        // Shadow geometry can legitimately overlap (a building's own roof/wall footprint at a
        // concave corner, or two adjacent buildings' shadows), so each is3D+stencil-enabled
        // drawable in this group shares one stencil ref for the pass: the first triangle to cover
        // a pixel wins and later overlapping triangles are stencil-rejected instead of blending.
        static_cast<TileLayerGroup*>(shadowGroup.get())->setStencilTiles(renderTiles);
    }
#endif

#if MLN_USE_FILL_EXTRUSION_INSTANCING
    if (!fillExtrusionInstancedGroup) {
        fillExtrusionInstancedGroup = shaders.getShaderGroup("FillExtrusionInstancedShader");
    }
    if (!fillExtrusionPatternInstancedGroup) {
        fillExtrusionPatternInstancedGroup = shaders.getShaderGroup("FillExtrusionPatternInstancedShader");
    }

    if (!staticDataVertices) {
        staticDataVertices = std::make_shared<FillExtrusionVertexVector>(RenderStaticData::fillExtrusionVertices());
    }
    if (!staticDataIndices) {
        staticDataIndices = std::make_shared<TriangleIndexVector>(RenderStaticData::fillExtrusionTriangleIndices());
    }
    if (!staticDataSegments) {
        staticDataSegments = std::make_shared<SegmentVector>(RenderStaticData::fillExtrusionSegments());
    }

    const auto& instancedShaderGroup = hasPattern ? fillExtrusionPatternInstancedGroup : fillExtrusionInstancedGroup;
    if (!instancedShaderGroup) {
        removeAllDrawables();
        return;
    }

    const auto instanceVertexCount = staticDataVertices->elements();
    std::unique_ptr<gfx::DrawableBuilder> instancedDepthBuilder;
    std::unique_ptr<gfx::DrawableBuilder> instancedColorBuilder;
    StringIDSetsPair instancePropertiesAsUniforms;
#endif

    // Drop a tile's drawables from the shadow group alongside the layer's own, so a tile whose
    // bucket was replaced does not leave a shadow drawable pointing at stale binders.
    const auto removeTileEverywhere = [&](RenderPass pass, const OverscaledTileID& id) {
        removeTile(pass, id);
#if MLN_USE_FILL_EXTRUSION_INSTANCING
        if (drawShadow) {
            if (auto* shadowTileGroup = static_cast<TileLayerGroup*>(shadowGroup.get())) {
                stats.drawablesRemoved += shadowTileGroup->removeDrawables(pass, id).size();
            }
        }
#endif
    };

    StringIDSetsPair propertiesAsUniforms;
    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const auto* optRenderData = getRenderDataForPass(tile, drawPass);
        if (!optRenderData || !optRenderData->bucket || !optRenderData->bucket->hasData()) {
            removeTileEverywhere(drawPass, tileID);
            continue;
        }

        const auto& renderData = *optRenderData;
        auto& bucket = static_cast<FillExtrusionBucket&>(*renderData.bucket);

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
            removeTileEverywhere(drawPass, tileID);
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

#if MLN_USE_FILL_EXTRUSION_INSTANCING
        if (instancedDepthBuilder) {
            instancedDepthBuilder->clearTweakers();
        }
        if (instancedColorBuilder) {
            instancedColorBuilder->clearTweakers();
        }

        instancePropertiesAsUniforms.first.clear();
        instancePropertiesAsUniforms.second.clear();

        auto instanceAttrs = context.createVertexAttributeArray();
        instanceAttrs->readDataDrivenPaintProperties<FillExtrusionBase,
                                                     FillExtrusionColor,
                                                     FillExtrusionHeight,
                                                     FillExtrusionPattern>(
            binders, evaluated, instancePropertiesAsUniforms, idFillExtrusionBaseVertexAttribute);

        const auto instancedShader = std::static_pointer_cast<gfx::ShaderProgramBase>(
            instancedShaderGroup->getOrCreateShader(context, instancePropertiesAsUniforms));
        if (!instancedShader) {
            continue;
        }
#endif

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

#if MLN_USE_FILL_EXTRUSION_INSTANCING
                if (instancedDepthBuilder) {
                    instancedDepthBuilder->addTweaker(tweaker);
                }
                if (instancedColorBuilder) {
                    instancedColorBuilder->addTweaker(tweaker);
                }
#endif
            }
        }

        if (const auto& attr = vertexAttrs->set(idFillExtrusionPosVertexAttribute)) {
            attr->setSharedRawData(bucket.sharedVertices,
                                   offsetof(FillExtrusionLayoutVertex, a1),
                                   /*vertexOffset=*/0,
                                   sizeof(FillExtrusionLayoutVertex),
                                   gfx::AttributeDataType::Short2);
        }

        if (const auto& attr = vertexAttrs->set(idFillExtrusionDecimalsEdAttribute)) {
            attr->setSharedRawData(bucket.sharedVertices,
                                   offsetof(FillExtrusionLayoutVertex, a2),
                                   /*vertexOffset=*/0,
                                   sizeof(FillExtrusionLayoutVertex),
                                   gfx::AttributeDataType::UShort2);
        }

#if !MLN_USE_FILL_EXTRUSION_INSTANCING
        if (const auto& attr = vertexAttrs->set(idFillExtrusionNormal2DVertexAttribute)) {
            attr->setSharedRawData(bucket.sharedVertices,
                                   offsetof(FillExtrusionLayoutVertex, a3),
                                   /*vertexOffset=*/0,
                                   sizeof(FillExtrusionLayoutVertex),
                                   gfx::AttributeDataType::Short2);
        }
#endif

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

#if MLN_USE_FILL_EXTRUSION_INSTANCING
        if (doDepthPass && !instancedDepthBuilder) {
            if (auto builder = context.createDrawableBuilder(layerPrefix + "depthInstanced")) {
                builder->setShader(instancedShader);
                builder->setIs3D(true);
                builder->setEnableColor(false);
                builder->setRenderPass(drawPass);
                builder->setCullFaceMode(gfx::CullFaceMode::backCCW());
                builder->setDrawPriority(0);
                if (tweaker) {
                    builder->addTweaker(tweaker);
                }
                instancedDepthBuilder = std::move(builder);
            }
        }
        if (!instancedColorBuilder) {
            if (auto builder = context.createDrawableBuilder(layerPrefix + "colorInstanced")) {
                builder->setShader(instancedShader);
                builder->setIs3D(true);
                builder->setEnableColor(true);
                builder->setColorMode(gfx::ColorMode::alphaBlended());
                builder->setRenderPass(drawPass);
                builder->setCullFaceMode(gfx::CullFaceMode::backCCW());
                builder->setDrawPriority(1);
                if (tweaker) {
                    builder->addTweaker(tweaker);
                }
                instancedColorBuilder = std::move(builder);
            }
        }

        auto instanceVertexAttrs = context.createVertexAttributeArray();
        if (const auto& attr = instanceVertexAttrs->set(idFillExtrusionPosVertexAttribute)) {
            attr->setSharedRawData(staticDataVertices,
                                   offsetof(FillExtrusionStaticVertex, a1),
                                   /*vertexOffset=*/0,
                                   sizeof(FillExtrusionStaticVertex),
                                   gfx::AttributeDataType::Short2);
        }
        if (const auto& attr = instanceAttrs->set(idFillExtrusionOutlinePosAttribute)) {
            attr->setSharedRawData(bucket.sharedVertices,
                                   offsetof(FillExtrusionLayoutVertex, a1),
                                   /*vertexOffset=*/0,
                                   sizeof(FillExtrusionLayoutVertex),
                                   gfx::AttributeDataType::Short2);
        }
        if (const auto& attr = instanceAttrs->set(idFillExtrusionDecimalsEdAttribute)) {
            attr->setSharedRawData(bucket.sharedVertices,
                                   offsetof(FillExtrusionLayoutVertex, a2),
                                   /*vertexOffset=*/0,
                                   sizeof(FillExtrusionLayoutVertex),
                                   gfx::AttributeDataType::UShort2);
        }

        if (doDepthPass) {
            instancedDepthBuilder->setRawVertices({}, instanceVertexCount, gfx::AttributeDataType::Short2);
            instancedDepthBuilder->setVertexAttributes(instanceVertexAttrs);
            instancedDepthBuilder->setInstanceAttributes(instanceAttrs);
        }

        instancedColorBuilder->setEnableStencil(doDepthPass);
        instancedColorBuilder->setRawVertices({}, instanceVertexCount, gfx::AttributeDataType::Short2);
        instancedColorBuilder->setVertexAttributes(std::move(instanceVertexAttrs));
        instancedColorBuilder->setInstanceAttributes(std::move(instanceAttrs));

        const auto finishInstance = [&](gfx::DrawableBuilder& instancedBuilder) {
            if (!staticDataIndices->elements()) {
                return;
            }
            instancedBuilder.setSegments(
                gfx::Triangles(), staticDataIndices, staticDataSegments->data(), staticDataSegments->size());

            instancedBuilder.flush(context);

            for (auto& drawable : instancedBuilder.clearDrawables()) {
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
            finishInstance(*instancedDepthBuilder);
        }
        finishInstance(*instancedColorBuilder);

        if (drawShadow) {
            auto* shadowTileGroup = static_cast<TileLayerGroup*>(shadowGroup.get());

            // The shadow reads only base and height. Its own attribute id space keeps it independent
            // of the building shaders' permutations.
            StringIDSetsPair shadowPropertiesAsUniforms;
            auto shadowRoofAttrs = context.createVertexAttributeArray();
            shadowRoofAttrs->readDataDrivenPaintProperties<FillExtrusionBase, FillExtrusionHeight>(
                binders, evaluated, shadowPropertiesAsUniforms, idFillExtrusionShadowBaseVertexAttribute);

            StringIDSetsPair shadowInstancePropertiesAsUniforms;
            auto shadowInstanceAttrs = context.createVertexAttributeArray();
            shadowInstanceAttrs->readDataDrivenPaintProperties<FillExtrusionBase, FillExtrusionHeight>(
                binders, evaluated, shadowInstancePropertiesAsUniforms, idFillExtrusionShadowBaseVertexAttribute);

            const auto maskShader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                shadowMaskShaderGroup->getOrCreateShader(context, shadowPropertiesAsUniforms));
            const auto maskInstancedShader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                shadowMaskInstancedShaderGroup->getOrCreateShader(context, shadowInstancePropertiesAsUniforms));

            if (maskShader && maskInstancedShader) {
                // Roof geometry, straight from the bucket's own vertices.
                if (const auto& attr = shadowRoofAttrs->set(idFillExtrusionShadowPosVertexAttribute)) {
                    attr->setSharedRawData(bucket.sharedVertices,
                                           offsetof(FillExtrusionLayoutVertex, a1),
                                           /*vertexOffset=*/0,
                                           sizeof(FillExtrusionLayoutVertex),
                                           gfx::AttributeDataType::Short2);
                }
                if (const auto& attr = shadowRoofAttrs->set(idFillExtrusionShadowDecimalsEdAttribute)) {
                    attr->setSharedRawData(bucket.sharedVertices,
                                           offsetof(FillExtrusionLayoutVertex, a2),
                                           /*vertexOffset=*/0,
                                           sizeof(FillExtrusionLayoutVertex),
                                           gfx::AttributeDataType::UShort2);
                }

                // Wall geometry: the static unit quad instanced once per outline edge.
                auto shadowQuadAttrs = context.createVertexAttributeArray();
                if (const auto& attr = shadowQuadAttrs->set(idFillExtrusionShadowPosVertexAttribute)) {
                    attr->setSharedRawData(staticDataVertices,
                                           offsetof(FillExtrusionStaticVertex, a1),
                                           /*vertexOffset=*/0,
                                           sizeof(FillExtrusionStaticVertex),
                                           gfx::AttributeDataType::Short2);
                }
                if (const auto& attr = shadowInstanceAttrs->set(idFillExtrusionShadowOutlinePosAttribute)) {
                    attr->setSharedRawData(bucket.sharedVertices,
                                           offsetof(FillExtrusionLayoutVertex, a1),
                                           /*vertexOffset=*/0,
                                           sizeof(FillExtrusionLayoutVertex),
                                           gfx::AttributeDataType::Short2);
                }
                if (const auto& attr = shadowInstanceAttrs->set(idFillExtrusionShadowDecimalsEdAttribute)) {
                    attr->setSharedRawData(bucket.sharedVertices,
                                           offsetof(FillExtrusionLayoutVertex, a2),
                                           /*vertexOffset=*/0,
                                           sizeof(FillExtrusionLayoutVertex),
                                           gfx::AttributeDataType::UShort2);
                }

                // Culling must be off: flattening the walls onto the ground makes them degenerate or
                // reverses their winding, which would punch holes in the shadow. No depth test:
                // occlusion by the buildings comes entirely from this group being registered ahead
                // of `layerGroup` (see markLayerRenderable()), not from a depth comparison. is3D +
                // stencil is enabled to get the overlap dedup (see the setStencilTiles() call
                // above); setEnableDepth(false) means it has no effect on 3D depth sorting.
                const auto configureShadow = [](gfx::DrawableBuilder& builder) {
                    builder.setIs3D(true);
                    builder.setEnableDepth(false);
                    builder.setEnableStencil(true);
                    builder.setEnableColor(true);
                    builder.setColorMode(gfx::ColorMode::alphaBlended());
                    builder.setCullFaceMode(gfx::CullFaceMode::disabled());
                    builder.setRenderPass(RenderPass::Translucent);
                    builder.setDrawPriority(0);
                };

                const auto finishShadow = [&](gfx::DrawableBuilder& builder) {
                    builder.flush(context);
                    for (auto& drawable : builder.clearDrawables()) {
                        drawable->setTileID(tileID);
                        drawable->setLayerTweaker(shadowTweaker);
                        drawable->setBinders(renderData.bucket, &binders);
                        drawable->setRenderTile(renderTilesOwner, &tile);
                        shadowTileGroup->addDrawable(drawPass, tileID, std::move(drawable));
                        ++stats.drawablesAdded;
                    }
                };

                if (bucket.sharedTriangles->elements()) {
                    if (auto builder = context.createDrawableBuilder(layerPrefix + "shadowRoof")) {
                        builder->setShader(maskShader);
                        configureShadow(*builder);
                        builder->setVertexAttributes(std::move(shadowRoofAttrs));
                        builder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
                        builder->setSegments(gfx::Triangles(),
                                             bucket.sharedTriangles,
                                             bucket.triangleSegments.data(),
                                             bucket.triangleSegments.size());
                        finishShadow(*builder);
                    }
                }

                if (staticDataIndices->elements()) {
                    if (auto builder = context.createDrawableBuilder(layerPrefix + "shadowWall")) {
                        builder->setShader(maskInstancedShader);
                        configureShadow(*builder);
                        builder->setVertexAttributes(std::move(shadowQuadAttrs));
                        builder->setInstanceAttributes(std::move(shadowInstanceAttrs));
                        builder->setRawVertices({}, instanceVertexCount, gfx::AttributeDataType::Short2);
                        builder->setSegments(gfx::Triangles(),
                                             staticDataIndices,
                                             staticDataSegments->data(),
                                             staticDataSegments->size());
                        finishShadow(*builder);
                    }
                }
            }
        }
#endif
    }

#if MLN_USE_FILL_EXTRUSION_INSTANCING
    if (drawShadow) {
        reportShadowStats(shadowSetupMs);
    }
#endif
}

} // namespace mln
