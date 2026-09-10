#include <mln/renderer/layers/plugin_layer_tweaker.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/drawable.hpp>
#include <mln/plugin/plugin_drawable_data.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/buckets/plugin_bucket.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/style/plugin_property.hpp>
#include <mln/style/types.hpp>
#include <mln/util/convert.hpp>
#include <mln/util/geo.hpp>
#include <mln/util/logging.hpp>

#include <algorithm>
#include <cstring>

namespace mln {

void PluginLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) return;

    {
        visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
            if (!drawable.getData() || !checkTweakDrawable(drawable)) return;
            const auto& data = static_cast<const plugin::DrawableData&>(*drawable.getData());
            const auto* shader = [&]() -> const plugin::ShaderDefinition* {
                const auto it = std::find_if(registration.shaders.begin(),
                                             registration.shaders.end(),
                                             [&](const auto& candidate) { return candidate.id == data.shaderID; });
                return it == registration.shaders.end() ? nullptr : &*it;
            }();
            if (!shader) return;

            const bool hasTile = drawable.getTileID().has_value();
            const std::optional<UnwrappedTileID> tileID =
                hasTile ? std::optional<UnwrappedTileID>{drawable.getTileID()->toUnwrapped()} : std::nullopt;
            mat4 tileMatrix = matrix::identity4();
            if (hasTile) {
                tileMatrix = getTileMatrix(*tileID,
                                           parameters,
                                           {0.0f, 0.0f},
                                           style::TranslateAnchorType::Viewport,
                                           false,
                                           false,
                                           drawable,
                                           true);
            }
            mln_plugin_uniform_context_v1 callbackContext{};
            callbackContext.struct_size = sizeof(callbackContext);
            callbackContext.bearing = parameters.state.getBearing();
            callbackContext.camera_to_center_distance = parameters.state.getCameraToCenterDistance();
            callbackContext.pixel_ratio = parameters.pixelRatio;
            callbackContext.pixels_to_gl_units[0] = parameters.pixelsToGLUnits[0];
            callbackContext.pixels_to_gl_units[1] = parameters.pixelsToGLUnits[1];
            const auto tileMatrixFloats = util::cast<float>(tileMatrix);
            std::copy(tileMatrixFloats.begin(), tileMatrixFloats.end(), callbackContext.tile_matrix);
            const auto viewportSize = parameters.state.getSize();
            callbackContext.viewport_width = viewportSize.width;
            callbackContext.viewport_height = viewportSize.height;
            callbackContext.pixels_to_tile_units = hasTile ? tileID->pixelsToTileUnits(
                                                                 1.0f, static_cast<float>(parameters.state.getZoom()))
                                                           : 0.0f;

            for (const auto& uniform : shader->uniformBlocks) {
                if (!registration.updateUniformBlock) continue;
                std::vector<uint8_t> bytes(uniform.byteSize);
                const auto status = registration.updateUniformBlock(
                    &callbackContext, uniform.id, bytes.data(), bytes.size());
                if (status != MLN_PLUGIN_STATUS_OK) {
                    Log::Error(Event::General,
                               "Plugin '" + registration.pluginID + "' failed to update uniform " +
                                   std::to_string(uniform.id) + " (status " + std::to_string(static_cast<int>(status)) +
                                   ")");
                    continue;
                }
                if (const auto* baseBinders = drawable.getBinders()) {
                    const auto* binders = static_cast<const PluginPaintPropertyBinders*>(baseBinders);
                    binders->writeUniforms(
                        static_cast<float>(parameters.state.getZoom()), uniform.id, bytes.data(), bytes.size());
                }
                drawable.mutableUniformBuffers().createOrUpdate(
                    uniform.bindingID, bytes.data(), bytes.size(), parameters.context);
            }
        });
        propertiesUpdated = false;
        return;
    }
}

} // namespace mln
