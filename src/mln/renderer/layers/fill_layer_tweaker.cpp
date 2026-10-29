#include <mln/renderer/layers/fill_layer_tweaker.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/drawable.hpp>
#include <mln/gfx/renderable.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/layers/render_fill_layer.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/render_tree.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/paint_property_binder.hpp>
#include <mln/shaders/feature_index.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/shader_program_base.hpp>
#include <mln/shaders/fill_layer_ubo.hpp>
#include <mln/style/layers/fill_layer_properties.hpp>
#include <mln/util/convert.hpp>
#include <mln/util/std.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

namespace mln {

#if MLN_UBO_CONSOLIDATION
namespace {

// Allow large buffers to shrink
bool needNewBuffer(std::size_t capacity, std::size_t bytes) {
    constexpr std::size_t slack = 256;
    return (capacity < bytes) || (capacity > slack && (capacity - slack) / 2 > bytes);
}

} // namespace
#endif

using namespace style;
using namespace shaders;

void FillLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

    auto& context = parameters.context;
    const auto& props = static_cast<const FillLayerProperties&>(*evaluatedProperties);
    const auto& evaluated = props.evaluated;
    const auto& crossfade = props.crossfade;

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    if (!evaluatedPropsUniformBuffer || propertiesUpdated) {
        const FillEvaluatedPropsUBO propsUBO = {
            .color = evaluated.get<FillColor>().constantOr(FillColor::defaultValue()),
            .outline_color = evaluated.get<FillOutlineColor>().constantOr(FillOutlineColor::defaultValue()),
            .opacity = evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
            .fade = crossfade.t,
            .from_scale = crossfade.fromScale,
            .to_scale = crossfade.toScale,
        };
        context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &propsUBO);
        propertiesUpdated = false;
    }
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.set(idFillEvaluatedPropsUBO, evaluatedPropsUniformBuffer);

#if MLN_UBO_CONSOLIDATION
    struct LiveBinder {
        util::SimpleIdentity bucketId;
        FillBinders* binders = nullptr;
    };
    std::vector<LiveBinder> liveBinders;
    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }
        auto* binders = static_cast<FillBinders*>(drawable.getBinders());
        const auto* tile = drawable.getRenderTile();
        const auto& bucket = drawable.getBucket();
        if (!binders || !tile || !bucket) {
            return;
        }
        const auto& fillPatternValue = evaluated.get<FillPattern>().constantOr(
            Faded<expression::Image>{.from = "", .to = ""});
        binders->setPatternParameters(
            tile->getPattern(fillPatternValue.from.id()), tile->getPattern(fillPatternValue.to.id()), crossfade);
        if (binders->featureCount(featureAttributes<BuiltIn::FillShader>()) == 0) {
            binders->setFeatureBase(0);
            return;
        }
        const auto bucketId = bucket->getID();
        const auto alreadySeen = std::find_if(
            liveBinders.begin(), liveBinders.end(), [&](const LiveBinder& live) { return live.bucketId == bucketId; });
        if (alreadySeen == liveBinders.end()) {
            liveBinders.push_back({.bucketId = bucketId, .binders = binders});
        }
    });

    const bool orderChanged = liveBinders.size() != featureBinderOrder.size() ||
                              !std::ranges::equal(liveBinders, featureBinderOrder, [](const auto& live, auto id) {
                                  return live.bucketId == id;
                              });

    bool anyDirty = false;
    if (!orderChanged) {
        for (const LiveBinder& live : liveBinders) {
            if (live.binders->geFeatureRecordsDirty()) {
                anyDirty = true;
                break;
            }
        }
    }
    if (orderChanged || anyDirty || !featureUniformBuffer) {
        std::vector<float> records;
        std::vector<util::SimpleIdentity> binderOrder;
        binderOrder.reserve(liveBinders.size());
        uint32_t stride = 0;
        uint32_t base = 0;
        for (const LiveBinder& live : liveBinders) {
            if (live.binders->geFeatureRecordsDirty() || live.binders->getFeatureRecords().empty()) {
                live.binders->packFeatureRecords(shaders::featureAttributes<shaders::BuiltIn::FillShader>());
            }

            const auto packedStride = live.binders->getFeatureStride();
            assert(packedStride > 0);
            if (stride == 0) {
                stride = packedStride;
            } else {
                assert(stride == packedStride);
            }
            live.binders->setFeatureBase(base);
            const auto& packed = live.binders->getFeatureRecords();
            records.insert(records.end(), packed.begin(), packed.end());
            base += static_cast<uint32_t>(packed.size() / packedStride);
            binderOrder.push_back(live.bucketId);
        }
        featureBinderOrder = std::move(binderOrder);

        const std::array<float, 4> dummy{};
        const void* featureData = records.empty() ? static_cast<const void*>(dummy.data())
                                                  : static_cast<const void*>(records.data());
        const size_t featureBytes = records.empty() ? sizeof(dummy) : records.size() * sizeof(float);
        if (!featureUniformBuffer || needNewBuffer(featureUniformBuffer->getSize(), featureBytes)) {
            featureUniformBuffer = context.createUniformBuffer(featureData, featureBytes, false, true);
        } else {
            featureUniformBuffer->update(featureData, featureBytes);
        }
    }
    layerUniforms.set(idFillFeatureUBO, featureUniformBuffer);
