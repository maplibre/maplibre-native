#include <mbgl/renderer/layers/fill_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) FillDrawableUBO {
    std::array<float, 4 * 4> matrix;
    /*  64 */ std::array<float, 4> scale;
    std::array<float, 2> world;
    /*  88 */ std::array<float, 2> pixel_coord_upper;
    /*  96 */ std::array<float, 2> pixel_coord_lower;
    /* 104 */ std::array<float, 2> texsize;
    /* 112 */ float fade;

    // Attribute interpolations (used without HAS_UNIFORM_u_*)
    /* 116 */ float color_t;
    /* 120 */ float opacity_t;
    /* 124 */ float outline_color_t;
    /* 128 */ float pattern_from_t;
    /* 132 */ float pattern_to_t;

    // Uniform alternates for attributes (used with HAS_UNIFORM_u_*)
    /* 136 */ std::array<float, 2> color;
    /* 144 */ std::array<float, 2> opacity;
    /* 152 */ std::array<float, 2> outline_color_pad;
    /* 160 */ std::array<float, 4> outline_color;
    /* 176 */ std::array<float, 4> pattern_from;
    /* 208 */ std::array<float, 4> pattern_to;

    /*  */ // std::array<float, 3> padding;
    /* 208 */
};
static_assert(sizeof(FillDrawableUBO) == 208);

// move to utils, or does it already exist somewhere?
template <typename T, std::size_t N, std::size_t M>
auto concat(const std::array<T, N>& a1, const std::array<T, M>& a2) {
    std::array<T, N + M> result;
    std::copy(a1.cbegin(), a1.cend(), result.begin());
    std::copy(a2.cbegin(), a2.cend(), result.begin() + N);
    return result;
}

void FillLayerTweaker::execute(LayerGroupBase& layerGroup,
                               const RenderTree& renderTree,
                               const PaintParameters& parameters) {
    const auto& props = static_cast<const FillLayerProperties&>(*evaluatedProperties);
    const auto& evaluated = props.evaluated;
    const auto& crossfade = props.crossfade;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getTileID()) {
            return;
        }
        /*mat4 matrix = drawable.getMatrix();
        if (drawable.getTileID()) {
            const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
            const auto tileMat = parameters.matrixForTile(tileID);
            matrix::multiply(matrix, drawable.getMatrix(), tileMat);
            matrix = tileMat;
        }

        const auto renderableSize = parameters.backend.getDefaultRenderable().getSize();

        gfx::DrawableUBO drawableUBO;
        drawableUBO.matrix = util::cast<float>(matrix);
        drawableUBO.world = {(float)renderableSize.width, (float)renderableSize.height};
        auto uniformBuffer = context.createUniformBuffer(&drawableUBO, sizeof(drawableUBO));
        drawable.mutableUniformBuffers().addOrReplace("DrawableUBO", uniformBuffer);*/

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto& colorProperty = evaluated.get<FillColor>();
        const auto color = (colorProperty.isConstant() && colorProperty.constant()) ? *colorProperty.constant()
                                                                                    : Color(0.0f, 0.0f, 0.0f, 0.0f);

        const auto& outlineColorProperty = evaluated.get<FillOutlineColor>();
        const auto outlineColor = (outlineColorProperty.isConstant() && outlineColorProperty.constant())
                                      ? *outlineColorProperty.constant()
                                      : Color(0.0f, 0.0f, 0.0f, 0.0f);

        const auto& opacityProperty = evaluated.get<FillOpacity>();
        const auto opacity = (opacityProperty.isConstant() && opacityProperty.constant()) ? *opacityProperty.constant()
                                                                                          : 1.0f;

        const auto& fillPatternProperty = evaluated.get<FillPattern>();
        if (fillPatternProperty.isConstant() && fillPatternProperty.constant()) {
            Faded<style::expression::Image> pattern = *fillPatternProperty.constant();
            const auto& patternFrom = pattern.from;
            const auto& patternTo = pattern.to;
            [[maybe_unused]] const auto& idFrom = patternFrom.id();
            [[maybe_unused]] const auto& idTo = patternTo.id();
        }

        const auto& translation = evaluated.get<FillTranslate>();
        const auto anchor = evaluated.get<FillTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translation, anchor, inViewportPixelUnits);

        // from FillPatternProgram::layoutUniformValues
        const auto renderableSize = parameters.backend.getDefaultRenderable().getSize();
        const auto intZoom = parameters.state.getIntegerZoom();
        const auto tileRatio = 1 / tileID.pixelsToTileUnits(1, intZoom);
        const int32_t tileSizeAtNearestZoom = static_cast<int32_t>(
            util::tileSize_D * parameters.state.zoomScale(intZoom - tileID.canonical.z));
        const int32_t pixelX = static_cast<int32_t>(
            tileSizeAtNearestZoom *
            (tileID.canonical.x + tileID.wrap * parameters.state.zoomScale(tileID.canonical.z)));
        const int32_t pixelY = tileSizeAtNearestZoom * tileID.canonical.y;
        const auto pixelRatio = parameters.pixelRatio;

        const auto& textures = drawable.getTextures();
        const auto hasTex = !textures.empty() && textures[0].texture;
        const Size textureSize = hasTex ? textures[0].texture->getSize() : Size(0, 0);

        const FillDrawableUBO drawableUBO = {
            /*.matrix=*/util::cast<float>(matrix),
            /*.scale=*/{pixelRatio, tileRatio, crossfade.fromScale, crossfade.toScale},
            /*.world=*/{(float)renderableSize.width, (float)renderableSize.height},
            /*.pixel_coord_upper=*/{static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
            /*.pixel_coord_lower=*/{static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
            /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
            /*.fade=*/crossfade.t,
            /*.color_t=*/crossfade.t,
            /*.opacity_t=*/crossfade.t,
            /*.outline_color_t=*/crossfade.t,
            /*.pattern_from_t=*/crossfade.t,
            /*.pattern_to_t=*/crossfade.t,
            /*.color=*/attributeValue(color),
            /*.opacity=*/{opacity, opacity},
            /*.outline_color_pad=*/{0.0f},
            /*.outline_color=*/util::cast<float>(outlineColor.toArray()),
            /*.pattern_from=*/{0.0f},
            /*.pattern_to=*/{0.0f},
        };

        if (auto& ubo = drawable.mutableUniformBuffers().get("FillDrawableUBO")) {
            ubo->update(&drawableUBO, sizeof(drawableUBO));
        } else {
            drawable.mutableUniformBuffers().addOrReplace(
                "FillDrawableUBO", parameters.context.createUniformBuffer(&drawableUBO, sizeof(drawableUBO)));
        }
    });
}
} // namespace mbgl
