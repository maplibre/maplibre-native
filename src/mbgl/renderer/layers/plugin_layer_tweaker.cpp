#include <mbgl/renderer/layers/plugin_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {
namespace {

struct alignas(16) PluginDrawableUBO {
    std::array<float, 16> matrix;
    std::array<float, 2> extrudeScale;
    std::array<float, 2> pad{};
};
static_assert(sizeof(PluginDrawableUBO) == 5 * 16);

} // namespace

void PluginLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) return;

#if MLN_UBO_CONSOLIDATION
    uint32_t index = 0;
    std::vector<PluginDrawableUBO> drawableUBOs(layerGroup.getDrawableCount());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) return;
        const auto tileID = drawable.getTileID()->toUnwrapped();
        const auto matrix = getTileMatrix(tileID,
                                          parameters,
                                          {{0.0f, 0.0f}},
                                          style::TranslateAnchorType::Viewport,
                                          false,
                                          false,
                                          drawable);
        const PluginDrawableUBO ubo{util::cast<float>(matrix), parameters.pixelsToGLUnits, {0.0f, 0.0f}};
#if MLN_UBO_CONSOLIDATION
        drawableUBOs[index] = ubo;
        drawable.setUBOIndex(index++);
#else
        drawable.mutableUniformBuffers().createOrUpdate(
            shaders::idDrawableReservedVertexOnlyUBO, &ubo, parameters.context);
#endif
    });

#if MLN_UBO_CONSOLIDATION
    const auto size = drawableUBOs.size() * sizeof(PluginDrawableUBO);
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < size) {
        drawableUniformBuffer = parameters.context.createUniformBuffer(drawableUBOs.data(), size, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOs.data(), size);
    }
    layerGroup.mutableUniformBuffers().set(shaders::idDrawableReservedVertexOnlyUBO, drawableUniformBuffer);
#endif
}

} // namespace mbgl
