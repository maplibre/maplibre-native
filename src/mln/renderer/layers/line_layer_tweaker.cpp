#include <mln/renderer/layers/line_layer_tweaker.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/drawable.hpp>
#include <mln/gfx/line_drawable_data.hpp>
#include <mln/geometry/line_atlas.hpp>
#include <mln/renderer/buckets/line_bucket.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/shaders/line_layer_ubo.hpp>
#include <mln/shaders/shader_program_base.hpp>
#include <mln/style/layers/line_layer_properties.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/math.hpp>

#include <algorithm>
#include <array>
#include <vector>

#if MLN_RENDER_BACKEND_METAL
#include <mln/shaders/mtl/line.hpp>
#endif // MLN_RENDER_BACKEND_METAL

namespace mln {

#if MLN_UBO_CONSOLIDATION
namespace {

template <typename Binder>
const std::vector<std::array<float, 4>>* featureValueTable(const Binder& binder) {
    return binder ? &binder->featureValues() : nullptr;
}

std::size_t featureTableSize(const std::vector<std::array<float, 4>>* values) {
    return values ? values->size() : 0;
}

template <std::size_t N>
void assignFloats(std::array<float, N>& dst,
                  const std::vector<std::array<float, 4>>* values,
                  std::size_t featureIndex) {
    static_assert(N > 0 && N <= 4);
    if (values && featureIndex < values->size()) {
        std::copy_n((*values)[featureIndex].begin(), N, dst.begin());
    }
}

struct LineFeatureSources {
    const std::vector<std::array<float, 4>>* color = nullptr;
    const std::vector<std::array<float, 4>>* blur = nullptr;
    const std::vector<std::array<float, 4>>* opacity = nullptr;
    const std::vector<std::array<float, 4>>* gapwidth = nullptr;
    const std::vector<std::array<float, 4>>* offset = nullptr;
    const std::vector<std::array<float, 4>>* width = nullptr;
    const std::vector<std::array<float, 4>>* floorwidth = nullptr;
    const std::vector<std::array<float, 4>>* patternFrom = nullptr;
    const std::vector<std::array<float, 4>>* patternTo = nullptr;
    std::size_t count = 0;
};

LineFeatureSources lineFeatureSources(const LineBinders& binders) {
    LineFeatureSources sources;
    sources.color = featureValueTable(binders.get<style::LineColor>());
    sources.blur = featureValueTable(binders.get<style::LineBlur>());
    sources.opacity = featureValueTable(binders.get<style::LineOpacity>());
    sources.gapwidth = featureValueTable(binders.get<style::LineGapWidth>());
    sources.offset = featureValueTable(binders.get<style::LineOffset>());
    sources.width = featureValueTable(binders.get<style::LineWidth>());
    sources.floorwidth = featureValueTable(binders.get<style::LineFloorWidth>());
    const auto& patternBinder = binders.get<style::LinePattern>();
    sources.patternTo = featureValueTable(patternBinder);
    sources.patternFrom = patternBinder ? &patternBinder->featurePatterns() : nullptr;
    const std::array tables{sources.color,
                            sources.blur,
                            sources.opacity,
                            sources.gapwidth,
                            sources.offset,
                            sources.width,
                            sources.floorwidth,
                            sources.patternTo,
                            sources.patternFrom};
    sources.count = featureTableSize(std::ranges::max(tables, {}, featureTableSize));
    return sources;
}

// `update` cannot shrink. Recreate when the live set does not fit, or once it drops to
// half the allocation, so a dense view does not pin the high-water mark. Spare of 256
// bytes is alignment padding (WebGPU rounds storage allocations up to that).
bool needNewBuffer(std::size_t capacity, std::size_t bytes) {
    constexpr std::size_t alignmentSlack = 256;
    return (capacity < bytes) || (capacity > alignmentSlack && (capacity - alignmentSlack) / 2 > bytes);
}

void packFeatureRecords(LineBinders& binders, const LineFeatureSources& sources) {
    auto& records = binders.getFeatureRecords();
    records.resize(sources.count);
    for (std::size_t featureIndex = 0; featureIndex < sources.count; ++featureIndex) {
        auto& record = records[featureIndex];
        record = {};
        assignFloats(record.color, sources.color, featureIndex);
        assignFloats(record.blur, sources.blur, featureIndex);
        assignFloats(record.opacity, sources.opacity, featureIndex);
        assignFloats(record.gapwidth, sources.gapwidth, featureIndex);
        assignFloats(record.offset, sources.offset, featureIndex);
        assignFloats(record.width, sources.width, featureIndex);
        assignFloats(record.floorwidth, sources.floorwidth, featureIndex);
        assignFloats(record.pattern_from, sources.patternFrom, featureIndex);
        assignFloats(record.pattern_to, sources.patternTo, featureIndex);
    }
    binders.clearFeatureRecordsDirty();
}

} // namespace
#endif

using namespace style;
using namespace shaders;

#if MLN_RENDER_BACKEND_METAL && !defined(NDEBUG)
constexpr bool diff(float actual, float expected, float e = 1.0e-6) {
    return actual != expected && (expected == 0 || std::fabs((actual - expected) / expected) > e);
}
constexpr bool diff(Color actual, Color expected, float e = 1.0e-6) {
    return diff(actual.r, expected.r, e) || diff(actual.g, expected.g, e) || diff(actual.b, expected.b, e) ||
           diff(actual.a, expected.a, e);
}
#endif // MLN_RENDER_BACKEND_METAL

#if MLN_RENDER_BACKEND_METAL && !defined(NDEBUG)
template <typename Result>
std::optional<Result> LineLayerTweaker::gpuEvaluate(
    [[maybe_unused]] const LinePaintProperties::PossiblyEvaluated& evaluated,
    const PaintParameters& parameters,
    const std::size_t index) const {
    if (const auto& gpu = gpuExpressions[index]) {
        const float effectiveZoom = (gpu->options & gfx::GPUOptions::IntegerZoom)
                                        ? parameters.state.getIntegerZoom()
                                        : static_cast<float>(parameters.state.getZoom());
        return gpu->template evaluate<Result>(effectiveZoom);
    }
    return {};
}
#endif // MLN_RENDER_BACKEND_METAL

template <typename Property>
auto LineLayerTweaker::evaluate([[maybe_unused]] const PaintParameters& parameters) const {
    const auto& evaluated = static_cast<const LineLayerProperties&>(*evaluatedProperties).evaluated;

#if MLN_RENDER_BACKEND_METAL && !defined(NDEBUG)
    constexpr auto index = propertyIndex<Property>();
    if (auto gpuValue = gpuEvaluate<typename Property::Type>(evaluated, parameters, index)) {
        assert(!diff(*gpuValue, evaluated.get<Property>().constantOr(Property::defaultValue())));
        return *gpuValue;
    }
#endif // MLN_RENDER_BACKEND_METAL

    return evaluated.get<Property>().constantOr(Property::defaultValue());
}

void LineLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

