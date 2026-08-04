#include <mbgl/plugin/plugin_registry.hpp>

#include <algorithm>
#include <cstring>
#include <set>

namespace mbgl {
namespace plugin {
namespace {

std::string copyString(const mln_plugin_string& value) {
    return value.data && value.size ? std::string(value.data, value.size) : std::string{};
}

Value copyValue(const mln_plugin_value& value) {
    switch (value.type) {
        case MLN_PLUGIN_VALUE_BOOLEAN:
            return Value{value.data.boolean_value != 0};
        case MLN_PLUGIN_VALUE_FLOAT:
            return Value{static_cast<double>(value.data.float_value)};
        case MLN_PLUGIN_VALUE_FLOAT2:
            return Value{mapbox::base::ValueArray{static_cast<double>(value.data.float2_value.x),
                                                  static_cast<double>(value.data.float2_value.y)}};
        case MLN_PLUGIN_VALUE_COLOR:
            return Value{mapbox::base::ValueArray{static_cast<double>(value.data.color_value.r),
                                                  static_cast<double>(value.data.color_value.g),
                                                  static_cast<double>(value.data.color_value.b),
                                                  static_cast<double>(value.data.color_value.a)}};
        case MLN_PLUGIN_VALUE_STRING:
            return Value{copyString(value.data.string_value)};
    }
    return {};
}

bool validString(const mln_plugin_string& value) {
    return value.data && value.size > 0;
}

bool descriptorEquals(const PluginRegistry::PluginRecord& existing,
                      const std::string& version,
                      const std::vector<PropertyDefinition>& properties,
                      const std::vector<LayerExtension>& extensions) {
    if (existing.version != version || existing.properties.size() != properties.size() ||
        existing.extensions.size() != extensions.size()) {
        return false;
    }
    for (size_t i = 0; i < properties.size(); ++i) {
        const auto& lhs = existing.properties[i];
        const auto& rhs = properties[i];
        if (lhs.targetLayerType != rhs.targetLayerType || lhs.name != rhs.name || lhs.type != rhs.type ||
            lhs.scope != rhs.scope || lhs.defaultValue != rhs.defaultValue) {
            return false;
        }
    }
    for (size_t i = 0; i < extensions.size(); ++i) {
        const auto& lhs = existing.extensions[i];
        const auto& rhs = extensions[i];
        if (lhs.targetLayerType != rhs.targetLayerType || lhs.renderPriority != rhs.renderPriority ||
            lhs.backendMask != rhs.backendMask || lhs.createInstance != rhs.createInstance ||
            lhs.destroyInstance != rhs.destroyInstance || lhs.prepareFrame != rhs.prepareFrame ||
            lhs.renderBeforeLayer != rhs.renderBeforeLayer || lhs.contextLost != rhs.contextLost) {
            return false;
        }
    }
    return true;
}

} // namespace

PluginRegistry& PluginRegistry::get() {
    static PluginRegistry registry;
    return registry;
}

mln_plugin_status PluginRegistry::registerPlugin(const mln_plugin_descriptor_v1& descriptor, std::string& error) {
    if (descriptor.struct_size < sizeof(mln_plugin_descriptor_v1) ||
        descriptor.abi_version != MLN_PLUGIN_ABI_VERSION_1 || descriptor.minimum_host_abi > MLN_PLUGIN_ABI_VERSION_1 ||
        descriptor.maximum_host_abi < MLN_PLUGIN_ABI_VERSION_1) {
        error = "plugin ABI is not compatible with host ABI 1";
        return MLN_PLUGIN_STATUS_UNSUPPORTED_ABI;
    }
    if (!validString(descriptor.plugin_id) || !validString(descriptor.plugin_version) || !descriptor.layer_extensions ||
        descriptor.layer_extension_count == 0) {
        error = "plugin descriptor is missing an id, version, or layer extension";
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }

    const auto pluginID = copyString(descriptor.plugin_id);
    const auto pluginVersion = copyString(descriptor.plugin_version);
    std::vector<PropertyDefinition> newProperties;
    std::vector<LayerExtension> newExtensions;
    std::set<std::pair<std::string, int32_t>> extensionKeys;
    std::set<std::pair<std::string, std::string>> propertyKeys;

    for (size_t i = 0; i < descriptor.layer_extension_count; ++i) {
        const auto& extension = descriptor.layer_extensions[i];
        if (extension.struct_size < sizeof(mln_plugin_layer_extension_v1) ||
            !validString(extension.target_layer_type) || (extension.property_count && !extension.properties) ||
            !extension.create_instance || !extension.destroy_instance ||
            (!extension.prepare_frame && !extension.render_before_layer) ||
            (extension.backend_mask &
             ~(MLN_PLUGIN_BACKEND_OPENGL | MLN_PLUGIN_BACKEND_VULKAN | MLN_PLUGIN_BACKEND_METAL)) != 0 ||
            (extension.backend_mask &
             (MLN_PLUGIN_BACKEND_OPENGL | MLN_PLUGIN_BACKEND_VULKAN | MLN_PLUGIN_BACKEND_METAL)) == 0) {
            error = "layer extension is malformed or has no supported backend";
            return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        }
        const auto targetLayerType = copyString(extension.target_layer_type);
        if (!extensionKeys.emplace(targetLayerType, extension.render_priority).second) {
            error = "plugin descriptor contains duplicate render extensions";
            return MLN_PLUGIN_STATUS_CONFLICT;
        }
        newExtensions.push_back(LayerExtension{pluginID,
                                               pluginVersion,
                                               targetLayerType,
                                               extension.render_priority,
                                               extension.backend_mask,
                                               extension.create_instance,
                                               extension.destroy_instance,
                                               extension.prepare_frame,
                                               extension.render_before_layer,
                                               extension.context_lost});

        for (size_t p = 0; p < extension.property_count; ++p) {
            const auto& property = extension.properties[p];
            if (property.struct_size < sizeof(mln_plugin_property_descriptor_v1) || !validString(property.name) ||
                property.default_value.struct_size < sizeof(mln_plugin_value) ||
                property.default_value.type != property.type || property.scope != MLN_PLUGIN_PROPERTY_PAINT) {
                error = "plugin property descriptor is malformed";
                return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
            }
            auto defaultValue = copyValue(property.default_value);
            if (!valueMatches(property.type, defaultValue)) {
                error = "plugin property default value has the wrong type";
                return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
            }
            const auto propertyName = copyString(property.name);
            if (!propertyKeys.emplace(targetLayerType, propertyName).second) {
                error = "plugin descriptor contains duplicate property names";
                return MLN_PLUGIN_STATUS_CONFLICT;
            }
            newProperties.push_back(PropertyDefinition{
                pluginID, targetLayerType, propertyName, property.type, property.scope, std::move(defaultValue)});
        }
    }

    std::lock_guard<std::mutex> lock(mutex);
    if (const auto existing = plugins.find(pluginID); existing != plugins.end()) {
        if (descriptorEquals(existing->second, pluginVersion, newProperties, newExtensions)) {
            return MLN_PLUGIN_STATUS_ALREADY_REGISTERED;
        }
        error = "plugin id is already registered with a different descriptor";
        return MLN_PLUGIN_STATUS_CONFLICT;
    }

    for (const auto& property : newProperties) {
        const auto key = std::make_pair(property.targetLayerType, property.name);
        if (properties.find(key) != properties.end()) {
            error = "property '" + property.name + "' is already registered for layer type '" +
                    property.targetLayerType + "'";
            return MLN_PLUGIN_STATUS_CONFLICT;
        }
    }

    PluginRecord record{pluginVersion, newProperties, newExtensions};
    for (const auto& property : newProperties) {
        properties.emplace(std::make_pair(property.targetLayerType, property.name), property);
    }
    plugins.emplace(pluginID, std::move(record));
    return MLN_PLUGIN_STATUS_OK;
}

std::optional<PropertyDefinition> PluginRegistry::findProperty(const std::string& layerType,
                                                               const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex);
    const auto it = properties.find(std::make_pair(layerType, name));
    return it == properties.end() ? std::nullopt : std::optional<PropertyDefinition>{it->second};
}

std::vector<PropertyDefinition> PluginRegistry::propertiesForLayer(const std::string& layerType) const {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<PropertyDefinition> result;
    for (const auto& [key, property] : properties) {
        if (key.first == layerType) {
            result.push_back(property);
        }
    }
    return result;
}

std::vector<LayerExtension> PluginRegistry::extensionsForLayer(const std::string& layerType) const {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<LayerExtension> result;
    for (const auto& [id, plugin] : plugins) {
        (void)id;
        for (const auto& extension : plugin.extensions) {
            if (extension.targetLayerType == layerType) result.push_back(extension);
        }
    }
    std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.renderPriority == rhs.renderPriority ? lhs.pluginID < rhs.pluginID
                                                        : lhs.renderPriority < rhs.renderPriority;
    });
    return result;
}

