#pragma once

#include <mln/plugin/plugin_api.h>
#include <mln/style/style_property.hpp>
#include <mln/style/layer.hpp>

#include <array>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace mln {
namespace plugin {

struct PropertyDefinition {
    std::string pluginID;
    std::string targetLayerType;
    std::string name;
    mln_plugin_value_type type = MLN_PLUGIN_VALUE_FLOAT;
    Value defaultValue;
    uint32_t expressionCapabilities = MLN_PLUGIN_EXPRESSION_NONE;
    bool supportsTransitions = false;
    std::optional<float> minimum;
    std::optional<float> maximum;
    std::vector<std::string> enumValues;
};

struct ShaderAttribute {
    uint32_t id = 0;
    uint32_t location = 0;
    std::string name;
    mln_plugin_vertex_attribute_type type = MLN_PLUGIN_VERTEX_FLOAT;
};

struct ShaderSource {
    mln_plugin_backend backend = MLN_PLUGIN_BACKEND_OPENGL;
    std::string vertex;
    std::string fragment;
    std::string vertexEntryPoint;
    std::string fragmentEntryPoint;
};

struct UniformBlockDefinition {
    uint32_t id = 0;
    std::string name;
    uint32_t byteSize = 0;
    uint32_t stageMask = 0;
    uint32_t bindingID = 0;
};

struct ShaderPropertyBindingDefinition {
    std::string propertyName;
    mln_plugin_property_encoding_v1 encoding = MLN_PLUGIN_PROPERTY_ENCODING_FLOAT;
    uint32_t uniformID = 0;
    uint32_t uniformByteOffset = 0;
    uint32_t minimumAttributeID = 0;
    uint32_t maximumAttributeID = 0;
    uint32_t interpolationUniformID = 0;
    uint32_t interpolationUniformByteOffset = 0;
    bool operator==(const ShaderPropertyBindingDefinition&) const = default;
};

struct ShaderDefinition {
    std::string pluginID;
    std::string id;
    std::vector<ShaderSource> sources;
    std::vector<ShaderAttribute> attributes;
    std::vector<UniformBlockDefinition> uniformBlocks;
    std::vector<ShaderPropertyBindingDefinition> propertyBindings;
};

struct LayerTypeIdentity;

struct LayerType {
    std::shared_ptr<const LayerTypeIdentity> identity;
    std::string pluginID;
    std::string pluginVersion;
    std::string type;
    uint32_t backendMask = 0;
    uint32_t geometryTypeMask = 0;
    std::vector<ShaderDefinition> shaders;
    mln_plugin_create_layout_fn createLayout = nullptr;
    mln_plugin_layout_feature_fn layoutFeature = nullptr;
    mln_plugin_finish_layout_fn finishLayout = nullptr;
    mln_plugin_destroy_layout_fn destroyLayout = nullptr;
    mln_plugin_query_feature_fn queryFeature = nullptr;
    mln_plugin_query_radius_fn queryRadius = nullptr;
    mln_plugin_update_uniform_block_fn updateUniformBlock = nullptr;
};

struct LayerTypeIdentity {
    explicit LayerTypeIdentity(const plugin::LayerType& registration)
        : name(registration.type),
          info{name.c_str(),
               style::LayerTypeInfo::Source::Required,
               style::LayerTypeInfo::Pass3D::NotRequired,
               style::LayerTypeInfo::Layout::Required,
               style::LayerTypeInfo::FadingTiles::NotRequired,
               style::LayerTypeInfo::CrossTileIndex::NotRequired,
               style::LayerTypeInfo::TileKind::Geometry} {}

    std::string name;
    style::LayerTypeInfo info;
};

class PluginRegistry final {
public:
    static PluginRegistry& get();

    mln_plugin_status registerPlugin(const mln_plugin_descriptor_v1&, std::string& error);
    std::optional<PropertyDefinition> findProperty(const std::string& layerType, const std::string& name) const;
    std::vector<PropertyDefinition> propertiesForLayer(const std::string& layerType) const;
    std::optional<LayerType> findLayerType(const std::string& layerType) const;
    std::vector<LayerType> allLayerTypes() const;

    static bool valueMatches(mln_plugin_value_type, const Value&);

    struct PluginRecord {
        std::string version;
        std::vector<PropertyDefinition> properties;
        std::vector<LayerType> layerTypes;
    };

private:
    mutable std::mutex mutex;
    std::map<std::string, PluginRecord> plugins;
    std::map<std::pair<std::string, std::string>, PropertyDefinition> properties;
    std::map<std::string, LayerType> layerTypes;
};

} // namespace plugin
} // namespace mln