    auto& context = parameters.context;
    const auto& evaluated = static_cast<const LineLayerProperties&>(*evaluatedProperties).evaluated;
    const auto& crossfade = static_cast<const LineLayerProperties&>(*evaluatedProperties).crossfade;

    const auto& linePatternValue = evaluated.get<LinePattern>().constantOr(
        Faded<expression::Image>{.from = "", .to = ""});

    const auto zoom = static_cast<float>(parameters.state.getZoom());
    const auto intZoom = parameters.state.getIntegerZoom();

#if MLN_RENDER_BACKEND_METAL
    const auto getExpressionBuffer = [&]() {
        const bool enableEval = gfx::Backend::getEnableGPUExpressionEval();
        if (!expressionUniformBuffer || (gpuExpressionsUpdated && enableEval)) {
            LineExpressionUBO exprUBO = {
                .color = enableEval ? gpuExpressions[propertyIndex<LineColor>()].get() : nullptr,
                .blur = enableEval ? gpuExpressions[propertyIndex<LineBlur>()].get() : nullptr,
                .opacity = enableEval ? gpuExpressions[propertyIndex<LineOpacity>()].get() : nullptr,
                .gapwidth = enableEval ? gpuExpressions[propertyIndex<LineGapWidth>()].get() : nullptr,
                .offset = enableEval ? gpuExpressions[propertyIndex<LineOffset>()].get() : nullptr,
                .width = enableEval ? gpuExpressions[propertyIndex<LineWidth>()].get() : nullptr,
                .floorWidth = enableEval ? gpuExpressions[propertyIndex<LineFloorWidth>()].get() : nullptr,
            };
            context.emplaceOrUpdateUniformBuffer(expressionUniformBuffer, &exprUBO);
            gpuExpressionsUpdated = false;
        }
        return expressionUniformBuffer;
    };
#endif // MLN_RENDER_BACKEND_METAL

