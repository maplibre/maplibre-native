#include <mbgl/renderer/layers/render_heatmap_layer.hpp>
#include <mbgl/renderer/buckets/heatmap_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/style/layers/heatmap_layer_impl.hpp>
#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/intersection_tests.hpp>

#if MLN_DRAWABLE_RENDERER
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
#include <mbgl/util/string_indexer.hpp>
#endif

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
    colorRamp = std::make_shared<PremultipliedImage>(Size(256, 1));
}

RenderHeatmapLayer::~RenderHeatmapLayer() = default;

void RenderHeatmapLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    updateColorRamp();
}

void RenderHeatmapLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto properties = makeMutable<HeatmapLayerProperties>(staticImmutableCast<HeatmapLayer::Impl>(baseImpl),
                                                          unevaluated.evaluate(parameters));

    passes = (properties->evaluated.get<style::HeatmapOpacity>() > 0) ? (RenderPass::Translucent | RenderPass::Pass3D)
                                                                      : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);

#if MLN_DRAWABLE_RENDERER
    if (layerGroup) {
        auto newTextureTweaker = std::make_shared<HeatmapTextureLayerTweaker>(getID(), evaluatedProperties);
        replaceTweaker(textureTweaker, std::move(newTextureTweaker), {layerGroup});
    }

    if (renderTarget) {
        if (auto tileLayerGroup = renderTarget->getLayerGroup(0)) {
            auto newTweaker = std::make_shared<HeatmapLayerTweaker>(getID(), evaluatedProperties);
            replaceTweaker(layerTweaker, std::move(newTweaker), {std::move(tileLayerGroup)});
        }
    }
#endif
}

bool RenderHeatmapLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderHeatmapLayer::hasCrossfade() const {
    return false;
}

#if MLN_LEGACY_RENDERER
void RenderHeatmapLayer::upload(gfx::UploadPass& uploadPass) {
    if (!colorRampTexture) {
        colorRampTexture = uploadPass.createTexture(*colorRamp, gfx::TextureChannelDataType::UnsignedByte);
    }
}

void RenderHeatmapLayer::render(PaintParameters& parameters) {
    assert(renderTiles);
    if (parameters.pass == RenderPass::Opaque) {
        return;
    }

    if (!parameters.shaders.getLegacyGroup().populate(heatmapProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(heatmapTextureProgram)) return;

    if (parameters.pass == RenderPass::Pass3D) {
        const auto& viewportSize = parameters.staticData.backendSize;
        const auto size = Size{viewportSize.width / 4, viewportSize.height / 4};

        assert(colorRampTexture);

        if (!renderTexture || renderTexture->getSize() != size) {
            renderTexture = parameters.context.createOffscreenTexture(size, gfx::TextureChannelDataType::HalfFloat);
        }

        auto renderPass = parameters.encoder->createRenderPass("heatmap texture",
                                                               {*renderTexture, Color{0.0f, 0.0f, 0.0f, 1.0f}, {}, {}});

        for (const RenderTile& tile : *renderTiles) {
            const LayerRenderData* renderData = getRenderDataForPass(tile, parameters.pass);
            if (!renderData) {
                continue;
            }
            auto& bucket = static_cast<HeatmapBucket&>(*renderData->bucket);
            const auto& evaluated = getEvaluated<HeatmapLayerProperties>(renderData->layerProperties);

            const auto extrudeScale = tile.id.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom()));

            const auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());

            const auto allUniformValues = HeatmapProgram::computeAllUniformValues(
                HeatmapProgram::LayoutUniformValues{
                    uniforms::intensity::Value(evaluated.get<style::HeatmapIntensity>()),
                    uniforms::matrix::Value(tile.matrix),
                    uniforms::heatmap::extrude_scale::Value(extrudeScale)},
                paintPropertyBinders,
                evaluated,
                static_cast<float>(parameters.state.getZoom()));
            const auto allAttributeBindings = HeatmapProgram::computeAllAttributeBindings(
                *bucket.vertexBuffer, paintPropertyBinders, evaluated);

            checkRenderability(parameters, HeatmapProgram::activeBindingCount(allAttributeBindings));

            heatmapProgram->draw(parameters.context,
                                 *renderPass,
                                 gfx::Triangles(),
                                 gfx::DepthMode::disabled(),
                                 gfx::StencilMode::disabled(),
                                 gfx::ColorMode::additive(),
                                 gfx::CullFaceMode::disabled(),
                                 *bucket.indexBuffer,
                                 bucket.segments,
                                 allUniformValues,
                                 allAttributeBindings,
                                 HeatmapProgram::TextureBindings{},
                                 getID());
        }

    } else if (parameters.pass == RenderPass::Translucent) {
        const auto& size = parameters.staticData.backendSize;

        mat4 viewportMat;
        matrix::ortho(viewportMat, 0, size.width, size.height, 0, 0, 1);

        const Properties<>::PossiblyEvaluated properties;
        const HeatmapTextureProgram::Binders paintAttributeData{properties, 0};

        const auto allUniformValues = HeatmapTextureProgram::computeAllUniformValues(
            HeatmapTextureProgram::LayoutUniformValues{
                uniforms::matrix::Value(viewportMat),
                uniforms::world::Value(size),
                uniforms::opacity::Value(
                    getEvaluated<HeatmapLayerProperties>(evaluatedProperties).get<HeatmapOpacity>())},
            paintAttributeData,
            properties,
            static_cast<float>(parameters.state.getZoom()));
        const auto allAttributeBindings = HeatmapTextureProgram::computeAllAttributeBindings(
            *parameters.staticData.heatmapTextureVertexBuffer, paintAttributeData, properties);

        checkRenderability(parameters, HeatmapTextureProgram::activeBindingCount(allAttributeBindings));

        if (segments.empty()) {
            // Copy over the segments so that we can create our own DrawScopes.
            segments = RenderStaticData::heatmapTextureSegments();
        }
        heatmapTextureProgram->draw(
            parameters.context,
            *parameters.renderPass,
            gfx::Triangles(),
            gfx::DepthMode::disabled(),
            gfx::StencilMode::disabled(),
            parameters.colorModeForRenderPass(),
            gfx::CullFaceMode::disabled(),
            *parameters.staticData.quadTriangleIndexBuffer,
            segments,
            allUniformValues,
            allAttributeBindings,
            HeatmapTextureProgram::TextureBindings{
                textures::image::Value{renderTexture->getTexture().getResource(), gfx::TextureFilterType::Linear},
                textures::color_ramp::Value{colorRampTexture->getResource(), gfx::TextureFilterType::Linear},
            },
            getID());
    }
}
#endif // MLN_LEGACY_RENDERER

