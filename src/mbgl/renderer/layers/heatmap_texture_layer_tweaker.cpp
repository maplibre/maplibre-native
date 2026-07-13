#include <mbgl/renderer/layers/heatmap_texture_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/heatmap_texture_layer_ubo.hpp>
#include <mbgl/style/layers/heatmap_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

void HeatmapTextureLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

    const auto& evaluated = static_cast<const HeatmapLayerProperties&>(*evaluatedProperties).evaluated;

#ifndef NDEBUG
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    propertiesUpdated = false;

    mat4 matrix;
    const auto& size = parameters.staticData.backendSize;
    matrix::ortho(matrix, 0, size.width, size.height, 0, -1, 1);

    const HeatmapTexturePropsUBO propsUBO = {.matrix = util::cast<float>(matrix),
                                             .opacity = evaluated.get<HeatmapOpacity>(),
                                             .pad1 = 0,
                                             .pad2 = 0,
                                             .pad3 = 0};
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.createOrUpdate(idHeatmapTexturePropsUBO, &propsUBO, parameters.context);
}

} // namespace mbgl
