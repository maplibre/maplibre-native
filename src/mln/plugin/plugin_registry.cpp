#include <mln/plugin/plugin_registry.hpp>
#include <mln/plugin/plugin_style_layer_factory.hpp>
#include <mln/layermanager/layer_manager.hpp>

#include <mln/shaders/shader_defines.hpp>
#include <mln/style/plugin_property.hpp>

#include <algorithm>
#include <cmath>
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
                      const std::vector<LayerType>& layerTypes) {
    if (existing.version != version || existing.properties.size() != properties.size() ||
        existing.layerTypes.size() != layerTypes.size()) {
        return false;
    }
    for (size_t i = 0; i < properties.size(); ++i) {
        const auto& lhs = existing.properties[i];
        const auto& rhs = properties[i];
        if (lhs.targetLayerType != rhs.targetLayerType || lhs.name != rhs.name || lhs.type != rhs.type ||
            lhs.defaultValue != rhs.defaultValue || lhs.expressionCapabilities != rhs.expressionCapabilities ||
            lhs.supportsTransitions != rhs.supportsTransitions || lhs.minimum != rhs.minimum ||
            lhs.maximum != rhs.maximum || lhs.enumValues != rhs.enumValues) {
            return false;
        }
    }
    for (size_t i = 0; i < layerTypes.size(); ++i) {
        const auto& lhs = existing.layerTypes[i];
        const auto& rhs = layerTypes[i];
        if (lhs.type != rhs.type || lhs.backendMask != rhs.backendMask ||
            lhs.geometryTypeMask != rhs.geometryTypeMask || lhs.createLayout != rhs.createLayout ||
            lhs.layoutFeature != rhs.layoutFeature || lhs.finishLayout != rhs.finishLayout ||
            lhs.destroyLayout != rhs.destroyLayout || lhs.queryFeature != rhs.queryFeature ||
            lhs.queryRadius != rhs.queryRadius || lhs.shaders.size() != rhs.shaders.size() ||
            lhs.updateUniformBlock != rhs.updateUniformBlock) {
            return false;
        }
        for (size_t shaderIndex = 0; shaderIndex < lhs.shaders.size(); ++shaderIndex) {
            const auto& lhsShader = lhs.shaders[shaderIndex];
            const auto& rhsShader = rhs.shaders[shaderIndex];
            if (lhsShader.id != rhsShader.id || lhsShader.sources.size() != rhsShader.sources.size() ||
                lhsShader.attributes.size() != rhsShader.attributes.size() ||
                lhsShader.uniformBlocks.size() != rhsShader.uniformBlocks.size() ||
                lhsShader.propertyBindings != rhsShader.propertyBindings) {
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
                    lu.stageMask != ru.stageMask || lu.bindingID != ru.bindingID) {
                    return false;
                }
            }
        }
    }
    return true;
}

constexpr uint32_t supportedBackends = MLN_PLUGIN_BACKEND_OPENGL | MLN_PLUGIN_BACKEND_VULKAN | MLN_PLUGIN_BACKEND_METAL;

bool validBackendMask(uint32_t mask) {
    return (mask & supportedBackends) != 0 && (mask & ~supportedBackends) == 0;
}

bool validVertexType(mln_plugin_vertex_attribute_type type) {
    return type >= MLN_PLUGIN_VERTEX_INT16 && type <= MLN_PLUGIN_VERTEX_UINT8_X4_NORMALIZED;
}

uint32_t propertyEncodingSize(mln_plugin_property_encoding_v1 encoding) {
    switch (encoding) {
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT:
        case MLN_PLUGIN_PROPERTY_ENCODING_ENUM_FLOAT:
            return 4;
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT2:
            return 8;
        case MLN_PLUGIN_PROPERTY_ENCODING_COLOR:
            return 16;
    }
    return 0;
}

uint32_t propertyEncodingAlignment(mln_plugin_property_encoding_v1 encoding) {
    switch (encoding) {
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT:
        case MLN_PLUGIN_PROPERTY_ENCODING_ENUM_FLOAT:
            return alignof(float);
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT2:
            return 2 * alignof(float);
        case MLN_PLUGIN_PROPERTY_ENCODING_COLOR:
            return 4 * alignof(float);
    }
    return 0;
}