    if (!evaluatedPropsUniformBuffer || propertiesUpdated) {
#if MLN_RENDER_BACKEND_METAL
        expressionMask =
            !gfx::Backend::getEnableGPUExpressionEval()
                ? LineExpressionMask::None
                : ((gpuExpressions[propertyIndex<LineColor>()] ? LineExpressionMask::Color : LineExpressionMask::None) |
                   (gpuExpressions[propertyIndex<LineBlur>()] ? LineExpressionMask::Blur : LineExpressionMask::None) |
                   (gpuExpressions[propertyIndex<LineOpacity>()] ? LineExpressionMask::Opacity
                                                                 : LineExpressionMask::None) |
                   (gpuExpressions[propertyIndex<LineGapWidth>()] ? LineExpressionMask::GapWidth
                                                                  : LineExpressionMask::None) |
                   (gpuExpressions[propertyIndex<LineOffset>()] ? LineExpressionMask::Offset
                                                                : LineExpressionMask::None) |
                   (gpuExpressions[propertyIndex<LineWidth>()] ? LineExpressionMask::Width : LineExpressionMask::None) |
                   (gpuExpressions[propertyIndex<LineFloorWidth>()] ? LineExpressionMask::FloorWidth
                                                                    : LineExpressionMask::None));
        const LineEvaluatedPropsUBO propsUBO{
            .color = (expressionMask & LineExpressionMask::Color) ? LineColor::defaultValue()
                                                                  : evaluate<LineColor>(parameters),

            .blur = (expressionMask & LineExpressionMask::Blur) ? LineBlur::defaultValue()
                                                                : evaluate<LineBlur>(parameters),

            .opacity = (expressionMask & LineExpressionMask::Opacity) ? LineOpacity::defaultValue()
                                                                      : evaluate<LineOpacity>(parameters),

            .gapwidth = (expressionMask & LineExpressionMask::GapWidth) ? LineGapWidth::defaultValue()
                                                                        : evaluate<LineGapWidth>(parameters),

            .offset = (expressionMask & LineExpressionMask::Offset) ? LineOffset::defaultValue()
                                                                    : evaluate<LineOffset>(parameters),

            .width = (expressionMask & LineExpressionMask::Width) ? LineWidth::defaultValue()
                                                                  : evaluate<LineWidth>(parameters),

            .floorwidth = (expressionMask & LineExpressionMask::FloorWidth) ? LineFloorWidth::defaultValue()
                                                                            : evaluate<LineFloorWidth>(parameters),
            .expressionMask = expressionMask,
            .pad1 = 0};
#elif MLN_RENDER_BACKEND_WEBGPU
        expressionMask = LineExpressionMask::None;
        if (evaluated.get<LineColor>().isConstant()) {
            expressionMask |= LineExpressionMask::Color;
        }
        if (evaluated.get<LineBlur>().isConstant()) {
            expressionMask |= LineExpressionMask::Blur;
        }
        if (evaluated.get<LineOpacity>().isConstant()) {
            expressionMask |= LineExpressionMask::Opacity;
        }
        if (evaluated.get<LineGapWidth>().isConstant()) {
            expressionMask |= LineExpressionMask::GapWidth;
        }
        if (evaluated.get<LineOffset>().isConstant()) {
            expressionMask |= LineExpressionMask::Offset;
        }
        if (evaluated.get<LineWidth>().isConstant()) {
            expressionMask |= LineExpressionMask::Width;
        }
        if (evaluated.get<LineFloorWidth>().isConstant()) {
            expressionMask |= LineExpressionMask::FloorWidth;
        }

        const LineEvaluatedPropsUBO propsUBO{
            .color = evaluated.get<LineColor>().constantOr(LineColor::defaultValue()),
            .blur = evaluated.get<LineBlur>().constantOr(LineBlur::defaultValue()),
            .opacity = evaluated.get<LineOpacity>().constantOr(LineOpacity::defaultValue()),
            .gapwidth = evaluated.get<LineGapWidth>().constantOr(LineGapWidth::defaultValue()),
            .offset = evaluated.get<LineOffset>().constantOr(LineOffset::defaultValue()),
            .width = evaluated.get<LineWidth>().constantOr(LineWidth::defaultValue()),
            .floorwidth = evaluated.get<LineFloorWidth>().constantOr(LineFloorWidth::defaultValue()),
            .expressionMask = expressionMask,
            .pad1 = 0};
#else
        const LineEvaluatedPropsUBO propsUBO{/*color =*/evaluate<LineColor>(parameters),
                                             /*blur =*/evaluate<LineBlur>(parameters),
                                             /*opacity =*/evaluate<LineOpacity>(parameters),
                                             /*gapwidth =*/evaluate<LineGapWidth>(parameters),
                                             /*offset =*/evaluate<LineOffset>(parameters),
                                             /*width =*/evaluate<LineWidth>(parameters),
                                             /*floorwidth =*/evaluate<LineFloorWidth>(parameters),
                                             LineExpressionMask::None,
                                             0};
#endif // MLN_RENDER_BACKEND_METAL

        context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &propsUBO);
        propertiesUpdated = false;
    }
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.set(idLineEvaluatedPropsUBO, evaluatedPropsUniformBuffer);

