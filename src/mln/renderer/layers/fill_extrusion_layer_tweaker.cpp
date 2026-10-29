#include <mln/renderer/layers/fill_extrusion_layer_tweaker.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/drawable.hpp>
#include <mln/gfx/renderable.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/render_tree.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/paint_property_binder.hpp>
#include <mln/shaders/feature_index.hpp>
#include <mln/shaders/fill_extrusion_layer_ubo.hpp>
#include <mln/shaders/shader_program_base.hpp>
#include <mln/style/layers/fill_extrusion_layer_properties.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

#if MLN_RENDER_BACKEND_METAL
#include <mln/shaders/mtl/fill_extrusion.hpp>
#endif

namespace mln {

#if MLN_UBO_CONSOLIDATION
namespace {

bool needNewBuffer(std::size_t capacity, std::size_t bytes) {
    constexpr std::size_t alignmentSlack = 256;
    return (capacity < bytes) || (capacity > alignmentSlack && (capacity - alignmentSlack) / 2 > bytes);
}

} // namespace
#endif

using namespace shaders;
using namespace style;

void FillExtrusionLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

    auto& context = parameters.context;
    const auto& props = static_cast<const FillExtrusionLayerProperties&>(*evaluatedProperties);
    const auto& evaluated = props.evaluated;
    const auto& crossfade = props.crossfade;
    const auto& state = parameters.state;

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    // UBO depends on more than just evaluated properties, so we need to update every time,
    // but the resulting buffer can be shared across all the drawables from the layer.
    const FillExtrusionPropsUBO propsUBO = {
        .color = constOrDefault<FillExtrusionColor>(evaluated),
        .light_color = FillExtrusionBucket::lightColor(parameters.evaluatedLight),
        .pad1 = 0,
        .light_position = FillExtrusionBucket::lightPosition(parameters.evaluatedLight, state),
        .base = constOrDefault<FillExtrusionBase>(evaluated),
        .height = constOrDefault<FillExtrusionHeight>(evaluated),
        .light_intensity = FillExtrusionBucket::lightIntensity(parameters.evaluatedLight),
        .vertical_gradient = evaluated.get<FillExtrusionVerticalGradient>() ? 1.0f : 0.0f,
        .opacity = evaluated.get<FillExtrusionOpacity>(),
        .fade = crossfade.t,
        .from_scale = crossfade.fromScale,
        .to_scale = crossfade.toScale,
        .pad2 = 0};
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.createOrUpdate(idFillExtrusionPropsUBO, &propsUBO, context);

    propertiesUpdated = false;

