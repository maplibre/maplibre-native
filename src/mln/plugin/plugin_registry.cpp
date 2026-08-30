#include <mln/plugin/plugin_registry.hpp>

#include <mln/shaders/shader_defines.hpp>

#include <algorithm>
#include <cstring>
#include <set>

namespace mln {
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
        case MLN_PLUGIN_VALUE_FLOAT_ARRAY: {
            mapbox::base::ValueArray values;
            if (value.data.float_array_value.data) {
                values.reserve(value.data.float_array_value.count);
                for (size_t i = 0; i < value.data.float_array_value.count; ++i) {
                    values.emplace_back(static_cast<double>(value.data.float_array_value.data[i]));
                }
            }
            return Value{std::move(values)};
        }
        case MLN_PLUGIN_VALUE_COLOR_ARRAY: {
            mapbox::base::ValueArray values;
            if (value.data.color_array_value.data) {
                values.reserve(value.data.color_array_value.count);
                for (size_t i = 0; i < value.data.color_array_value.count; ++i) {
                    const auto& color = value.data.color_array_value.data[i];
                    values.emplace_back(mapbox::base::ValueArray{static_cast<double>(color.r),
                                                                 static_cast<double>(color.g),
                                                                 static_cast<double>(color.b),
                                                                 static_cast<double>(color.a)});
                }
            }
            return Value{std::move(values)};
        }
    }
    return {};
}

bool validString(const mln_plugin_string& value) {
    return value.data && value.size > 0;
}

