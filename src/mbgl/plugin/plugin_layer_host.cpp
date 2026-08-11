#include <mbgl/plugin/plugin_layer_host.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/renderer/renderer_observer.hpp>
#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/style/layers/custom_layer_render_parameters.hpp>
#include <mbgl/util/async_request.hpp>
#include <mbgl/util/logging.hpp>

#if MLN_RENDER_BACKEND_VULKAN
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#endif

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/renderer_backend.hpp>

#include <Metal/Metal.hpp>
#endif

#include <algorithm>
#include <cstring>
#include <map>
#include <utility>

namespace mbgl {
namespace plugin {
namespace {

void hostLog(int32_t severity, mln_plugin_string message) {
    const std::string text = message.data ? std::string(message.data, message.size) : std::string{};
    if (severity >= 3) {
        Log::Error(Event::Render, text);
    } else if (severity == 2) {
        Log::Warning(Event::Render, text);
    } else {
        Log::Info(Event::Render, text);
    }
}

mln_plugin_render_stage stageFor(RenderPass pass) {
    switch (pass) {
        case RenderPass::Pass3D:
            return MLN_PLUGIN_RENDER_STAGE_PASS_3D;
        case RenderPass::Opaque:
            return MLN_PLUGIN_RENDER_STAGE_OPAQUE;
        case RenderPass::Translucent:
            return MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT;
        default:
            return MLN_PLUGIN_RENDER_STAGE_PREPARE;
    }
}

double numericValue(const Value& value) {
    if (const auto* number = value.getDouble()) return *number;
    if (const auto* number = value.getInt()) return static_cast<double>(*number);
    if (const auto* number = value.getUint()) return static_cast<double>(*number);
    return 0.0;
}

mln_plugin_value makeCValue(const Value& value, mln_plugin_value_type type, std::vector<std::string>& stringStorage) {
    mln_plugin_value result{};
    result.struct_size = sizeof(result);
    result.type = type;
    switch (type) {
        case MLN_PLUGIN_VALUE_BOOLEAN:
            result.data.boolean_value = value.getBool() && *value.getBool() ? 1 : 0;
            break;
        case MLN_PLUGIN_VALUE_FLOAT:
            result.data.float_value = static_cast<float>(numericValue(value));
            break;
        case MLN_PLUGIN_VALUE_FLOAT2: {
            const auto& array = *value.getArray();
            result.data.float2_value = {static_cast<float>(numericValue(array[0])),
                                        static_cast<float>(numericValue(array[1]))};
            break;
        }
        case MLN_PLUGIN_VALUE_COLOR: {
            const auto& array = *value.getArray();
            result.data.color_value = {static_cast<float>(numericValue(array[0])),
                                       static_cast<float>(numericValue(array[1])),
                                       static_cast<float>(numericValue(array[2])),
                                       static_cast<float>(numericValue(array[3]))};
            break;
        }
        case MLN_PLUGIN_VALUE_STRING:
            stringStorage.push_back(*value.getString());
            result.data.string_value = {stringStorage.back().data(), stringStorage.back().size()};
            break;
    }
    return result;
}

#if MLN_RENDER_BACKEND_VULKAN
mln_plugin_proc resolveVulkanProc(void* context, const char* name) {
    if (!context || !name) return nullptr;
    auto& backend = *static_cast<vulkan::RendererBackend*>(context);
    const auto& dispatcher = backend.getDispatcher();
    if (const auto proc = dispatcher.vkGetDeviceProcAddr(static_cast<VkDevice>(backend.getDevice().get()), name)) {
        return reinterpret_cast<mln_plugin_proc>(proc);
    }
    return reinterpret_cast<mln_plugin_proc>(
        dispatcher.vkGetInstanceProcAddr(static_cast<VkInstance>(backend.getInstance().get()), name));
}
#endif

} // namespace

struct PluginLayerHost::Instance {
    explicit Instance(LayerExtension extension_)
        : extension(std::move(extension_)) {
        hostAPI.struct_size = sizeof(hostAPI);
        hostAPI.abi_version = MLN_PLUGIN_ABI_VERSION_1;
        hostAPI.log = hostLog;
        hostAPI.context = this;
        hostAPI.request_resource = requestResource;
        hostAPI.cancel_resource_request = cancelResourceRequest;
        hostAPI.request_repaint = requestRepaint;
    }

