#include <mbgl/renderer/layers/symbol_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/symbol_drawable_data.hpp>
#include <mbgl/layout/symbol_projection.hpp>
#include <mbgl/programs/symbol_program.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
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

Size getTexSize(const gfx::Drawable& drawable, const StringIdentity nameId) {
    if (const auto& shader = drawable.getShader()) {
        if (const auto index = shader->getSamplerLocation(nameId)) {
            if (const auto& tex = drawable.getTexture(*index)) {
                return tex->getSize();
            }
        }
    }
    return {0, 0};
}

std::array<float, 2> toArray(const Size& s) {
    return util::cast<float>(std::array<uint32_t, 2>{s.width, s.height});
}

const StringIdentity idTexUniformName = stringIndexer().get("u_texture");
const StringIdentity idTexIconUniformName = stringIndexer().get("u_texture_icon");

SymbolDrawablePaintUBO buildPaintUBO(bool isText, const SymbolPaintProperties::PossiblyEvaluated& evaluated) {
    return {
        /*.fill_color=*/isText ? constOrDefault<TextColor>(evaluated) : constOrDefault<IconColor>(evaluated),
        /*.halo_color=*/
        isText ? constOrDefault<TextHaloColor>(evaluated) : constOrDefault<IconHaloColor>(evaluated),
        /*.opacity=*/isText ? constOrDefault<TextOpacity>(evaluated) : constOrDefault<IconOpacity>(evaluated),
        /*.halo_width=*/
        isText ? constOrDefault<TextHaloWidth>(evaluated) : constOrDefault<IconHaloWidth>(evaluated),
        /*.halo_blur=*/isText ? constOrDefault<TextHaloBlur>(evaluated) : constOrDefault<IconHaloBlur>(evaluated),
        /*.padding=*/0,
    };
}

} // namespace

const size_t SymbolLayerTweaker::idSymbolDrawableUBOName = 10;
const size_t SymbolLayerTweaker::idSymbolDynamicUBOName = 11;
const size_t SymbolLayerTweaker::idSymbolDrawablePaintUBOName = 12;
const size_t SymbolLayerTweaker::idSymbolDrawableTilePropsUBOName = 13;
const size_t SymbolLayerTweaker::idSymbolDrawableInterpolateUBOName = 14;

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

    if (propertiesUpdated) {
        textPropertiesUpdated = true;
        iconPropertiesUpdated = true;
        propertiesUpdated = false;
    }

    const auto camDist = state.getCameraToCenterDistance();

    const SymbolDynamicUBO dynamicUBO = {/*.fade_change=*/parameters.symbolFadeChange,
                                         /*.camera_to_center_distance=*/camDist,
                                         /*.aspect_ratio=*/state.getSize().aspectRatio(),
                                         0};

    if (!dynamicBuffer) {
        dynamicBuffer = parameters.context.createUniformBuffer(&dynamicUBO, sizeof(dynamicUBO));
    } else {
        dynamicBuffer->update(&dynamicUBO, sizeof(dynamicUBO));
    }

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !drawable.getData()) {
            return;
        }

        const auto tileID = drawable.getTileID()->toUnwrapped();
        const auto& symbolData = static_cast<gfx::SymbolDrawableData&>(*drawable.getData());
        const auto isText = (symbolData.symbolType == SymbolType::Text);

        if (isText && (!textPaintBuffer || textPropertiesUpdated)) {
            const auto props = buildPaintUBO(true, evaluated);
            textPaintBuffer = parameters.context.createUniformBuffer(&props, sizeof(props));
            textPropertiesUpdated = false;
        } else if (!isText && (!iconPaintBuffer || iconPropertiesUpdated)) {
            const auto props = buildPaintUBO(false, evaluated);
            iconPaintBuffer = parameters.context.createUniformBuffer(&props, sizeof(props));
            iconPropertiesUpdated = false;
        }

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

        const SymbolDrawableUBO drawableUBO = {
            /*.matrix=*/util::cast<float>(matrix),
            /*.label_plane_matrix=*/util::cast<float>(labelPlaneMatrix),
            /*.coord_matrix=*/util::cast<float>(glCoordMatrix),

            /*.texsize=*/toArray(getTexSize(drawable, idTexUniformName)),
            /*.texsize_icon=*/toArray(getTexSize(drawable, idTexIconUniformName)),

            /*.gamma_scale=*/gammaScale,
            /*.rotate_symbol=*/rotateInShader,
            /*.pad=*/{0},
        };

        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.createOrUpdate(idSymbolDrawableUBOName, &drawableUBO, context);

        uniforms.addOrReplace(idSymbolDynamicUBOName, dynamicBuffer);
        uniforms.addOrReplace(idSymbolDrawablePaintUBOName, isText ? textPaintBuffer : iconPaintBuffer);
    });
}

} // namespace mbgl
