#include <mbgl/renderer/layers/render_hillshade_layer.hpp>
#include <mbgl/renderer/buckets/hillshade_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/sources/render_raster_dem_source.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/style/layers/hillshade_layer_impl.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/offscreen_texture.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/util/geo.hpp>

#include <mbgl/renderer/layers/hillshade_layer_tweaker.hpp>
#include <mbgl/renderer/layers/hillshade_prepare_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_target.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/hillshade_prepare_drawable_data.hpp>
#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/gfx/shader_registry.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {

inline const HillshadeLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == HillshadeLayer::Impl::staticTypeInfo());
    return static_cast<const HillshadeLayer::Impl&>(*impl);
}

} // namespace

RenderHillshadeLayer::RenderHillshadeLayer(Immutable<style::HillshadeLayer::Impl> _impl)
    : RenderLayer(makeMutable<HillshadeLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {
    styleDependencies = unevaluated.getDependencies();
}

RenderHillshadeLayer::~RenderHillshadeLayer() = default;

std::array<float, 2> RenderHillshadeLayer::getLatRange(const UnwrappedTileID& id) {
    const LatLng latlng0 = LatLng(id);
    const LatLng latlng1 = LatLng(UnwrappedTileID(id.canonical.z, id.canonical.x, id.canonical.y + 1));
    return {{static_cast<float>(latlng0.latitude()), static_cast<float>(latlng1.latitude())}};
}

std::array<float, 2> RenderHillshadeLayer::getLight(const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const HillshadeLayerProperties&>(*evaluatedProperties).evaluated;
    float azimuthal = util::deg2radf(evaluated.get<HillshadeIlluminationDirection>());
    if (evaluated.get<HillshadeIlluminationAnchor>() == HillshadeIlluminationAnchorType::Viewport)
        azimuthal = azimuthal - static_cast<float>(parameters.state.getBearing());
    return {{evaluated.get<HillshadeExaggeration>(), azimuthal}};
}

void RenderHillshadeLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    styleDependencies = unevaluated.getDependencies();
}

void RenderHillshadeLayer::layerChanged(const TransitionParameters& parameters,
                                        const Immutable<style::Layer::Impl>& impl,
                                        UniqueChangeRequestVec& changes) {
    RenderLayer::layerChanged(parameters, impl, changes);
    prepareLayerTweaker.reset();
}

void RenderHillshadeLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    const auto previousProperties = staticImmutableCast<HillshadeLayerProperties>(evaluatedProperties);
    auto properties = makeMutable<HillshadeLayerProperties>(
        staticImmutableCast<HillshadeLayer::Impl>(baseImpl),
        unevaluated.evaluate(parameters, previousProperties->evaluated));

    passes = (properties->evaluated.get<style::HillshadeExaggeration>() > 0)
                 ? (RenderPass::Translucent | RenderPass::Pass3D)
                 : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);
    if (layerTweaker) {
        layerTweaker->updateProperties(evaluatedProperties);
    }
    if (prepareLayerTweaker) {
        prepareLayerTweaker->updateProperties(evaluatedProperties);
    }
}

bool RenderHillshadeLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderHillshadeLayer::hasCrossfade() const {
    return false;
}

void RenderHillshadeLayer::prepare(const LayerPrepareParameters& params) {
    renderTiles = params.source->getRenderTiles();
    maxzoom = params.source->getMaxZoom();

    updateRenderTileIDs();
}

namespace {
void activateRenderTarget(const RenderTargetPtr& renderTarget_, bool activate, UniqueChangeRequestVec& changes) {
    if (renderTarget_) {
        if (activate) {
            // The RenderTree has determined this render target should be included in the renderable set for a frame
            changes.emplace_back(std::make_unique<AddRenderTargetRequest>(renderTarget_));
        } else {
            // The RenderTree is informing us we should not render anything
            changes.emplace_back(std::make_unique<RemoveRenderTargetRequest>(renderTarget_));
        }
    }
}
} // namespace

void RenderHillshadeLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
    RenderLayer::markLayerRenderable(willRender, changes);
    removeRenderTargets(changes);
}

void RenderHillshadeLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    RenderLayer::layerRemoved(changes);
    removeRenderTargets(changes);
}

void RenderHillshadeLayer::addRenderTarget(const RenderTargetPtr& renderTarget, UniqueChangeRequestVec& changes) {
    activateRenderTarget(renderTarget, true, changes);
    activatedRenderTargets.emplace_back(renderTarget);
}

void RenderHillshadeLayer::removeRenderTargets(UniqueChangeRequestVec& changes) {
    for (const auto& renderTarget : activatedRenderTargets) {
        activateRenderTarget(renderTarget, false, changes);
    }
    activatedRenderTargets.clear();
}