    static mln_plugin_status requestResource(void* context,
                                             mln_plugin_string url,
                                             mln_plugin_resource_callback_fn callback,
                                             void* callbackContext,
                                             uint64_t* requestID) {
        auto* self = static_cast<Instance*>(context);
        if (!self || !self->fileSource || !url.data || url.size == 0 || !callback || !requestID) {
            return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        }
        const uint64_t id = self->nextRequestID++;
        *requestID = id;
        const std::string resourceURL(url.data, url.size);
        auto request = self->fileSource->request(
            Resource(Resource::Kind::Unknown, resourceURL), [self, id, callback, callbackContext](Response response) {
                std::string error;
                if (response.error) error = response.error->message;
                mln_plugin_resource_response_v1 result{};
                result.struct_size = sizeof(result);
                result.request_id = id;
                if (response.data) {
                    result.data = reinterpret_cast<const uint8_t*>(response.data->data());
                    result.data_size = response.data->size();
                }
                result.error_message = {error.data(), error.size()};
                callback(callbackContext, &result);
                requestRepaint(self);
            });
        if (!request) return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
        self->requests.emplace(id, std::move(request));
        return MLN_PLUGIN_STATUS_OK;
    }

    static void cancelResourceRequest(void* context, uint64_t requestID) {
        if (auto* self = static_cast<Instance*>(context)) {
            self->requests.erase(requestID);
        }
    }

    static void requestRepaint(void* context) {
        if (auto* self = static_cast<Instance*>(context); self && self->observer) {
            self->observer->onInvalidate();
        }
    }

