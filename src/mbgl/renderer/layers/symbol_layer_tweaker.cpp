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
    /* 232 */ std::array<float, 2> fade_change_pad;
    /* 240 */
};
static_assert(sizeof(SymbolDrawableUBO) == 15 * 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) SymbolDrawablePaintUBO {
    /*  0 */ std::array<float, 4> fill_color;
    /* 16 */ std::array<float, 4> halo_color;
    /* 32 */ float opacity;
    /* 36 */ float halo_width;
    /* 40 */ float halo_blur;
    /* 44 */ float padding;
    /* 48 */
};
static_assert(sizeof(SymbolDrawablePaintUBO) == 3 * 16);

void SymbolLayerTweaker::execute(LayerGroupBase& layerGroup,
                                 const RenderTree& renderTree,
                                 const PaintParameters& parameters) {
    const auto& state = parameters.state;
    const auto& props = static_cast<const SymbolLayerProperties&>(*evaluatedProperties);
    const auto& evaluated = props.evaluated;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    if (!textBuffer) {
        const SymbolDrawablePaintUBO paramsUBO = {
            /*.fill_color=*/util::cast<float>(
                evaluated.get<TextColor>().constantOr(TextColor::defaultValue()).toArray()),
            /*.halo_color=*/
            util::cast<float>(evaluated.get<TextHaloColor>().constantOr(TextHaloColor::defaultValue()).toArray()),
            /*.opacity=*/evaluated.get<TextOpacity>().constantOr(TextOpacity::defaultValue()),
            /*.halo_width=*/evaluated.get<TextHaloWidth>().constantOr(TextHaloWidth::defaultValue()),
            /*.halo_blur=*/evaluated.get<TextHaloBlur>().constantOr(TextHaloBlur::defaultValue()),
            /*.padding=*/0,
        };
        textBuffer = parameters.context.createUniformBuffer(&paramsUBO, sizeof(paramsUBO));
    }
    if (!iconBuffer) {
        const SymbolDrawablePaintUBO paramsUBO = {
            /*.fill_color=*/util::cast<float>(
                evaluated.get<IconColor>().constantOr(IconColor::defaultValue()).toArray()),
            /*.halo_color=*/
            util::cast<float>(evaluated.get<IconHaloColor>().constantOr(IconHaloColor::defaultValue()).toArray()),
            /*.opacity=*/evaluated.get<IconOpacity>().constantOr(IconOpacity::defaultValue()),
            /*.halo_width=*/evaluated.get<IconHaloWidth>().constantOr(IconHaloWidth::defaultValue()),
            /*.halo_blur=*/evaluated.get<IconHaloBlur>().constantOr(IconHaloBlur::defaultValue()),
            /*.padding=*/0,
        };
        iconBuffer = parameters.context.createUniformBuffer(&paramsUBO, sizeof(paramsUBO));
    }

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getTileID()) {
            return;
        }
        if (!drawable.getData() || !*drawable.getData()) {
            return;
        }
        const auto& symbolData = static_cast<gfx::SymbolDrawableData&>(**drawable.getData());

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto translate = evaluated.get<style::TextTranslate>();
        const auto anchor = evaluated.get<style::TextTranslateAnchor>();

        drawable.mutableUniformBuffers().addOrReplace(SymbolDrawablePaintUBOName,
                                                      symbolData.isText ? textBuffer : iconBuffer);

        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translate, anchor, inViewportPixelUnits);

        const auto currentZoom = static_cast<float>(parameters.state.getZoom());

        const float pixelsToTileUnits = tileID.pixelsToTileUnits(1.f, currentZoom);
        const bool pitchWithMap = symbolData.pitchAlignment == style::AlignmentType::Map;
        const bool rotateWithMap = symbolData.rotationAlignment == style::AlignmentType::Map;

        const bool alongLine = symbolData.placement != SymbolPlacementType::Point &&
                               symbolData.rotationAlignment == AlignmentType::Map;

        // Line label rotation happens in `updateLineLabels`/`reprojectLineLabels``
        // Pitched point labels are automatically rotated by the labelPlaneMatrix projection
        // Unpitched point labels need to have their rotation applied after projection
        const bool rotateInShader = rotateWithMap && !pitchWithMap && !alongLine;

        const bool hasVariablePlacement = symbolData.hasVariablePlacement &&
                                          symbolData.textFit != IconTextFitType::None;

        mat4 labelPlaneMatrix;
        if (alongLine || hasVariablePlacement) {
            // For labels that follow lines the first part of the projection is
            // handled on the cpu. Pass an identity matrix because no transformation
            // needs to be done in the vertex shader.
            matrix::identity(labelPlaneMatrix);
        } else {
            labelPlaneMatrix = getLabelPlaneMatrix(matrix, pitchWithMap, rotateWithMap, state, pixelsToTileUnits);
        }

        const mat4 glCoordMatrix = getGlCoordMatrix(matrix, pitchWithMap, rotateWithMap, state, pixelsToTileUnits);

        const auto camDist = state.getCameraToCenterDistance();
        const float gammaScale = (symbolData.pitchAlignment == AlignmentType::Map
                                      ? static_cast<float>(std::cos(state.getPitch())) * camDist
                                      : 1.0f);

        Size textureSize = {0, 0};
        Size iconTtextureSize = {0, 0};
        if (const auto shader = drawable.getShader()) {
            if (const auto index = shader->getSamplerLocation("u_texture")) {
                if (const auto& tex = drawable.getTexture(*index)) {
                    textureSize = tex->getSize();
                }
            }
            if (const auto index = shader->getSamplerLocation("u_texture_icon")) {
                if (const auto& tex = drawable.getTexture(*index)) {
                    iconTtextureSize = tex->getSize();
                }
            }
        }

        const SymbolDrawableUBO drawableUBO = {
            /*.matrix=*/util::cast<float>(matrix),
            /*.label_plane_matrix=*/util::cast<float>(labelPlaneMatrix),
            /*.coord_matrix=*/util::cast<float>(glCoordMatrix),

            /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
            /*.texsize_icon=*/{static_cast<float>(iconTtextureSize.width), static_cast<float>(iconTtextureSize.height)},

            /*.gamma_scale=*/gammaScale,
            /*.device_pixel_ratio=*/parameters.pixelRatio,

            /*.camera_to_center_distance=*/camDist,
            /*.pitch=*/static_cast<float>(state.getPitch()),
            /*.rotate_symbol=*/rotateInShader,
            /*.aspect_ratio=*/state.getSize().aspectRatio(),
            /*.fade_change=*/{parameters.symbolFadeChange},
        };

        drawable.mutableUniformBuffers().createOrUpdate(SymbolDrawableUBOName, &drawableUBO, parameters.context);
    });
}

} // namespace mbgl
