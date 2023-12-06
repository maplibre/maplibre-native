#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/gfx/context.hpp>
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
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/style/layers/fill_layer_impl.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/intersection_tests.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/std.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gfx/drawable_atlases_tweaker.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/renderer/layers/fill_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/util/string_indexer.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>
#endif

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {

#if MLN_DRAWABLE_RENDERER
constexpr auto FillShaderName = "FillShader";
#if MLN_TRIANGULATE_FILL_OUTLINES
constexpr auto FillOutlineTriangulatedShaderName = "LineBasicShader";
#endif
constexpr auto FillOutlineShaderName = "FillOutlineShader";
constexpr auto FillPatternShaderName = "FillPatternShader";
constexpr auto FillOutlinePatternShaderName = "FillOutlinePatternShader";

const StringIdentity idFillOutlineInterpolateUBOName = stringIndexer().get("FillOutlineInterpolateUBO");
const StringIdentity idFillPatternInterpolateUBOName = stringIndexer().get("FillPatternInterpolateUBO");
const StringIdentity idFillPatternTilePropsUBOName = stringIndexer().get("FillPatternTilePropsUBO");
const StringIdentity idFillOutlinePatternInterpolateUBOName = stringIndexer().get("FillOutlinePatternInterpolateUBO");
const StringIdentity idFillOutlinePatternTilePropsUBOName = stringIndexer().get("FillOutlinePatternTilePropsUBO");

const StringIdentity idPosAttribName = stringIndexer().get("a_pos");
const StringIdentity idIconTextureName = stringIndexer().get("u_image");
#endif // MLN_DRAWABLE_RENDERER

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

#if MLN_DRAWABLE_RENDERER
    if (layerTweaker) {
        layerTweaker->updateProperties(evaluatedProperties);
    }
#endif
}

bool RenderFillLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderFillLayer::hasCrossfade() const {
    return getCrossfade<FillLayerProperties>(evaluatedProperties).t != 1;
}

#if MLN_TRIANGULATE_FILL_OUTLINES
void RenderFillLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
    RenderLayer::markLayerRenderable(willRender, changes);
    if (outlineLayerGroup) {
        activateLayerGroup(outlineLayerGroup, willRender, changes);
    }
}

void RenderFillLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    RenderLayer::layerRemoved(changes);
    if (outlineLayerGroup) {
        activateLayerGroup(outlineLayerGroup, false, changes);
    }
}

void RenderFillLayer::layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec& changes) {
    RenderLayer::layerIndexChanged(newLayerIndex, changes);

    changeLayerIndex(outlineLayerGroup, newLayerIndex, changes);
}

std::size_t RenderFillLayer::removeAllDrawables() {
    auto count = RenderLayer::removeAllDrawables();
    if (outlineLayerGroup) {
        auto count2 = outlineLayerGroup->clearDrawables();
        stats.drawablesRemoved += count2;
        count += count2;
    }
    return count;
}

std::size_t RenderFillLayer::removeTile(RenderPass renderPass, const OverscaledTileID& tileID) {
    RenderLayer::removeTile(renderPass, tileID);
    if (const auto tileGroup = static_cast<TileLayerGroup*>(outlineLayerGroup.get())) {
        const auto n = tileGroup->removeDrawables(renderPass, tileID).size();
        stats.drawablesRemoved += n;
        return n;
    }
    return 0;
}

#endif // MLN_TRIANGULATE_FILL_OUTLINES

#if MLN_LEGACY_RENDERER
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
                     bucket.basicLineSegments,
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
                     bucket.basicLineSegments,
                     FillOutlinePatternProgram::TextureBindings{
                         tile.getIconAtlasTextureBinding(gfx::TextureFilterType::Linear),
                     });
            }
        }
    }
}
#endif // MLN_LEGACY_RENDERER

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

#if MLN_DRAWABLE_RENDERER

#if MLN_TRIANGULATE_FILL_OUTLINES
class OutlineDrawableTweaker : public gfx::DrawableTweaker {
public:
    OutlineDrawableTweaker(Color color_, float opacity_)
        : color(color_),
          opacity(opacity_) {}
    ~OutlineDrawableTweaker() override = default;

    void init(gfx::Drawable&) override{};