    const auto zoom = static_cast<float>(parameters.state.getZoom());
    const auto defPattern = mln::Faded<expression::Image>{.from = "", .to = ""};
    const auto fillPatternValue = evaluated.get<FillExtrusionPattern>().constantOr(defPattern);

#if MLN_UBO_CONSOLIDATION
    struct LiveBinder {
        util::SimpleIdentity bucketId;
        FillExtrusionBinders* binders = nullptr;
    };
    std::vector<LiveBinder> liveBinders;
    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }
        auto* binders = static_cast<FillExtrusionBinders*>(drawable.getBinders());
        const auto* tile = drawable.getRenderTile();
        const auto& bucket = drawable.getBucket();
        if (!binders || !tile || !bucket) {
            return;
        }
        if (drawable.getType()) {
            binders->setPatternParameters(
                tile->getPattern(fillPatternValue.from.id()), tile->getPattern(fillPatternValue.to.id()), crossfade);
        }
        if (binders->featureCount(featureAttributes<BuiltIn::FillExtrusionShader>()) == 0) {
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
                live.binders->packFeatureRecords(shaders::featureAttributes<shaders::BuiltIn::FillExtrusionShader>());
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
    layerUniforms.set(idFillExtrusionFeatureUBO, featureUniformBuffer);
#endif

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<FillExtrusionDrawableUBO> drawableUBOVector(layerGroup.getDrawableCount());
    std::vector<FillExtrusionTilePropsUBO> tilePropsUBOVector(layerGroup.getDrawableCount());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        auto* binders = static_cast<FillExtrusionBinders*>(drawable.getBinders());
        const auto* tile = drawable.getRenderTile();
        if (!binders || !tile) {
            assert(false);
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto& translation = evaluated.get<FillExtrusionTranslate>();
        const auto anchor = evaluated.get<FillExtrusionTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = true;
        const auto matrix = getTileMatrix(
            tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits, drawable);

        const auto tileRatio = 1 / tileID.pixelsToTileUnits(1, state.getIntegerZoom());
        const auto zoomScale = state.zoomScale(tileID.canonical.z);
        const auto nearestZoomScale = state.zoomScale(state.getIntegerZoom() - tileID.canonical.z);
        const auto tileSizeAtNearestZoom = std::floor(util::tileSize_D * nearestZoomScale);
        const auto pixelX = static_cast<int32_t>(tileSizeAtNearestZoom *
                                                 (tileID.canonical.x + tileID.wrap * zoomScale));
        const auto pixelY = static_cast<int32_t>(tileSizeAtNearestZoom * tileID.canonical.y);
        const auto numTiles = std::pow(2, tileID.canonical.z);
        const auto heightFactor = static_cast<float>(-numTiles / util::tileSize_D / 8.0);

        Size textureSize = {0, 0};
        if (const auto& tex = drawable.getTexture(idFillExtrusionImageTexture)) {
            textureSize = tex->getSize();
        }

        const auto hasPattern = !!drawable.getType();
        const auto patternPosA = tile->getPattern(fillPatternValue.from.id());
        const auto patternPosB = tile->getPattern(fillPatternValue.to.id());
        if (hasPattern) {
            binders->setPatternParameters(patternPosA, patternPosB, crossfade);
        }

#if MLN_UBO_CONSOLIDATION
        drawableUBOVector[i] = {
#else
        const FillExtrusionDrawableUBO drawableUBO = {
#endif
            .matrix = util::cast<float>(matrix),
            .pixel_coord_upper = {static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
            .pixel_coord_lower = {static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
            .height_factor = heightFactor,
            .tile_ratio = tileRatio,

            .base_t = std::get<0>(binders->get<FillExtrusionBase>()->interpolationFactor(zoom)),
            .height_t = std::get<0>(binders->get<FillExtrusionHeight>()->interpolationFactor(zoom)),
            .color_t = std::get<0>(binders->get<FillExtrusionColor>()->interpolationFactor(zoom)),
            .pattern_from_t = std::get<0>(binders->get<FillExtrusionPattern>()->interpolationFactor(zoom)),
            .pattern_to_t = std::get<0>(binders->get<FillExtrusionPattern>()->interpolationFactor(zoom)),
#if MLN_UBO_CONSOLIDATION
            .feature_base = binders->getFeatureBase()
#else
            .pad1 = 0
#endif
        };

#if MLN_UBO_CONSOLIDATION
        tilePropsUBOVector[i] = {
#else
        const FillExtrusionTilePropsUBO tilePropsUBO = {
#endif
            .pattern_from = patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},
            .pattern_to = patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},
            .texsize = {static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
            .pad1 = 0,
            .pad2 = 0
        };

#if MLN_UBO_CONSOLIDATION
        drawable.setUBOIndex(i++);
#else
        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idFillExtrusionDrawableUBO, &drawableUBO, context);
        drawableUniforms.createOrUpdate(idFillExtrusionTilePropsUBO, &tilePropsUBO, context);
#endif
    });

#if MLN_UBO_CONSOLIDATION
    const size_t drawableUBOVectorSize = sizeof(FillExtrusionDrawableUBO) * drawableUBOVector.size();
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < drawableUBOVectorSize) {
        drawableUniformBuffer = context.createUniformBuffer(
            drawableUBOVector.data(), drawableUBOVectorSize, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    const size_t tilePropsUBOVectorSize = sizeof(FillExtrusionTilePropsUBO) * tilePropsUBOVector.size();
    if (!tilePropsUniformBuffer || tilePropsUniformBuffer->getSize() < tilePropsUBOVectorSize) {
        tilePropsUniformBuffer = context.createUniformBuffer(
            tilePropsUBOVector.data(), tilePropsUBOVectorSize, false, true);
    } else {
        tilePropsUniformBuffer->update(tilePropsUBOVector.data(), tilePropsUBOVectorSize);
    }

    layerUniforms.set(idFillExtrusionDrawableUBO, drawableUniformBuffer);
    layerUniforms.set(idFillExtrusionTilePropsUBO, tilePropsUniformBuffer);
#endif
}

} // namespace mln