#endif

    const auto& translation = evaluated.get<FillTranslate>();
    const auto anchor = evaluated.get<FillTranslateAnchor>();
    const auto zoom = static_cast<float>(parameters.state.getZoom());
    const auto intZoom = parameters.state.getIntegerZoom();

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<FillDrawableUnionUBO> drawableUBOVector(layerGroup.getDrawableCount());
    std::vector<FillTilePropsUnionUBO> tilePropsUBOVector(layerGroup.getDrawableCount());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        auto* binders = static_cast<FillBinders*>(drawable.getBinders());
        const auto* tile = drawable.getRenderTile();
        if (!binders || !tile) {
            assert(false);
            return;
        }

        const auto& bucket = drawable.getBucket();
        const float sdfPattern = bucket && bucket->isSDFPattern(id) ? 1.0f : 0.0f;

        const auto& fillPatternValue = evaluated.get<FillPattern>().constantOr(
            Faded<expression::Image>{.from = "", .to = ""});
        const auto patternPosA = tile->getPattern(fillPatternValue.from.id());
        const auto patternPosB = tile->getPattern(fillPatternValue.to.id());
        binders->setPatternParameters(patternPosA, patternPosB, crossfade);

        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = false;
        const auto matrix = getTileMatrix(
            tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits, drawable);

        // from FillPatternProgram::layoutUniformValues
        const auto tileRatio = 1.0f / tileID.pixelsToTileUnits(1.0f, intZoom);
        const int32_t tileSizeAtNearestZoom = static_cast<int32_t>(
            util::tileSize_D * parameters.state.zoomScale(intZoom - tileID.canonical.z));
        const int32_t pixelX = static_cast<int32_t>(
            tileSizeAtNearestZoom *
            (tileID.canonical.x + tileID.wrap * parameters.state.zoomScale(tileID.canonical.z)));
        const int32_t pixelY = tileSizeAtNearestZoom * tileID.canonical.y;

        Size textureSize = {0, 0};
        if (const auto& tex = drawable.getTexture(idFillImageTexture)) {
            textureSize = tex->getSize();
        }

#if !MLN_UBO_CONSOLIDATION
        auto& drawableUniforms = drawable.mutableUniformBuffers();
#endif
        switch (static_cast<RenderFillLayer::FillVariant>(drawable.getType())) {
            case RenderFillLayer::FillVariant::Fill: {
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].fillDrawableUBO = {
#else
                const FillDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),

                    .color_t = std::get<0>(binders->get<FillColor>()->interpolationFactor(zoom)),
                    .opacity_t = std::get<0>(binders->get<FillOpacity>()->interpolationFactor(zoom)),
#if MLN_UBO_CONSOLIDATION
                    .feature_base = binders->getFeatureBase(),
#else
                    .pad1 = 0,
#endif
                    .pad2 = 0
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
#endif
                break;
            }
            case RenderFillLayer::FillVariant::FillOutline: {
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].fillOutlineDrawableUBO = {
#else
                const FillOutlineDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),

                    .outline_color_t = std::get<0>(binders->get<FillOutlineColor>()->interpolationFactor(zoom)),
                    .opacity_t = std::get<0>(binders->get<FillOpacity>()->interpolationFactor(zoom)),
#if MLN_UBO_CONSOLIDATION
                    .feature_base = binders->getFeatureBase(),
#else
                    .pad1 = 0,