static const std::string HillshadePrepareShaderGroupName = "HillshadePrepareShader";
static const std::string HillshadeShaderGroupName = "HillshadeShader";

void RenderHillshadeLayer::update(gfx::ShaderRegistry& shaders,
                                  gfx::Context& context,
                                  [[maybe_unused]] const TransformState& state,
                                  const std::shared_ptr<UpdateParameters>&,
                                  [[maybe_unused]] const RenderTree& renderTree,
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
        layerTweaker = std::make_shared<HillshadeLayerTweaker>(getID(), evaluatedProperties);
        layerGroup->addLayerTweaker(layerTweaker);
    }

    if (!hillshadePrepareShader) {
        hillshadePrepareShader = context.getGenericShader(shaders, HillshadePrepareShaderGroupName);
    }
    if (!hillshadeShader) {
        hillshadeShader = context.getGenericShader(shaders, HillshadeShaderGroupName);
    }
    if (!hillshadePrepareShader || !hillshadeShader) {
        removeAllDrawables();
        return;
    }

    auto renderPass = RenderPass::Translucent;
    if (!(mbgl::underlying_type(renderPass) & evaluatedProperties->renderPasses)) {
        return;
    }

    stats.drawablesRemoved += tileLayerGroup->removeDrawablesIf(
        [&](gfx::Drawable& drawable) { return drawable.getTileID() && !hasRenderTile(*drawable.getTileID()); });

    if (!staticDataSharedVertices) {
        staticDataSharedVertices = std::make_shared<HillshadeVertexVector>(RenderStaticData::rasterVertices());
    }
    const auto staticDataIndices = RenderStaticData::quadTriangleIndices();
    const auto staticDataSegments = RenderStaticData::rasterSegments();

    std::unique_ptr<gfx::DrawableBuilder> hillshadeBuilder;
    std::unique_ptr<gfx::DrawableBuilder> hillshadePrepareBuilder;

    gfx::VertexAttributeArrayPtr hillshadePrepareVertexAttrs;
    const auto getPrepareVertexAttributes = [&] {
        if (!hillshadePrepareVertexAttrs) {
            hillshadePrepareVertexAttrs = context.createVertexAttributeArray();

            if (const auto& attr = hillshadePrepareVertexAttrs->set(idHillshadePosVertexAttribute)) {
                attr->setSharedRawData(staticDataSharedVertices,
                                       offsetof(HillshadeLayoutVertex, a1),
                                       0,
                                       sizeof(HillshadeLayoutVertex),
                                       gfx::AttributeDataType::Short2);
            }
            if (const auto& attr = hillshadePrepareVertexAttrs->set(idHillshadeTexturePosVertexAttribute)) {
                attr->setSharedRawData(staticDataSharedVertices,
                                       offsetof(HillshadeLayoutVertex, a2),
                                       0,
                                       sizeof(HillshadeLayoutVertex),
                                       gfx::AttributeDataType::Short2);
            }
        }
        return hillshadePrepareVertexAttrs;
    };

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        auto* bucket_ = tile.getBucket(*baseImpl);
        if (!bucket_ || !bucket_->hasData()) {
            removeTile(renderPass, tileID);
            continue;
        }

        auto& bucket = static_cast<HillshadeBucket&>(*bucket_);

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
            removeTile(renderPass, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());

        if (!bucket.renderTargetPrepared) {
            // Set up tile render target
            const uint16_t tilesize = bucket.getDEMData().dim;
            auto renderTarget = context.createRenderTarget({tilesize, tilesize},
                                                           gfx::TextureChannelDataType::UnsignedByte);
            if (!renderTarget) {
                continue;
            }
            bucket.renderTarget = renderTarget;
            bucket.renderTargetPrepared = true;
            addRenderTarget(renderTarget, changes);

            auto singleTileLayerGroup = context.createTileLayerGroup(0, /*initialCapacity=*/1, getID());
            if (!singleTileLayerGroup) {
                return;
            }
            renderTarget->addLayerGroup(singleTileLayerGroup, /*replace=*/true);

            if (!prepareLayerTweaker) {
                prepareLayerTweaker = std::make_shared<HillshadePrepareLayerTweaker>(getID(), evaluatedProperties);
            }
            singleTileLayerGroup->addLayerTweaker(prepareLayerTweaker);

            hillshadePrepareBuilder = context.createDrawableBuilder("hillshadePrepare");
            hillshadePrepareBuilder->setShader(hillshadePrepareShader);
            hillshadePrepareBuilder->setDepthType(gfx::DepthMaskType::ReadOnly);
            hillshadePrepareBuilder->setColorMode(gfx::ColorMode::unblended());
            hillshadePrepareBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());
            hillshadePrepareBuilder->setRenderPass(renderPass);
            hillshadePrepareBuilder->setVertexAttributes(getPrepareVertexAttributes());
            hillshadePrepareBuilder->setRawVertices(
                {}, staticDataSharedVertices->elements(), gfx::AttributeDataType::Short2);
            hillshadePrepareBuilder->setSegments(
                gfx::Triangles(), staticDataIndices.vector(), staticDataSegments.data(), staticDataSegments.size());

            std::shared_ptr<gfx::Texture2D> texture = context.createTexture2D();
            texture->setImage(bucket.getDEMData().getImagePtr());
            texture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                              .wrapU = gfx::TextureWrapType::Clamp,
                                              .wrapV = gfx::TextureWrapType::Clamp});
            hillshadePrepareBuilder->setTexture(texture, idHillshadeImageTexture);

            hillshadePrepareBuilder->flush(context);

            for (auto& drawable : hillshadePrepareBuilder->clearDrawables()) {
                drawable->setTileID(tileID);
                drawable->setLayerTweaker(prepareLayerTweaker);
                drawable->setData(std::make_unique<gfx::HillshadePrepareDrawableData>(
                    bucket.getDEMData().stride, bucket.getDEMData().encoding, maxzoom));
                singleTileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                ++stats.drawablesAdded;
            }
        }

        // Set up tile drawable
        std::shared_ptr<HillshadeVertexVector> vertices;
        std::shared_ptr<gfx::IndexVector<gfx::Triangles>> indices;
        auto* segments = &staticDataSegments;

        if (!bucket.vertices.empty() && !bucket.indices.empty() && !bucket.segments.empty()) {
            vertices = bucket.sharedVertices;
            indices = bucket.sharedIndices;
            segments = &bucket.segments;
        } else {
            vertices = staticDataSharedVertices;
            indices = std::make_shared<gfx::IndexVector<gfx::Triangles>>(staticDataIndices);
        }

        if (!hillshadeBuilder) {
            hillshadeBuilder = context.createDrawableBuilder("hillshade");
        }

        gfx::VertexAttributeArrayPtr hillshadeVertexAttrs;
        auto buildVertexAttributes = [&] {
            if (!hillshadeVertexAttrs) {
                hillshadeVertexAttrs = context.createVertexAttributeArray();

                if (const auto& attr = hillshadeVertexAttrs->set(idHillshadePosVertexAttribute)) {
                    attr->setSharedRawData(vertices,
                                           offsetof(HillshadeLayoutVertex, a1),
                                           0,
                                           sizeof(HillshadeLayoutVertex),
                                           gfx::AttributeDataType::Short2);
                }
                if (const auto& attr = hillshadeVertexAttrs->set(idHillshadeTexturePosVertexAttribute)) {
                    attr->setSharedRawData(vertices,
                                           offsetof(HillshadeLayoutVertex, a2),
                                           0,
                                           sizeof(HillshadeLayoutVertex),
                                           gfx::AttributeDataType::Short2);
                }
            }
            return hillshadeVertexAttrs;
        };

        const auto updateExisting = [&](gfx::Drawable& drawable) {
            // Only current drawables are updated, ones produced for
            // a previous style retain the attribute values for that style.
            if (drawable.getLayerTweaker() != layerTweaker) {
                return false;
            }

            drawable.updateVertexAttributes(buildVertexAttributes(),
                                            vertices->elements(),
                                            gfx::Triangles(),
                                            std::move(indices),
                                            segments->data(),
                                            segments->size());
            drawable.setTexture(bucket.renderTarget->getTexture(), idHillshadeImageTexture);

            return true;
        };
        if (updateTile(renderPass, tileID, std::move(updateExisting))) {
            continue;
        }

        hillshadeBuilder->setShader(hillshadeShader);
        hillshadeBuilder->setDepthType(gfx::DepthMaskType::ReadOnly);
        hillshadeBuilder->setColorMode(gfx::ColorMode::alphaBlended());
        hillshadeBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());
        hillshadeBuilder->setRenderPass(renderPass);
        hillshadeBuilder->setVertexAttributes(buildVertexAttributes());
        hillshadeBuilder->setRawVertices({}, vertices->elements(), gfx::AttributeDataType::Short2);
        hillshadeBuilder->setSegments(gfx::Triangles(), indices->vector(), segments->data(), segments->size());
        hillshadeBuilder->setTexture(bucket.renderTarget->getTexture(), idHillshadeImageTexture);

        hillshadeBuilder->flush(context);

        for (auto& drawable : hillshadeBuilder->clearDrawables()) {
            drawable->setTileID(tileID);
            drawable->setLayerTweaker(layerTweaker);

            tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
            ++stats.drawablesAdded;
        }
    }
}

} // namespace mbgl
