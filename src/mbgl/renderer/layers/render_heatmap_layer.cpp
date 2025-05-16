#include <mbgl/renderer/layers/render_heatmap_layer.hpp>
#include <mbgl/renderer/buckets/heatmap_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/style/layers/heatmap_layer_impl.hpp>
#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/intersection_tests.hpp>

#include <mbgl/renderer/layers/heatmap_layer_tweaker.hpp>
#include <mbgl/renderer/layers/heatmap_texture_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_target.hpp>
#include <mbgl/shaders/heatmap_layer_ubo.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/gfx/shader_registry.hpp>

namespace mbgl {

using namespace style;

namespace {

inline const HeatmapLayer::Impl& impl_cast(const Immutable<Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == HeatmapLayer::Impl::staticTypeInfo());
    return static_cast<const HeatmapLayer::Impl&>(*impl);
}

} // namespace

RenderHeatmapLayer::RenderHeatmapLayer(Immutable<HeatmapLayer::Impl> _impl)
    : RenderLayer(makeMutable<HeatmapLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {
    styleDependencies = unevaluated.getDependencies();
    colorRamp = std::make_shared<PremultipliedImage>(Size(256, 1));
}

RenderHeatmapLayer::~RenderHeatmapLayer() = default;

void RenderHeatmapLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    styleDependencies = unevaluated.getDependencies();
    updateColorRamp();
}

void RenderHeatmapLayer::layerChanged(const TransitionParameters& parameters,
                                      const Immutable<style::Layer::Impl>& impl,
                                      UniqueChangeRequestVec& changes) {
    RenderLayer::layerChanged(parameters, impl, changes);
    textureTweaker.reset();
}

void RenderHeatmapLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    const auto previousProperties = staticImmutableCast<HeatmapLayerProperties>(evaluatedProperties);
    auto properties = makeMutable<HeatmapLayerProperties>(
        staticImmutableCast<HeatmapLayer::Impl>(baseImpl),
        unevaluated.evaluate(parameters, previousProperties->evaluated));

    passes = (properties->evaluated.get<style::HeatmapOpacity>() > 0) ? (RenderPass::Translucent | RenderPass::Pass3D)
                                                                      : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);

    if (layerTweaker) {
        layerTweaker->updateProperties(evaluatedProperties);
    }
    if (textureTweaker) {
        textureTweaker->updateProperties(evaluatedProperties);
    }
}

bool RenderHeatmapLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderHeatmapLayer::hasCrossfade() const {
    return false;
}

void RenderHeatmapLayer::updateColorRamp() {
    if (colorRamp) {
        auto colorValue = unevaluated.get<HeatmapColor>().getValue();
        if (colorValue.isUndefined()) {
            colorValue = HeatmapLayer::getDefaultHeatmapColor();
        }

        applyColorRamp(colorValue, *colorRamp);
    }
}

bool RenderHeatmapLayer::queryIntersectsFeature(const GeometryCoordinates& queryGeometry,
                                                const GeometryTileFeature& feature,
                                                const float zoom,
                                                const TransformState&,
                                                const float pixelsToTileUnits,
                                                const mat4&,
                                                const FeatureState&) const {
    (void)queryGeometry;
    (void)feature;
    (void)zoom;
    (void)pixelsToTileUnits;
    return false;
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

void RenderHeatmapLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
    RenderLayer::markLayerRenderable(willRender, changes);
    activateRenderTarget(renderTarget, willRender, changes);
}

std::size_t RenderHeatmapLayer::removeTile(RenderPass renderPass, const OverscaledTileID& tileID) {
    if (auto* tileLayerGroup = static_cast<TileLayerGroup*>(renderTarget->getLayerGroup(0).get())) {
        const auto count = tileLayerGroup->removeDrawables(renderPass, tileID).size();
        stats.drawablesRemoved += count;
        return count;
    }
    return 0;
}

std::size_t RenderHeatmapLayer::removeAllDrawables() {
    auto removed = RenderLayer::removeAllDrawables();
    if (renderTarget) {
        const auto count = renderTarget->getLayerGroup(0)->getDrawableCount();
        removed += count;
        stats.drawablesRemoved += count;
        renderTarget->getLayerGroup(0)->clearDrawables();
    }
    return removed;
}

namespace {

constexpr auto HeatmapShaderGroupName = "HeatmapShader";
constexpr auto HeatmapTextureShaderGroupName = "HeatmapTextureShader";

} // namespace

using namespace shaders;