#if MLN_RENDER_BACKEND_METAL
    // GPU Expressions
    layerUniforms.set(idLineExpressionUBO, getExpressionBuffer());
#endif // MLN_RENDER_BACKEND_METAL

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<LineDrawableUnionUBO> drawableUBOVector(layerGroup.getDrawableCount());
    std::vector<LineTilePropsUnionUBO> tilePropsUBOVector(layerGroup.getDrawableCount());

    // Pattern parameters can flip `pattern_from` before records are packed. Drawable UBOs then read the cached base.
    // Pointers stay in this frame's list. The cache keeps bucket ids, which outlive the binders.
    struct LiveBinder {
        util::SimpleIdentity bucketId;
        LineBinders* binders = nullptr;
    };
    std::vector<LiveBinder> liveBinders;
    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !drawable.getShader() || !checkTweakDrawable(drawable)) {
            return;
        }
        auto* binders = static_cast<LineBinders*>(drawable.getBinders());
        const auto* tile = drawable.getRenderTile();
        const auto& bucket = drawable.getBucket();
        if (!binders || !tile || !bucket) {
            return;
        }
        const auto patternPosA = tile->getPattern(linePatternValue.from.id());
        const auto patternPosB = tile->getPattern(linePatternValue.to.id());
        binders->setPatternParameters(patternPosA, patternPosB, crossfade);

        const auto sources = lineFeatureSources(*binders);
        if (sources.count == 0) {
            binders->setFeatureBase(0);
            return;
        }
        const auto bucketId = bucket->getID();
        const auto alreadySeen = std::find_if(
            liveBinders.begin(), liveBinders.end(), [&](const LiveBinder& live) { return live.bucketId == bucketId; });
        if (alreadySeen == liveBinders.end()) {
            liveBinders.push_back({bucketId, binders});
        }
    });

    const bool orderChanged = liveBinders.size() != featureBinderOrder.size() ||
                              !std::equal(
                                  liveBinders.begin(),
                                  liveBinders.end(),
                                  featureBinderOrder.begin(),
                                  [](const LiveBinder& live, util::SimpleIdentity id) { return live.bucketId == id; });
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
        std::vector<LineFeatureUBO> records;
        std::vector<util::SimpleIdentity> binderOrder;
        binderOrder.reserve(liveBinders.size());
        uint32_t base = 0;
        for (const LiveBinder& live : liveBinders) {
            auto* binders = live.binders;
            if (binders->geFeatureRecordsDirty() || binders->getFeatureRecords().empty()) {
                packFeatureRecords(*binders, lineFeatureSources(*binders));
            }
            binders->setFeatureBase(base);
            const auto& packed = binders->getFeatureRecords();
            records.insert(records.end(), packed.begin(), packed.end());
            base += static_cast<uint32_t>(packed.size());
            binderOrder.push_back(live.bucketId);
        }
        featureBinderOrder = std::move(binderOrder);

        LineFeatureUBO dummyFeature{};
        const void* featureData = records.empty() ? static_cast<const void*>(&dummyFeature)
                                                  : static_cast<const void*>(records.data());
        const size_t featureBytes = records.empty() ? sizeof(dummyFeature) : sizeof(LineFeatureUBO) * records.size();
        if (!featureUniformBuffer || needNewBuffer(featureUniformBuffer->getSize(), featureBytes)) {
            featureUniformBuffer = context.createUniformBuffer(featureData, featureBytes, false, true);
        } else {
            featureUniformBuffer->update(featureData, featureBytes);
        }
    }
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        const auto shader = drawable.getShader();
        if (!drawable.getTileID() || !shader || !checkTweakDrawable(drawable)) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        auto* binders = static_cast<LineBinders*>(drawable.getBinders());
        const auto* tile = drawable.getRenderTile();
        if (!binders || !tile) {
            assert(false);
            return;
        }

        const auto patternPosA = tile->getPattern(linePatternValue.from.id());
        const auto patternPosB = tile->getPattern(linePatternValue.to.id());
