#include <mln/renderer/layers/render_fill_extrusion_layer.hpp>

#include <mln/geometry/feature_index.hpp>
#include <mln/gfx/cull_face_mode.hpp>
#include <mln/gfx/drawable_atlases_tweaker.hpp>
#include <mln/gfx/drawable_builder.hpp>
#include <mln/gfx/render_pass.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/gfx/shader_registry.hpp>
#include <mln/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mln/renderer/image_manager.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/layers/fill_extrusion_ground_occlusion_layer_tweaker.hpp>
#include <mln/renderer/layers/fill_extrusion_layer_tweaker.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/render_static_data.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/render_tree.hpp>
#include <mln/renderer/tile_render_data.hpp>
#include <mln/renderer/update_parameters.hpp>
#include <mln/shaders/fill_extrusion_layer_ubo.hpp>
#include <mln/shaders/shader_program_base.hpp>
#include <mln/style/expression/image.hpp>
#include <mln/style/layers/fill_extrusion_layer_impl.hpp>
#include <mln/tile/geometry_tile.hpp>
#include <mln/tile/tile.hpp>
#include <mln/util/intersection_tests.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/math.hpp>

namespace mln {

using namespace style;
using namespace shaders;

namespace {

inline const FillExtrusionLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == FillExtrusionLayer::Impl::staticTypeInfo());
    return static_cast<const FillExtrusionLayer::Impl&>(*impl);
}

/// Whether a paint property can evaluate above zero at some zoom or point of a transition. Only a
/// constant (or default) of zero, with no transition running, rules it out.
template <class Property>
bool canBePositive(const FillExtrusionPaintProperties::Unevaluated& unevaluated) {
    const auto& property = unevaluated.get<Property>();
    if (property.hasTransition()) {
        return true;
    }
    const auto& value = property.getValue();
    if (value.isUndefined()) {
        return Property::defaultValue() > 0.0f;
    }
    return !value.isConstant() || value.asConstant() > 0.0f;
}

} // namespace

void RenderFillExtrusionLayer::captureRenderedFeatures(
    const FillExtrusionBucket& bucket,
    const RenderTile& tile,
    const FillExtrusionBinders& binders,
    const style::FillExtrusionPaintProperties::PossiblyEvaluated& evaluated,
    const TransformState& state,
    const TransformParameters& transformParams) {
    const auto& tileID = tile.getOverscaledTileID();

    // fill-extrusion-opacity does not support data-driven expressions
    const auto constantAlpha = evaluated.get<FillExtrusionOpacity>();
    if (constantAlpha == 0) {
        return;
    }

    const bool colorIsConstant = evaluated.template get<FillExtrusionColor>().isConstant();
    const auto& colorBinder = binders.template get<FillExtrusionColor>();
    if (colorIsConstant && std::get<0>(colorBinder->uniformValue(evaluated.get<FillExtrusionColor>())).a == 0) {
        return;
    }

    constexpr bool inViewportPixelUnits = false;
    constexpr bool nearClipped = true;
    constexpr bool aligned = false;
    const auto translation = evaluated.get<FillExtrusionTranslate>();
    const auto translationAnchor = evaluated.get<FillExtrusionTranslateAnchor>();
    const auto base = constOrDefault<FillExtrusionBase>(evaluated);
    const auto height = constOrDefault<FillExtrusionHeight>(evaluated);
    const std::optional<mln::Point<double>> origin = std::nullopt;
    const auto zoomFraction = state.getZoomFraction();
    std::optional<mat4> tileMatrix;

    const auto& features = bucket.getRetainedFeatures();
    stats.renderedFeatures.reserve(features.size());

    for (std::size_t i = 0; i < features.size(); ++i) {
        const auto& featureEntry = features[i];
        const auto& featureID = featureEntry.featureId;
        assert(!featureID.empty());

        if (!colorIsConstant) {
            // Consider only the value for the first vertex of this feature, for now.
            const auto& [vertex] = colorBinder->getVertexValue(featureEntry.vertexOffset);
            if (unpack_mix_alpha(vertex.a1, zoomFraction) == 0) {
                continue;
            }
        }

        // Compute the tile matrix once
        if (!tileMatrix.has_value()) {
            tileMatrix = LayerTweaker::getTileMatrix(tileID.toUnwrapped(),
                                                     state,
                                                     transformParams,
                                                     translation,
                                                     translationAnchor,
                                                     origin,
                                                     nearClipped,
                                                     inViewportPixelUnits,
                                                     aligned);
        }

        // Each vertex is considered twice, once at the base z and once at the height z
        const auto vertexOffset = featureEntry.vertexOffset;
        const auto vertexCount = featureEntry.vertexCount;
        const auto getVertex = [&bucket, vertexOffset, vertexCount, base, height](std::size_t vi) {
            const auto& vertex = bucket.vertices.at(vertexOffset + (vi % vertexCount)).a1;
            const auto z = (vi < vertexCount) ? base : height;
            return vec3{vertex[0] + 0.0, vertex[1] + 0.0, z};
        };
        if (const auto bound = computeFeatureNDCBound(2 * vertexCount, *tileMatrix, getVertex)) {
            stats.addRenderedFeature(featureID, *bound, {tileID});
        }
    }
}

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
    // The ground ambient occlusion has its own tweaker, on its own layer group. Without this it
    // keeps whatever properties it was constructed with.
    if (groundOcclusionTweaker) {
        groundOcclusionTweaker->updateProperties(evaluatedProperties);
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

void RenderFillExtrusionLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
    // Layer groups live in a std::multimap keyed by layer index, and insert() places equivalent
    // keys at the upper bound of their range, so insertion order decides draw order within one
    // index. The ground ambient occlusion goes after the buildings, so it is depth tested against
    // them.
    isRenderable = willRender;
    activateLayerGroup(layerGroup, willRender, changes);
    activateLayerGroup(groundOcclusionGroup, willRender, changes);
}

