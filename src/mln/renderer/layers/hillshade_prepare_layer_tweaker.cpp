#include <mln/renderer/layers/hillshade_prepare_layer_tweaker.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/drawable.hpp>
#include <mln/gfx/hillshade_prepare_drawable_data.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/render_tree.hpp>
#include <mln/shaders/hillshade_prepare_layer_ubo.hpp>
#include <mln/style/layers/hillshade_layer_properties.hpp>
#include <mln/util/convert.hpp>

#include <cmath>

namespace mln {

using namespace style;
using namespace shaders;

float hillshadeLog2WorldWidth(const WorldCoordinateHelper& helper) {
    return helper.wraps() ? 25.2562f : static_cast<float>(std::log2(helper.metersPerWorldUnit({0.5, 0.5})));
}

void HillshadePrepareLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    propertiesUpdated = false;

    const float log2WorldWidth = hillshadeLog2WorldWidth(parameters.state.getProjection().worldCoordinateHelper());

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !drawable.getData() || !checkTweakDrawable(drawable)) {
            return;
        }
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto& drawableData = static_cast<const gfx::HillshadePrepareDrawableData&>(*drawable.getData());

        mat4 matrix;
        matrix::ortho(matrix, 0, util::EXTENT, -util::EXTENT, 0, -1, 1);
        matrix::translate(matrix, matrix, 0, -util::EXTENT, 0);

        const HillshadePrepareDrawableUBO drawableUBO = {/* .matrix = */ util::cast<float>(matrix)};
        const HillshadePrepareTilePropsUBO tilePropsUBO = {
            .unpack = drawableData.unpack,
            .dimension = {static_cast<float>(drawableData.stride), static_cast<float>(drawableData.stride)},
            .zoom = static_cast<float>(tileID.canonical.z),
            .maxzoom = static_cast<float>(drawableData.maxzoom),
            .log2_world_width = log2WorldWidth,
            .pad1 = 0,
            .pad2 = 0,
            .pad3 = 0};

        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idHillshadePrepareDrawableUBO, &drawableUBO, parameters.context);
        drawableUniforms.createOrUpdate(idHillshadePrepareTilePropsUBO, &tilePropsUBO, parameters.context);
    });
}

} // namespace mln
