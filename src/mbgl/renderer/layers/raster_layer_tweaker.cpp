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

namespace mbgl {

using namespace style;
using namespace shaders;

void RasterLayerTweaker::execute([[maybe_unused]] LayerGroupBase& layerGroup,
                                 [[maybe_unused]] const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

    const auto& evaluated = static_cast<const RasterLayerProperties&>(*evaluatedProperties).evaluated;

    const auto spinWeights = [](float spin) -> std::array<float, 4> {
        spin = util::deg2radf(spin);
        const float s = std::sin(spin);
        const float c = std::cos(spin);
        std::array<float, 4> spin_weights = {{(2 * c + 1) / 3,
                                              (-std::numbers::sqrt3_v<float> * s - c + 1) / 3,
                                              (std::numbers::sqrt3_v<float> * s - c + 1) / 3,
                                              0}};
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

    if (!evaluatedPropsUniformBuffer || propertiesUpdated) {
        const RasterEvaluatedPropsUBO propsUBO = {
            .spin_weights = spinWeights(evaluated.get<RasterHueRotate>()),
            .tl_parent = {{0.0f, 0.0f}},
            .scale_parent = 1.0f,
            .buffer_scale = 1.0f,
            .fade_t = 1.0f,
            .opacity = evaluated.get<RasterOpacity>(),
            .brightness_low = evaluated.get<RasterBrightnessMin>(),
            .brightness_high = evaluated.get<RasterBrightnessMax>(),
            .saturation_factor = saturationFactor(evaluated.get<RasterSaturation>()),
            .contrast_factor = contrastFactor(evaluated.get<RasterContrast>()),
            .pad1 = 0,
            .pad2 = 0};
        parameters.context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &propsUBO);
        propertiesUpdated = false;
    }
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.set(idRasterEvaluatedPropsUBO, evaluatedPropsUniformBuffer);

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<RasterDrawableUBO> drawableUBOVector(layerGroup.getDrawableCount());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!checkTweakDrawable(drawable)) {
            return;
        }

        mat4 matrix;
        if (!drawable.getTileID()) {
            // this is an image drawable
            if (const auto& data = drawable.getData()) {
                const gfx::ImageDrawableData& imageData = static_cast<const gfx::ImageDrawableData&>(*data);
                matrix = imageData.matrix;
                multiplyWithProjectionMatrix(
                    /*in-out*/ matrix, parameters, drawable, /*nearClipped*/ false, /*aligned*/ true);
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

#if MLN_UBO_CONSOLIDATION
        drawableUBOVector[i] = {
#else
        const RasterDrawableUBO drawableUBO = {
#endif
            /* .matrix = */ util::cast<float>(matrix)
        };
#if MLN_UBO_CONSOLIDATION
        drawable.setUBOIndex(i++);
#else
        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idRasterDrawableUBO, &drawableUBO, parameters.context);
#endif
    });

#if MLN_UBO_CONSOLIDATION
    auto& context = parameters.context;
    const size_t drawableUBOVectorSize = sizeof(RasterDrawableUBO) * drawableUBOVector.size();
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < drawableUBOVectorSize) {
        drawableUniformBuffer = context.createUniformBuffer(
            drawableUBOVector.data(), drawableUBOVectorSize, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    layerUniforms.set(idRasterDrawableUBO, drawableUniformBuffer);
#endif
}

} // namespace mbgl
