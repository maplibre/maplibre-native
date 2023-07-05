#include <mbgl/renderer/layers/hillshade_prepare_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/hillshade_prepare_drawable_data.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/style/layers/hillshade_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) HillshadePrepareDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 4> unpack;
    std::array<float, 2> dimension;
    float zoom;
    float maxzoom;
};
static_assert(sizeof(HillshadePrepareDrawableUBO) % 16 == 0);

static constexpr std::string_view HillshadePrepareDrawableUBOName = "HillshadePrepareDrawableUBO";

const std::array<float, 4>& getUnpackVector(Tileset::DEMEncoding encoding) {
    // https://www.mapbox.com/help/access-elevation-data/#mapbox-terrain-rgb
    static const std::array<float, 4> unpackMapbox = {{6553.6f, 25.6f, 0.1f, 10000.0f}};
    // https://aws.amazon.com/public-datasets/terrain/
    static const std::array<float, 4> unpackTerrarium = {{256.0f, 1.0f, 1.0f / 256.0f, 32768.0f}};

    return encoding == Tileset::DEMEncoding::Terrarium ? unpackTerrarium : unpackMapbox;
}

void HillshadePrepareLayerTweaker::execute(LayerGroupBase& layerGroup,
                                           [[maybe_unused]] const RenderTree& renderTree,
                                           const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const HillshadeLayerProperties&>(*evaluatedProperties).evaluated;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !drawable.getData()) {
            return;
        }
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto& drawableData = static_cast<const gfx::HillshadePrepareDrawableData&>(*drawable.getData());

        mat4 matrix;
        matrix::ortho(matrix, 0, util::EXTENT, -util::EXTENT, 0, 0, 1);
        matrix::translate(matrix, matrix, 0, -util::EXTENT, 0);

        HillshadePrepareDrawableUBO drawableUBO = {/* .matrix = */ util::cast<float>(matrix),
                                                   /* .unpack = */ getUnpackVector(drawableData.encoding),
                                                   /* .dimension = */ {drawableData.stride, drawableData.stride},
                                                   /* .zoom = */ static_cast<float>(tileID.canonical.z),
                                                   /* .maxzoom = */ drawableData.maxzoom};

        drawable.mutableUniformBuffers().createOrUpdate(
            HillshadePrepareDrawableUBOName, &drawableUBO, parameters.context);
    });
}

} // namespace mbgl