    void execute(gfx::Drawable& drawable, const PaintParameters& parameters) override {
        if (!drawable.getTileID().has_value()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto zoom = parameters.state.getZoom();
        auto& uniforms = drawable.mutableUniformBuffers();

        static const StringIdentity idLineUBOName = stringIndexer().get("LineBasicUBO");
        {
            const auto matrix = LayerTweaker::getTileMatrix(
                tileID, parameters, {{0, 0}}, style::TranslateAnchorType::Viewport, false, false, false);

            const shaders::LineBasicUBO lineUBO{
                /*matrix = */ util::cast<float>(matrix),
                /*units_to_pixels = */ {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                /*ratio = */ 1.0f / tileID.pixelsToTileUnits(1.0f, zoom),
                /*device_pixel_ratio = */ parameters.pixelRatio};
            parameters.context.emplaceOrUpdateUniformBuffer(lineUniformBuffer, &lineUBO);
        }
        uniforms.addOrReplace(idLineUBOName, lineUniformBuffer);

        static const StringIdentity idLinePropertiesUBOName = stringIndexer().get("LineBasicPropertiesUBO");
        if (!linePropertiesUniformBuffer) {
            const shaders::LineBasicPropertiesUBO linePropertiesUBO{/*color =*/color,
                                                                    /*opacity =*/opacity,
                                                                    /*width =*/1.f,
                                                                    0,
                                                                    0};
            parameters.context.emplaceOrUpdateUniformBuffer(linePropertiesUniformBuffer, &linePropertiesUBO);
        }
        if (!uniforms.get(idLinePropertiesUBOName)) {
            uniforms.addOrReplace(idLinePropertiesUBOName, linePropertiesUniformBuffer);
        }
    };

private:
    Color color;
    float opacity;

    gfx::UniformBufferPtr lineUniformBuffer;
    gfx::UniformBufferPtr linePropertiesUniformBuffer;
};
#endif // MLN_TRIANGULATE_FILL_OUTLINES

void RenderFillLayer::update(gfx::ShaderRegistry& shaders,
                             gfx::Context& context,
                             const TransformState& state,
                             const std::shared_ptr<UpdateParameters>& updateParameters,
                             [[maybe_unused]] const RenderTree& renderTree,
                             [[maybe_unused]] UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

    if (!renderTiles || renderTiles->empty()) {
        removeAllDrawables();
        return;
    }

    // Set up a layer group for fill
    if (!layerGroup) {
        if (auto layerGroup_ = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID())) {
            setLayerGroup(std::move(layerGroup_), changes);
        } else {
            return;
        }
    }
    auto* fillTileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());

#if MLN_TRIANGULATE_FILL_OUTLINES
    // Set up a layer group for outlines
    if (!outlineLayerGroup) {
        outlineLayerGroup = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID());
        activateLayerGroup(outlineLayerGroup, isRenderable, changes);
    }
    auto* outlineTileLayerGroup = static_cast<TileLayerGroup*>(outlineLayerGroup.get());
    if (!outlineTriangulatedShaderGroup) {
        outlineTriangulatedShaderGroup = shaders.getShaderGroup(std::string(FillOutlineTriangulatedShaderName));
    }
#endif

    if (!layerTweaker) {
        layerTweaker = std::make_shared<FillLayerTweaker>(getID(), evaluatedProperties);
        layerGroup->addLayerTweaker(layerTweaker);
    }

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
        removeAllDrawables();
        return;
    }

    std::unique_ptr<gfx::DrawableBuilder> fillBuilder;
    std::unique_ptr<gfx::DrawableBuilder> outlineBuilder;
    std::unique_ptr<gfx::DrawableBuilder> patternBuilder;
    std::unique_ptr<gfx::DrawableBuilder> outlinePatternBuilder;

    const auto layerPrefix = getID() + "/";
    constexpr auto renderPass = RenderPass::Translucent;

    const auto commonInit = [&](gfx::DrawableBuilder& builder) {
        builder.setCullFaceMode(gfx::CullFaceMode::disabled());
        builder.setEnableStencil(true);
    };

#if MLN_TRIANGULATE_FILL_OUTLINES
    for (auto layer : {fillTileLayerGroup, outlineTileLayerGroup}) {
        stats.drawablesRemoved += layer->removeDrawablesIf([&](gfx::Drawable& drawable) {
            // If the render pass has changed or the tile has dropped out of the cover set, remove it.
            const auto& tileID = drawable.getTileID();
            return tileID && !hasRenderTile(*tileID);
        });
    }