#if !MLN_UBO_CONSOLIDATION
        binders->setPatternParameters(patternPosA, patternPosB, crossfade);
#endif

        const auto& translation = evaluated.get<LineTranslate>();
        const auto anchor = evaluated.get<LineTranslateAnchor>();
        constexpr bool nearClipped = false;
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        const auto matrix = getTileMatrix(
            tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits, drawable);

#if !MLN_UBO_CONSOLIDATION
        auto& drawableUniforms = drawable.mutableUniformBuffers();
#endif
        switch (static_cast<LineType>(drawable.getType())) {
            case LineType::Simple: {
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].lineDrawableUBO = {
#else
                const LineDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),
                    .ratio = 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),

                    .color_t = std::get<0>(binders->get<LineColor>()->interpolationFactor(zoom)),
                    .blur_t = std::get<0>(binders->get<LineBlur>()->interpolationFactor(zoom)),
                    .opacity_t = std::get<0>(binders->get<LineOpacity>()->interpolationFactor(zoom)),
                    .gapwidth_t = std::get<0>(binders->get<LineGapWidth>()->interpolationFactor(zoom)),
                    .offset_t = std::get<0>(binders->get<LineOffset>()->interpolationFactor(zoom)),
                    .width_t = std::get<0>(binders->get<LineWidth>()->interpolationFactor(zoom)),
#if MLN_UBO_CONSOLIDATION
                    .feature_base = binders->getFeatureBase()
#else
                    .pad1 = 0
#endif
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idLineDrawableUBO, &drawableUBO, context);
#endif

            } break;

            case LineType::Gradient: {
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].lineGradientDrawableUBO = {
#else
                const LineGradientDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),
                    .ratio = 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),

                    .blur_t = std::get<0>(binders->get<LineBlur>()->interpolationFactor(zoom)),
                    .opacity_t = std::get<0>(binders->get<LineOpacity>()->interpolationFactor(zoom)),
                    .gapwidth_t = std::get<0>(binders->get<LineGapWidth>()->interpolationFactor(zoom)),
                    .offset_t = std::get<0>(binders->get<LineOffset>()->interpolationFactor(zoom)),
                    .width_t = std::get<0>(binders->get<LineWidth>()->interpolationFactor(zoom)),