    LayerExtension extension;
    mln_plugin_host_api_v1 hostAPI{};
    void* handle = nullptr;
    bool created = false;
    bool disabled = false;
    std::shared_ptr<FileSource> fileSource;
    RendererObserver* observer = nullptr;
    uint64_t nextRequestID = 1;
    std::map<uint64_t, std::unique_ptr<AsyncRequest>> requests;
};

struct PluginLayerHost::PropertySnapshot {
    std::vector<std::string> names;
    std::vector<std::string> stringValues;
    std::vector<mln_plugin_property_value_v1> values;
};

PluginLayerHost::PluginLayerHost(std::string layerID_, std::string layerType_, Immutable<style::Layer::Impl> layerImpl_)
    : layerID(std::move(layerID_)),
      layerType(std::move(layerType_)),
      layerImpl(std::move(layerImpl_)) {
    for (auto& extension : PluginRegistry::get().extensionsForLayer(layerType)) {
        instances.push_back(std::make_unique<Instance>(std::move(extension)));
    }
    if (const auto registeredType = PluginRegistry::get().findLayerType(layerType)) {
        instances.push_back(std::make_unique<Instance>(LayerExtension{registeredType->pluginID,
                                                                      registeredType->pluginVersion,
                                                                      registeredType->type,
                                                                      0,
                                                                      registeredType->backendMask,
                                                                      registeredType->createInstance,
                                                                      registeredType->destroyInstance,
                                                                      registeredType->prepareFrame,
                                                                      registeredType->renderLayer,
                                                                      registeredType->contextLost}));
    }
}

PluginLayerHost::~PluginLayerHost() {
    for (auto& instance : instances) {
        if (instance->created && instance->extension.destroyInstance) {
            instance->extension.destroyInstance(instance->handle);
        }
    }
}

bool PluginLayerHost::empty() const noexcept {
    return instances.empty();
}

void PluginLayerHost::updateLayer(Immutable<style::Layer::Impl> impl) {
    layerImpl = std::move(impl);
}

void PluginLayerHost::updateEnvironment(std::shared_ptr<FileSource> fileSource, RendererObserver* observer) {
    for (auto& instance : instances) {
        instance->fileSource = fileSource;
        instance->observer = observer;
    }
}

PluginLayerHost::PropertySnapshot PluginLayerHost::makePropertySnapshot(const Instance& instance) const {
    PropertySnapshot snapshot;
    const auto definitions = PluginRegistry::get().propertiesForLayer(layerType);
    const auto count = std::count_if(definitions.begin(), definitions.end(), [&](const auto& definition) {
        return definition.pluginID == instance.extension.pluginID;
    });
    snapshot.names.reserve(count);
    snapshot.stringValues.reserve(count);
    snapshot.values.reserve(count);

    for (const auto& definition : definitions) {
        if (definition.pluginID != instance.extension.pluginID) continue;
        const auto explicitValue = layerImpl->pluginProperties.find(definition.name);
        const bool explicitlySet = explicitValue != layerImpl->pluginProperties.end();
        const auto& value = explicitlySet ? explicitValue->second : definition.defaultValue;
        snapshot.names.push_back(definition.name);
        snapshot.values.push_back(
            mln_plugin_property_value_v1{sizeof(mln_plugin_property_value_v1),
                                         {snapshot.names.back().data(), snapshot.names.back().size()},
                                         makeCValue(value, definition.type, snapshot.stringValues),
                                         static_cast<uint8_t>(explicitlySet ? 1 : 0)});
    }
    return snapshot;
}

void PluginLayerHost::prepareFrame(PaintParameters& parameters, const std::vector<mln_plugin_draw_packet_v1>& packets) {
    for (auto& instance : instances) {
        if (!instance->disabled && instance->extension.prepareFrame) {
            invoke(*instance, parameters, true, packets);
        }
    }
}

void PluginLayerHost::renderBeforeLayer(PaintParameters& parameters,
                                        const std::vector<mln_plugin_draw_packet_v1>& packets) {
    for (auto& instance : instances) {
        if (!instance->disabled && instance->extension.renderBeforeLayer) {
            invoke(*instance, parameters, false, packets);
        }
    }
}

void PluginLayerHost::contextLost() {
    for (auto& instance : instances) {
        if (instance->created && !instance->disabled && instance->extension.contextLost) {
            instance->extension.contextLost(instance->handle);
        }
    }
}

void PluginLayerHost::invoke(Instance& instance,
                             PaintParameters& parameters,
                             bool prepare,
                             const std::vector<mln_plugin_draw_packet_v1>& packets) {
    const uint32_t activeBackend = [] {
        switch (gfx::Backend::GetType()) {
            case gfx::Backend::Type::OpenGL:
                return static_cast<uint32_t>(MLN_PLUGIN_BACKEND_OPENGL);
            case gfx::Backend::Type::Vulkan:
                return static_cast<uint32_t>(MLN_PLUGIN_BACKEND_VULKAN);
            case gfx::Backend::Type::Metal:
                return static_cast<uint32_t>(MLN_PLUGIN_BACKEND_METAL);
            default:
                return uint32_t{0};
        }
    }();
    if ((instance.extension.backendMask & activeBackend) == 0) return;

    if (!instance.created) {
        if (!instance.extension.createInstance) {
            disable(instance, "create_instance", MLN_PLUGIN_STATUS_INVALID_ARGUMENT);
            return;
        }
        const mln_plugin_string id{layerID.data(), layerID.size()};
        const auto status = instance.extension.createInstance(&instance.hostAPI, id, &instance.handle);
        if (status != MLN_PLUGIN_STATUS_OK || !instance.handle) {
            disable(instance, "create_instance", status);
            return;
        }
        instance.created = true;
    }

    const auto size = parameters.backend.getDefaultRenderable().getSize();
    auto properties = makePropertySnapshot(instance);
    mln_plugin_backend_context_v1 backend{};
    backend.struct_size = sizeof(backend);
    backend.backend = static_cast<mln_plugin_backend>(activeBackend);

#if MLN_RENDER_BACKEND_METAL
    mln_plugin_metal_context_v1 metal{};
    if (activeBackend == MLN_PLUGIN_BACKEND_METAL) {
        metal.struct_size = sizeof(metal);
        auto& context = static_cast<mtl::Context&>(parameters.context);
        const auto& rendererBackend = context.getBackend();
        metal.device = rendererBackend.getDevice().get();
        metal.command_queue = rendererBackend.getCommandQueue().get();
        metal.sample_count = 1;

        auto& resource = parameters.backend.getDefaultRenderable().getResource<mtl::RenderableResource>();
        metal.command_buffer = resource.getCommandBuffer().get();
        if (const auto& descriptor = resource.getRenderPassDescriptor()) {
            if (const auto* attachment = descriptor->colorAttachments()->object(0)) {
                if (const auto* texture = attachment->texture()) {
                    metal.color_pixel_format = static_cast<uint32_t>(texture->pixelFormat());
                    metal.sample_count = static_cast<uint32_t>(texture->sampleCount());
                }
            }
            if (const auto* attachment = descriptor->depthAttachment()) {
                if (const auto* texture = attachment->texture()) {
                    metal.depth_pixel_format = static_cast<uint32_t>(texture->pixelFormat());
                }
            }
            if (const auto* attachment = descriptor->stencilAttachment()) {
                if (const auto* texture = attachment->texture()) {
                    metal.stencil_pixel_format = static_cast<uint32_t>(texture->pixelFormat());
                }
            }
        }
        if (parameters.renderPass) {
            auto& renderPass = static_cast<mtl::RenderPass&>(*parameters.renderPass);
            metal.render_command_encoder = renderPass.getMetalEncoder().get();
        }
        backend.metal = &metal;
    }
#endif

#if MLN_RENDER_BACKEND_VULKAN
    if (activeBackend == MLN_PLUGIN_BACKEND_VULKAN) {
        auto& context = static_cast<vulkan::Context&>(parameters.context);
        auto& rendererBackend = context.getBackend();
        backend.instance = reinterpret_cast<uint64_t>(static_cast<VkInstance>(rendererBackend.getInstance().get()));
        backend.physical_device = reinterpret_cast<uint64_t>(
            static_cast<VkPhysicalDevice>(rendererBackend.getPhysicalDevice()));
        backend.device = reinterpret_cast<uint64_t>(static_cast<VkDevice>(rendererBackend.getDevice().get()));
        backend.graphics_queue = reinterpret_cast<uint64_t>(static_cast<VkQueue>(rendererBackend.getGraphicsQueue()));
        backend.graphics_queue_family = static_cast<uint32_t>(rendererBackend.getGraphicsQueueIndex());
        backend.resolver_context = &rendererBackend;
        backend.get_proc_address = resolveVulkanProc;
        if (parameters.renderPass) {
            auto& renderPass = static_cast<vulkan::RenderPass&>(*parameters.renderPass);
            auto& encoder = renderPass.getEncoder();
            auto& resource = renderPass.getDescriptor().renderable.getResource<vulkan::RenderableResource>();
            backend.command_buffer = reinterpret_cast<uint64_t>(
                static_cast<VkCommandBuffer>(encoder.getCommandBuffer().get()));
            backend.render_pass = reinterpret_cast<uint64_t>(static_cast<VkRenderPass>(resource.getRenderPass().get()));
            backend.framebuffer = reinterpret_cast<uint64_t>(
                static_cast<VkFramebuffer>(resource.getFramebuffer().get()));
            backend.screen_pre_rotation_radians_clockwise = resource.getRotation();
        } else {
            backend.command_buffer = reinterpret_cast<uint64_t>(
                static_cast<VkCommandBuffer>(context.getCommandBuffer().get()));
        }
    }
#endif

    const style::CustomLayerRenderParameters cameraParameters(parameters);
    mln_plugin_camera_context_v1 camera{};
    camera.struct_size = sizeof(camera);
    camera.latitude = cameraParameters.latitude;
    camera.longitude = cameraParameters.longitude;
    camera.zoom = cameraParameters.zoom;
    camera.bearing = cameraParameters.bearing;
    camera.pitch = cameraParameters.pitch;
    camera.field_of_view = cameraParameters.fieldOfView;
    camera.pixel_ratio = parameters.pixelRatio;
    std::copy(
        cameraParameters.projectionMatrix.begin(), cameraParameters.projectionMatrix.end(), camera.projection_matrix);
    std::copy(cameraParameters.nearClippedProjectionMatrix.begin(),
              cameraParameters.nearClippedProjectionMatrix.end(),
              camera.near_clipped_projection_matrix);

    mln_plugin_frame_context_v1 frame{};
    frame.struct_size = sizeof(frame);
    frame.stage = prepare ? MLN_PLUGIN_RENDER_STAGE_PREPARE : stageFor(parameters.pass);
    frame.width = size.width;
    frame.height = size.height;
    frame.frame_number = parameters.frameCount;
    frame.backend = &backend;
    frame.properties = properties.values.data();
    frame.property_count = properties.values.size();
    frame.fill_extrusion_packets = packets.data();
    frame.fill_extrusion_packet_count = packets.size();
    frame.camera = &camera;

    const auto callback = prepare ? instance.extension.prepareFrame : instance.extension.renderBeforeLayer;
    const auto status = callback(instance.handle, &frame);
    // Plugins may bind arbitrary backend state. Restore MapLibre's cached
    // state and global descriptors before the next ordinary drawable.
    parameters.context.setDirtyState();
#if MLN_RENDER_BACKEND_METAL
    if (parameters.renderPass) {
        static_cast<mtl::RenderPass&>(*parameters.renderPass).resetState();
    }
#endif
    if (parameters.renderPass) {
        parameters.context.bindGlobalUniformBuffers(*parameters.renderPass);
    }
    if (status != MLN_PLUGIN_STATUS_OK) {
        disable(instance, prepare ? "prepare_frame" : "render_before_layer", status);
    }
}

void PluginLayerHost::disable(Instance& instance, const char* callbackName, mln_plugin_status status) {
    if (instance.disabled) return;
    Log::Error(Event::Render,
               "Plugin '" + instance.extension.pluginID + "' callback " + callbackName + " failed for layer '" +
                   layerID + "' with status " + std::to_string(status) + "; disabling this instance");
    if (instance.created && instance.extension.destroyInstance) {
        instance.extension.destroyInstance(instance.handle);
    }
    instance.handle = nullptr;
    instance.created = false;
    instance.disabled = true;
}

} // namespace plugin
} // namespace mbgl