void RenderFillExtrusionLayer::keepGroundOcclusionLast(UniqueChangeRequestVec& changes) {
    if (groundOcclusionGroup && isRenderable) {
        activateLayerGroup(groundOcclusionGroup, false, changes);
        activateLayerGroup(groundOcclusionGroup, true, changes);
    }
}

void RenderFillExtrusionLayer::layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec& changes) {
    // Same ordering rule as above: the re-insert must keep the ground ambient occlusion after the
    // buildings.
    layerIndex = newLayerIndex;
    changeLayerIndex(layerGroup, newLayerIndex, changes);
    changeLayerIndex(groundOcclusionGroup, newLayerIndex, changes);
}

void RenderFillExtrusionLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    RenderLayer::layerRemoved(changes);
    // The base class only knows about `layerGroup`. Without this the ground ambient occlusion group
    // would stay registered with the orchestrator, empty.
    teardownGroundOcclusion(changes);
    groundOcclusionWasEnabled = false;
}

std::size_t RenderFillExtrusionLayer::removeAllDrawables() {
    auto removed = RenderLayer::removeAllDrawables();

    if (groundOcclusionGroup) {
        const auto count = groundOcclusionGroup->getDrawableCount();
        removed += count;
        stats.drawablesRemoved += count;
        groundOcclusionGroup->clearDrawables();
    }

    return removed;
}

bool RenderFillExtrusionLayer::groundOcclusionWanted() const {
    return canBePositive<FillExtrusionAmbientOcclusionGroundIntensity>(unevaluated) &&
           canBePositive<FillExtrusionAmbientOcclusionGroundRadius>(unevaluated);
}

bool RenderFillExtrusionLayer::hasOpaqueBackdrop(const PaintParameters& parameters, const RenderTree& renderTree) {
    // The ground ambient occlusion keeps its value in the framebuffer's alpha between its two
    // draws and resets it to 1 afterwards, so it needs an opaque framebuffer. That is only known
    // when the renderer clears it with an opaque background color: a shared context is not cleared
    // here, and a translucent or missing background leaves the alpha below 1.
    const bool opaque = !parameters.backend.contextIsShared() && renderTree.getParameters().backgroundColor.a >= 1.0f;
    if (!opaque && !groundOcclusionBackdropWarned) {
        Log::Warning(Event::Render,
                     "Layer '" + getID() +
                         "': fill-extrusion-ambient-occlusion-ground-* needs an opaque background layer at the "
                         "bottom of the style, the ground ambient occlusion is disabled");
        groundOcclusionBackdropWarned = true;
    }
    return opaque;
}

void RenderFillExtrusionLayer::teardownGroundOcclusion(UniqueChangeRequestVec& changes) {
    if (groundOcclusionGroup) {
        activateLayerGroup(groundOcclusionGroup, false, changes);
        groundOcclusionGroup.reset();
    }
    groundOcclusionTweaker.reset();
}