void RenderHeatmapLayer::updateColorRamp() {
    if (colorRamp) {
        auto colorValue = unevaluated.get<HeatmapColor>().getValue();
        if (colorValue.isUndefined()) {
            colorValue = HeatmapLayer::getDefaultHeatmapColor();
        }

        if (applyColorRamp(colorValue, *colorRamp)) {
            colorRampTexture = std::nullopt;
        }
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

#if MLN_DRAWABLE_RENDERER
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
const StringIdentity idHeatmapInterpolateUBOName = stringIndexer().get("HeatmapInterpolateUBO");
const StringIdentity idVertexAttribName = stringIndexer().get("a_pos");
const StringIdentity idTexImageName = stringIndexer().get("u_image");
const StringIdentity idTexColorRampName = stringIndexer().get("u_color_ramp");

} // namespace

using namespace shaders;

void RenderHeatmapLayer::update(gfx::ShaderRegistry& shaders,
                                gfx::Context& context,
                                const TransformState& state,
                                const std::shared_ptr<UpdateParameters>& updateParameters,
                                [[maybe_unused]] const RenderTree& renderTree,
                                UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

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
    layerTweaker->enableOverdrawInspector(!!(updateParameters->debugOptions & MapDebugOptions::Overdraw));

    std::unique_ptr<gfx::DrawableBuilder> heatmapBuilder;
    constexpr auto renderPass = RenderPass::Translucent;

    if (!(mbgl::underlying_type(renderPass) & evaluatedProperties->renderPasses)) {
        return;
    }

    stats.drawablesRemoved += tileLayerGroup->removeDrawablesIf(
        [&](gfx::Drawable& drawable) { return drawable.getTileID() && !hasRenderTile(*drawable.getTileID()); });

    const auto& evaluated = static_cast<const HeatmapLayerProperties&>(*evaluatedProperties).evaluated;
    std::unordered_set<StringIdentity> propertiesAsUniforms;

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
        if (!renderData) {
            removeTile(renderPass, tileID);
            continue;
        }

        const auto& bucket = static_cast<HeatmapBucket&>(*renderData->bucket);
        const auto vertexCount = bucket.vertices.elements();
        const auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
            removeTile(renderPass, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());

        const float zoom = static_cast<float>(state.getZoom());

        gfx::UniformBufferPtr interpolateBuffer;
        const auto getInterpolateBuffer = [&]() {
            if (!interpolateBuffer) {
                const HeatmapInterpolateUBO interpolateUBO = {
                    /* .weight_t = */ std::get<0>(paintPropertyBinders.get<HeatmapWeight>()->interpolationFactor(zoom)),
                    /* .radius_t = */ std::get<0>(paintPropertyBinders.get<HeatmapRadius>()->interpolationFactor(zoom)),
                    /* .padding = */ {0}};
                interpolateBuffer = context.createUniformBuffer(&interpolateUBO, sizeof(interpolateUBO), false);
            }
            return interpolateBuffer;
        };

        gfx::VertexAttributeArray heatmapVertexAttrs;
        propertiesAsUniforms.clear();
        heatmapVertexAttrs.readDataDrivenPaintProperties<HeatmapWeight, HeatmapRadius>(
            paintPropertyBinders, evaluated, propertiesAsUniforms);

        if (layerTweaker) {
            layerTweaker->setPropertiesAsUniforms(propertiesAsUniforms);
        }

        const auto updatedCount = tileLayerGroup->visitDrawables(renderPass, tileID, [&](gfx::Drawable& drawable) {
            if (drawable.getLayerTweaker() != layerTweaker) {
                // This drawable was produced on a previous style/bucket, and should not be updated.
                return;
            }

            // We assume vertex attributes don't change, and so don't update them to avoid re-uploading
            // drawable.setVertexAttributes(heatmapVertexAttrs);

            auto& uniforms = drawable.mutableUniformBuffers();

            if (auto buffer = getInterpolateBuffer()) {
                uniforms.addOrReplace(idHeatmapInterpolateUBOName, std::move(buffer));
            }
        });

        if (updatedCount > 0) {
            continue;
        }

        const auto heatmapShader = heatmapShaderGroup->getOrCreateShader(context, propertiesAsUniforms);
        if (!heatmapShader) {
            continue;
        }

        if (const auto& attr = heatmapVertexAttrs.add(idVertexAttribName)) {
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
        heatmapBuilder->setEnableStencil(false);
        heatmapBuilder->setRenderPass(renderPass);
        heatmapBuilder->setVertexAttributes(std::move(heatmapVertexAttrs));
        heatmapBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
        heatmapBuilder->setSegments(
            gfx::Triangles(), bucket.sharedTriangles, bucket.segments.data(), bucket.segments.size());

        heatmapBuilder->flush();

        for (auto& drawable : heatmapBuilder->clearDrawables()) {
            drawable->setTileID(tileID);
            drawable->setLayerTweaker(layerTweaker);

            auto& uniforms = drawable->mutableUniformBuffers();

            if (auto buffer = getInterpolateBuffer()) {
                uniforms.addOrReplace(idHeatmapInterpolateUBOName, std::move(buffer));
            }

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
    textureTweaker->enableOverdrawInspector(!!(updateParameters->debugOptions & MapDebugOptions::Overdraw));

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

    std::unique_ptr<gfx::DrawableBuilder> heatmapTextureBuilder;

    if (!sharedTextureVertices) {
        sharedTextureVertices = std::make_shared<TextureVertexVector>(RenderStaticData::heatmapTextureVertices());
    }
    const auto textureVertexCount = sharedTextureVertices->elements();

    gfx::VertexAttributeArray textureVertexAttrs;
    if (const auto& attr = textureVertexAttrs.add(idVertexAttribName)) {
        attr->setSharedRawData(sharedTextureVertices,
                               offsetof(HeatmapLayoutVertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(HeatmapLayoutVertex),
                               gfx::AttributeDataType::Short2);
    }

    heatmapTextureBuilder = context.createDrawableBuilder("heatmapTexture");
    heatmapTextureBuilder->setShader(heatmapTextureShader);
    heatmapTextureBuilder->setEnableDepth(false);
    heatmapTextureBuilder->setColorMode(gfx::ColorMode::alphaBlended());
    heatmapTextureBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());
    heatmapTextureBuilder->setEnableStencil(false);
    heatmapTextureBuilder->setRenderPass(renderPass);
    heatmapTextureBuilder->setVertexAttributes(std::move(textureVertexAttrs));
    heatmapTextureBuilder->setRawVertices({}, textureVertexCount, gfx::AttributeDataType::Short2);
    if (segments.empty()) {
        segments = RenderStaticData::heatmapTextureSegments();
    }
    heatmapTextureBuilder->setSegments(
        gfx::Triangles(), RenderStaticData::quadTriangleIndices().vector(), segments.data(), segments.size());

    auto imageLocation = heatmapTextureShader->getSamplerLocation(idTexImageName);
    if (imageLocation.has_value()) {
        heatmapTextureBuilder->setTexture(renderTarget->getTexture(), imageLocation.value());
    }
    auto colorRampLocation = heatmapTextureShader->getSamplerLocation(idTexColorRampName);
    if (colorRampLocation.has_value()) {
        std::shared_ptr<gfx::Texture2D> texture = context.createTexture2D();
        texture->setImage(colorRamp);
        texture->setSamplerConfiguration(
            {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        heatmapTextureBuilder->setTexture(std::move(texture), colorRampLocation.value());
    }

    heatmapTextureBuilder->flush();

    for (auto& drawable : heatmapTextureBuilder->clearDrawables()) {
        drawable->setLayerTweaker(textureTweaker);
        textureLayerGroup->addDrawable(std::move(drawable));
        ++stats.drawablesAdded;
    }
}
#endif // MLN_DRAWABLE_RENDERER

} // namespace mbgl