#if MLN_UBO_CONSOLIDATION
                    .feature_base = binders->getFeatureBase(),
#else
                    .pad1 = 0,
#endif
                    .pad2 = 0
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idLineDrawableUBO, &drawableUBO, context);
#endif
            } break;

            case LineType::Pattern: {
                Size textureSize{0, 0};
                if (const auto& texture = drawable.getTexture(idLineImageTexture)) {
                    textureSize = texture->getSize();
                }
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].linePatternDrawableUBO = {
#else
                const LinePatternDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),
                    .ratio = 1.0f / tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),

                    .blur_t = std::get<0>(binders->get<LineBlur>()->interpolationFactor(zoom)),
                    .opacity_t = std::get<0>(binders->get<LineOpacity>()->interpolationFactor(zoom)),
                    .gapwidth_t = std::get<0>(binders->get<LineGapWidth>()->interpolationFactor(zoom)),
                    .offset_t = std::get<0>(binders->get<LineOffset>()->interpolationFactor(zoom)),
                    .width_t = std::get<0>(binders->get<LineWidth>()->interpolationFactor(zoom)),
                    .pattern_from_t = std::get<0>(binders->get<LinePattern>()->interpolationFactor(zoom)),
                    .pattern_to_t = std::get<1>(binders->get<LinePattern>()->interpolationFactor(zoom)),
                    .floorwidth_t = std::get<0>(binders->get<LineFloorWidth>()->interpolationFactor(zoom)),
#if MLN_UBO_CONSOLIDATION
                    .feature_base = binders->getFeatureBase(),
#else
                    .pad1 = 0,
#endif
                    .pad2 = 0,
                    .pad3 = 0
                };

#if MLN_UBO_CONSOLIDATION
                tilePropsUBOVector[i].linePatternTilePropsUBO = LinePatternTilePropsUBO {
#else
                const LinePatternTilePropsUBO tilePropsUBO = {
#endif
                    .pattern_from = patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},

                    .pattern_to = patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},

                    .scale = {parameters.pixelRatio,
                              1 / tileID.pixelsToTileUnits(1, intZoom),
                              crossfade.fromScale,
                              crossfade.toScale},

                    .texsize = {static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                    .fade = crossfade.t, .pad1 = 0
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idLineDrawableUBO, &drawableUBO, context);
                drawableUniforms.createOrUpdate(idLineTilePropsUBO, &tilePropsUBO, context);