bool descriptorEquals(const PluginRegistry::PluginRecord& existing,
                      const std::string& version,
                      const std::vector<PropertyDefinition>& properties,
                      const std::vector<LayerExtension>& extensions,
                      const std::vector<LayerType>& layerTypes) {
    if (existing.version != version || existing.properties.size() != properties.size() ||
        existing.extensions.size() != extensions.size() || existing.layerTypes.size() != layerTypes.size()) {
        return false;
    }
    for (size_t i = 0; i < properties.size(); ++i) {
        const auto& lhs = existing.properties[i];
        const auto& rhs = properties[i];
        if (lhs.targetLayerType != rhs.targetLayerType || lhs.name != rhs.name || lhs.type != rhs.type ||
            lhs.scope != rhs.scope || lhs.defaultValue != rhs.defaultValue ||
            lhs.supportsExpressions != rhs.supportsExpressions || lhs.supportsTransitions != rhs.supportsTransitions ||
            lhs.acceptsScalar != rhs.acceptsScalar || lhs.minimum != rhs.minimum || lhs.maximum != rhs.maximum ||
            lhs.maximumArrayLength != rhs.maximumArrayLength || lhs.enumValues != rhs.enumValues) {
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
    for (size_t i = 0; i < layerTypes.size(); ++i) {
        const auto& lhs = existing.layerTypes[i];
        const auto& rhs = layerTypes[i];
        if (lhs.type != rhs.type || lhs.backendMask != rhs.backendMask || lhs.renderStage != rhs.renderStage ||
            lhs.requires3D != rhs.requires3D || lhs.participatesIn3DPass != rhs.participatesIn3DPass ||
            lhs.sourceKind != rhs.sourceKind || lhs.geometryTypeMask != rhs.geometryTypeMask ||
            lhs.createLayout != rhs.createLayout || lhs.layoutFeature != rhs.layoutFeature ||
            lhs.finishLayout != rhs.finishLayout || lhs.destroyLayout != rhs.destroyLayout ||
            lhs.queryFeature != rhs.queryFeature || lhs.shaders.size() != rhs.shaders.size() ||
            lhs.renderGraph != rhs.renderGraph || lhs.updateUniformBlock != rhs.updateUniformBlock) {
            return false;
        }
        for (size_t shaderIndex = 0; shaderIndex < lhs.shaders.size(); ++shaderIndex) {
            const auto& lhsShader = lhs.shaders[shaderIndex];
            const auto& rhsShader = rhs.shaders[shaderIndex];
            if (lhsShader.id != rhsShader.id || lhsShader.sources.size() != rhsShader.sources.size() ||
                lhsShader.attributes.size() != rhsShader.attributes.size() ||
                lhsShader.uniformBlocks.size() != rhsShader.uniformBlocks.size() ||
                lhsShader.textures.size() != rhsShader.textures.size()) {
                return false;
            }
            for (size_t sourceIndex = 0; sourceIndex < lhsShader.sources.size(); ++sourceIndex) {
                const auto& ls = lhsShader.sources[sourceIndex];
                const auto& rs = rhsShader.sources[sourceIndex];
                if (ls.backend != rs.backend || ls.vertex != rs.vertex || ls.fragment != rs.fragment ||
                    ls.vertexEntryPoint != rs.vertexEntryPoint || ls.fragmentEntryPoint != rs.fragmentEntryPoint) {
                    return false;
                }
            }
            for (size_t attrIndex = 0; attrIndex < lhsShader.attributes.size(); ++attrIndex) {
                const auto& la = lhsShader.attributes[attrIndex];
                const auto& ra = rhsShader.attributes[attrIndex];
                if (la.id != ra.id || la.location != ra.location || la.name != ra.name || la.type != ra.type) {
                    return false;
                }
            }
            for (size_t uniformIndex = 0; uniformIndex < lhsShader.uniformBlocks.size(); ++uniformIndex) {
                const auto& lu = lhsShader.uniformBlocks[uniformIndex];
                const auto& ru = rhsShader.uniformBlocks[uniformIndex];
                if (lu.id != ru.id || lu.name != ru.name || lu.byteSize != ru.byteSize ||
                    lu.stageMask != ru.stageMask || lu.scope != ru.scope || lu.bindingID != ru.bindingID) {
                    return false;
                }
            }
            for (size_t textureIndex = 0; textureIndex < lhsShader.textures.size(); ++textureIndex) {
                const auto& lt = lhsShader.textures[textureIndex];
                const auto& rt = rhsShader.textures[textureIndex];
                if (lt.id != rt.id || lt.location != rt.location || lt.name != rt.name) return false;
            }
        }
    }
    return true;
}

constexpr uint32_t supportedBackends = MLN_PLUGIN_BACKEND_OPENGL | MLN_PLUGIN_BACKEND_VULKAN | MLN_PLUGIN_BACKEND_METAL;

bool validBackendMask(uint32_t mask) {
    return (mask & supportedBackends) != 0 && (mask & ~supportedBackends) == 0;
}

bool validDrawMode(mln_plugin_draw_mode mode) {
    return mode == MLN_PLUGIN_DRAW_MODE_TRIANGLES || mode == MLN_PLUGIN_DRAW_MODE_LINES ||
           mode == MLN_PLUGIN_DRAW_MODE_POINTS;
}

bool validDepthMode(mln_plugin_depth_mode mode) {
    return mode >= MLN_PLUGIN_DEPTH_DISABLED && mode <= MLN_PLUGIN_DEPTH_READ_WRITE;
}

bool validBlendMode(mln_plugin_blend_mode mode) {
    return mode >= MLN_PLUGIN_BLEND_REPLACE && mode <= MLN_PLUGIN_BLEND_MULTIPLY;
}

bool validVertexType(mln_plugin_vertex_attribute_type type) {
    return type >= MLN_PLUGIN_VERTEX_INT16 && type <= MLN_PLUGIN_VERTEX_UINT8_X4_NORMALIZED;
}

bool appendShaders(const std::string& pluginID,
                   const mln_plugin_shader_descriptor_v1* shaders,
                   size_t shaderCount,
                   uint32_t backendMask,
                   std::vector<ShaderDefinition>& output,
                   std::string& error) {
    if (shaderCount == 0 || !shaders) {
        error = "host-drawable plugin layers must declare at least one shader";
        return false;
    }
    std::set<std::string> shaderIDs;
    for (size_t i = 0; i < shaderCount; ++i) {
        const auto& input = shaders[i];
        if (input.struct_size < sizeof(mln_plugin_shader_descriptor_v1) || !validString(input.shader_id) ||
            input.source_count == 0 || !input.sources || input.attribute_count == 0 || !input.attributes) {
            error = "plugin shader descriptor is malformed";
            return false;
        }
        ShaderDefinition shader;
        shader.pluginID = pluginID;
        shader.id = copyString(input.shader_id);
        if (!shaderIDs.emplace(shader.id).second) {
            error = "plugin layer contains duplicate shader ids";
            return false;
        }

        uint32_t sourceBackends = 0;
        for (size_t sourceIndex = 0; sourceIndex < input.source_count; ++sourceIndex) {
            const auto& source = input.sources[sourceIndex];
            if (source.struct_size < sizeof(mln_plugin_shader_source_v1) || !validBackendMask(source.backend) ||
                (source.backend & (source.backend - 1u)) != 0 || !validString(source.vertex_source) ||
                (source.backend != MLN_PLUGIN_BACKEND_METAL && !validString(source.fragment_source)) ||
                (source.backend == MLN_PLUGIN_BACKEND_METAL &&
                 (!validString(source.vertex_entry_point) || !validString(source.fragment_entry_point))) ||
                (sourceBackends & source.backend) != 0) {
                error = "plugin shader source is malformed or duplicated";
                return false;
            }
            sourceBackends |= source.backend;
            shader.sources.push_back(ShaderSource{source.backend,
                                                  copyString(source.vertex_source),
                                                  copyString(source.fragment_source),
                                                  copyString(source.vertex_entry_point),
                                                  copyString(source.fragment_entry_point)});
        }
        if ((sourceBackends & backendMask) != backendMask) {
            error = "plugin shader does not provide every declared backend";
            return false;
        }

        std::set<uint32_t> attributeIDs;
        std::set<uint32_t> locations;
        for (size_t attrIndex = 0; attrIndex < input.attribute_count; ++attrIndex) {
            const auto& attr = input.attributes[attrIndex];
            if (attr.struct_size < sizeof(mln_plugin_shader_attribute_v1) || !validString(attr.name) ||
                !validVertexType(attr.type) || !attributeIDs.emplace(attr.attribute_id).second ||
                !locations.emplace(attr.location).second) {
                error = "plugin shader attribute is malformed or duplicated";
                return false;
            }
            shader.attributes.push_back(
                ShaderAttribute{attr.attribute_id, attr.location, copyString(attr.name), attr.type});
        }
        std::sort(shader.attributes.begin(), shader.attributes.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.location < rhs.location;
        });
        for (size_t attrIndex = 0; attrIndex < shader.attributes.size(); ++attrIndex) {
            if (shader.attributes[attrIndex].location != attrIndex) {
                error = "plugin shader attribute locations must be contiguous and start at zero";
                return false;
            }
        }

        if (input.uniform_block_count && !input.uniform_blocks) {
            error = "plugin shader uniform block array is missing";
            return false;
        }
        std::set<uint32_t> uniformIDs;
        std::set<std::string> uniformNames;
        uint32_t layerUniformCount = 0;
#if MLN_RENDER_BACKEND_VULKAN
        uint32_t drawableUniformCount = 0;
#else
        std::set<uint32_t> drawableBindingIDs;
#endif
        for (size_t uniformIndex = 0; uniformIndex < input.uniform_block_count; ++uniformIndex) {
            const auto& uniform = input.uniform_blocks[uniformIndex];
            const auto validStages = MLN_PLUGIN_SHADER_STAGE_VERTEX | MLN_PLUGIN_SHADER_STAGE_FRAGMENT;
            if (uniform.struct_size < sizeof(mln_plugin_uniform_block_descriptor_v1) || !validString(uniform.name) ||
                uniform.byte_size == 0 || uniform.byte_size % 16 != 0 || uniform.stage_mask == 0 ||
                (uniform.stage_mask & ~validStages) != 0 ||
                (uniform.scope != MLN_PLUGIN_UNIFORM_SCOPE_LAYER &&
                 uniform.scope != MLN_PLUGIN_UNIFORM_SCOPE_DRAWABLE) ||
                !uniformIDs.emplace(uniform.uniform_id).second ||
                !uniformNames.emplace(copyString(uniform.name)).second) {
                error = "plugin shader uniform block is malformed or duplicated";
                return false;
            }
            uint32_t bindingID = 0;
            if (uniform.scope == MLN_PLUGIN_UNIFORM_SCOPE_LAYER) {
                if (layerUniformCount >= shaders::maxUBOCountPerLayer) {
                    error = "plugin shader declares too many layer uniform blocks";
                    return false;
                }
                bindingID = shaders::layerUBOStartId + layerUniformCount++;
            } else {
#if MLN_RENDER_BACKEND_VULKAN
                constexpr uint32_t drawableUniformStart = shaders::drawableUBOStartId;
                constexpr uint32_t drawableUniformCapacity = shaders::maxUBOCountPerDrawable;
                if (drawableUniformCount >= drawableUniformCapacity) {
                    error = "plugin shader declares too many drawable uniform blocks";
                    return false;
                }
                bindingID = drawableUniformStart + drawableUniformCount++;
#else
                // Metal and OpenGL pack drawable blocks by visibility. These
                // slots also control which shader stages receive the buffer.
                // A both-stage block therefore cannot occupy either of the
                // single-stage reserved slots.
                if (uniform.stage_mask == MLN_PLUGIN_SHADER_STAGE_VERTEX) {
                    bindingID = shaders::idDrawableReservedVertexOnlyUBO;
                } else if (uniform.stage_mask == MLN_PLUGIN_SHADER_STAGE_FRAGMENT) {
                    bindingID = shaders::idDrawableReservedFragmentOnlyUBO;
                } else {
                    bindingID = shaders::drawableReservedUBOCount;
                }
                if (!drawableBindingIDs.emplace(bindingID).second) {
                    error = "plugin shader declares more than one drawable uniform block for a packed stage slot";
                    return false;
                }
#endif
            }
            shader.uniformBlocks.push_back({uniform.uniform_id,
                                            copyString(uniform.name),
                                            uniform.byte_size,
                                            uniform.stage_mask,
                                            uniform.scope,
                                            bindingID});
        }

        if (input.texture_count && !input.textures) {
            error = "plugin shader texture array is missing";
            return false;
        }
        std::set<uint32_t> textureIDs;
        std::set<uint32_t> textureLocations;
        for (size_t textureIndex = 0; textureIndex < input.texture_count; ++textureIndex) {
            const auto& texture = input.textures[textureIndex];
            if (texture.struct_size < sizeof(mln_plugin_shader_texture_v1) || !validString(texture.name) ||
                texture.location >= shaders::maxTextureCountPerShader ||
                !textureIDs.emplace(texture.texture_id).second || !textureLocations.emplace(texture.location).second) {
                error = "plugin shader texture is malformed or duplicated";
                return false;
            }
            shader.textures.push_back({texture.texture_id, texture.location, copyString(texture.name)});
        }
        output.push_back(std::move(shader));
    }
    return true;
}

bool appendProperties(const std::string& pluginID,
                      const std::string& targetLayerType,
                      const mln_plugin_property_descriptor_v1* properties,
                      size_t propertyCount,
                      std::set<std::pair<std::string, std::string>>& propertyKeys,
                      std::vector<PropertyDefinition>& output,
                      std::string& error) {
    if (propertyCount && !properties) {
        error = "plugin property array is missing";
        return false;
    }
    for (size_t p = 0; p < propertyCount; ++p) {
        const auto& property = properties[p];
        if (property.struct_size < sizeof(mln_plugin_property_descriptor_v1) || !validString(property.name) ||
            property.default_value.struct_size < sizeof(mln_plugin_value) ||
            property.default_value.type != property.type ||
            (property.scope != MLN_PLUGIN_PROPERTY_PAINT && property.scope != MLN_PLUGIN_PROPERTY_LAYOUT)) {
            error = "plugin property descriptor is malformed";
            return false;
        }
        auto defaultValue = copyValue(property.default_value);
        if (!PluginRegistry::valueMatches(property.type, defaultValue)) {
            error = "plugin property default value has the wrong type";
            return false;
        }
        const auto propertyName = copyString(property.name);
        if (!propertyKeys.emplace(targetLayerType, propertyName).second) {
            error = "plugin descriptor contains duplicate property names";
            return false;
        }
        std::vector<std::string> enumValues;
        if (property.enum_value_count) {
            if (!property.enum_values || property.type != MLN_PLUGIN_VALUE_STRING) {
                error = "plugin property enum values require a string property";
                return false;
            }
            std::set<std::string> uniqueValues;
            for (size_t valueIndex = 0; valueIndex < property.enum_value_count; ++valueIndex) {
                if (!validString(property.enum_values[valueIndex])) {
                    error = "plugin property enum contains an empty value";
                    return false;
                }
                auto value = copyString(property.enum_values[valueIndex]);
                if (!uniqueValues.emplace(value).second) {
                    error = "plugin property enum contains duplicate values";
                    return false;
                }
                enumValues.push_back(std::move(value));
            }
            const auto* defaultString = defaultValue.getString();
            if (!defaultString || uniqueValues.find(*defaultString) == uniqueValues.end()) {
                error = "plugin property enum default is not an allowed value";
                return false;
            }
        }
        const auto* defaultArray = defaultValue.getArray();
        if (property.maximum_array_length &&
            (property.type != MLN_PLUGIN_VALUE_FLOAT_ARRAY && property.type != MLN_PLUGIN_VALUE_COLOR_ARRAY)) {
            error = "plugin property array limit requires an array value type";
            return false;
        }
        if (defaultArray && property.maximum_array_length && defaultArray->size() > property.maximum_array_length) {
            error = "plugin property default exceeds its array length limit";
            return false;
        }
        const auto inRange = [&](double value) {
            return (!property.has_minimum || value >= property.minimum) &&
                   (!property.has_maximum || value <= property.maximum);
        };
        if (property.has_minimum && property.has_maximum && property.minimum > property.maximum) {
            error = "plugin property has an invalid numeric range";
            return false;
        }
        if (property.type == MLN_PLUGIN_VALUE_FLOAT) {
            const auto value = numericValue<double>(defaultValue);
            if (!value || !inRange(*value)) {
                error = "plugin property default is outside its numeric range";
                return false;
            }
        } else if (property.type == MLN_PLUGIN_VALUE_FLOAT_ARRAY && defaultArray) {
            for (const auto& item : *defaultArray) {
                const auto value = numericValue<double>(item);
                if (!value || !inRange(*value)) {
                    error = "plugin property array default is outside its numeric range";
                    return false;
                }
            }
        }
        output.push_back(
            PropertyDefinition{pluginID,
                               targetLayerType,
                               propertyName,
                               property.type,
                               property.scope,
                               std::move(defaultValue),
                               property.supports_expressions != 0,
                               property.supports_transitions != 0,
                               property.accepts_scalar != 0,
                               property.has_minimum ? std::optional<float>{property.minimum} : std::nullopt,
                               property.has_maximum ? std::optional<float>{property.maximum} : std::nullopt,
                               property.maximum_array_length,
                               std::move(enumValues)});
    }
    return true;
}

bool appendRenderGraph(const mln_plugin_render_graph_v1& input,
                       const std::vector<ShaderDefinition>& shaders,
                       mln_plugin_render_stage layerStage,
                       RenderGraphDefinition& output,
                       std::string& error) {
    if (input.struct_size < sizeof(mln_plugin_render_graph_v1) || input.render_target_count == 0 ||
        !input.render_targets || input.pass_count == 0 || !input.passes) {
        error = "plugin render graph is malformed";
        return false;
    }
    std::set<uint32_t> targetIDs;
    for (size_t targetIndex = 0; targetIndex < input.render_target_count; ++targetIndex) {
        const auto& target = input.render_targets[targetIndex];
        if (target.struct_size < sizeof(mln_plugin_render_target_descriptor_v1) || target.target_id == 0 ||
            target.size != MLN_PLUGIN_RENDER_TARGET_SOURCE_TILE || target.format != MLN_PLUGIN_RENDER_TARGET_RGBA8 ||
            !targetIDs.emplace(target.target_id).second) {
            error = "plugin render target is malformed or duplicated";
            return false;
        }
        output.renderTargets.push_back({target.target_id, target.size, target.format});
    }

    std::set<uint32_t> passIDs;
    std::set<uint32_t> writtenTargets;
    for (size_t passIndex = 0; passIndex < input.pass_count; ++passIndex) {
        const auto& pass = input.passes[passIndex];
        if (pass.struct_size < sizeof(mln_plugin_render_pass_descriptor_v1) || pass.pass_id == 0 ||
            !validString(pass.shader_id) || !passIDs.emplace(pass.pass_id).second ||
            (pass.geometry != MLN_PLUGIN_GRAPH_GEOMETRY_RASTER_DEM_FULL_TILE &&
             pass.geometry != MLN_PLUGIN_GRAPH_GEOMETRY_RASTER_DEM_MASKED_TILE) ||
            !validDrawMode(pass.draw_mode) || !validDepthMode(pass.depth_mode) || !validBlendMode(pass.blend_mode) ||
            (pass.texture_count && !pass.textures)) {
            error = "plugin render pass is malformed or duplicated";
            return false;
        }
        const auto shaderID = copyString(pass.shader_id);
        const auto shaderIt = std::find_if(
            shaders.begin(), shaders.end(), [&](const auto& shader) { return shader.id == shaderID; });
        if (shaderIt == shaders.end()) {
            error = "plugin render pass references an unknown shader";
            return false;
        }
        if (pass.render_target_id) {
            if (targetIDs.find(pass.render_target_id) == targetIDs.end() ||
                pass.render_stage != MLN_PLUGIN_RENDER_STAGE_PREPARE ||
                !writtenTargets.emplace(pass.render_target_id).second) {
                error = "plugin offscreen pass has an invalid or multiply-written target";
                return false;
            }
        } else if (pass.render_stage != layerStage) {
            error = "plugin main render pass does not match the layer render stage";
            return false;
        }

        RenderPassDefinition copied;
        copied.id = pass.pass_id;
        copied.shaderID = shaderID;
        copied.geometry = pass.geometry;
        copied.renderTargetID = pass.render_target_id;
        copied.renderStage = pass.render_stage;
        copied.drawMode = pass.draw_mode;
        copied.depthMode = pass.depth_mode;
        copied.blendMode = pass.blend_mode;
        copied.enableStencil = pass.enable_stencil != 0;
        copied.enableCullFace = pass.enable_cull_face != 0;
        std::set<uint32_t> boundTextures;
        for (size_t bindingIndex = 0; bindingIndex < pass.texture_count; ++bindingIndex) {
            const auto& binding = pass.textures[bindingIndex];
            const auto textureIt = std::find_if(shaderIt->textures.begin(),
                                                shaderIt->textures.end(),
                                                [&](const auto& texture) { return texture.id == binding.texture_id; });
            const bool validSource = binding.source == MLN_PLUGIN_TEXTURE_SOURCE_RASTER_DEM ||
                                     binding.source == MLN_PLUGIN_TEXTURE_SOURCE_RENDER_TARGET;
            if (binding.struct_size < sizeof(mln_plugin_texture_binding_v1) || textureIt == shaderIt->textures.end() ||
                !boundTextures.emplace(binding.texture_id).second || !validSource ||
                (binding.source == MLN_PLUGIN_TEXTURE_SOURCE_RENDER_TARGET &&
                 (targetIDs.find(binding.render_target_id) == targetIDs.end() ||
                  writtenTargets.find(binding.render_target_id) == writtenTargets.end())) ||
                (binding.source == MLN_PLUGIN_TEXTURE_SOURCE_RASTER_DEM && binding.render_target_id != 0) ||
                (binding.filter != MLN_PLUGIN_TEXTURE_FILTER_NEAREST &&
                 binding.filter != MLN_PLUGIN_TEXTURE_FILTER_LINEAR) ||
                (binding.wrap_u != MLN_PLUGIN_TEXTURE_WRAP_CLAMP && binding.wrap_u != MLN_PLUGIN_TEXTURE_WRAP_REPEAT) ||
                (binding.wrap_v != MLN_PLUGIN_TEXTURE_WRAP_CLAMP && binding.wrap_v != MLN_PLUGIN_TEXTURE_WRAP_REPEAT)) {
                error = "plugin render pass contains an invalid texture binding or forward dependency";
                return false;
            }
            copied.textures.push_back({binding.texture_id,
                                       binding.source,
                                       binding.render_target_id,
                                       binding.filter,
                                       binding.wrap_u,
                                       binding.wrap_v});
        }
        if (boundTextures.size() != shaderIt->textures.size()) {
            error = "plugin render pass does not bind every shader texture";
            return false;
        }
        output.passes.push_back(std::move(copied));
    }
    if (writtenTargets != targetIDs) {
        error = "plugin render graph contains an unwritten target";
        return false;
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
    if (!validString(descriptor.plugin_id) || !validString(descriptor.plugin_version) ||
        (descriptor.layer_extension_count && !descriptor.layer_extensions) ||
        (descriptor.layer_type_count && !descriptor.layer_types) ||
        (descriptor.layer_extension_count == 0 && descriptor.layer_type_count == 0)) {
        error = "plugin descriptor is missing an id, version, or layer registration";
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }

    const auto pluginID = copyString(descriptor.plugin_id);
    const auto pluginVersion = copyString(descriptor.plugin_version);
    std::vector<PropertyDefinition> newProperties;
    std::vector<LayerExtension> newExtensions;
    std::vector<LayerType> newLayerTypes;
    std::set<std::pair<std::string, int32_t>> extensionKeys;
    std::set<std::pair<std::string, std::string>> propertyKeys;

    for (size_t i = 0; i < descriptor.layer_extension_count; ++i) {
        const auto& extension = descriptor.layer_extensions[i];
        if (extension.struct_size < sizeof(mln_plugin_layer_extension_v1) ||
            !validString(extension.target_layer_type) || !extension.create_instance || !extension.destroy_instance ||
            (!extension.prepare_frame && !extension.render_before_layer) || !validBackendMask(extension.backend_mask)) {
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
            if (extension.properties[p].scope != MLN_PLUGIN_PROPERTY_PAINT) {
                error = "existing-layer extension properties must be paint properties";
                return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
            }
        }

        if (!appendProperties(pluginID,
                              targetLayerType,
                              extension.properties,
                              extension.property_count,
                              propertyKeys,
                              newProperties,
                              error)) {
            return error.find("duplicate") != std::string::npos ? MLN_PLUGIN_STATUS_CONFLICT
                                                                : MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        }
    }

    std::set<std::string> layerTypeKeys;
    for (size_t i = 0; i < descriptor.layer_type_count; ++i) {
        const auto& layerType = descriptor.layer_types[i];
        const uint32_t validGeometryMask = MLN_PLUGIN_GEOMETRY_POINT | MLN_PLUGIN_GEOMETRY_LINESTRING |
                                           MLN_PLUGIN_GEOMETRY_POLYGON;
        if (layerType.struct_size < sizeof(mln_plugin_layer_type_v1) || !validString(layerType.layer_type) ||
            !validBackendMask(layerType.backend_mask) ||
            (layerType.render_stage != MLN_PLUGIN_RENDER_STAGE_PASS_3D &&
             layerType.render_stage != MLN_PLUGIN_RENDER_STAGE_OPAQUE &&
             layerType.render_stage != MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT)) {
            error = "plugin layer type is malformed or has no supported backend";
            return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        }
        const bool geometryLayer = layerType.source_kind == MLN_PLUGIN_SOURCE_GEOMETRY;
        const bool rasterDEMLayer = layerType.source_kind == MLN_PLUGIN_SOURCE_RASTER_DEM;
        if ((!geometryLayer && !rasterDEMLayer) ||
            (geometryLayer && (!layerType.create_layout || !layerType.layout_feature || !layerType.finish_layout ||
                               !layerType.destroy_layout || layerType.geometry_type_mask == 0 ||
                               (layerType.geometry_type_mask & ~validGeometryMask) != 0 || layerType.render_graph)) ||
            (rasterDEMLayer &&
             (layerType.create_layout || layerType.layout_feature || layerType.finish_layout ||
              layerType.destroy_layout || layerType.geometry_type_mask != 0 || !layerType.render_graph))) {
            error = geometryLayer ? "geometry plugin layer has an invalid layout or render graph contract"
                                  : "RasterDEM plugin layer requires a render graph and no geometry callbacks";
            return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        }
        const auto type = copyString(layerType.layer_type);
        if (!layerTypeKeys.emplace(type).second) {
            error = "plugin descriptor contains duplicate layer types";
            return MLN_PLUGIN_STATUS_CONFLICT;
        }
        LayerType copiedLayerType;
        copiedLayerType.pluginID = pluginID;
        copiedLayerType.pluginVersion = pluginVersion;
        copiedLayerType.type = type;
        copiedLayerType.backendMask = layerType.backend_mask;
        copiedLayerType.renderStage = layerType.render_stage;
        copiedLayerType.requires3D = layerType.requires_3d != 0;
        // Every depth-producing 3D layer must be scheduled in the 3D pass.
        // Non-3D layers such as hillshade can opt in as well for ordering.
        copiedLayerType.participatesIn3DPass = layerType.requires_3d != 0 || layerType.participates_in_3d_pass != 0;
        copiedLayerType.sourceKind = layerType.source_kind;
        copiedLayerType.geometryTypeMask = layerType.geometry_type_mask;
        copiedLayerType.createLayout = layerType.create_layout;
        copiedLayerType.layoutFeature = layerType.layout_feature;
        copiedLayerType.finishLayout = layerType.finish_layout;
        copiedLayerType.destroyLayout = layerType.destroy_layout;
        copiedLayerType.queryFeature = layerType.query_feature;
        copiedLayerType.updateUniformBlock = layerType.update_uniform_block;
        if (!appendShaders(pluginID,
                           layerType.shaders,
                           layerType.shader_count,
                           layerType.backend_mask,
                           copiedLayerType.shaders,
                           error)) {
            return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        }
        const bool needsUniformCallback = std::any_of(copiedLayerType.shaders.begin(),
                                                      copiedLayerType.shaders.end(),
                                                      [](const auto& shader) { return !shader.uniformBlocks.empty(); });
        if (needsUniformCallback && !layerType.update_uniform_block) {
            error = "plugin layer declares uniforms without an update callback";
            return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        }
        if (rasterDEMLayer) {
            RenderGraphDefinition graph;
            if (!appendRenderGraph(
                    *layerType.render_graph, copiedLayerType.shaders, layerType.render_stage, graph, error)) {
                return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
            }
            copiedLayerType.renderGraph = std::move(graph);
        }
        newLayerTypes.push_back(std::move(copiedLayerType));
        if (!appendProperties(
                pluginID, type, layerType.properties, layerType.property_count, propertyKeys, newProperties, error)) {
            return error.find("duplicate") != std::string::npos ? MLN_PLUGIN_STATUS_CONFLICT
                                                                : MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        }
    }

    std::lock_guard<std::mutex> lock(mutex);
    if (const auto existing = plugins.find(pluginID); existing != plugins.end()) {
        if (descriptorEquals(existing->second, pluginVersion, newProperties, newExtensions, newLayerTypes)) {
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
    for (const auto& layerType : newLayerTypes) {
        if (layerTypes.find(layerType.type) != layerTypes.end()) {
            error = "layer type '" + layerType.type + "' is already registered";
            return MLN_PLUGIN_STATUS_CONFLICT;
        }
    }

    PluginRecord record{pluginVersion, newProperties, newExtensions, newLayerTypes};
    for (const auto& property : newProperties) {
        properties.emplace(std::make_pair(property.targetLayerType, property.name), property);
    }
    for (const auto& layerType : newLayerTypes) {
        layerTypes.emplace(layerType.type, layerType);
    }
    plugins.emplace(pluginID, std::move(record));
    return MLN_PLUGIN_STATUS_OK;
}

std::optional<LayerType> PluginRegistry::findLayerType(const std::string& layerType) const {
    std::lock_guard<std::mutex> lock(mutex);
    const auto it = layerTypes.find(layerType);
    return it == layerTypes.end() ? std::nullopt : std::optional<LayerType>{it->second};
}

std::vector<LayerType> PluginRegistry::allLayerTypes() const {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<LayerType> result;
    result.reserve(layerTypes.size());
    for (const auto& [type, registration] : layerTypes) {
        (void)type;
        result.push_back(registration);
    }
    return result;
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
        case MLN_PLUGIN_VALUE_FLOAT_ARRAY: {
            const auto* array = value.getArray();
            return array && !array->empty() && std::all_of(array->begin(), array->end(), [&](const auto& item) {
                       return numericValue<double>(item).has_value();
                   });
        }
        case MLN_PLUGIN_VALUE_COLOR_ARRAY: {
            const auto* array = value.getArray();
            return array && !array->empty() && std::all_of(array->begin(), array->end(), [&](const auto& item) {
                       const auto* color = item.getArray();
                       return color && color->size() == 4 &&
                              std::all_of(color->begin(), color->end(), [&](const auto& component) {
                                  return numericValue<double>(component).has_value();
                              });
                   });
        }
    }
    return false;
}

} // namespace plugin
} // namespace mln

extern "C" {

mln_plugin_status mln_plugin_register_v1(const mln_plugin_descriptor_v1* descriptor,
                                         char* errorMessage,
                                         size_t errorMessageCapacity) {
    if (!descriptor) return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    std::string error;
    const auto status = mln::plugin::PluginRegistry::get().registerPlugin(*descriptor, error);
    if (errorMessage && errorMessageCapacity) {
        const auto count = std::min(error.size(), errorMessageCapacity - 1);
        std::memcpy(errorMessage, error.data(), count);
        errorMessage[count] = '\0';
    }
    return status;
}

uint8_t mln_plugin_is_registered_v1(const char* pluginID) {
    return pluginID && mln::plugin::PluginRegistry::get().isRegistered(pluginID) ? 1 : 0;
}

size_t mln_plugin_count_v1(void) {
    return mln::plugin::PluginRegistry::get().pluginIDs().size();
}

size_t mln_plugin_id_at_v1(size_t index, char* output, size_t outputCapacity) {
    const auto ids = mln::plugin::PluginRegistry::get().pluginIDs();
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
