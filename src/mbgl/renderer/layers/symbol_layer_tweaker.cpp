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
const StringIdentity idExpressionInputsUBOName = stringIndexer().get("ExpressionInputsUBO");
const StringIdentity idSymbolPermutationUBOName = stringIndexer().get("SymbolPermutationUBO");

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

const StringIdentity SymbolLayerTweaker::idSymbolDrawableUBOName = stringIndexer().get("SymbolDrawableUBO");
const StringIdentity SymbolLayerTweaker::idSymbolDynamicUBOName = stringIndexer().get("SymbolDynamicUBO");
const StringIdentity SymbolLayerTweaker::idSymbolDrawablePaintUBOName = stringIndexer().get("SymbolDrawablePaintUBO");
const StringIdentity SymbolLayerTweaker::idSymbolDrawableTilePropsUBOName = stringIndexer().get(
    "SymbolDrawableTilePropsUBO");
const StringIdentity SymbolLayerTweaker::idSymbolDrawableInterpolateUBOName = stringIndexer().get(
    "SymbolDrawableInterpolateUBO");

void SymbolLayerTweaker::execute(LayerGroupBase& layerGroup,
                                 const RenderTree& renderTree,
                                 const PaintParameters& parameters) {
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

    const auto zoom = parameters.state.getZoom();

#if MLN_RENDER_BACKEND_METAL
    if (permutationUpdated) {
        const SymbolPermutationUBO permutationUBO = {
            /* .fill_color = */ {/*.source=*/getAttributeSource<BuiltIn::SymbolSDFIconShader>(5), /*.expression=*/{}},
            /* .halo_color = */ {/*.source=*/getAttributeSource<BuiltIn::SymbolSDFIconShader>(6), /*.expression=*/{}},
            /* .opacity = */ {/*.source=*/getAttributeSource<BuiltIn::SymbolSDFIconShader>(7), /*.expression=*/{}},
            /* .halo_width = */ {/*.source=*/getAttributeSource<BuiltIn::SymbolSDFIconShader>(8), /*.expression=*/{}},
            /* .halo_blur = */ {/*.source=*/getAttributeSource<BuiltIn::SymbolSDFIconShader>(9), /*.expression=*/{}},
            /* .overdrawInspector = */ overdrawInspector,
            /* .pad = */ 0,
            0,
            0};

        if (permutationUniformBuffer) {
            permutationUniformBuffer->update(&permutationUBO, sizeof(permutationUBO));
        } else {
            permutationUniformBuffer = context.createUniformBuffer(&permutationUBO, sizeof(permutationUBO));
        }

        permutationUpdated = false;
    }
    if (!expressionUniformBuffer) {
        const auto expressionUBO = buildExpressionUBO(zoom, parameters.frameCount);
        expressionUniformBuffer = context.createUniformBuffer(&expressionUBO, sizeof(expressionUBO));
    }
#endif

    if (propertiesUpdated) {
        textPropertiesUpdated = true;
        iconPropertiesUpdated = true;
        propertiesUpdated = false;
    }

    layerGroup.visitDrawables([&](gfx::Drawable& drawable) {
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
            tileID, renderTree, state, translate, anchor, nearClipped, inViewportPixelUnits);

        // from symbol_program, makeValues
        const auto currentZoom = static_cast<float>(zoom);
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

        const auto camDist = state.getCameraToCenterDistance();
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
            /*.device_pixel_ratio=*/parameters.pixelRatio,

            /*.camera_to_center_distance=*/camDist,
            /*.pitch=*/static_cast<float>(state.getPitch()),
            /*.rotate_symbol=*/rotateInShader,
            /*.aspect_ratio=*/state.getSize().aspectRatio(),
            /*.pad=*/{0},
        };

        const SymbolDynamicUBO dynamicUBO = {/*.fade_change=*/parameters.symbolFadeChange,
                                             /*.pad1=*/0,
                                             /*.pad2=*/{0, 0}};

        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.createOrUpdate(idSymbolDrawableUBOName, &drawableUBO, context);
        uniforms.createOrUpdate(idSymbolDynamicUBOName, &dynamicUBO, context);
        uniforms.addOrReplace(idSymbolDrawablePaintUBOName, isText ? textPaintBuffer : iconPaintBuffer);

#if MLN_RENDER_BACKEND_METAL
        uniforms.addOrReplace(idExpressionInputsUBOName, expressionUniformBuffer);
        uniforms.addOrReplace(idSymbolPermutationUBOName, permutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
    });
}

} // namespace mbgl