bool RenderFillExtrusionLayer::prepareGroundOcclusion([[maybe_unused]] gfx::ShaderRegistry& shaders,
                                                      [[maybe_unused]] gfx::Context& context,
                                                      [[maybe_unused]] UniqueChangeRequestVec& changes) {
#if MLN_USE_FILL_EXTRUSION_GROUND_OCCLUSION
    // Backends without the shaders simply draw no ground ambient occlusion
    if (!groundOcclusionShaderGroup) {
        groundOcclusionShaderGroup = shaders.getShaderGroup("FillExtrusionGroundOcclusionShader");
    }
    if (!groundOcclusionResolveShaderGroup) {
        groundOcclusionResolveShaderGroup = shaders.getShaderGroup("FillExtrusionGroundOcclusionResolveShader");
    }
    if (!groundOcclusionShaderGroup || !groundOcclusionResolveShaderGroup) {
        return false;
    }

    if (!groundOcclusionGroup) {
        auto group = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID() + "-occlusion");
        if (!group) {
            return false;
        }
        groundOcclusionGroup = std::move(group);
        activateLayerGroup(groundOcclusionGroup, isRenderable, changes);
    }

    if (!groundOcclusionTweaker) {
        groundOcclusionTweaker = std::make_shared<FillExtrusionGroundOcclusionLayerTweaker>(getID(),
                                                                                            evaluatedProperties);
        groundOcclusionGroup->addLayerTweaker(groundOcclusionTweaker);
    }
    return true;
#else
    return false;
