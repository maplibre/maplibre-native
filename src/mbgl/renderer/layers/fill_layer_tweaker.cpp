#include <mbgl/renderer/layers/fill_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/std.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) FillDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> matrix;
    /*  64 */ std::array<float, 4> scale;
    /*  80 */ std::array<float, 2> world;
    /*  88 */ std::array<float, 2> pixel_coord_upper;
    /*  96 */ std::array<float, 2> pixel_coord_lower;
    /* 104 */ std::array<float, 2> texsize;
    /* 112 */
};
static_assert(sizeof(FillDrawableUBO) == 112);

/// Evaluated properties that do not depend on the tile
struct alignas(16) FillDrawablePropsUBO {
    /*  0 */ Color color;
    /* 16 */ Color outline_color;
    /* 32 */ float opacity;
    /* 36 */ float pad1, pad2, pad3;
    /* 48 */
};
static_assert(sizeof(FillDrawablePropsUBO) == 48);

static constexpr std::string_view FillDrawableUBOName = "FillDrawableUBO";
static constexpr std::string_view FillDrawablePropsUBOName = "FillDrawablePropsUBO";

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

    if (!propsBuffer) {
        const FillDrawablePropsUBO paramsUBO = {
            /* .color = */ evaluated.get<FillColor>().constantOr(FillColor::defaultValue()),
            /* .outline_color = */ evaluated.get<FillOutlineColor>().constantOr(FillOutlineColor::defaultValue()),
            /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
            /* .padding = */ 0,
            0,
            0};
        propsBuffer = parameters.context.createUniformBuffer(&paramsUBO, sizeof(paramsUBO));
    }

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        drawable.mutableUniformBuffers().addOrReplace(FillDrawablePropsUBOName, propsBuffer);

        if (!drawable.getTileID()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto& translation = evaluated.get<FillTranslate>();
        const auto anchor = evaluated.get<FillTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = false;
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translation, anchor, nearClipped, inViewportPixelUnits);

        // from FillPatternProgram::layoutUniformValues
        const auto renderableSize = parameters.backend.getDefaultRenderable().getSize();
        const auto intZoom = parameters.state.getIntegerZoom();
        const auto tileRatio = 1.0f / tileID.pixelsToTileUnits(1.0f, intZoom);
        const int32_t tileSizeAtNearestZoom = static_cast<int32_t>(
            util::tileSize_D * parameters.state.zoomScale(intZoom - tileID.canonical.z));
        const int32_t pixelX = static_cast<int32_t>(
            tileSizeAtNearestZoom *
            (tileID.canonical.x + tileID.wrap * parameters.state.zoomScale(tileID.canonical.z)));
        const int32_t pixelY = tileSizeAtNearestZoom * tileID.canonical.y;
        const auto pixelRatio = parameters.pixelRatio;

        Size textureSize = {0, 0};
        if (const auto shader = drawable.getShader()) {
            if (const auto index = shader->getSamplerLocation("u_image")) {
                if (const auto& tex = drawable.getTexture(*index)) {
                    textureSize = tex->getSize();
                }
            }
        }

        const FillDrawableUBO drawableUBO = {
            /*.matrix=*/util::cast<float>(matrix),
            /*.scale=*/{pixelRatio, tileRatio, crossfade.fromScale, crossfade.toScale},
            /*.world=*/{(float)renderableSize.width, (float)renderableSize.height},
            /*.pixel_coord_upper=*/{static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
            /*.pixel_coord_lower=*/{static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
            /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
        };

        drawable.mutableUniformBuffers().createOrUpdate(FillDrawableUBOName, &drawableUBO, parameters.context);
    });
}

} // namespace mbgl
