#include <mbgl/renderer/layers/symbol_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/symbol_drawable_data.hpp>
#include <mbgl/layout/symbol_projection.hpp>
#include <mbgl/programs/symbol_program.hpp>
#include <mbgl/renderer/buckets/symbol_bucket.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/std.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/symbol_icon.hpp>
#include <mbgl/shaders/mtl/symbol_sdf.hpp>
#endif // MLN_RENDER_BACKEND_METAL

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {

Size getTexSize(const gfx::Drawable& drawable, const size_t texId) {
    if (const auto& tex = drawable.getTexture(texId)) {
        return tex->getSize();
    }
    return {0, 0};
}

std::array<float, 2> toArray(const Size& s) {
    return util::cast<float>(std::array<uint32_t, 2>{s.width, s.height});
}

template <typename TText, typename TIcon>
const auto& getProperty(const SymbolBucket::PaintProperties& paintProps, bool isText) {
    return isText ? paintProps.textBinders.get<TText>() : paintProps.iconBinders.get<TIcon>();
}

template <typename TText, typename TIcon, std::size_t N>
auto getInterpFactor(const SymbolBucket::PaintProperties& paintProps, bool isText, float currentZoom) {
    return std::get<N>(getProperty<TText, TIcon>(paintProps, isText)->interpolationFactor(currentZoom));
}

SymbolInterpolateUBO buildInterpUBO(const SymbolBucket::PaintProperties& paint, const bool t, const float z) {
    return {/* .fill_color_t = */ getInterpFactor<TextColor, IconColor, 0>(paint, t, z),
            /* .halo_color_t = */ getInterpFactor<TextHaloColor, IconHaloColor, 0>(paint, t, z),
            /* .opacity_t = */ getInterpFactor<TextOpacity, IconOpacity, 0>(paint, t, z),
            /* .halo_width_t = */ getInterpFactor<TextHaloWidth, IconHaloWidth, 0>(paint, t, z),
            /* .halo_blur_t = */ getInterpFactor<TextHaloBlur, IconHaloBlur, 0>(paint, t, z),
            /* .padding = */ 0,
            0,
            0};
}

} // namespace

void SymbolLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    auto& context = parameters.context;
    const auto& state = parameters.state;
    const auto& evaluated = static_cast<const SymbolLayerProperties&>(*evaluatedProperties).evaluated;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    const auto zoom = static_cast<float>(state.getZoom());

    if (!evaluatedPropsUniformBuffer || propertiesUpdated) {
        const SymbolEvaluatedPropsUBO propsUBO = {/*.text_fill_color=*/constOrDefault<TextColor>(evaluated),
                                                  /*.text_halo_color=*/constOrDefault<TextHaloColor>(evaluated),
                                                  /*.text_opacity=*/constOrDefault<TextOpacity>(evaluated),
                                                  /*.text_halo_width=*/constOrDefault<TextHaloWidth>(evaluated),
                                                  /*.text_halo_blur=*/constOrDefault<TextHaloBlur>(evaluated),
                                                  /* pad */ 0,
                                                  /*.icon_fill_color=*/constOrDefault<IconColor>(evaluated),
                                                  /*.icon_halo_color=*/constOrDefault<IconHaloColor>(evaluated),
                                                  /*.icon_opacity=*/constOrDefault<IconOpacity>(evaluated),
                                                  /*.icon_halo_width=*/constOrDefault<IconHaloWidth>(evaluated),
                                                  /*.icon_halo_blur=*/constOrDefault<IconHaloBlur>(evaluated),
                                                  /* pad */ 0};
        context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &propsUBO);
        propertiesUpdated = false;
    }
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.set(idSymbolEvaluatedPropsUBO, evaluatedPropsUniformBuffer);

    int i = 0;
    std::vector<SymbolDrawableUBO> drawableUBOVector(layerGroup.getDrawableCount());
    std::vector<SymbolTilePropsUBO> tilePropsUBOVector(layerGroup.getDrawableCount());
    std::vector<SymbolInterpolateUBO> interpolateUBOVector(layerGroup.getDrawableCount());

    const auto camDist = state.getCameraToCenterDistance();
    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !drawable.getData()) {
            return;
        }

        const auto tileID = drawable.getTileID()->toUnwrapped();
        const auto& symbolData = static_cast<gfx::SymbolDrawableData&>(*drawable.getData());
        const auto isText = (symbolData.symbolType == SymbolType::Text);

        const auto* textBinders = isText ? static_cast<SymbolSDFTextProgram::Binders*>(drawable.getBinders()) : nullptr;
        const auto* iconBinders = isText ? nullptr : static_cast<SymbolIconProgram::Binders*>(drawable.getBinders());

        const auto bucket = std::static_pointer_cast<SymbolBucket>(drawable.getBucket());
        const auto* tile = drawable.getRenderTile();
        if (!bucket || !tile || (!textBinders && !iconBinders)) {
            assert(false);
            return;
        }

        const auto& paintProperties = bucket->paintProperties.at(id);

        // from RenderTile::translatedMatrix
        const auto translate = isText ? evaluated.get<style::TextTranslate>() : evaluated.get<style::IconTranslate>();
        const auto anchor = isText ? evaluated.get<style::TextTranslateAnchor>()
                                   : evaluated.get<style::IconTranslateAnchor>();
        constexpr bool nearClipped = false;
        constexpr bool inViewportPixelUnits = false;
        const auto matrix = getTileMatrix(
            tileID, parameters, translate, anchor, nearClipped, inViewportPixelUnits, drawable);

        // from symbol_program, makeValues
        const auto currentZoom = static_cast<float>(parameters.state.getZoom());
        const float pixelsToTileUnits = tileID.pixelsToTileUnits(1.f, currentZoom);
        const bool pitchWithMap = symbolData.pitchAlignment == style::AlignmentType::Map;
        const bool rotateWithMap = symbolData.rotationAlignment == style::AlignmentType::Map;
        const bool alongLine = symbolData.placement != SymbolPlacementType::Point &&
                               symbolData.rotationAlignment == AlignmentType::Map;
        const bool hasVariablePlacement = symbolData.bucketVariablePlacement &&
                                          (isText || symbolData.textFit != IconTextFitType::None);
        const mat4 labelPlaneMatrix = (alongLine || hasVariablePlacement)
                                          ? matrix::identity4()
                                          : getLabelPlaneMatrix(
                                                matrix, pitchWithMap, rotateWithMap, state, pixelsToTileUnits);
        const mat4 glCoordMatrix = getGlCoordMatrix(matrix, pitchWithMap, rotateWithMap, state, pixelsToTileUnits);

        const float gammaScale = (symbolData.pitchAlignment == AlignmentType::Map
                                      ? static_cast<float>(std::cos(state.getPitch())) * camDist
                                      : 1.0f);

        // Line label rotation happens in `updateLineLabels`/`reprojectLineLabels``
        // Pitched point labels are automatically rotated by the labelPlaneMatrix projection
        // Unpitched point labels need to have their rotation applied after projection
        const bool rotateInShader = rotateWithMap && !pitchWithMap && !alongLine;

        drawableUBOVector[i] = {
            /*.matrix=*/util::cast<float>(matrix),
            /*.label_plane_matrix=*/util::cast<float>(labelPlaneMatrix),
            /*.coord_matrix=*/util::cast<float>(glCoordMatrix),

            /*.texsize=*/toArray(getTexSize(drawable, idSymbolImageTexture)),
            /*.texsize_icon=*/toArray(getTexSize(drawable, idSymbolImageIconTexture)),

            /*.gamma_scale=*/gammaScale,
            /*.rotate_symbol=*/rotateInShader,
            /*.pad=*/{0},
        };

        const auto& sizeBinder = isText ? bucket->textSizeBinder : bucket->iconSizeBinder;
        const auto size = sizeBinder->evaluateForZoom(currentZoom);
        tilePropsUBOVector[i] = SymbolTilePropsUBO{
            /* .is_text = */ isText,
            /* .is_halo = */ symbolData.isHalo,
            /* .pitch_with_map = */ (symbolData.pitchAlignment == style::AlignmentType::Map),
            /* .is_size_zoom_constant = */ size.isZoomConstant,
            /* .is_size_feature_constant = */ size.isFeatureConstant,
            /* .size_t = */ size.sizeT,
            /* .size = */ size.size,
            /* .padding = */ 0,
        };

        interpolateUBOVector[i] = buildInterpUBO(paintProperties, isText, zoom);

        drawable.setUBOIndex(i);
        i++;
    });

    if (layerGroup.getDrawableCount() > 60) {
        assert(false);
    }

    const size_t drawableUBOVectorSize = sizeof(SymbolDrawableUBO) * drawableUBOVector.size();
    if (!drawableBuffer || drawableBuffer->getSize() < drawableUBOVectorSize) {
        drawableBuffer = context.createUniformBuffer(drawableUBOVector.data(), drawableUBOVectorSize);
    } else {
        drawableBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    const size_t tilePropsUBOVectorSize = sizeof(SymbolTilePropsUBO) * tilePropsUBOVector.size();
    if (!tilePropsBuffer || tilePropsBuffer->getSize() < tilePropsUBOVectorSize) {
        tilePropsBuffer = context.createUniformBuffer(tilePropsUBOVector.data(), tilePropsUBOVectorSize);
    } else {
        tilePropsBuffer->update(tilePropsUBOVector.data(), tilePropsUBOVectorSize);
    }

    const size_t interpolateUBOVectorSize = sizeof(SymbolInterpolateUBO) * interpolateUBOVector.size();
    if (!interpolateBuffer || interpolateBuffer->getSize() < interpolateUBOVectorSize) {
        interpolateBuffer = context.createUniformBuffer(interpolateUBOVector.data(), interpolateUBOVectorSize);
    } else {
        interpolateBuffer->update(interpolateUBOVector.data(), interpolateUBOVectorSize);
    }

    layerUniforms.set(idSymbolDrawableUBO, drawableBuffer);
    layerUniforms.set(idSymbolTilePropsUBO, tilePropsBuffer);
    layerUniforms.set(idSymbolInterpolateUBO, interpolateBuffer);
}

} // namespace mbgl