bool PluginRegistry::isRegistered(const std::string& pluginID) const {
    std::lock_guard<std::mutex> lock(mutex);
    return plugins.find(pluginID) != plugins.end();
}

std::vector<std::string> PluginRegistry::pluginIDs() const {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<std::string> result;
    result.reserve(plugins.size());
    for (const auto& [id, plugin] : plugins) {
        (void)plugin;
        result.push_back(id);
    }
    return result;
}

bool PluginRegistry::valueMatches(mln_plugin_value_type type, const Value& value) {
    const auto numeric = [&] {
        return value.getDouble() || value.getInt() || value.getUint();
    };
    switch (type) {
        case MLN_PLUGIN_VALUE_BOOLEAN:
            return value.getBool() != nullptr;
        case MLN_PLUGIN_VALUE_FLOAT:
            return numeric();
        case MLN_PLUGIN_VALUE_STRING:
            return value.getString() != nullptr;
        case MLN_PLUGIN_VALUE_FLOAT2:
        case MLN_PLUGIN_VALUE_COLOR: {
            const auto* array = value.getArray();
            const auto size = type == MLN_PLUGIN_VALUE_FLOAT2 ? 2u : 4u;
            return array && array->size() == size && std::all_of(array->begin(), array->end(), [](const auto& item) {
                       return item.getDouble() || item.getInt() || item.getUint();
                   });
        }
    }
    return false;
}

} // namespace plugin
} // namespace mbgl

