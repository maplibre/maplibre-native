#pragma once

#include <mbgl/plugin/plugin_api.h>
#include <mbgl/style/style_property.hpp>

#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace mbgl {
namespace plugin {

struct PropertyDefinition {
    std::string pluginID;
    std::string targetLayerType;
    std::string name;
    mln_plugin_value_type type = MLN_PLUGIN_VALUE_BOOLEAN;
    mln_plugin_property_scope scope = MLN_PLUGIN_PROPERTY_PAINT;
    Value defaultValue;
};

struct LayerExtension {
    std::string pluginID;
    std::string pluginVersion;
    std::string targetLayerType;
    int32_t renderPriority = 0;
    uint32_t backendMask = 0;
    mln_plugin_create_instance_fn createInstance = nullptr;
    mln_plugin_destroy_instance_fn destroyInstance = nullptr;
    mln_plugin_prepare_frame_fn prepareFrame = nullptr;
    mln_plugin_render_before_layer_fn renderBeforeLayer = nullptr;
    mln_plugin_context_lost_fn contextLost = nullptr;
};

struct LayerType {
    std::string pluginID;
    std::string pluginVersion;
    std::string type;
    uint32_t backendMask = 0;
    mln_plugin_render_stage renderStage = MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT;
    bool requires3D = false;
    mln_plugin_create_instance_fn createInstance = nullptr;
    mln_plugin_destroy_instance_fn destroyInstance = nullptr;
    mln_plugin_prepare_frame_fn prepareFrame = nullptr;
    mln_plugin_render_before_layer_fn renderLayer = nullptr;
    mln_plugin_context_lost_fn contextLost = nullptr;
};

class PluginRegistry final {
public:
    static PluginRegistry& get();

    mln_plugin_status registerPlugin(const mln_plugin_descriptor_v1&, std::string& error);
    std::optional<PropertyDefinition> findProperty(const std::string& layerType, const std::string& name) const;
    std::vector<PropertyDefinition> propertiesForLayer(const std::string& layerType) const;
    std::vector<LayerExtension> extensionsForLayer(const std::string& layerType) const;
    std::optional<LayerType> findLayerType(const std::string& layerType) const;
    bool isRegistered(const std::string& pluginID) const;
    std::vector<std::string> pluginIDs() const;

    static bool valueMatches(mln_plugin_value_type, const Value&);

    struct PluginRecord {
        std::string version;
        std::vector<PropertyDefinition> properties;
        std::vector<LayerExtension> extensions;
        std::vector<LayerType> layerTypes;
    };

private:
    mutable std::mutex mutex;
    std::map<std::string, PluginRecord> plugins;
    std::map<std::pair<std::string, std::string>, PropertyDefinition> properties;
    std::map<std::string, LayerType> layerTypes;
};

} // namespace plugin
} // namespace mbgl