#else
    stats.drawablesRemoved += fillTileLayerGroup->removeDrawablesIf([&](gfx::Drawable& drawable) {
        // If the render pass has changed or the tile has dropped out of the cover set, remove it.
        const auto& tileID = drawable.getTileID();
        return tileID && !hasRenderTile(*tileID);
    });
#endif

    fillTileLayerGroup->setStencilTiles(renderTiles);

    mbgl::unordered_set<StringIdentity> propertiesAsUniforms;
    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
        if (!renderData || !renderData->bucket || !renderData->bucket->hasData()) {
            removeTile(renderPass, tileID);
            continue;
        }

        auto& bucket = static_cast<FillBucket&>(*renderData->bucket);
        const auto& binders = bucket.paintPropertyBinders.at(getID());

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
            removeTile(renderPass, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());

        const auto& evaluated = getEvaluated<FillLayerProperties>(renderData->layerProperties);
        const auto& crossfade = getCrossfade<FillLayerProperties>(renderData->layerProperties);

        const auto& fillPatternValue = evaluated.get<FillPattern>().constantOr(Faded<expression::Image>{"", ""});
        const auto patternPosA = tile.getPattern(fillPatternValue.from.id());
        const auto patternPosB = tile.getPattern(fillPatternValue.to.id());
        binders.setPatternParameters(patternPosA, patternPosB, crossfade);

        const auto zoom = static_cast<float>(state.getZoom());

        std::optional<FillInterpolateUBO> fillInterpolateUBO = std::nullopt;
        std::optional<FillOutlineInterpolateUBO> fillOutlineInterpolateUBO = std::nullopt;
        std::optional<FillPatternInterpolateUBO> fillPatternInterpolateUBO = std::nullopt;
        std::optional<FillOutlinePatternInterpolateUBO> fillOutlinePatternInterpolateUBO = std::nullopt;

        auto getFillInterpolateUBO = [&]() -> const FillInterpolateUBO& {
            if (!fillInterpolateUBO) {
                fillInterpolateUBO = {
                    /* .color_t = */ std::get<0>(binders.get<FillColor>()->interpolationFactor(zoom)),
                    /* .opacity_t = */ std::get<0>(binders.get<FillOpacity>()->interpolationFactor(zoom)),
                    0,
                    0,
                };
            }

            return *fillInterpolateUBO;
        };

        auto getFillOutlineInterpolateUBO = [&]() -> const FillOutlineInterpolateUBO& {
            if (!fillOutlineInterpolateUBO) {
                fillOutlineInterpolateUBO = {
                    /* .color_t = */ std::get<0>(binders.get<FillOutlineColor>()->interpolationFactor(zoom)),
                    /* .opacity_t = */ std::get<0>(binders.get<FillOpacity>()->interpolationFactor(zoom)),
                    0,
                    0,
                };
            }

            return *fillOutlineInterpolateUBO;
        };

        auto getFillPatternInterpolateUBO = [&]() -> const FillPatternInterpolateUBO& {
            if (!fillPatternInterpolateUBO) {
                fillPatternInterpolateUBO = {
                    /* .pattern_from_t = */ std::get<0>(binders.get<FillPattern>()->interpolationFactor(zoom)),
                    /* .pattern_to_t = */ std::get<0>(binders.get<FillPattern>()->interpolationFactor(zoom)),
                    /* .opacity_t = */ std::get<0>(binders.get<FillOpacity>()->interpolationFactor(zoom)),
                    0,
                };
            }

            return *fillPatternInterpolateUBO;
        };

        auto getFillOutlinePatternInterpolateUBO = [&]() -> const FillOutlinePatternInterpolateUBO& {
            if (!fillOutlinePatternInterpolateUBO) {
                fillOutlinePatternInterpolateUBO = {
                    /* .pattern_from_t = */ std::get<0>(binders.get<FillPattern>()->interpolationFactor(zoom)),
                    /* .pattern_to_t = */ std::get<0>(binders.get<FillPattern>()->interpolationFactor(zoom)),
                    /* .opacity_t = */ std::get<0>(binders.get<FillOpacity>()->interpolationFactor(zoom)),
                    0,
                };
            }

            return *fillOutlinePatternInterpolateUBO;
        };

        std::optional<FillPatternTilePropsUBO> fillPatternTilePropsUBO = std::nullopt;
        auto getFillPatternTilePropsUBO = [&]() -> const FillPatternTilePropsUBO& {
            if (!fillPatternTilePropsUBO) {
                fillPatternTilePropsUBO = {
                    /* pattern_from = */ patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},
                    /* pattern_to = */ patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},
                };
            }

            return *fillPatternTilePropsUBO;
        };

        std::optional<FillOutlinePatternTilePropsUBO> fillOutlinePatternTilePropsUBO = std::nullopt;
        auto getFillOutlinePatternTilePropsUBO = [&]() -> const FillOutlinePatternTilePropsUBO& {
            if (!fillOutlinePatternTilePropsUBO) {
                fillOutlinePatternTilePropsUBO = {
                    /* pattern_from = */ patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},
                    /* pattern_to = */ patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},
                };
            }

            return *fillOutlinePatternTilePropsUBO;
        };

        gfx::DrawableTweakerPtr atlasTweaker;
        auto getAtlasTweaker = [&]() {
            if (!atlasTweaker) {
                if (const auto& atlases = tile.getAtlasTextures(); atlases && atlases->icon) {
                    atlasTweaker = std::make_shared<gfx::DrawableAtlasesTweaker>(
                        atlases,
                        0,
                        idIconTextureName,
                        /*isText*/ false,
                        /*sdfIcons*/ true, // to force linear filter
                        /*rotationAlignment_*/ AlignmentType::Auto,
                        /*iconScaled*/ false,
                        /*textSizeIsZoomConstant_*/ false);
                }
            }
            return atlasTweaker;
        };

        propertiesAsUniforms.clear();

        // `Fill*Program` all use `style::FillPaintProperties`
        auto vertexAttrs = context.createVertexAttributeArray();
        vertexAttrs->readDataDrivenPaintProperties<FillColor, FillOpacity, FillOutlineColor, FillPattern>(
            binders, evaluated, propertiesAsUniforms);

        const auto vertexCount = bucket.vertices.elements();
        if (const auto& attr = vertexAttrs->add(idPosAttribName)) {
            attr->setSharedRawData(bucket.sharedVertices,
                                   offsetof(FillLayoutVertex, a1),
                                   /*vertexOffset=*/0,
                                   sizeof(FillLayoutVertex),
                                   gfx::AttributeDataType::Short2);
        }

        // If we already have drawables for this tile, update them.
        auto updateExisting = [&](gfx::Drawable& drawable) {
            auto& uniforms = drawable.mutableUniformBuffers();
            switch (static_cast<FillVariant>(drawable.getType())) {
                case FillVariant::Fill: {
                    uniforms.createOrUpdate(
                        FillLayerTweaker::idFillInterpolateUBOName, &getFillInterpolateUBO(), context);
                    break;
                }
                case FillVariant::FillOutline: {
                    uniforms.createOrUpdate(idFillOutlineInterpolateUBOName, &getFillOutlineInterpolateUBO(), context);
                    break;
                }
                case FillVariant::FillPattern: {
                    uniforms.createOrUpdate(idFillPatternInterpolateUBOName, &getFillPatternInterpolateUBO(), context);
                    uniforms.createOrUpdate(idFillPatternTilePropsUBOName, &getFillPatternTilePropsUBO(), context);
                    break;
                }
                case FillVariant::FillOutlinePattern: {
                    uniforms.createOrUpdate(
                        idFillOutlinePatternInterpolateUBOName, &getFillOutlinePatternInterpolateUBO(), context);
                    uniforms.createOrUpdate(
                        idFillOutlinePatternTilePropsUBOName, &getFillOutlinePatternTilePropsUBO(), context);
                    break;
                }
                default: {
#ifndef NDEBUG
                    mbgl::Log::Error(mbgl::Event::Render, "Invalid fill variant type supplied during drawable update!");
#endif
                    break;
                }
            }

            drawable.setVertexAttributes(vertexAttrs);
            return true;
        };
        if (updateTile(renderPass, tileID, std::move(updateExisting))) {
            continue;
        }

        // Outline always occurs in translucent pass, defaults to fill color
        // Outline does not default to fill in the pattern case
        const auto doOutline = evaluated.get<FillAntialias>() && (unevaluated.get<FillPattern>().isUndefined() ||
                                                                  unevaluated.get<FillOutlineColor>().isUndefined());