#endif
            } break;

            case LineType::SDF:
            case LineType::GradientSDF: {
                if (const auto& data = drawable.getData()) {
                    const gfx::LineDrawableData& lineData = static_cast<const gfx::LineDrawableData&>(*data);
                    const auto& dashPatternTexture = parameters.lineAtlas.getDashPatternTexture(
                        evaluated.get<LineDasharray>().from,
                        evaluated.get<LineDasharray>().to,
                        lineData.linePatternCap);

                    // The gradient variant keeps its ramp in the image slot, so the dash atlas takes the second.
                    const auto dashTextureId = static_cast<LineType>(drawable.getType()) == LineType::GradientSDF
                                                   ? idLineDashTexture
                                                   : idLineImageTexture;
                    if (!drawable.getTexture(dashTextureId)) {
                        const auto& texture = dashPatternTexture.getTexture();
                        drawable.setEnabled(!!texture);
                        if (texture) {
                            drawable.setTexture(texture, dashTextureId);
                        }
                    }

                    const LinePatternPos& posA = dashPatternTexture.getFrom();
                    const LinePatternPos& posB = dashPatternTexture.getTo();
                    const float widthA = posA.width * crossfade.fromScale;
                    const float widthB = posB.width * crossfade.toScale;

#if MLN_UBO_CONSOLIDATION
                    drawableUBOVector[i].lineSDFDrawableUBO = {
#else
                    const LineSDFDrawableUBO drawableUBO = {
#endif
                        .matrix = util::cast<float>(matrix),
                        .patternscale_a = {1.0f / tileID.pixelsToTileUnits(widthA, intZoom), -posA.height / 2.0f},
                        .patternscale_b = {1.0f / tileID.pixelsToTileUnits(widthB, intZoom), -posB.height / 2.0f},
                        .tex_y_a = posA.y,
                        .tex_y_b = posB.y,
                        .ratio = 1.0f / tileID.pixelsToTileUnits(1.0f, zoom),

                        .color_t = std::get<0>(binders->get<LineColor>()->interpolationFactor(zoom)),
                        .blur_t = std::get<0>(binders->get<LineBlur>()->interpolationFactor(zoom)),
                        .opacity_t = std::get<0>(binders->get<LineOpacity>()->interpolationFactor(zoom)),
                        .gapwidth_t = std::get<0>(binders->get<LineGapWidth>()->interpolationFactor(zoom)),
                        .offset_t = std::get<0>(binders->get<LineOffset>()->interpolationFactor(zoom)),
                        .width_t = std::get<0>(binders->get<LineWidth>()->interpolationFactor(zoom)),
                        .floorwidth_t = std::get<0>(binders->get<LineFloorWidth>()->interpolationFactor(zoom)),
#if MLN_UBO_CONSOLIDATION
                        .feature_base = binders->getFeatureBase(),
#else
                        .pad1 = 0,
#endif
                        .pad2 = 0
                    };

#if MLN_UBO_CONSOLIDATION
                    tilePropsUBOVector[i].lineSDFTilePropsUBO = LineSDFTilePropsUBO {
#else
                    const LineSDFTilePropsUBO tilePropsUBO = {
#endif
                        .sdfgamma = static_cast<float>(dashPatternTexture.getSize().width) /
                                    (std::min(widthA, widthB) * 256.0f * parameters.pixelRatio) / 2.0f,
                        .mix = crossfade.t, .pad1 = 0, .pad2 = 0
                    };

#if !MLN_UBO_CONSOLIDATION
                    drawableUniforms.createOrUpdate(idLineDrawableUBO, &drawableUBO, context);
                    drawableUniforms.createOrUpdate(idLineTilePropsUBO, &tilePropsUBO, context);
#endif
                }
            } break;

            default: {
                Log::Error(Event::General,
                           "LineLayerTweaker: unknown line type: " + std::to_string(drawable.getType()));
            } break;
        }

#if MLN_UBO_CONSOLIDATION
        drawable.setUBOIndex(i++);
#endif
    });

#if MLN_UBO_CONSOLIDATION
    const size_t drawableUBOVectorSize = sizeof(LineDrawableUnionUBO) * drawableUBOVector.size();
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < drawableUBOVectorSize) {
        drawableUniformBuffer = context.createUniformBuffer(
            drawableUBOVector.data(), drawableUBOVectorSize, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    const size_t tilePropsUBOVectorSize = sizeof(LineTilePropsUnionUBO) * tilePropsUBOVector.size();
    if (!tilePropsUniformBuffer || tilePropsUniformBuffer->getSize() < tilePropsUBOVectorSize) {
        tilePropsUniformBuffer = context.createUniformBuffer(
            tilePropsUBOVector.data(), tilePropsUBOVectorSize, false, true);
    } else {
        tilePropsUniformBuffer->update(tilePropsUBOVector.data(), tilePropsUBOVectorSize);
    }

    layerUniforms.set(idLineDrawableUBO, drawableUniformBuffer);
    layerUniforms.set(idLineTilePropsUBO, tilePropsUniformBuffer);
    layerUniforms.set(idLineFeatureUBO, featureUniformBuffer);
#endif
}

#if MLN_RENDER_BACKEND_METAL
void LineLayerTweaker::updateGPUExpressions(const Unevaluated& unevaluated, TimePoint now) {
    if (gfx::Backend::getEnableGPUExpressionEval()) {
        if (unevaluated.updateGPUExpressions(gpuExpressions, now)) {
            gpuExpressionsUpdated = true;

            // Masks also need to be updated
            propertiesUpdated = true;
        }
    }
}

#endif // MLN_RENDER_BACKEND_METAL

} // namespace mln
