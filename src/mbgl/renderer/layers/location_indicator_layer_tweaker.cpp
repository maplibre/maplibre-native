#include <mbgl/renderer/layers/location_indicator_layer_tweaker.hpp>

#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/layers/render_location_indicator_layer.hpp>
#include <mbgl/style/layers/location_indicator_layer_properties.hpp>
#include <mbgl/shaders/location_indicator_ubo.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

void LocationIndicatorLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& params) {
    if (layerGroup.empty()) {
        return;
    }

    const auto& props = static_cast<const LocationIndicatorLayerProperties&>(*evaluatedProperties);

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        auto& drawableUniforms = drawable.mutableUniformBuffers();

        if (!drawable.getEnabled()) {
            return;
        }

        switch (static_cast<RenderLocationIndicatorLayer::LocationIndicatorComponentType>(drawable.getType())) {
            case RenderLocationIndicatorLayer::LocationIndicatorComponentType::Circle: {
                LocationIndicatorDrawableUBO drawableUBO = {.matrix = util::cast<float>(projectionCircle),
                                                            .color = props.evaluated.get<AccuracyRadiusColor>()};
                drawableUniforms.createOrUpdate(idLocationIndicatorDrawableUBO, &drawableUBO, params.context);
                break;
            }

            case RenderLocationIndicatorLayer::LocationIndicatorComponentType::CircleOutline: {
                LocationIndicatorDrawableUBO drawableUBO = {.matrix = util::cast<float>(projectionCircle),
                                                            .color = props.evaluated.get<AccuracyRadiusBorderColor>()};
                drawableUniforms.createOrUpdate(idLocationIndicatorDrawableUBO, &drawableUBO, params.context);
                break;
            }

            case RenderLocationIndicatorLayer::LocationIndicatorComponentType::PuckShadow:
                [[fallthrough]];
            case RenderLocationIndicatorLayer::LocationIndicatorComponentType::Puck:
                [[fallthrough]];
            case RenderLocationIndicatorLayer::LocationIndicatorComponentType::PuckHat: {
                const LocationIndicatorDrawableUBO drawableUBO = {.matrix = util::cast<float>(projectionPuck),
                                                                  .color = Color::black()};
                drawableUniforms.createOrUpdate(idLocationIndicatorDrawableUBO, &drawableUBO, params.context);
                break;
            }

            default:
                assert(false);
                break;
        }
    });
}

} // namespace mbgl
