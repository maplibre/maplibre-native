#pragma once

#include <mln/plugin/plugin_api.h>
#include <mln/style/style_property.hpp>

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
    mln_plugin_value_type type = MLN_PLUGIN_VALUE_BOOLEAN;
    mln_plugin_property_scope scope = MLN_PLUGIN_PROPERTY_PAINT;
    Value defaultValue;
    bool supportsExpressions = false;
    bool supportsTransitions = false;
    bool acceptsScalar = false;
    std::optional<float> minimum;
    std::optional<float> maximum;
    uint32_t maximumArrayLength = 0;
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
    mln_plugin_uniform_scope scope = MLN_PLUGIN_UNIFORM_SCOPE_DRAWABLE;
    uint32_t bindingID = 0;
};

struct ShaderTextureDefinition {
    uint32_t id = 0;
    uint32_t location = 0;
    std::string name;
};

struct ShaderDefinition {
    std::string pluginID;
    std::string id;
    std::vector<ShaderSource> sources;
    std::vector<ShaderAttribute> attributes;
    std::vector<UniformBlockDefinition> uniformBlocks;
    std::vector<ShaderTextureDefinition> textures;
};

struct TextureBindingDefinition {
    uint32_t textureID = 0;
    mln_plugin_texture_source source = MLN_PLUGIN_TEXTURE_SOURCE_RASTER_DEM;
    uint32_t renderTargetID = 0;
    std::string propertyName;
    mln_plugin_texture_filter filter = MLN_PLUGIN_TEXTURE_FILTER_LINEAR;
    mln_plugin_texture_wrap wrapU = MLN_PLUGIN_TEXTURE_WRAP_CLAMP;
    mln_plugin_texture_wrap wrapV = MLN_PLUGIN_TEXTURE_WRAP_CLAMP;
    bool operator==(const TextureBindingDefinition&) const = default;
};

struct RenderTargetDefinition {
    uint32_t id = 0;
    mln_plugin_render_target_size size = MLN_PLUGIN_RENDER_TARGET_SOURCE_TILE;
    mln_plugin_render_target_format format = MLN_PLUGIN_RENDER_TARGET_RGBA8;
    mln_plugin_render_target_scope scope = MLN_PLUGIN_RENDER_TARGET_PER_TILE;
    float widthScale = 1.0f;
    float heightScale = 1.0f;
    std::array<float, 4> clearColor{0.0f, 0.0f, 0.0f, 1.0f};
    bool operator==(const RenderTargetDefinition&) const = default;
};

struct RenderPassDefinition {
    uint32_t id = 0;
    std::string shaderID;
    mln_plugin_graph_geometry geometry = MLN_PLUGIN_GRAPH_GEOMETRY_PLUGIN_BUCKET;
    uint32_t renderTargetID = 0;
    mln_plugin_render_stage renderStage = MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT;
    mln_plugin_draw_mode drawMode = MLN_PLUGIN_DRAW_MODE_TRIANGLES;
    mln_plugin_depth_mode depthMode = MLN_PLUGIN_DEPTH_DISABLED;
    mln_plugin_blend_mode blendMode = MLN_PLUGIN_BLEND_ALPHA;
    bool enableStencil = false;
    bool enableCullFace = false;
    mln_plugin_tile_projection tileProjection = MLN_PLUGIN_TILE_PROJECTION_MAP;
    std::vector<TextureBindingDefinition> textures;
    bool operator==(const RenderPassDefinition&) const = default;
};

struct RenderGraphDefinition {
    std::vector<RenderTargetDefinition> renderTargets;
    std::vector<RenderPassDefinition> passes;
    bool operator==(const RenderGraphDefinition&) const = default;
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
    bool participatesIn3DPass = false;
    mln_plugin_source_kind sourceKind = MLN_PLUGIN_SOURCE_NONE;
    uint32_t geometryTypeMask = 0;
    std::vector<ShaderDefinition> shaders;
    mln_plugin_create_layout_fn createLayout = nullptr;
    mln_plugin_layout_feature_fn layoutFeature = nullptr;
    mln_plugin_finish_layout_fn finishLayout = nullptr;
    mln_plugin_destroy_layout_fn destroyLayout = nullptr;
    mln_plugin_query_feature_fn queryFeature = nullptr;
    std::optional<RenderGraphDefinition> renderGraph;
    mln_plugin_update_uniform_block_fn updateUniformBlock = nullptr;
};

class PluginRegistry final {
public:
    static PluginRegistry& get();

    mln_plugin_status registerPlugin(const mln_plugin_descriptor_v1&, std::string& error);
    std::optional<PropertyDefinition> findProperty(const std::string& layerType, const std::string& name) const;
    std::vector<PropertyDefinition> propertiesForLayer(const std::string& layerType) const;
    std::vector<LayerExtension> extensionsForLayer(const std::string& layerType) const;
    std::optional<LayerType> findLayerType(const std::string& layerType) const;
    std::vector<LayerType> allLayerTypes() const;
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
} // namespace mln
