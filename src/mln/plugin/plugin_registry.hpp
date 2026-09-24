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
    std::string name;
    mln_plugin_value_type type = MLN_PLUGIN_VALUE_FLOAT;
    Value defaultValue;
    uint32_t expressionCapabilities = MLN_PLUGIN_EXPRESSION_NONE;
    bool supportsTransitions = false;
    std::optional<float> minimum;
    std::optional<float> maximum;
    std::vector<std::string> enumValues;
    bool operator==(const PropertyDefinition&) const = default;
};

struct ShaderAttribute {
    uint32_t id = 0;
    uint32_t location = 0;
    std::string name;
    mln_plugin_vertex_attribute_type type = MLN_PLUGIN_VERTEX_FLOAT;
    bool operator==(const ShaderAttribute&) const = default;
};

struct ShaderSource {
    mln_plugin_backend backend = MLN_PLUGIN_BACKEND_OPENGL;
    std::string vertex;
    std::string fragment;
    std::string vertexEntryPoint;
    std::string fragmentEntryPoint;
    bool operator==(const ShaderSource&) const = default;
};

struct UniformBlockDefinition {
    uint32_t id = 0;
    std::string name;
    uint32_t byteSize = 0;
    uint32_t stageMask = 0;
    uint32_t bindingID = 0;
    mln_plugin_uniform_scope_v1 scope = MLN_PLUGIN_UNIFORM_DRAWABLE;
    bool operator==(const UniformBlockDefinition&) const = default;
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
    bool operator==(const ShaderDefinition&) const = default;
};

struct DrawPass {
    bool depthTest = true;
    bool depthWrite = false;
    bool colorWrite = true;
    bool blend = true;
    bool stencilDedup = false;
    mln_plugin_cull_mode_v1 cull = MLN_PLUGIN_CULL_NONE;
    bool operator==(const DrawPass&) const = default;
};

struct LayerType {
    std::string pluginID;
    std::string type;
    uint32_t backendMask = 0;
    uint32_t geometryTypeMask = 0;
    std::vector<ShaderDefinition> shaders;
    std::vector<PropertyDefinition> properties;
    mln_plugin_create_layout_fn createLayout = nullptr;
    mln_plugin_layout_feature_fn layoutFeature = nullptr;
    mln_plugin_finish_layout_fn finishLayout = nullptr;
    mln_plugin_destroy_layout_fn destroyLayout = nullptr;
    mln_plugin_query_feature_fn queryFeature = nullptr;
    mln_plugin_query_radius_fn queryRadius = nullptr;
    mln_plugin_update_uniform_block_fn updateUniformBlock = nullptr;
    bool replaceBuiltin = false;
    bool is3D = false;
    std::vector<DrawPass> drawPasses;
    mln_plugin_evaluate_layer_fn evaluateLayer = nullptr;
    bool enableNearClippedMatrix = false;
    bool operator==(const LayerType&) const = default;

    const PropertyDefinition* findProperty(const std::string& name) const;
};

// Allocated once and shared as const by factories, styles, workers and renderers.
// Never copy/move the registered object: info.type points into its owned name.
struct RegisteredLayer final : LayerType {
    explicit RegisteredLayer(LayerType definition)
        : LayerType(std::move(definition)),
          info{type.c_str(),
               style::LayerTypeInfo::Source::Required,
               is3D ? style::LayerTypeInfo::Pass3D::Required : style::LayerTypeInfo::Pass3D::NotRequired,
               style::LayerTypeInfo::Layout::Required,
               style::LayerTypeInfo::FadingTiles::NotRequired,
               style::LayerTypeInfo::CrossTileIndex::NotRequired,
               style::LayerTypeInfo::TileKind::Geometry} {}

    RegisteredLayer(const RegisteredLayer&) = delete;
    RegisteredLayer& operator=(const RegisteredLayer&) = delete;
    style::LayerTypeInfo info;
};

using RegisteredLayerPtr = std::shared_ptr<const RegisteredLayer>;

class PluginRegistry final {
public:
    static PluginRegistry& get();

    mln_plugin_status registerPlugin(const mln_plugin_descriptor_v1&, std::string& error);
    RegisteredLayerPtr findLayerType(const std::string& layerType) const;

    static bool valueMatches(mln_plugin_value_type, const Value&);

    struct PluginRecord {
        std::string version;
        std::vector<RegisteredLayerPtr> layerTypes;
    };

private:
    mutable std::mutex mutex;
    std::map<std::string, PluginRecord> plugins;
    std::map<std::string, RegisteredLayerPtr> layerTypes;
};

} // namespace plugin
} // namespace mln