#endif
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
                                      const TransformState& state,
                                      const std::shared_ptr<UpdateParameters>& updateParameters,
                                      const PaintParameters& paintParameters,
                                      const RenderTree& renderTree,
                                      UniqueChangeRequestVec& changes) {
    stats.renderedFeatures.clear();

    if (!renderTiles || renderTiles->empty() || passes == RenderPass::None) {
        removeAllDrawables();
        teardownGroundOcclusion(changes);
        return;
    }

    // This decides which drawables exist, so it only changes with the style. Whether they are
    // visible at the current zoom or point of a transition is set on the group below, so a value
    // that crosses zero does not rebuild every drawable of the layer.
    const bool occludeGround = groundOcclusionWanted() && hasOpaqueBackdrop(paintParameters, renderTree) &&
                               prepareGroundOcclusion(shaders, context, changes);
    if (!occludeGround) {
        teardownGroundOcclusion(changes);
    }
    if (occludeGround && !groundOcclusionWasEnabled) {
        // Enabling forces a full rebuild: otherwise tiles that already have building drawables are
        // short-circuited by updateTile() below and never get their occlusion drawables. Disabling
        // needs none, the teardown above already dropped the occlusion drawables.
        removeAllDrawables();
    }
    groundOcclusionWasEnabled = occludeGround;

    const auto& evaluated = static_cast<const FillExtrusionLayerProperties&>(*evaluatedProperties).evaluated;
    if (groundOcclusionGroup) {
        groundOcclusionGroup->setEnabled(evaluated.get<FillExtrusionAmbientOcclusionGroundIntensity>() > 0.0f &&
                                         evaluated.get<FillExtrusionAmbientOcclusionGroundRadius>() > 0.0f);
    }

    // Set up a layer group
    if (!layerGroup) {
        if (auto layerGroup_ = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID())) {
            setLayerGroup(std::move(layerGroup_), changes);
            keepGroundOcclusionLast(changes);
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
    auto* occlusionTileGroup = static_cast<TileLayerGroup*>(groundOcclusionGroup.get());

    constexpr auto drawPass = RenderPass::Translucent;

    const auto dropStaleDrawables = [&](gfx::Drawable& drawable) {
        // If the render pass has changed or the tile has  dropped out of the cover set, remove it.
        const auto& tileID = drawable.getTileID();
        return !(drawable.getRenderPass() & drawPass) || (tileID && !hasRenderTile(*tileID));
    };
    stats.drawablesRemoved += tileLayerGroup->removeDrawablesIf(dropStaleDrawables);
    if (occlusionTileGroup) {
        stats.drawablesRemoved += occlusionTileGroup->removeDrawablesIf(dropStaleDrawables);
    }

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
    if (occlusionTileGroup) {
        occlusionTileGroup->setStencilTiles(renderTiles);
    }

#if MLN_USE_FILL_EXTRUSION_GROUND_OCCLUSION
    if (!staticDataVertices) {
        staticDataVertices = std::make_shared<FillExtrusionVertexVector>(RenderStaticData::fillExtrusionVertices());
    }
    if (!staticDataIndices) {
        staticDataIndices = std::make_shared<TriangleIndexVector>(RenderStaticData::fillExtrusionTriangleIndices());
    }
#endif

#if MLN_USE_FILL_EXTRUSION_INSTANCING
    if (!fillExtrusionInstancedGroup) {
        fillExtrusionInstancedGroup = shaders.getShaderGroup("FillExtrusionInstancedShader");
    }
    if (!fillExtrusionPatternInstancedGroup) {
        fillExtrusionPatternInstancedGroup = shaders.getShaderGroup("FillExtrusionPatternInstancedShader");
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

    // Drop a tile's drawables from the ground ambient occlusion group alongside the layer's own, so a
    // tile whose bucket was replaced does not leave an occlusion drawable pointing at stale binders.
    const auto removeTileEverywhere = [&](RenderPass pass, const OverscaledTileID& id) {
        removeTile(pass, id);
        if (occlusionTileGroup) {
            stats.drawablesRemoved += occlusionTileGroup->removeDrawables(pass, id).size();
        }
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

        // Capture rendered features if requested
        if (updateParameters && updateParameters->captureRenderedFeatures) {
            const auto& params = renderTree.getParameters().transformParams;
            captureRenderedFeatures(bucket, tile, binders, evaluated, state, params);
        }

        // If we already have drawables for this tile, update them.
        auto updateExisting = [&](gfx::Drawable& drawable) {
            // A drawable produced on a previous style/bucket should not be updated.
            return drawable.getLayerTweaker() == layerTweaker;
        };
        if (updateTile(drawPass, tileID, std::move(updateExisting))) {
            continue;
        }

        // The tile's drawables are rebuilt below, for example after a paint property change replaced
        // the layer tweaker. updateTile() only drops the stale ones from `layerGroup`, so drop them
        // from the ground ambient occlusion group too. Otherwise the old occlusion blends on top of
        // the new one until the source reloads.
        if (occlusionTileGroup) {
            stats.drawablesRemoved += occlusionTileGroup->removeDrawables(drawPass, tileID).size();
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
                gfx::Triangles(), staticDataIndices, bucket.instanceSegments.data(), bucket.instanceSegments.size());

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
#endif

#if MLN_USE_FILL_EXTRUSION_GROUND_OCCLUSION
        // Ground ambient occlusion: one flat quad per outline edge, fading out away from the wall.
        // Drawn after the buildings and depth tested against them, so it does not show through
        // translucent ones, neither behind them nor under them. Overlapping quads are combined
        // with their maximum over two draws, see the shader.
        //
        // All of it is flat, so the stencil clips it to its tile like other 2D drawables. Edges
        // along the tile boundary, where the tile cut a building, would otherwise shade the
        // building's other half in the next tile.
        if (occludeGround && occlusionTileGroup && staticDataIndices->elements() && vertexCount > 1) {
            const auto buildOcclusion = [&](const gfx::ShaderGroupPtr& shaderGroup,
                                            const std::string& name,
                                            const gfx::ColorMode& colorMode,
                                            gfx::DrawPriority priority) -> std::vector<gfx::UniqueDrawable> {
                StringIDSetsPair occlusionPropertiesAsUniforms;
                auto instanceAttrs = context.createVertexAttributeArray();
                instanceAttrs->readDataDrivenPaintProperties<FillExtrusionBase, FillExtrusionHeight>(
                    binders,
                    evaluated,
                    occlusionPropertiesAsUniforms,
                    idFillExtrusionGroundOcclusionBaseVertexAttribute);

                const auto shader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                    shaderGroup->getOrCreateShader(context, occlusionPropertiesAsUniforms));
                auto builder = shader ? context.createDrawableBuilder(layerPrefix + name) : nullptr;
                if (!builder) {
                    return {};
                }

                auto quadAttrs = context.createVertexAttributeArray();
                if (const auto& attr = quadAttrs->set(idFillExtrusionGroundOcclusionPosVertexAttribute)) {
                    attr->setSharedRawData(staticDataVertices,
                                           offsetof(FillExtrusionStaticVertex, a1),
                                           /*vertexOffset=*/0,
                                           sizeof(FillExtrusionStaticVertex),
                                           gfx::AttributeDataType::Short2);
                }
                const auto setVertexAttribute =
                    [&](std::size_t id, std::size_t offset, uint32_t vertexOffset, gfx::AttributeDataType type) {
                        if (const auto& attr = instanceAttrs->set(id)) {
                            attr->setSharedRawData(
                                bucket.sharedVertices, offset, vertexOffset, sizeof(FillExtrusionLayoutVertex), type);
                        }
                    };
#if MLN_USE_FILL_EXTRUSION_INSTANCING
                // The bucket's outline, as the instanced walls read it
                setVertexAttribute(idFillExtrusionGroundOcclusionOutlinePosAttribute,
                                   offsetof(FillExtrusionLayoutVertex, a1),
                                   0,
                                   gfx::AttributeDataType::Short2);
                setVertexAttribute(idFillExtrusionGroundOcclusionDecimalsEdAttribute,
                                   offsetof(FillExtrusionLayoutVertex, a2),
                                   0,
                                   gfx::AttributeDataType::UShort2);
                const SegmentBase* segments = bucket.instanceSegments.data();
                const auto segmentCount = bucket.instanceSegments.size();
#else
                // Each vertex of the mesh and the next one, with one instance per vertex but the last.
                // The shader only draws the edges, a roof vertex followed by a wall vertex.
                for (const uint32_t next : {0u, 1u}) {
                    setVertexAttribute(next ? idFillExtrusionGroundOcclusionNextPosAttribute
                                            : idFillExtrusionGroundOcclusionOutlinePosAttribute,
                                       offsetof(FillExtrusionLayoutVertex, a1),
                                       next,
                                       gfx::AttributeDataType::Short2);
                    setVertexAttribute(next ? idFillExtrusionGroundOcclusionNextDecimalsEdAttribute
                                            : idFillExtrusionGroundOcclusionDecimalsEdAttribute,
                                       offsetof(FillExtrusionLayoutVertex, a2),
                                       next,
                                       gfx::AttributeDataType::UShort2);
                    setVertexAttribute(next ? idFillExtrusionGroundOcclusionNextNormal2DAttribute
                                            : idFillExtrusionGroundOcclusionNormal2DAttribute,
                                       offsetof(FillExtrusionLayoutVertex, a3),
                                       next,
                                       gfx::AttributeDataType::Short2);
                }
                auto segment = RenderStaticData::fillExtrusionSegment();
                segment.instanceCount = vertexCount - 1;
                const SegmentBase* segments = &segment;
                const std::size_t segmentCount = 1;
#endif

                // No culling: the capsule is drawn around the edge whatever the winding
                builder->setShader(shader);
                builder->setIs3D(false);
                builder->setEnableDepth(true);
                builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                builder->setEnableStencil(true);
                builder->setEnableColor(true);
                builder->setColorMode(colorMode);
                builder->setCullFaceMode(gfx::CullFaceMode::disabled());
                builder->setRenderPass(RenderPass::Translucent);
                // Every tile's first draw ahead of any second one
                builder->setDrawPriority(priority);
                builder->setVertexAttributes(std::move(quadAttrs));
                builder->setInstanceAttributes(std::move(instanceAttrs));
                builder->setRawVertices({}, staticDataVertices->elements(), gfx::AttributeDataType::Short2);
                builder->setSegments(gfx::Triangles(), staticDataIndices, segments, segmentCount);
                builder->flush(context);
                return builder->clearDrawables();
            };

            // The minimum of 1 - occlusion into the alpha only
            auto occlusionDrawables = buildOcclusion(
                groundOcclusionShaderGroup,
                "groundOcclusion",
                gfx::ColorMode{.blendFunction = gfx::ColorMode::Min{},
                               .blendColor = {},
                               .mask = {.r = false, .g = false, .b = false, .a = true}},
                0);
            // Darkens by it, once: rgb * alpha, and the alpha clamps back to 1
            auto resolveDrawables = buildOcclusion(
                groundOcclusionResolveShaderGroup,
                "groundOcclusionResolve",
                gfx::ColorMode{.blendFunction = gfx::ColorMode::Add{gfx::ColorBlendFactorType::One,
                                                                    gfx::ColorBlendFactorType::DstAlpha},
                               .blendColor = {},
                               .mask = {.r = true, .g = true, .b = true, .a = true}},
                1);

            // Both draws or neither: the first alone would leave the framebuffer's alpha below 1
            if (!occlusionDrawables.empty() && !resolveDrawables.empty()) {
                for (auto* drawables : {&occlusionDrawables, &resolveDrawables}) {
                    for (auto& drawable : *drawables) {
                        drawable->setTileID(tileID);
                        drawable->setLayerTweaker(groundOcclusionTweaker);
                        drawable->setBinders(renderData.bucket, &binders);
                        drawable->setRenderTile(renderTilesOwner, &tile);
                        occlusionTileGroup->addDrawable(drawPass, tileID, std::move(drawable));
                        ++stats.drawablesAdded;
                    }
                }
            }
        }
#endif
    }
}

} // namespace mln
