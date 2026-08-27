#include <mln/renderer/layers/location_indicator_layer_tweaker.hpp>

#include <mln/map/transform_state.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/layers/render_location_indicator_layer.hpp>
#include <mln/style/layers/location_indicator_layer_properties.hpp>
#include <mln/shaders/location_indicator_ubo.hpp>
#include <mln/util/projection.hpp>

namespace mln {

using namespace style;
using namespace shaders;

void LocationIndicatorLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& params) {
    if (layerGroup.empty()) {
        return;
    }

    const auto& props = static_cast<const LocationIndicatorLayerProperties&>(*evaluatedProperties);
    const auto& state = params.state;

    // A "tile" one world pixel wide with its origin at the puck, so the sphere gets the offsets as-is.
    const double worldSize = Projection::worldSize(state.getScale());
    const vec4 puckMercatorCoords{
        {positionMercator.x / worldSize, positionMercator.y / worldSize, 1.0 / worldSize, 1.0 / worldSize}};
    const auto projectionFor = [&](const mat4& fallbackMatrix) {
        auto data = state.getProjectionDataForMatrix(UnwrappedTileID{0, 0, 0}, fallbackMatrix);
        data.tileMercatorCoords = puckMercatorCoords;
        return toProjectionUBO(data);
    };

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<ProjectionUBO> projectionUBOVector(layerGroup.getDrawableCount());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        auto& drawableUniforms = drawable.mutableUniformBuffers();

        if (!drawable.getEnabled()) {
            return;
        }

        const auto type = static_cast<RenderLocationIndicatorLayer::LocationIndicatorComponentType>(drawable.getType());
        const bool isCircle = type == RenderLocationIndicatorLayer::LocationIndicatorComponentType::Circle ||
                              type == RenderLocationIndicatorLayer::LocationIndicatorComponentType::CircleOutline;
        const auto projectionUBO = projectionFor(isCircle ? projectionCircle : projectionPuck);
#if MLN_UBO_CONSOLIDATION
        projectionUBOVector[i] = projectionUBO;
        drawable.setUBOIndex(i++);
#else
        drawableUniforms.createOrUpdate(idProjectionUBO, &projectionUBO, params.context);
#endif

        switch (type) {
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

#if MLN_UBO_CONSOLIDATION
    uploadProjectionUBOs(layerGroup.mutableUniformBuffers(), projectionUBOVector, params.context);
#endif
}

} // namespace mln
