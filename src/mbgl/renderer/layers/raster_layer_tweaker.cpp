#include <mbgl/renderer/layers/raster_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/style/layers/raster_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) RasterDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 4> spin_weights;
    std::array<float, 2> tl_parent;
    float scale_parent;
    float buffer_scale;
    float fade_t;
    float opacity;
    float brightness_low;
    float brightness_high;
    float saturation_factor;
    float contrast_factor;
    float pad1;
    float pad2;
};
static_assert(sizeof(RasterDrawableUBO) == 128);
static_assert(sizeof(RasterDrawableUBO) % 16 == 0);

void RasterLayerTweaker::execute([[maybe_unused]] LayerGroup& layerGroup,
                                 [[maybe_unused]] const RenderTree& renderTree,
                                 [[maybe_unused]] const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const RasterLayerProperties&>(*evaluatedProperties).evaluated;

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getTileID()) {
            return;
        }
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto matrix = parameters.matrixForTile(tileID, !parameters.state.isChanging());

        auto spinWeights = [](float spin) -> std::array<float, 4> {
            spin = util::deg2radf(spin);
            float s = std::sin(spin);
            float c = std::cos(spin);
            std::array<float, 4> spin_weights = {
                {(2 * c + 1) / 3, (-std::sqrt(3.0f) * s - c + 1) / 3, (std::sqrt(3.0f) * s - c + 1) / 3, 0}};
            return spin_weights;
        };
        auto saturationFactor = [](float saturation) -> float {
            if (saturation > 0) {
                return 1.f - 1.f / (1.001f - saturation);
            } else {
                return -saturation;
            }
        };
        auto contrastFactor = [](float contrast) -> float {
            if (contrast > 0) {
                return 1 / (1 - contrast);
            } else {
                return 1 + contrast;
            }
        };

        RasterDrawableUBO drawableUBO{/*.matrix = */ util::cast<float>(matrix),
                                      /*.spin_weigths = */ spinWeights(evaluated.get<RasterHueRotate>()),
                                      /*.tl_parent = */ {{0.0f, 0.0f}},
                                      /*.scale_parent = */ 1.0f,
                                      /*.buffer_scale = */ 1.0f,
                                      /*.fade_t = */ 1.0f,
                                      /*.opacity = */ evaluated.get<RasterOpacity>(),
                                      /*.brightness_low = */ evaluated.get<RasterBrightnessMin>(),
                                      /*.brightness_high = */ evaluated.get<RasterBrightnessMax>(),
                                      /*.saturation_factor = */ saturationFactor(evaluated.get<RasterSaturation>()),
                                      /*.contrast_factor = */ contrastFactor(evaluated.get<RasterContrast>()),
                                      0,
                                      0};
        auto drawableUniformBuffer = parameters.context.createUniformBuffer(&drawableUBO, sizeof(drawableUBO));
        drawable.mutableUniformBuffers().addOrReplace("RasterDrawableUBO", drawableUniformBuffer);
    });
}

} // namespace mbgl