extern "C" {

mln_plugin_status mln_plugin_register_v1(const mln_plugin_descriptor_v1* descriptor,
                                         char* errorMessage,
                                         size_t errorMessageCapacity) {
    if (!descriptor) return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    std::string error;
    const auto status = mbgl::plugin::PluginRegistry::get().registerPlugin(*descriptor, error);
    if (errorMessage && errorMessageCapacity) {
        const auto count = std::min(error.size(), errorMessageCapacity - 1);
        std::memcpy(errorMessage, error.data(), count);
        errorMessage[count] = '\0';
    }
    return status;
}

uint8_t mln_plugin_is_registered_v1(const char* pluginID) {
    return pluginID && mbgl::plugin::PluginRegistry::get().isRegistered(pluginID) ? 1 : 0;
}

size_t mln_plugin_count_v1(void) {
    return mbgl::plugin::PluginRegistry::get().pluginIDs().size();
}

size_t mln_plugin_id_at_v1(size_t index, char* output, size_t outputCapacity) {
    const auto ids = mbgl::plugin::PluginRegistry::get().pluginIDs();
    if (index >= ids.size()) return 0;
    const auto& id = ids[index];
    if (output && outputCapacity) {
        const auto count = std::min(id.size(), outputCapacity - 1);
        std::memcpy(output, id.data(), count);
        output[count] = '\0';
    }
    return id.size();
}

} // extern "C"
