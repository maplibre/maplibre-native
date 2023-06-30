#include <mbgl/renderer/layers/heatmap_texture_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/style/layers/heatmap_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) HeatmapTextureDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> world;
    float opacity;
    float padding;
};
static_assert(sizeof(HeatmapTextureDrawableUBO) % 16 == 0);

static constexpr std::string_view HeatmapTextureDrawableUBOName = "HeatmapTextureDrawableUBO";

void HeatmapTextureLayerTweaker::execute(LayerGroupBase& layerGroup,
                                         [[maybe_unused]] const RenderTree& renderTree,
                                         const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const HeatmapLayerProperties&>(*evaluatedProperties).evaluated;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        const auto& size = parameters.staticData.backendSize;
        mat4 viewportMat;
        matrix::ortho(viewportMat, 0, size.width, size.height, 0, 0, 1);
        HeatmapTextureDrawableUBO drawableUBO = {
            /* .matrix = */ util::cast<float>(viewportMat),
            /* .world = */ {static_cast<float>(size.width), static_cast<float>(size.height)},
            /* .opacity = */ evaluated.get<HeatmapOpacity>(),
            /* .padding = */ 0};

        drawable.mutableUniformBuffers().createOrUpdate(HeatmapTextureDrawableUBOName, &drawableUBO, parameters.context);
    });
}

} // namespace mbgl