mln_plugin_vertex_attribute_type propertyAttributeType(mln_plugin_property_encoding_v1 encoding) {
    switch (encoding) {
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT:
        case MLN_PLUGIN_PROPERTY_ENCODING_ENUM_FLOAT:
            return MLN_PLUGIN_VERTEX_FLOAT;
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT2:
            return MLN_PLUGIN_VERTEX_FLOAT_X2;
        case MLN_PLUGIN_PROPERTY_ENCODING_COLOR:
            return MLN_PLUGIN_VERTEX_FLOAT_X4;
    }
    return static_cast<mln_plugin_vertex_attribute_type>(0);
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
                attr.attribute_id >= shaders::maxAttributeCountPerShader || attr.location >= 16 ||
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
                (uniform.stage_mask & ~validStages) != 0 || !uniformIDs.emplace(uniform.uniform_id).second ||
                !uniformNames.emplace(copyString(uniform.name)).second) {
                error = "plugin shader uniform block is malformed or duplicated";
                return false;
            }
            uint32_t bindingID = 0;
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
            shader.uniformBlocks.push_back(
                {uniform.uniform_id, copyString(uniform.name), uniform.byte_size, uniform.stage_mask, bindingID});
        }

        if (input.property_binding_count && !input.property_bindings) {
            error = "plugin shader property binding array is missing";
            return false;
        }
        std::set<std::string> boundProperties;
        std::set<uint32_t> boundPaintAttributes;
        std::map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>> hostUniformRanges;
        for (size_t bindingIndex = 0; bindingIndex < input.property_binding_count; ++bindingIndex) {
            const auto& binding = input.property_bindings[bindingIndex];
            const auto propertyName = copyString(binding.property_name);
            const auto encodingSize = propertyEncodingSize(binding.encoding);
            const auto encodingAlignment = propertyEncodingAlignment(binding.encoding);
            const bool packed = binding.minimum_attribute_id == binding.maximum_attribute_id;
            const auto attributeType = packed ? (encodingSize == 4 ? MLN_PLUGIN_VERTEX_FLOAT_X2
                                                                   : MLN_PLUGIN_VERTEX_FLOAT_X4)
                                              : propertyAttributeType(binding.encoding);
            const auto uniform = std::find_if(
                shader.uniformBlocks.begin(), shader.uniformBlocks.end(), [&](const auto& candidate) {
                    return candidate.id == binding.uniform_id;
                });
            const auto interpolationUniform = std::find_if(
                shader.uniformBlocks.begin(), shader.uniformBlocks.end(), [&](const auto& candidate) {
                    return candidate.id == binding.interpolation_uniform_id;
                });
            const auto minimumAttribute = std::find_if(
                shader.attributes.begin(), shader.attributes.end(), [&](const auto& candidate) {
                    return candidate.id == binding.minimum_attribute_id;
                });
            const auto maximumAttribute = std::find_if(
                shader.attributes.begin(), shader.attributes.end(), [&](const auto& candidate) {
                    return candidate.id == binding.maximum_attribute_id;
                });
            if (binding.struct_size < sizeof(mln_plugin_shader_property_binding_v1) || propertyName.empty() ||
                !encodingSize || !boundProperties.emplace(propertyName).second || (packed && encodingSize > 8) ||
                !boundPaintAttributes.emplace(binding.minimum_attribute_id).second ||
                (!packed && !boundPaintAttributes.emplace(binding.maximum_attribute_id).second) ||
                uniform == shader.uniformBlocks.end() || interpolationUniform == shader.uniformBlocks.end() ||
                binding.uniform_byte_offset % encodingAlignment != 0 ||
                binding.interpolation_uniform_byte_offset % alignof(float) != 0 ||
                binding.uniform_byte_offset > uniform->byteSize ||
                encodingSize > uniform->byteSize - binding.uniform_byte_offset ||
                binding.interpolation_uniform_byte_offset > interpolationUniform->byteSize ||
                sizeof(float) > interpolationUniform->byteSize - binding.interpolation_uniform_byte_offset ||
                minimumAttribute == shader.attributes.end() || maximumAttribute == shader.attributes.end() ||
                minimumAttribute->type != attributeType || maximumAttribute->type != attributeType) {
                error = "plugin shader property binding is malformed";
                return false;
            }
            const auto addUniformRange = [&](uint32_t uniformID, uint32_t offset, uint32_t size) {
                auto& ranges = hostUniformRanges[uniformID];
                const auto end = offset + size;
                if (std::any_of(ranges.begin(), ranges.end(), [&](const auto& range) {
                        return offset < range.second && range.first < end;
                    })) {
                    return false;
                }
                ranges.emplace_back(offset, end);
                return true;
            };
            if (!addUniformRange(binding.uniform_id, binding.uniform_byte_offset, encodingSize) ||
                !addUniformRange(
                    binding.interpolation_uniform_id, binding.interpolation_uniform_byte_offset, sizeof(float))) {
                error = "plugin shader property bindings contain overlapping uniform ranges";
                return false;
            }
            shader.propertyBindings.push_back({propertyName,
                                               binding.encoding,
                                               binding.uniform_id,
                                               binding.uniform_byte_offset,
                                               binding.minimum_attribute_id,
                                               binding.maximum_attribute_id,
                                               binding.interpolation_uniform_id,
                                               binding.interpolation_uniform_byte_offset});
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
        constexpr uint32_t validExpressionCapabilities = MLN_PLUGIN_EXPRESSION_CAMERA | MLN_PLUGIN_EXPRESSION_FEATURE |
                                                         MLN_PLUGIN_EXPRESSION_COMPOSITE |
                                                         MLN_PLUGIN_EXPRESSION_FEATURE_STATE;
        if (property.struct_size < sizeof(mln_plugin_property_descriptor_v1) || !validString(property.name) ||
            property.default_value.struct_size < sizeof(mln_plugin_value) ||
            property.default_value.type != property.type ||
            (property.expression_capabilities & ~validExpressionCapabilities) != 0) {
            error = "plugin property descriptor is malformed";
            return false;
        }
        const bool transitionableType = property.type == MLN_PLUGIN_VALUE_FLOAT ||
                                        property.type == MLN_PLUGIN_VALUE_FLOAT2 ||
                                        property.type == MLN_PLUGIN_VALUE_COLOR;
        if (property.supports_transitions && !transitionableType) {
            error = "plugin transitions require an interpolatable paint property";
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
        }
        output.push_back(
            PropertyDefinition{pluginID,
                               targetLayerType,
                               propertyName,
                               property.type,
                               std::move(defaultValue),
                               property.expression_capabilities,
                               property.supports_transitions != 0,
                               property.has_minimum ? std::optional<float>{property.minimum} : std::nullopt,
                               property.has_maximum ? std::optional<float>{property.maximum} : std::nullopt,
                               std::move(enumValues)});
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
        (descriptor.layer_type_count && !descriptor.layer_types) || descriptor.layer_type_count == 0) {
        error = "plugin descriptor is missing an id, version, or layer registration";
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }

    const auto pluginID = copyString(descriptor.plugin_id);
    const auto pluginVersion = copyString(descriptor.plugin_version);
    std::vector<PropertyDefinition> newProperties;
    std::vector<LayerType> newLayerTypes;
    std::set<std::pair<std::string, std::string>> propertyKeys;

    std::set<std::string> layerTypeKeys;
    for (size_t i = 0; i < descriptor.layer_type_count; ++i) {
        const auto& layerType = descriptor.layer_types[i];
        const uint32_t validGeometryMask = MLN_PLUGIN_GEOMETRY_POINT | MLN_PLUGIN_GEOMETRY_LINESTRING |
                                           MLN_PLUGIN_GEOMETRY_POLYGON;
        if (layerType.struct_size < sizeof(mln_plugin_layer_type_v1) || !validString(layerType.layer_type) ||
            !validBackendMask(layerType.backend_mask)) {
            error = "plugin layer type is malformed or has no supported backend";
            return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        }
        if (!layerType.create_layout || !layerType.layout_feature || !layerType.finish_layout ||
            !layerType.destroy_layout || layerType.geometry_type_mask == 0 ||
            (layerType.geometry_type_mask & ~validGeometryMask) != 0) {
            error = "geometry plugin requires source layout callbacks and a valid geometry mask";
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
        copiedLayerType.geometryTypeMask = layerType.geometry_type_mask;
        copiedLayerType.createLayout = layerType.create_layout;
        copiedLayerType.layoutFeature = layerType.layout_feature;
        copiedLayerType.finishLayout = layerType.finish_layout;
        copiedLayerType.destroyLayout = layerType.destroy_layout;
        copiedLayerType.queryFeature = layerType.query_feature;
        copiedLayerType.queryRadius = layerType.get_query_radius;
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
        if (!appendProperties(
                pluginID, type, layerType.properties, layerType.property_count, propertyKeys, newProperties, error)) {
            return error.find("duplicate") != std::string::npos ? MLN_PLUGIN_STATUS_CONFLICT
                                                                : MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        }
        for (const auto& shader : copiedLayerType.shaders) {
            for (const auto& binding : shader.propertyBindings) {
                const auto property = std::find_if(newProperties.begin(), newProperties.end(), [&](const auto& item) {
                    return item.targetLayerType == type && item.name == binding.propertyName;
                });
                const bool encodingMatches = property != newProperties.end() &&
                                             ((property->type == MLN_PLUGIN_VALUE_FLOAT &&
                                               binding.encoding == MLN_PLUGIN_PROPERTY_ENCODING_FLOAT) ||
                                              (property->type == MLN_PLUGIN_VALUE_FLOAT2 &&
                                               binding.encoding == MLN_PLUGIN_PROPERTY_ENCODING_FLOAT2) ||
                                              (property->type == MLN_PLUGIN_VALUE_COLOR &&
                                               binding.encoding == MLN_PLUGIN_PROPERTY_ENCODING_COLOR) ||
                                              (property->type == MLN_PLUGIN_VALUE_STRING &&
                                               !property->enumValues.empty() &&
                                               binding.encoding == MLN_PLUGIN_PROPERTY_ENCODING_ENUM_FLOAT));
                if (property == newProperties.end() || !encodingMatches) {
                    error = "plugin shader property binding references an incompatible paint property";
                    return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
                }
            }
        }
        for (const auto& property : newProperties) {
            if (property.targetLayerType != type) continue;
            const auto dataDependencies = MLN_PLUGIN_EXPRESSION_FEATURE | MLN_PLUGIN_EXPRESSION_COMPOSITE |
                                          MLN_PLUGIN_EXPRESSION_FEATURE_STATE;
            if ((property.expressionCapabilities & dataDependencies) == 0) continue;
            const bool bound = std::any_of(
                copiedLayerType.shaders.begin(), copiedLayerType.shaders.end(), [&](const auto& shader) {
                    return std::any_of(shader.propertyBindings.begin(),
                                       shader.propertyBindings.end(),
                                       [&](const auto& binding) { return binding.propertyName == property.name; });
                });
            if (!bound) {
                error = "data-driven plugin properties require a shader property binding";
                return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
            }
        }
        copiedLayerType.identity = std::make_shared<LayerTypeIdentity>(copiedLayerType);
        newLayerTypes.push_back(std::move(copiedLayerType));
    }

    std::lock_guard<std::mutex> lock(mutex);
    if (const auto existing = plugins.find(pluginID); existing != plugins.end()) {
        if (descriptorEquals(existing->second, pluginVersion, newProperties, newLayerTypes)) {
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

    // Allocate metadata before publishing factories. Lock ordering is registry ->
    // manager, and factory callbacks are never invoked with either lock held.
    std::map<std::string, PluginRecord> pendingPlugins;
    pendingPlugins.emplace(pluginID, PluginRecord{pluginVersion, newProperties, newLayerTypes});
    std::map<std::pair<std::string, std::string>, PropertyDefinition> pendingProperties;
    std::map<std::string, LayerType> pendingTypes;
    std::vector<std::unique_ptr<LayerFactory>> factories;
    for (const auto& type : newLayerTypes) {
        pendingTypes.emplace(type.type, type);
        factories.push_back(std::make_unique<PluginStyleLayerFactory>(type));
    }
    for (const auto& property : newProperties) {
        pendingProperties.emplace(std::make_pair(property.targetLayerType, property.name), property);
    }
    if (!LayerManager::get()->registerLayerFactories(std::move(factories), error)) {
        return MLN_PLUGIN_STATUS_CONFLICT;
    }
    properties.merge(pendingProperties);
    layerTypes.merge(pendingTypes);
    plugins.merge(pendingPlugins);
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

bool PluginRegistry::valueMatches(mln_plugin_value_type type, const Value& value) {
    const auto numeric = [&] {
        return value.getDouble() || value.getInt() || value.getUint();
    };
    switch (type) {
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
} // namespace mln

extern "C" {

mln_plugin_status mln_plugin_register_v1(const mln_plugin_descriptor_v1* descriptor,
                                         char* errorMessage,
                                         size_t errorMessageCapacity) try {
    std::string error;
    const auto status = descriptor ? mln::plugin::PluginRegistry::get().registerPlugin(*descriptor, error)
                                   : MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    if (!descriptor) error = "Plugin descriptor must not be null";
    if (errorMessage && errorMessageCapacity) {
        const auto count = std::min(error.size(), errorMessageCapacity - 1);
        std::memcpy(errorMessage, error.data(), count);
        errorMessage[count] = '\0';
    }
    return status;
} catch (...) {
    // Never unwind through a plugin's C call frame or a different C++ runtime.
    constexpr char message[] = "Plugin registration failed with an internal exception";
    if (errorMessage && errorMessageCapacity) {
        const auto count = std::min(sizeof(message) - 1, errorMessageCapacity - 1);
        std::memcpy(errorMessage, message, count);
        errorMessage[count] = '\0';
    }
    return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
}

} // extern "C"
