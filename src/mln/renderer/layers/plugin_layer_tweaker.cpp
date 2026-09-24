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
#include <limits>

namespace mln {

void PluginLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) return;

    const auto& rendering = static_cast<const style::PluginStyleLayerProperties&>(*evaluatedProperties).rendering;
    const auto drawableCount = layerGroup.getDrawableCount();
    if (drawableCount > std::numeric_limits<uint32_t>::max()) return;
    for (auto& [key, uniform] : sharedUniforms) {
        uniform.drawables.clear();
        uniform.failed = false;
    }
    uint32_t drawableIndex = 0;
    {
        visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
            if (!drawable.getData() || !checkTweakDrawable(drawable)) return;
            auto& data = static_cast<plugin::DrawableData&>(*drawable.getData());
            drawable.setEnabled((rendering.enabled_passes & (uint32_t{1} << data.passIndex)) != 0);
            if (!drawable.getEnabled()) return;
            data.uniformFailed = false;
            const auto* shader = [&]() -> const plugin::ShaderDefinition* {
                const auto it = std::find_if(registration->shaders.begin(),
                                             registration->shaders.end(),
                                             [&](const auto& candidate) { return candidate.id == data.shaderID; });
                return it == registration->shaders.end() ? nullptr : &*it;
            }();
            if (!shader) return;
            const auto index = drawableIndex++;
            drawable.setUBOIndex(index);

            const bool hasTile = drawable.getTileID().has_value();
            const std::optional<UnwrappedTileID> tileID =
                hasTile ? std::optional<UnwrappedTileID>{drawable.getTileID()->toUnwrapped()} : std::nullopt;
            mat4 tileMatrix = matrix::identity4();
            if (hasTile) {
                tileMatrix = getTileMatrix(*tileID,
                                           parameters,
                                           {rendering.translation.x, rendering.translation.y},
                                           rendering.translation_anchor_viewport ? style::TranslateAnchorType::Viewport : style::TranslateAnchorType::Map,
                                           registration->enableNearClippedMatrix,
                                           false,
                                           drawable,
                                           !registration->enableNearClippedMatrix);
            }
            mln_plugin_uniform_context_v1 callbackContext{};
            callbackContext.struct_size = sizeof(callbackContext);
            callbackContext.bearing = parameters.state.getBearing();
            callbackContext.camera_to_center_distance = parameters.state.getCameraToCenterDistance();
            callbackContext.pixel_ratio = parameters.pixelRatio;
            const auto lightColor = parameters.evaluatedLight.get<style::LightColor>();
            callbackContext.light_color[0] = lightColor.r;
            callbackContext.light_color[1] = lightColor.g;
            callbackContext.light_color[2] = lightColor.b;
            callbackContext.light_intensity = parameters.evaluatedLight.get<style::LightIntensity>();
            auto direction = parameters.evaluatedLight.get<style::LightPosition>().getCartesian();
            mat3 lightMatrix;
            matrix::identity(lightMatrix);
            if (parameters.evaluatedLight.get<style::LightAnchor>() == style::LightAnchorType::Viewport)
                matrix::rotate(lightMatrix, lightMatrix, -parameters.state.getBearing());
            matrix::transformMat3f(direction, direction, lightMatrix);
            std::copy(direction.begin(), direction.end(), callbackContext.light_direction);
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
                if (!registration->updateUniformBlock) continue;
                const bool layerScope = uniform.scope == MLN_PLUGIN_UNIFORM_LAYER;
                bool arrayScope = false;
#if MLN_RENDER_BACKEND_METAL
                arrayScope = uniform.scope == MLN_PLUGIN_UNIFORM_DRAWABLE_ARRAY;
#endif
                SharedUniform* shared = nullptr;
                plugin::DrawableData::UniformData* individual = nullptr;
                uint8_t* output = nullptr;
                auto context = callbackContext;
                if (layerScope || arrayScope) {
                    shared = &sharedUniforms[{shader->id, uniform.id}];
                    const bool first = shared->drawables.empty();
                    shared->drawables.push_back(&drawable);
                    shared->bindingID = uniform.bindingID;
                    if (layerScope && !first) continue;
                    if (first) {
                        const auto count = arrayScope ? drawableCount : 1;
                        if (count > shared->scratch.max_size() / uniform.byteSize) {
                            shared->failed = true;
                            continue;
                        }
                        shared->scratch.assign(count * uniform.byteSize, 0);
                    }
                    if (shared->failed) continue;
                    output = shared->scratch.data() + (arrayScope ? std::size_t(index) * uniform.byteSize : 0);
                    if (layerScope) {
                        const auto identity = util::cast<float>(matrix::identity4());
                        std::copy(identity.begin(), identity.end(), context.tile_matrix);
                        context.pixels_to_tile_units = 0;
                    }
                } else {
                    individual = &data.uniforms[uniform.id];
                    individual->scratch.assign(uniform.byteSize, 0);
                    output = individual->scratch.data();
                }
                const auto status = registration->updateUniformBlock(&context, uniform.id, output, uniform.byteSize);
                if (status != MLN_PLUGIN_STATUS_OK) {
                    Log::Error(Event::General,
                               "Plugin '" + registration->pluginID + "' failed to update uniform " +
                                   std::to_string(uniform.id) + " (status " + std::to_string(static_cast<int>(status)) +
                                   ")");
                    if (shared) shared->failed = true;
                    drawable.setEnabled(false);
                    data.uniformFailed = true;
                    continue;
                }
                if (const auto* baseBinders = drawable.getBinders()) {
                    const auto* binders = static_cast<const PluginPaintPropertyBinders*>(baseBinders);
                    binders->writeUniforms(
                        static_cast<float>(parameters.state.getZoom()), uniform.id, output, uniform.byteSize);
                }
                // Callbacks still execute on every frame, including stateful
                // callbacks. Only identical GPU uploads can be elided safely.
                if (individual) {
                    auto& buffers = drawable.mutableUniformBuffers();
                    if (!buffers.get(uniform.bindingID) || individual->scratch != individual->uploaded) {
                        buffers.createOrUpdate(uniform.bindingID, output, uniform.byteSize, parameters.context);
                        individual->uploaded = individual->scratch;
                    }
                }
            }
        });
        for (auto& [key, uniform] : sharedUniforms) {
            if (!uniform.drawables.empty() && !uniform.failed) {
                const auto& bytes = uniform.scratch;
                if (!uniform.buffer || uniform.buffer->getSize() != bytes.size()) {
                    uniform.buffer = parameters.context.createUniformBuffer(bytes.data(), bytes.size());
                    uniform.uploaded = bytes;
                } else if (bytes != uniform.uploaded) {
                    uniform.buffer->update(bytes.data(), bytes.size());
                    uniform.uploaded = bytes;
                }
                // Keep shader-local binding slots local to each drawable. The
                // backend deduplicates binding the shared resource, even when
                // different shader variants are interleaved in a layer group.
                for (auto* drawable : uniform.drawables) {
                    auto& buffers = drawable->mutableUniformBuffers();
                    if (buffers.get(uniform.bindingID) != uniform.buffer) {
                        buffers.set(uniform.bindingID, uniform.buffer);
                    }
                }
            } else if (uniform.failed) {
                // Never draw with an incomplete array or an old, smaller one
                // after tile churn. Retry callbacks on the next frame.
                for (auto* drawable : uniform.drawables) {
                    drawable->setEnabled(false);
                    static_cast<plugin::DrawableData&>(*drawable->getData()).uniformFailed = true;
                }
            }
            uniform.drawables.clear();
        }
        propertiesUpdated = false;
        return;
    }
}

} // namespace mln