#if MLN_TRIANGULATE_FILL_OUTLINES
        const bool dataDrivenOutline = !evaluated.get<FillOutlineColor>().isConstant() ||
                                       !evaluated.get<FillOpacity>().isConstant();
#endif

        if (unevaluated.get<FillPattern>().isUndefined()) {
            // Simple fill
            if (!fillShaderGroup || (doOutline && !outlineShaderGroup)) {
                continue;
            }
            const auto fillShader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                fillShaderGroup->getOrCreateShader(context, propertiesAsUniforms));

#if MLN_TRIANGULATE_FILL_OUTLINES
            const auto outlineTriangulatedShader = doOutline && !dataDrivenOutline ? [&]() -> auto {
                static const mbgl::unordered_set<StringIdentity> outlinePropertiesAsUniforms{
                    stringIndexer().get("a_color"),
                    stringIndexer().get("a_opacity"),
                    stringIndexer().get("a_width"),
                };
                return std::static_pointer_cast<gfx::ShaderProgramBase>(
                    outlineTriangulatedShaderGroup->getOrCreateShader(context, outlinePropertiesAsUniforms));
            }()
                : nullptr;

            auto createOutline = [&](auto& builder, Color color, float opacity) {
                if (doOutline && builder && bucket.sharedLineIndexes->elements()) {
                    static const StringIdentity idVertexAttribName = stringIndexer().get("a_pos_normal");
                    static const StringIdentity idDataAttribName = stringIndexer().get("a_data");
                    builder->setVertexAttrNameId(idVertexAttribName);
                    builder->setShader(outlineTriangulatedShader);
                    builder->setRawVertices({}, bucket.lineVertices.elements(), gfx::AttributeDataType::Short2);

                    auto attrs = context.createVertexAttributeArray();
                    if (const auto& attr = attrs->add(idVertexAttribName)) {
                        attr->setSharedRawData(bucket.sharedLineVertices,
                                               offsetof(LineLayoutVertex, a1),
                                               /*vertexOffset=*/0,
                                               sizeof(LineLayoutVertex),
                                               gfx::AttributeDataType::Short2);
                    }
                    if (const auto& attr = attrs->add(idDataAttribName)) {
                        attr->setSharedRawData(bucket.sharedLineVertices,
                                               offsetof(LineLayoutVertex, a2),
                                               /*vertexOffset=*/0,
                                               sizeof(LineLayoutVertex),
                                               gfx::AttributeDataType::UByte4);
                    }
                    builder->setVertexAttributes(std::move(attrs));

                    builder->setSegments(gfx::Triangles(),
                                         bucket.sharedLineIndexes,
                                         bucket.lineSegments.data(),
                                         bucket.lineSegments.size());

                    auto tweaker = std::make_shared<OutlineDrawableTweaker>(color, opacity);

                    // finish
                    builder->flush(context);
                    for (auto& drawable : builder->clearDrawables()) {
                        drawable->setTileID(tileID);
                        drawable->addTweaker(tweaker);
                        outlineTileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                        ++stats.drawablesAdded;
                    }
                }
            };
