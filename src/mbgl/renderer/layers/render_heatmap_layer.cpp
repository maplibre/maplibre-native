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
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/gfx/shader_registry.hpp>
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
    if (renderTarget) {
        renderTarget->getLayerGroup(0)->setLayerTweaker(std::make_shared<HeatmapLayerTweaker>(evaluatedProperties));
    }
    if (layerGroup) {
        layerGroup->setLayerTweaker(std::make_shared<HeatmapTextureLayerTweaker>(evaluatedProperties));
    }
#endif
}

bool RenderHeatmapLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderHeatmapLayer::hasCrossfade() const {
    return false;
}

void RenderHeatmapLayer::upload(gfx::UploadPass& uploadPass) {
    if (!colorRampTexture) {
        colorRampTexture = uploadPass.createTexture(*colorRamp, gfx::TextureChannelDataType::UnsignedByte);
    }
}

#if MLN_LEGACY_RENDERER
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
    auto colorValue = unevaluated.get<HeatmapColor>().getValue();
    if (colorValue.isUndefined()) {
        colorValue = HeatmapLayer::getDefaultHeatmapColor();
    }

    const auto length = (*colorRamp).bytes();

    for (uint32_t i = 0; i < length; i += 4) {
        const auto color = colorValue.evaluate(static_cast<double>(i) / length);
        (*colorRamp).data[i + 0] = static_cast<uint8_t>(std::floor(color.r * 255.f));
        (*colorRamp).data[i + 1] = static_cast<uint8_t>(std::floor(color.g * 255.f));
        (*colorRamp).data[i + 2] = static_cast<uint8_t>(std::floor(color.b * 255.f));
        (*colorRamp).data[i + 3] = static_cast<uint8_t>(std::floor(color.a * 255.f));
    }

    if (colorRampTexture) {
        colorRampTexture = std::nullopt;
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
void RenderHeatmapLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
    RenderLayer::markLayerRenderable(willRender, changes);

    if (renderTarget) {
        if (willRender) {
            // The RenderTree has determined this render target should be included in the renderable set for a frame
            changes.emplace_back(std::make_unique<AddRenderTargetRequest>(renderTarget));
        } else {
            // The RenderTree is informing us we should not render anything
            changes.emplace_back(std::make_unique<RemoveRenderTargetRequest>(renderTarget));
        }
    }
}

void RenderHeatmapLayer::removeTile(RenderPass renderPass, const OverscaledTileID& tileID) {
    auto* tileLayerGroup = static_cast<TileLayerGroup*>(renderTarget->getLayerGroup(0).get());
    stats.drawablesRemoved += tileLayerGroup->removeDrawables(renderPass, tileID).size();
}

void RenderHeatmapLayer::removeAllDrawables() {
    RenderLayer::removeAllDrawables();
    if (renderTarget) {
        stats.drawablesRemoved += renderTarget->getLayerGroup(0)->getDrawableCount();
        renderTarget->getLayerGroup(0)->clearDrawables();
    }
}

struct alignas(16) HeatmapInterpolateUBO {
    float weight_t;
    float radius_t;
    std::array<float, 2> padding;
};
static_assert(sizeof(HeatmapInterpolateUBO) % 16 == 0);

static const std::string HeatmapShaderGroupName = "HeatmapShader";
static const std::string HeatmapTextureShaderGroupName = "HeatmapTextureShader";
static constexpr std::string_view HeatmapInterpolateUBOName = "HeatmapInterpolateUBO";

