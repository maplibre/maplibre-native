#include <mbgl/renderer/layers/heatmap_layer_tweaker.hpp>

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

struct alignas(16) HeatmapDrawableUBO {
    std::array<float, 4 * 4> matrix;
    float extrude_scale;
    std::array<float, 3> padding;
};
static_assert(sizeof(HeatmapDrawableUBO) % 16 == 0);

struct alignas(16) HeatmapEvaluatedPropsUBO {
    float weight;
    float radius;
    float intensity;
    float padding;
};
static_assert(sizeof(HeatmapEvaluatedPropsUBO) % 16 == 0);

static constexpr std::string_view HeatmapDrawableUBOName = "HeatmapDrawableUBO";
static constexpr std::string_view HeatmapEvaluatedPropsUBOName = "HeatmapEvaluatedPropsUBO";

void HeatmapLayerTweaker::execute(LayerGroupBase& layerGroup,
                                  const RenderTree& renderTree,
                                  const PaintParameters& parameters) {
    auto& context = parameters.context;
    const auto& evaluated = static_cast<const HeatmapLayerProperties&>(*evaluatedProperties).evaluated;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    if (!evaluatedPropsUniformBuffer) {
        const HeatmapEvaluatedPropsUBO evaluatedPropsUBO = {
            /* .weight = */ evaluated.get<HeatmapWeight>().constantOr(HeatmapWeight::defaultValue()),
            /* .radius = */ evaluated.get<HeatmapRadius>().constantOr(HeatmapRadius::defaultValue()),
            /* .intensity = */ evaluated.get<HeatmapIntensity>(),
            /* .padding = */ 0};
        evaluatedPropsUniformBuffer = parameters.context.createUniformBuffer(&evaluatedPropsUBO,
                                                                             sizeof(evaluatedPropsUBO));
    }

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.addOrReplace(HeatmapEvaluatedPropsUBOName, evaluatedPropsUniformBuffer);

        if (!drawable.getTileID()) {
            return;
        }
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        constexpr bool nearClipped = false;
        constexpr bool inViewportPixelUnits = false;
        const auto matrix = getTileMatrix(tileID,
                                          renderTree,
                                          parameters.state,
                                          {0.f, 0.f},
                                          TranslateAnchorType::Viewport,
                                          nearClipped,
                                          inViewportPixelUnits);
        const HeatmapDrawableUBO drawableUBO = {
            /* .matrix = */ util::cast<float>(matrix),
            /* .extrude_scale = */ tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom())),
            /* .padding = */ {0}};

        uniforms.createOrUpdate(HeatmapDrawableUBOName, &drawableUBO, context);
    });
}

} // namespace mbgl