#endif
            const auto outlineShader = doOutline
                                           ? std::static_pointer_cast<gfx::ShaderProgramBase>(
                                                 outlineShaderGroup->getOrCreateShader(context, propertiesAsUniforms))
                                           : nullptr;

            if (!fillBuilder && fillShader) {
                if (auto builder = context.createDrawableBuilder(layerPrefix + "fill")) {
                    // Only write opaque fills to the depth buffer, matching `fillRenderPass` in legacy rendering
                    const bool opaque = (evaluated.get<FillColor>().constantOr(Color()).a >= 1.0f &&
                                         evaluated.get<FillOpacity>().constantOr(0) >= 1.0f);

                    commonInit(*builder);
                    builder->setDepthType(opaque ? gfx::DepthMaskType::ReadWrite : gfx::DepthMaskType::ReadOnly);
                    builder->setColorMode(opaque ? gfx::ColorMode::unblended() : gfx::ColorMode::alphaBlended());
                    builder->setSubLayerIndex(1);
                    builder->setRenderPass(renderPass);
                    fillBuilder = std::move(builder);
                }
            }
            if (doOutline && !outlineBuilder && outlineShader) {
                if (auto builder = context.createDrawableBuilder(layerPrefix + "fill-outline")) {
                    commonInit(*builder);
                    builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                    builder->setLineWidth(2.0f);
                    builder->setSubLayerIndex(unevaluated.get<FillOutlineColor>().isUndefined() ? 2 : 0);
                    builder->setColorMode(gfx::ColorMode::alphaBlended());
                    builder->setRenderPass(RenderPass::Translucent);
                    outlineBuilder = std::move(builder);
                }
            }

            const auto finish = [&](gfx::DrawableBuilder& builder,
                                    const StringIdentity interpolateUBONameId,
                                    const auto& interpolateUBO,
                                    FillVariant type) {
                builder.setVertexAttrNameId(idPosAttribName);
                builder.flush(context);

                for (auto& drawable : builder.clearDrawables()) {
                    drawable->setTileID(tileID);
                    drawable->setLayerTweaker(layerTweaker);
                    drawable->setType(static_cast<size_t>(type));

                    auto& uniforms = drawable->mutableUniformBuffers();
                    uniforms.createOrUpdate(interpolateUBONameId, &interpolateUBO, context);
                    fillTileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                    ++stats.drawablesAdded;
                }
            };

            if (fillBuilder && bucket.sharedTriangles->elements()) {
                fillBuilder->setShader(fillShader);
#if MLN_TRIANGULATE_FILL_OUTLINES
                if (doOutline && dataDrivenOutline && outlineBuilder) {
                    fillBuilder->setVertexAttributes(vertexAttrs);
                    outlineBuilder->setVertexAttributes(std::move(vertexAttrs));
                } else {
                    fillBuilder->setVertexAttributes(std::move(vertexAttrs));
                }
#else
                if (doOutline && outlineBuilder) {
                    fillBuilder->setVertexAttributes(vertexAttrs);
                    outlineBuilder->setVertexAttributes(std::move(vertexAttrs));
                } else {
                    fillBuilder->setVertexAttributes(std::move(vertexAttrs));
                }
#endif
                fillBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
                fillBuilder->setSegments(gfx::Triangles(),
                                         bucket.sharedTriangles,
                                         bucket.triangleSegments.data(),
                                         bucket.triangleSegments.size());
                finish(*fillBuilder,
                       FillLayerTweaker::idFillInterpolateUBOName,
                       getFillInterpolateUBO(),
                       FillVariant::Fill);
            }