#endif
                    .pad2 = 0
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
#endif
                break;
            }
            case RenderFillLayer::FillVariant::FillPattern: {
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].fillPatternDrawableUBO = {
#else
                const FillPatternDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),
                    .pixel_coord_upper = {static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},

                    .pixel_coord_lower = {static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                    .tile_ratio = tileRatio,

                    .pattern_from_t = std::get<0>(binders->get<FillPattern>()->interpolationFactor(zoom)),
                    .pattern_to_t = std::get<0>(binders->get<FillPattern>()->interpolationFactor(zoom)),
                    .opacity_t = std::get<0>(binders->get<FillOpacity>()->interpolationFactor(zoom)),
                    .color_t = std::get<0>(binders->get<FillColor>()->interpolationFactor(zoom)),
#if MLN_UBO_CONSOLIDATION
                    .feature_base = binders->getFeatureBase(),
#else
                    .pad1 = 0,
#endif
                    .pad2 = 0,
                    .pad3 = 0
                };

#if MLN_UBO_CONSOLIDATION
                tilePropsUBOVector[i].fillPatternTilePropsUBO = FillPatternTilePropsUBO {
#else
                const FillPatternTilePropsUBO tilePropsUBO = {
#endif
                    .pattern_from = patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},

                    .pattern_to = patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},

                    .texsize = {static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                    .sdf = sdfPattern, .pad2 = 0
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
                drawableUniforms.createOrUpdate(idFillTilePropsUBO, &tilePropsUBO, context);
#endif
                break;
            }
            case RenderFillLayer::FillVariant::FillOutlinePattern: {
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].fillOutlinePatternDrawableUBO = {
#else
                const FillOutlinePatternDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),
                    .pixel_coord_upper = {static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},

                    .pixel_coord_lower = {static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                    .tile_ratio = tileRatio,

                    .pattern_from_t = std::get<0>(binders->get<FillPattern>()->interpolationFactor(zoom)),
                    .pattern_to_t = std::get<0>(binders->get<FillPattern>()->interpolationFactor(zoom)),
                    .opacity_t = std::get<0>(binders->get<FillOpacity>()->interpolationFactor(zoom)),
                    .color_t = std::get<0>(binders->get<FillColor>()->interpolationFactor(zoom)),
#if MLN_UBO_CONSOLIDATION
                    .feature_base = binders->getFeatureBase(),
#else
                    .pad1 = 0,
#endif
                    .pad2 = 0,
                    .pad3 = 0
                };

#if MLN_UBO_CONSOLIDATION
                tilePropsUBOVector[i].fillOutlinePatternTilePropsUBO = FillOutlinePatternTilePropsUBO {
#else
                const FillOutlinePatternTilePropsUBO tilePropsUBO = {
#endif
                    .pattern_from = patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},

                    .pattern_to = patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},

                    .texsize = {static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                    .sdf = sdfPattern, .pad2 = 0
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
                drawableUniforms.createOrUpdate(idFillTilePropsUBO, &tilePropsUBO, context);
#endif
                break;
            }
            case RenderFillLayer::FillVariant::FillOutlineTriangulated: {
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].fillOutlineTriangulatedDrawableUBO = {
#else
                const FillOutlineTriangulatedDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),
                    .ratio = 1.0f / tileID.pixelsToTileUnits(1.0f, parameters.state.getZoom()),
                    .pad1 = 0,
                    .pad2 = 0,
                    .pad3 = 0
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
#endif
                break;
            }
            default: {
#ifndef NDEBUG
                mln::Log::Error(mln::Event::Render, "Invalid fill variant type supplied during drawable update!");
#endif
                break;
            }
        }
#if MLN_UBO_CONSOLIDATION
        drawable.setUBOIndex(i++);
#endif
    });

#if MLN_UBO_CONSOLIDATION
    const size_t drawableUBOVectorSize = sizeof(FillDrawableUnionUBO) * drawableUBOVector.size();
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < drawableUBOVectorSize) {
        drawableUniformBuffer = context.createUniformBuffer(
            drawableUBOVector.data(), drawableUBOVectorSize, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    const size_t tilePropsUBOVectorSize = sizeof(FillTilePropsUnionUBO) * tilePropsUBOVector.size();
    if (!tilePropsUniformBuffer || tilePropsUniformBuffer->getSize() < tilePropsUBOVectorSize) {
        tilePropsUniformBuffer = context.createUniformBuffer(
            tilePropsUBOVector.data(), tilePropsUBOVectorSize, false, true);
    } else {
        tilePropsUniformBuffer->update(tilePropsUBOVector.data(), tilePropsUBOVectorSize);
    }

    layerUniforms.set(idFillDrawableUBO, drawableUniformBuffer);
    layerUniforms.set(idFillTilePropsUBO, tilePropsUniformBuffer);
#endif
}

} // namespace mln
