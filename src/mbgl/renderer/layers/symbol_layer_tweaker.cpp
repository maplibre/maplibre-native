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
#include <mbgl/style/layers/symbol_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/std.hpp>

namespace mbgl {

using namespace style;

namespace {

struct alignas(16) SymbolDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> matrix;
    /*  64 */ std::array<float, 4 * 4> label_plane_matrix;
    /* 128 */ std::array<float, 4 * 4> coord_matrix;

    /* 192 */ std::array<float, 2> texsize;
    /* 200 */ std::array<float, 2> texsize_icon;

    /* 208 */ float gamma_scale;
    /* 212 */ float device_pixel_ratio;

    /* 216 */ float camera_to_center_distance;
    /* 220 */ float pitch;
    /* 224 */ /*bool*/ int rotate_symbol;
    /* 228 */ float aspect_ratio;
    /* 232 */ float fade_change;
    /* 236 */ float pad;
    /* 240 */
};
static_assert(sizeof(SymbolDrawableUBO) == 15 * 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) SymbolDrawablePaintUBO {
    /*  0 */ Color fill_color;
    /* 16 */ Color halo_color;
    /* 32 */ float opacity;
    /* 36 */ float halo_width;
    /* 40 */ float halo_blur;
    /* 44 */ float padding;
    /* 48 */
};
static_assert(sizeof(SymbolDrawablePaintUBO) == 3 * 16);

Size getTexSize(const gfx::Drawable& drawable, const std::string_view name) {
    if (const auto& shader = drawable.getShader()) {
        if (const auto index = shader->getSamplerLocation(name)) {
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

constexpr auto texUniformName = "u_texture";
constexpr auto texIconUniformName = "u_texture_icon";

template <typename T, class... Is, class... Ts>
auto constOrDefault(const IndexedTuple<TypeList<Is...>, TypeList<Ts...>>& evaluated) {
    return evaluated.template get<T>().constantOr(T::defaultValue());
}

SymbolDrawablePaintUBO buildPaintUBO(bool isText, const SymbolPaintProperties::PossiblyEvaluated& evaluated) {
    return {
        /*.fill_color=*/isText ? constOrDefault<TextColor>(evaluated)
                               : constOrDefault<IconColor>(evaluated),
        /*.halo_color=*/
        isText ? constOrDefault<TextHaloColor>(evaluated)
               : constOrDefault<IconHaloColor>(evaluated),
        /*.opacity=*/isText ? constOrDefault<TextOpacity>(evaluated) : constOrDefault<IconOpacity>(evaluated),
        /*.halo_width=*/
        isText ? constOrDefault<TextHaloWidth>(evaluated) : constOrDefault<IconHaloWidth>(evaluated),
        /*.halo_blur=*/isText ? constOrDefault<TextHaloBlur>(evaluated) : constOrDefault<IconHaloBlur>(evaluated),
        /*.padding=*/0,
    };
}

} // namespace

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

    layerGroup.visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !drawable.getData()) {
            return;
        }

        const auto tileID = drawable.getTileID()->toUnwrapped();
        const auto& symbolData = static_cast<gfx::SymbolDrawableData&>(*drawable.getData());
        const auto isText = (symbolData.symbolType == SymbolType::Text);

        if (isText && !textPaintBuffer) {
            const auto props = buildPaintUBO(true, evaluated);
            textPaintBuffer = parameters.context.createUniformBuffer(&props, sizeof(props));
        }
        if (!isText && !iconPaintBuffer) {
            const auto props = buildPaintUBO(false, evaluated);
            iconPaintBuffer = parameters.context.createUniformBuffer(&props, sizeof(props));
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
        const auto currentZoom = static_cast<float>(state.getZoom());
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

            /*.texsize=*/toArray(getTexSize(drawable, texUniformName)),
            /*.texsize_icon=*/toArray(getTexSize(drawable, texIconUniformName)),

            /*.gamma_scale=*/gammaScale,
            /*.device_pixel_ratio=*/parameters.pixelRatio,

            /*.camera_to_center_distance=*/camDist,
            /*.pitch=*/static_cast<float>(state.getPitch()),
            /*.rotate_symbol=*/rotateInShader,
            /*.aspect_ratio=*/state.getSize().aspectRatio(),
            /*.fade_change=*/parameters.symbolFadeChange,
            /*.pad=*/0,
        };

        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.createOrUpdate(SymbolDrawableUBOName, &drawableUBO, context);
        uniforms.addOrReplace(SymbolDrawablePaintUBOName, isText ? textPaintBuffer : iconPaintBuffer);
    });
}

} // namespace mbgl