#if MLN_TRIANGULATE_FILL_OUTLINES
            if (doOutline && outlineBuilder) {
                if (!dataDrivenOutline) {
                    outlineBuilder->setSubLayerIndex(unevaluated.get<FillOutlineColor>().isUndefined() ? 2 : 0);
                    createOutline(outlineBuilder,
                                  evaluated.get<FillOutlineColor>().constantOr(FillOutlineColor::defaultValue()),
                                  evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()));
                } else {
                    if (bucket.sharedBasicLineIndexes->elements()) {
                        outlineBuilder->setShader(outlineShader);
                        outlineBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
                        outlineBuilder->setSegments(gfx::Lines(2),
                                                    bucket.sharedBasicLineIndexes,
                                                    bucket.basicLineSegments.data(),
                                                    bucket.basicLineSegments.size());
                        finish(*outlineBuilder,
                               FillLayerTweaker::idFillOutlineInterpolateUBOName,
                               getFillOutlineInterpolateUBO(),
                               FillVariant::FillOutline);
                    }
                }
            }
#else
            if (doOutline && outlineBuilder && bucket.sharedBasicLineIndexes->elements()) {
                outlineBuilder->setShader(outlineShader);
                outlineBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
                outlineBuilder->setSegments(gfx::Lines(2),
                                            bucket.sharedBasicLineIndexes,
                                            bucket.basicLineSegments.data(),
                                            bucket.basicLineSegments.size());
                finish(*outlineBuilder,
                       FillLayerTweaker::idFillOutlineInterpolateUBOName,
                       getFillOutlineInterpolateUBO(),
                       FillVariant::FillOutline);
            }
