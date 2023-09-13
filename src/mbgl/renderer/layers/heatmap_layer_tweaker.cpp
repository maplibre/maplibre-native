#include <mbgl/renderer/layers/heatmap_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/heatmap_layer_ubo.hpp>
#include <mbgl/style/layers/heatmap_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/heatmap.hpp>
#endif

namespace mbgl {

using namespace style;
using namespace shaders;

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

    const auto zoom = parameters.state.getZoom();

#if MLN_RENDER_BACKEND_METAL
    using ShaderClass = shaders::ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Metal>;
    if (propertiesChanged) {
        const auto source = [this](const std::string_view& attrName) {
            return hasPropertyAsUniform(attrName) ? AttributeSource::Constant : AttributeSource::PerVertex;
        };

        const HeatmapPermutationUBO permutationUBO = {
            /* .weight = */ {/*.source=*/source(ShaderClass::attributes[1].name), /*.expression=*/{}},
            /* .radius = */ {/*.source=*/source(ShaderClass::attributes[2].name), /*.expression=*/{}},
            /* .overdrawInspector = */ overdrawInspector,
            /* .pad1/2/3 = */ 0,
            0,
            0,
            /* .pad4/5/6 = */ 0,
            0,
            0};

        if (permutationUniformBuffer) {
            permutationUniformBuffer->update(&permutationUBO, sizeof(permutationUBO));
        } else {
            permutationUniformBuffer = context.createUniformBuffer(&permutationUBO, sizeof(permutationUBO));
        }

        propertiesChanged = false;
    }
    if (!expressionUniformBuffer) {
        const auto expressionUBO = buildExpressionUBO(zoom, parameters.frameCount);
        expressionUniformBuffer = context.createUniformBuffer(&expressionUBO, sizeof(expressionUBO));
    }
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

    layerGroup.visitDrawables([&](gfx::Drawable& drawable) {
        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.addOrReplace(MLN_STRINGIZE(HeatmapEvaluatedPropsUBO), evaluatedPropsUniformBuffer);

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
            /* .extrude_scale = */ tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom)),
            /* .padding = */ {0}};

        uniforms.createOrUpdate(MLN_STRINGIZE(HeatmapDrawableUBO), &drawableUBO, context);

#if MLN_RENDER_BACKEND_METAL
        uniforms.addOrReplace(MLN_STRINGIZE(ExpressionInputsUBO), expressionUniformBuffer);
        uniforms.addOrReplace(MLN_STRINGIZE(HeatmapPermutationUBO), permutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
    });
}

} // namespace mbgl
