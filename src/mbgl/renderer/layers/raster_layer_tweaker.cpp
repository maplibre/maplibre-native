#include <mbgl/renderer/layers/raster_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/raster_layer_ubo.hpp>
#include <mbgl/style/layers/raster_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/gfx/image_drawable_data.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/string_indexer.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

static const size_t idRasterDrawableUBOName = 2;

void RasterLayerTweaker::execute([[maybe_unused]] LayerGroupBase& layerGroup,
                                 [[maybe_unused]] const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const RasterLayerProperties&>(*evaluatedProperties).evaluated;

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!checkTweakDrawable(drawable)) {
            return;
        }

        const auto spinWeights = [](float spin) -> std::array<float, 4> {
            spin = util::deg2radf(spin);
            const float s = std::sin(spin);
            const float c = std::cos(spin);
            std::array<float, 4> spin_weights = {
                {(2 * c + 1) / 3, (-std::sqrt(3.0f) * s - c + 1) / 3, (std::sqrt(3.0f) * s - c + 1) / 3, 0}};
            return spin_weights;
        };
        const auto saturationFactor = [](float saturation) -> float {
            if (saturation > 0) {
                return 1.f - 1.f / (1.001f - saturation);
            } else {
                return -saturation;
            }
        };
        const auto contrastFactor = [](float contrast) -> float {
            if (contrast > 0) {
                return 1 / (1 - contrast);
            } else {
                return 1 + contrast;
            }
        };

        mat4 matrix;
        if (!drawable.getTileID()) {
            // this is an image drawable
            if (const auto& data = drawable.getData()) {
                const gfx::ImageDrawableData& imageData = static_cast<const gfx::ImageDrawableData&>(*data);

                matrix = imageData.matrix;
            } else {
                Log::Error(Event::General, "Invalid raster layer drawable: neither tile id nor image data is set.");
                return;
            }
        } else {
            // this is a tile drawable
            const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
            matrix = getTileMatrix(tileID,
                                   parameters,
                                   {0.f, 0.f},
                                   TranslateAnchorType::Viewport,
                                   false,
                                   false,
                                   drawable,
                                   !parameters.state.isChanging());
        }

        const RasterDrawableUBO drawableUBO{
            /*.matrix = */ util::cast<float>(matrix),
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
        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.createOrUpdate(idRasterDrawableUBOName, &drawableUBO, parameters.context);
    });
}

} // namespace mbgl