void RenderHeatmapLayer::update(gfx::ShaderRegistry& shaders,
                                gfx::Context& context,
                                const TransformState& state,
                                [[maybe_unused]] const RenderTree& renderTree,
                                [[maybe_unused]] UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

    if (!renderTiles || renderTiles->empty()) {
        removeAllDrawables();
        return;
    }

    // Set up a render target
    if (!renderTarget) {
        renderTarget = context.createRenderTarget();
        if (!renderTarget) {
            return;
        }

        // Set up tile layer group
        auto tileLayerGroup = context.createTileLayerGroup(0, /*initialCapacity=*/64, getID());
        if (!tileLayerGroup) {
            return;
        }
        tileLayerGroup->setLayerTweaker(std::make_shared<HeatmapLayerTweaker>(evaluatedProperties));
        renderTarget->addLayerGroup(tileLayerGroup, /*canReplace=*/true);
    }

    auto* tileLayerGroup = static_cast<TileLayerGroup*>(renderTarget->getLayerGroup(0).get());

    if (!heatmapShaderGroup) {
        heatmapShaderGroup = shaders.getShaderGroup(HeatmapShaderGroupName);
    }
    if (!heatmapShaderGroup) {
        removeAllDrawables();
        return;
    }

    std::unordered_set<OverscaledTileID> newTileIDs(renderTiles->size());
    std::transform(renderTiles->begin(),
                   renderTiles->end(),
                   std::inserter(newTileIDs, newTileIDs.begin()),
                   [](const auto& renderTile) -> OverscaledTileID { return renderTile.get().getOverscaledTileID(); });

    std::unique_ptr<gfx::DrawableBuilder> heatmapBuilder;
    std::vector<gfx::DrawablePtr> newTiles;
    gfx::VertexAttributeArray heatmapVertexAttrs;
    auto renderPass = RenderPass::Translucent;

    if (!(mbgl::underlying_type(renderPass) & evaluatedProperties->renderPasses)) {
        return;
    }

    tileLayerGroup->observeDrawables([&](gfx::UniqueDrawable& drawable) {
        const auto tileID = drawable->getTileID();
        if (tileID && newTileIDs.find(*tileID) == newTileIDs.end()) {
            // remove it
            drawable.reset();
            ++stats.drawablesRemoved;
        }
    });

    const auto& evaluated = static_cast<const HeatmapLayerProperties&>(*evaluatedProperties).evaluated;

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
        if (!renderData) {
            removeTile(renderPass, tileID);
            continue;
        }

        auto& bucket = static_cast<HeatmapBucket&>(*renderData->bucket);
        const auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());

        float zoom = static_cast<float>(state.getZoom());
        const HeatmapInterpolateUBO interpolateUBO = {
            /* .weight_t = */ std::get<0>(paintPropertyBinders.get<HeatmapWeight>()->interpolationFactor(zoom)),
            /* .radius_t = */ std::get<0>(paintPropertyBinders.get<HeatmapRadius>()->interpolationFactor(zoom)),
            /* .padding = */ {0}};

        tileLayerGroup->observeDrawables(renderPass, tileID, [&](gfx::Drawable& drawable) {
            drawable.mutableUniformBuffers().createOrUpdate(HeatmapInterpolateUBOName, &interpolateUBO, context);
        });

        if (tileLayerGroup->getDrawableCount(renderPass, tileID) > 0) {
            continue;
        }

        heatmapVertexAttrs.clear();

        auto propertiesAsUniforms = heatmapVertexAttrs.readDataDrivenPaintProperties<HeatmapWeight, HeatmapRadius>(
            paintPropertyBinders, evaluated);

        auto heatmapShader = heatmapShaderGroup->getOrCreateShader(context, propertiesAsUniforms);
        if (!heatmapShader) {
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

        heatmapBuilder = context.createDrawableBuilder("heatmap");
        heatmapBuilder->setShader(std::static_pointer_cast<gfx::ShaderProgramBase>(heatmapShader));
        heatmapBuilder->setDepthType((renderPass == RenderPass::Opaque) ? gfx::DepthMaskType::ReadWrite
                                                                        : gfx::DepthMaskType::ReadOnly);
        heatmapBuilder->setColorMode(gfx::ColorMode::additive());
        heatmapBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());

        heatmapBuilder->setRenderPass(renderPass);
        heatmapBuilder->setVertexAttributes(heatmapVertexAttrs);

        buildVertices();
        heatmapBuilder->addVertices(rawVerts, 0, rawVerts.size());

        heatmapBuilder->setSegments(
            gfx::Triangles(), bucket.triangles.vector(), bucket.segments.data(), bucket.segments.size());

        heatmapBuilder->flush();

        for (auto& drawable : heatmapBuilder->clearDrawables()) {
            drawable->setTileID(tileID);
            drawable->mutableUniformBuffers().createOrUpdate(HeatmapInterpolateUBOName, &interpolateUBO, context);

            tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
            ++stats.drawablesAdded;
        }
    }

    // Set up texture layer group
    if (!layerGroup) {
        auto layerGroup_ = context.createLayerGroup(layerIndex, /*initialCapacity=*/1, getID());
        if (!layerGroup_) {
            return;
        }
        layerGroup_->setLayerTweaker(std::make_shared<HeatmapTextureLayerTweaker>(evaluatedProperties));
        setLayerGroup(std::move(layerGroup_), changes);
    }

    auto* textureLayerGroup = static_cast<LayerGroup*>(layerGroup.get());

    if (!heatmapTextureShader) {
        heatmapTextureShader = context.getGenericShader(shaders, HeatmapTextureShaderGroupName);
    }
    if (!heatmapTextureShader) {
        removeAllDrawables();
        return;
    }

    textureLayerGroup->clearDrawables();

    std::unique_ptr<gfx::DrawableBuilder> heatmapTextureBuilder;

    std::vector<std::array<int16_t, 2>> textureRawVerts;
    const auto buildTextureVertices = [&]() {
        const std::vector<gfx::VertexVector<gfx::detail::VertexType<gfx::AttributeType<int16_t, 2>>>::Vertex> verts =
            RenderStaticData::heatmapTextureVertices().vector();
        if (textureRawVerts.size() < verts.size()) {
            textureRawVerts.resize(verts.size());
            std::transform(verts.begin(), verts.end(), textureRawVerts.begin(), [](const auto& x) { return x.a1; });
        }
    };

    heatmapTextureBuilder = context.createDrawableBuilder("heatmapTexture");
    heatmapTextureBuilder->setShader(heatmapTextureShader);
    heatmapTextureBuilder->setDepthType((renderPass == RenderPass::Opaque) ? gfx::DepthMaskType::ReadWrite
                                                                           : gfx::DepthMaskType::ReadOnly);
    heatmapTextureBuilder->setColorMode(gfx::ColorMode::alphaBlended());
    heatmapTextureBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());

    heatmapTextureBuilder->setRenderPass(renderPass);

    buildTextureVertices();
    heatmapTextureBuilder->addVertices(textureRawVerts, 0, textureRawVerts.size());

    if (segments.empty()) {
        segments = RenderStaticData::heatmapTextureSegments();
    }
    
    heatmapTextureBuilder->setSegments(
        gfx::Triangles(), RenderStaticData::quadTriangleIndices().vector(), segments.data(), segments.size());

    auto imageLocation = heatmapTextureShader->getSamplerLocation("u_image");
    if (imageLocation.has_value()) {
        heatmapTextureBuilder->setTexture(renderTarget->getTexture(), imageLocation.value());
    }
    auto colorRampLocation = heatmapTextureShader->getSamplerLocation("u_color_ramp");
    if (colorRampLocation.has_value()) {
        std::shared_ptr<gfx::Texture2D> texture = context.createTexture2D();
        texture->setImage(colorRamp);
        texture->setSamplerConfiguration(
            {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        heatmapTextureBuilder->setTexture(texture, colorRampLocation.value());
    }

    heatmapTextureBuilder->flush();

    for (auto& drawable : heatmapTextureBuilder->clearDrawables()) {
        textureLayerGroup->addDrawable(std::move(drawable));
        ++stats.drawablesAdded;
    }
}
#endif // MLN_DRAWABLE_RENDERER

} // namespace mbgl