#endif
        } else {
            // Fill with pattern
            if ((renderPass & RenderPass::Translucent) == 0) {
                continue;
            }

            if (!patternShaderGroup || (doOutline && !outlinePatternShaderGroup)) {
                continue;
            }

            const auto fillShader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                patternShaderGroup->getOrCreateShader(context, propertiesAsUniforms));
            const auto outlineShader = doOutline ? std::static_pointer_cast<gfx::ShaderProgramBase>(
                                                       outlinePatternShaderGroup->getOrCreateShader(
                                                           context, propertiesAsUniforms))
                                                 : nullptr;

            if (!patternBuilder) {
                if (auto builder = context.createDrawableBuilder(layerPrefix + "fill-pattern")) {
                    commonInit(*builder);
                    builder->setShader(fillShader);
                    builder->setDepthType(gfx::DepthMaskType::ReadWrite);
                    builder->setColorMode(gfx::ColorMode::alphaBlended());
                    builder->setSubLayerIndex(1);
                    builder->setRenderPass(RenderPass::Translucent);
                    patternBuilder = std::move(builder);
                }
            }
            if (doOutline && !outlinePatternBuilder) {
                if (auto builder = context.createDrawableBuilder(layerPrefix + "fill-outline-pattern")) {
                    commonInit(*builder);
                    builder->setShader(outlineShader);
                    builder->setLineWidth(2.0f);
                    builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                    builder->setColorMode(gfx::ColorMode::alphaBlended());
                    builder->setSubLayerIndex(2);
                    builder->setRenderPass(RenderPass::Translucent);
                    outlinePatternBuilder = std::move(builder);
                }
            }

            if (patternBuilder) {
                patternBuilder->clearTweakers();
                patternBuilder->addTweaker(getAtlasTweaker());
            }
            if (doOutline && outlinePatternBuilder) {
                outlinePatternBuilder->clearTweakers();
                outlinePatternBuilder->addTweaker(getAtlasTweaker());
            }

            const auto finish = [&](gfx::DrawableBuilder& builder,
                                    const StringIdentity interpolateNameId,
                                    const auto& interpolateUBO,
                                    const StringIdentity tileUBONameId,
                                    const auto& tileUBO,
                                    FillVariant type) {
                builder.flush(context);

                for (auto& drawable : builder.clearDrawables()) {
                    drawable->setTileID(tileID);
                    drawable->setLayerTweaker(layerTweaker);
                    drawable->setType(static_cast<size_t>(type));

                    auto& uniforms = drawable->mutableUniformBuffers();
                    uniforms.createOrUpdate(interpolateNameId, &interpolateUBO, context);
                    uniforms.createOrUpdate(tileUBONameId, &tileUBO, context);
                    fillTileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                    ++stats.drawablesAdded;
                }
            };

            if (patternBuilder && bucket.sharedTriangles->elements()) {
                patternBuilder->setShader(fillShader);
                patternBuilder->setRenderPass(renderPass);
                if (doOutline && outlinePatternBuilder) {
                    patternBuilder->setVertexAttributes(vertexAttrs);
                    outlinePatternBuilder->setVertexAttributes(std::move(vertexAttrs));
                } else {
                    patternBuilder->setVertexAttributes(std::move(vertexAttrs));
                }
                patternBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
                patternBuilder->setSegments(gfx::Triangles(),
                                            bucket.sharedTriangles,
                                            bucket.triangleSegments.data(),
                                            bucket.triangleSegments.size());

                finish(*patternBuilder,
                       idFillPatternInterpolateUBOName,
                       getFillPatternInterpolateUBO(),
                       idFillPatternTilePropsUBOName,
                       getFillPatternTilePropsUBO(),
                       FillVariant::FillPattern);
            }

            if (doOutline && outlinePatternBuilder && bucket.sharedBasicLineIndexes->elements()) {
                outlinePatternBuilder->setShader(outlineShader);
                outlinePatternBuilder->setRenderPass(renderPass);
                outlinePatternBuilder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short2);
                outlinePatternBuilder->setSegments(gfx::Lines(2),
                                                   bucket.sharedBasicLineIndexes,
                                                   bucket.basicLineSegments.data(),
                                                   bucket.basicLineSegments.size());

                finish(*outlinePatternBuilder,
                       idFillOutlinePatternInterpolateUBOName,
                       getFillOutlinePatternInterpolateUBO(),
                       idFillOutlinePatternTilePropsUBOName,
                       getFillOutlinePatternTilePropsUBO(),
                       FillVariant::FillOutlinePattern);
            }
        }
    }
}
#endif

} // namespace mbgl
