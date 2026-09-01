#include <mln/renderer/layers/plugin_layer_tweaker.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/drawable.hpp>
#include <mln/gfx/plugin_render_graph_drawable_data.hpp>
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
namespace {

std::array<float, 2> getLatRange(const UnwrappedTileID& id) {
    const LatLng north = LatLng(id);
    const LatLng south = LatLng(UnwrappedTileID(id.canonical.z, id.canonical.x, id.canonical.y + 1));
    return {static_cast<float>(north.latitude()), static_cast<float>(south.latitude())};
}

const plugin::ShaderDefinition* findShader(const plugin::LayerType& registration,
                                           const plugin::RenderPassDefinition& pass) {
    const auto it = std::find_if(registration.shaders.begin(), registration.shaders.end(), [&](const auto& shader) {
        return shader.id == pass.shaderID;
    });
    return it == registration.shaders.end() ? nullptr : &*it;
}

const plugin::RenderPassDefinition* findPass(const plugin::LayerType& registration, uint32_t passID) {
    if (!registration.renderGraph) return nullptr;
    const auto& passes = registration.renderGraph->passes;
    const auto it = std::find_if(passes.begin(), passes.end(), [&](const auto& pass) { return pass.id == passID; });
    return it == passes.end() ? nullptr : &*it;
}

} // namespace

void PluginLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) return;

    {
        const auto& properties = static_cast<const style::PluginStyleLayerProperties&>(*evaluatedProperties);
        const auto& impl = static_cast<const style::PluginStyleLayer::Impl&>(*properties.baseImpl);
        const auto propertyDefinitions = plugin::PluginRegistry::get().propertiesForLayer(registration.type);
        std::vector<style::PluginPropertyValue::EvaluationStorage> propertyStorage(propertyDefinitions.size());
        std::vector<mln_plugin_property_value_v1> propertyValues;
        propertyValues.reserve(propertyDefinitions.size());
        for (size_t i = 0; i < propertyDefinitions.size(); ++i) {
            const auto& definition = propertyDefinitions[i];
            const auto propertyIt = properties.evaluatedPaintProperties.find(definition.name);
            const auto value = propertyIt == properties.evaluatedPaintProperties.end()
                                   ? style::defaultPluginPropertyValue(definition)
                                   : propertyIt->second;
            mln_plugin_property_value_v1 property{};
            property.struct_size = sizeof(property);
            property.name = {definition.name.data(), definition.name.size()};
            property.value = value.evaluate(
                static_cast<float>(parameters.state.getZoom()), definition, propertyStorage[i]);
            property.explicitly_set = impl.pluginProperties.find(definition.name) != impl.pluginProperties.end();
            propertyValues.push_back(property);
        }

        visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
            if (!drawable.getData() || !checkTweakDrawable(drawable)) return;
            const auto& data = static_cast<const gfx::PluginRenderGraphDrawableData&>(*drawable.getData());
            const auto* pass = registration.renderGraph ? findPass(registration, data.passID) : nullptr;
            const auto* shader = pass ? findShader(registration, *pass) : [&]() -> const plugin::ShaderDefinition* {
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
                const bool nearClipped = pass ? pass->tileProjection == MLN_PLUGIN_TILE_PROJECTION_NEAR_CLIPPED
                                              : requires3D;
                const bool aligned = pass ? pass->tileProjection == MLN_PLUGIN_TILE_PROJECTION_ALIGNED : true;
                tileMatrix = getTileMatrix(*tileID,
                                           parameters,
                                           {0.0f, 0.0f},
                                           style::TranslateAnchorType::Viewport,
                                           nearClipped,
                                           false,
                                           drawable,
                                           aligned);
            }
            mat4 targetMatrix;
            matrix::ortho(targetMatrix, 0, util::EXTENT, -util::EXTENT, 0, -1, 1);
            matrix::translate(targetMatrix, targetMatrix, 0, -util::EXTENT, 0);
            const auto latitudeRange = hasTile ? getLatRange(*tileID) : std::array<float, 2>{0.0f, 0.0f};
            mat4 viewportMatrix;
            matrix::ortho(viewportMatrix, 0, 1, 1, 0, -1, 1);

            mln_plugin_uniform_context_v1 callbackContext{};
            callbackContext.struct_size = sizeof(callbackContext);
            callbackContext.pass_id = pass ? pass->id : 0;
            callbackContext.canonical_z = hasTile ? tileID->canonical.z : 0;
            callbackContext.canonical_x = hasTile ? tileID->canonical.x : 0;
            callbackContext.canonical_y = hasTile ? tileID->canonical.y : 0;
            callbackContext.wrap = hasTile ? tileID->wrap : 0;
            callbackContext.overscaled_z = hasTile ? drawable.getTileID()->overscaledZ : 0;
            callbackContext.source_max_zoom = data.sourceMaxZoom;
            callbackContext.dem_dimension = data.dimension;
            callbackContext.dem_stride = data.stride;
            callbackContext.dem_encoding = data.encoding;
            callbackContext.zoom = parameters.state.getZoom();
            callbackContext.bearing = parameters.state.getBearing();
            callbackContext.pixels_to_gl_units[0] = parameters.pixelsToGLUnits[0];
            callbackContext.pixels_to_gl_units[1] = parameters.pixelsToGLUnits[1];
            const auto tileMatrixFloats = util::cast<float>(tileMatrix);
            const auto targetMatrixFloats = util::cast<float>(targetMatrix);
            std::copy(tileMatrixFloats.begin(), tileMatrixFloats.end(), callbackContext.tile_matrix);
            std::copy(targetMatrixFloats.begin(), targetMatrixFloats.end(), callbackContext.render_target_matrix);
            std::copy(latitudeRange.begin(), latitudeRange.end(), callbackContext.latitude_range);
            const auto viewportSize = parameters.state.getSize();
            callbackContext.viewport_width = viewportSize.width;
            callbackContext.viewport_height = viewportSize.height;
            callbackContext.render_target_width = data.renderTargetWidth;
            callbackContext.render_target_height = data.renderTargetHeight;
            callbackContext.pixels_to_tile_units = hasTile ? tileID->pixelsToTileUnits(
                                                                 1.0f, static_cast<float>(parameters.state.getZoom()))
                                                           : 0.0f;
            const auto viewportMatrixFloats = util::cast<float>(viewportMatrix);
            std::copy(viewportMatrixFloats.begin(), viewportMatrixFloats.end(), callbackContext.viewport_matrix);
            callbackContext.properties = propertyValues.data();
            callbackContext.property_count = propertyValues.size();

            for (const auto& uniform : shader->uniformBlocks) {
                if (!registration.updateUniformBlock) continue;
                std::vector<uint8_t> bytes(uniform.byteSize);
                const auto status = registration.updateUniformBlock(
                    &callbackContext, uniform.id, bytes.data(), bytes.size());
                if (status != MLN_PLUGIN_STATUS_OK) {
                    Log::Error(Event::General,
                               "Plugin '" + registration.pluginID + "' failed to update uniform " +
                                   std::to_string(uniform.id) + " for pass " + std::to_string(callbackContext.pass_id) +
                                   " (status " + std::to_string(static_cast<int>(status)) + ")");
                    continue;
                }
                if (const auto* baseBinders = drawable.getBinders()) {
                    const auto* binders = static_cast<const PluginPaintPropertyBinders*>(baseBinders);
                    binders->writeUniforms(static_cast<float>(parameters.state.getZoom()),
                                           uniform.id,
                                           bytes.data(),
                                           bytes.size());
                }
                if (uniform.scope == MLN_PLUGIN_UNIFORM_SCOPE_LAYER) {
                    auto& buffer = layerUniformBuffers[{callbackContext.pass_id, uniform.id}];
                    parameters.context.emplaceOrUpdateUniformBuffer(
                        buffer, static_cast<const void*>(bytes.data()), bytes.size());
                    layerGroup.mutableUniformBuffers().set(uniform.bindingID, buffer);
                } else {
                    drawable.mutableUniformBuffers().createOrUpdate(
                        uniform.bindingID, bytes.data(), bytes.size(), parameters.context);
                }
            }
        });
        propertiesUpdated = false;
        return;
    }
}

} // namespace mln