void RenderHeatmapLayer::update(gfx::ShaderRegistry& shaders,
                                gfx::Context& context,
                                const TransformState& state,
                                const std::shared_ptr<UpdateParameters>&,
                                [[maybe_unused]] const RenderTree& renderTree,
                                UniqueChangeRequestVec& changes) {
    if (!renderTiles || renderTiles->empty()) {
        removeAllDrawables();
        return;
    }

    const auto& viewportSize = state.getSize();
    const auto size = Size{viewportSize.width / 2, viewportSize.height / 2};

    // Set up a render target
    if (!renderTarget) {
        renderTarget = context.createRenderTarget(size, gfx::TextureChannelDataType::HalfFloat);
        if (!renderTarget) {
            return;
        }
        activateRenderTarget(renderTarget, isRenderable, changes);

        // Set up tile layer group
        auto tileLayerGroup = context.createTileLayerGroup(0, /*initialCapacity=*/64, getID());
        if (!tileLayerGroup) {
            return;
        }
        renderTarget->addLayerGroup(tileLayerGroup, /*replace=*/true);
        textureTweaker.reset();
    }

    if (renderTarget->getTexture()->getSize() != size) {
        renderTarget->getTexture()->setSize(size);
    }

    auto* tileLayerGroup = static_cast<TileLayerGroup*>(renderTarget->getLayerGroup(0).get());

    if (!heatmapShaderGroup) {
        heatmapShaderGroup = shaders.getShaderGroup(HeatmapShaderGroupName);
    }
    if (!heatmapShaderGroup) {
        removeAllDrawables();
        return;
    }

    if (!layerTweaker) {
        layerTweaker = std::make_shared<HeatmapLayerTweaker>(getID(), evaluatedProperties);
        tileLayerGroup->addLayerTweaker(layerTweaker);
    }

    std::unique_ptr<gfx::DrawableBuilder> heatmapBuilder;
    constexpr auto renderPass = RenderPass::Translucent;

    if (!(mbgl::underlying_type(renderPass) & evaluatedProperties->renderPasses)) {
        return;
    }

    stats.drawablesRemoved += tileLayerGroup->removeDrawablesIf(
        [&](gfx::Drawable& drawable) { return drawable.getTileID() && !hasRenderTile(*drawable.getTileID()); });

    const auto& evaluated = static_cast<const HeatmapLayerProperties&>(*evaluatedProperties).evaluated;
    std::optional<StringIDSetsPair> propertiesAsUniforms;
#if !defined(NDEBUG)
    std::optional<StringIDSetsPair> previousPropertiesAsUniforms;
#endif

    gfx::ShaderPtr heatmapShader;

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
        if (!renderData) {
            removeTile(renderPass, tileID);
            continue;
        }

        auto& bucket = static_cast<HeatmapBucket&>(*renderData->bucket);
        const auto vertexCount = bucket.vertices.elements();
        auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
            removeTile(renderPass, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());

        auto updateExisting = [&](gfx::Drawable& drawable) {
            if (drawable.getLayerTweaker() != layerTweaker) {
                // This drawable was produced on a previous style/bucket, and should not be updated.
                return false;
            }
            return true;
        };
        if (updateTile(renderPass, tileID, std::move(updateExisting))) {
            continue;
        }

        if (!propertiesAsUniforms) {
            propertiesAsUniforms.emplace();
        }
#if !defined(NDEBUG)
        else {
            propertiesAsUniforms->first.clear();
            propertiesAsUniforms->second.clear();
        }
#endif

        auto heatmapVertexAttrs = context.createVertexAttributeArray();
        heatmapVertexAttrs->readDataDrivenPaintProperties<HeatmapWeight, HeatmapRadius>(
            paintPropertyBinders, evaluated, *propertiesAsUniforms, idHeatmapWeightVertexAttribute);

#if !defined(NDEBUG)
        // We assume the properties are the same across tiles.
        if (previousPropertiesAsUniforms) {
            assert(*propertiesAsUniforms == previousPropertiesAsUniforms);
        } else {
            previousPropertiesAsUniforms = propertiesAsUniforms;
        }
#endif

        if (!heatmapShader) {
            heatmapShader = heatmapShaderGroup->getOrCreateShader(context, *propertiesAsUniforms);
            if (!heatmapShader) {
                continue;
            }
        }

        if (const auto& attr = heatmapVertexAttrs->set(idHeatmapPosVertexAttribute)) {
            attr->setSharedRawData(bucket.sharedVertices,
                                   offsetof(HeatmapLayoutVertex, a1),
                                   /*vertexOffset=*/0,
                                   sizeof(HeatmapLayoutVertex),
                                   gfx::AttributeDataType::Short2);
        }

        heatmapBuilder = context.createDrawableBuilder("heatmap");
        heatmapBuilder->setShader(std::static_pointer_cast<gfx::ShaderProgramBase>(heatmapShader));
        heatmapBuilder->setEnableDepth(false);
        heatmapBuilder->setColorMode(gfx::ColorMode::additive());
        heatmapBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());
        heatmapBuilder->setRenderPass(renderPass);
        heatmapBuilder->setVertexAttributes(std::move(heatmapVertexAttrs));
        heatmapBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
        heatmapBuilder->setSegments(
            gfx::Triangles(), bucket.sharedTriangles, bucket.segments.data(), bucket.segments.size());

        heatmapBuilder->flush(context);

        for (auto& drawable : heatmapBuilder->clearDrawables()) {
            drawable->setTileID(tileID);
            drawable->setLayerTweaker(layerTweaker);
            drawable->setBinders(renderData->bucket, &paintPropertyBinders);
            drawable->setRenderTile(renderTilesOwner, &tile);
            tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
            ++stats.drawablesAdded;
        }
    }

    // Set up texture layer group
    if (!layerGroup) {
        if (auto layerGroup_ = context.createLayerGroup(layerIndex, /*initialCapacity=*/1, getID())) {
            if (textureTweaker) {
                layerGroup_->addLayerTweaker(textureTweaker);
            }
            setLayerGroup(std::move(layerGroup_), changes);
        } else {
            return;
        }
    }
    if (!textureTweaker) {
        textureTweaker = std::make_shared<HeatmapTextureLayerTweaker>(getID(), evaluatedProperties);
        layerGroup->addLayerTweaker(textureTweaker);
    }

    if (!heatmapTextureShader) {
        heatmapTextureShader = context.getGenericShader(shaders, HeatmapTextureShaderGroupName);
    }
    if (!heatmapTextureShader) {
        removeAllDrawables();
        return;
    }

    auto* textureLayerGroup = static_cast<LayerGroup*>(layerGroup.get());
    // TODO: Don't rebuild drawables every time
    textureLayerGroup->clearDrawables();

    if (!sharedTextureVertices) {
        sharedTextureVertices = std::make_shared<TextureVertexVector>(RenderStaticData::heatmapTextureVertices());
    }
    const auto textureVertexCount = sharedTextureVertices->elements();

    auto textureVertexAttrs = context.createVertexAttributeArray();
    if (const auto& attr = textureVertexAttrs->set(idHeatmapPosVertexAttribute)) {
        attr->setSharedRawData(sharedTextureVertices,
                               offsetof(HeatmapLayoutVertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(HeatmapLayoutVertex),
                               gfx::AttributeDataType::Short2);
    }

    auto heatmapTextureBuilder = context.createDrawableBuilder("heatmapTexture");
    heatmapTextureBuilder->setShader(heatmapTextureShader);
    heatmapTextureBuilder->setEnableDepth(false);
    heatmapTextureBuilder->setColorMode(gfx::ColorMode::alphaBlended());
    heatmapTextureBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());
    heatmapTextureBuilder->setRenderPass(renderPass);
    heatmapTextureBuilder->setVertexAttributes(std::move(textureVertexAttrs));
    heatmapTextureBuilder->setRawVertices({}, textureVertexCount, gfx::AttributeDataType::Short2);
    if (segments.empty()) {
        segments = RenderStaticData::heatmapTextureSegments();
    }
    heatmapTextureBuilder->setSegments(
        gfx::Triangles(), RenderStaticData::quadTriangleIndices().vector(), segments.data(), segments.size());

    heatmapTextureBuilder->setTexture(renderTarget->getTexture(), idHeatmapImageTexture);

    std::shared_ptr<gfx::Texture2D> texture = context.createTexture2D();
    texture->setImage(colorRamp);
    texture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                      .wrapU = gfx::TextureWrapType::Clamp,
                                      .wrapV = gfx::TextureWrapType::Clamp});
    heatmapTextureBuilder->setTexture(std::move(texture), idHeatmapColorRampTexture);

    heatmapTextureBuilder->flush(context);

    for (auto& drawable : heatmapTextureBuilder->clearDrawables()) {
        drawable->setLayerTweaker(textureTweaker);
        textureLayerGroup->addDrawable(std::move(drawable));
        ++stats.drawablesAdded;
    }
}

} // namespace mbgl
