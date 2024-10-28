#include <mbgl/renderer/layers/location_indicator_layer_tweaker.hpp>

#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/layers/render_location_indicator_layer.hpp>
#include <mbgl/style/layers/location_indicator_layer_properties.hpp>
#include <mbgl/shaders/common_ubo.hpp>

namespace mbgl {

void LocationIndicatorLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& params) {
    if (layerGroup.empty()) {
        return;
    }

    const auto& props = static_cast<const style::LocationIndicatorLayerProperties&>(*evaluatedProperties);

    const shaders::CommonUBO quadUBO = {/* .matrix */ util::cast<float>(projectionPuck),
                                        /* .color */ Color::black()};

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        auto& drawableUniforms = drawable.mutableUniformBuffers();

        if (!drawable.getEnabled()) {
            return;
        }

        switch (static_cast<RenderLocationIndicatorLayer::LocationIndicatorComponentType>(drawable.getType())) {
            case RenderLocationIndicatorLayer::LocationIndicatorComponentType::Circle: {
                shaders::CommonUBO circleUBO = {/* .matrix */ util::cast<float>(projectionCircle),
                                                /* .color */ props.evaluated.get<style::AccuracyRadiusColor>()};

                drawableUniforms.createOrUpdate(shaders::idCommonUBO, &circleUBO, params.context);
                break;
            }

            case RenderLocationIndicatorLayer::LocationIndicatorComponentType::CircleOutline: {
                shaders::CommonUBO circleUBO = {/* .matrix */ util::cast<float>(projectionCircle),
                                                /* .color */ props.evaluated.get<style::AccuracyRadiusBorderColor>()};

                drawableUniforms.createOrUpdate(shaders::idCommonUBO, &circleUBO, params.context);
                break;
            }

            case RenderLocationIndicatorLayer::LocationIndicatorComponentType::PuckShadow:
                [[fallthrough]];
            case RenderLocationIndicatorLayer::LocationIndicatorComponentType::Puck:
                [[fallthrough]];
            case RenderLocationIndicatorLayer::LocationIndicatorComponentType::PuckHat:
                drawableUniforms.createOrUpdate(shaders::idCommonUBO, &quadUBO, params.context);
                break;

            default:
                assert(false);
                break;
        }
    });
}

} // namespace mbgl
