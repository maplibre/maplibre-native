#include <mbgl/renderer/layers/hillshade_prepare_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/hillshade_prepare_drawable_data.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/hillshade_prepare_layer_ubo.hpp>
#include <mbgl/style/layers/hillshade_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/string_indexer.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

static const size_t idHillshadePrepareDrawableUBOName = 2;

const std::array<float, 4>& getUnpackVector(Tileset::DEMEncoding encoding) {
    // https://www.mapbox.com/help/access-elevation-data/#mapbox-terrain-rgb
    static const std::array<float, 4> unpackMapbox = {{6553.6f, 25.6f, 0.1f, 10000.0f}};
    // https://aws.amazon.com/public-datasets/terrain/
    static const std::array<float, 4> unpackTerrarium = {{256.0f, 1.0f, 1.0f / 256.0f, 32768.0f}};

    return encoding == Tileset::DEMEncoding::Terrarium ? unpackTerrarium : unpackMapbox;
}

void HillshadePrepareLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    // const auto& evaluated = static_cast<const HillshadeLayerProperties&>(*evaluatedProperties).evaluated;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !drawable.getData() || !checkTweakDrawable(drawable)) {
            return;
        }
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto& drawableData = static_cast<const gfx::HillshadePrepareDrawableData&>(*drawable.getData());

        mat4 matrix;
        matrix::ortho(matrix, 0, util::EXTENT, -util::EXTENT, 0, -1, 1);
        matrix::translate(matrix, matrix, 0, -util::EXTENT, 0);

        const HillshadePrepareDrawableUBO drawableUBO = {
            /* .matrix = */ util::cast<float>(matrix),
            /* .unpack = */ getUnpackVector(drawableData.encoding),
            /* .dimension = */ {static_cast<float>(drawableData.stride), static_cast<float>(drawableData.stride)},
            /* .zoom = */ static_cast<float>(tileID.canonical.z),
            /* .maxzoom = */ static_cast<float>(drawableData.maxzoom)};

        drawable.mutableUniformBuffers().createOrUpdate(
            idHillshadePrepareDrawableUBOName, &drawableUBO, parameters.context);
    });
}

} // namespace mbgl
