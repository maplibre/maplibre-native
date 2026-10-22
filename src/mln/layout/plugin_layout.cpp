#include <mln/layout/plugin_layout.hpp>
#include <mln/plugin/plugin_conversion.hpp>

#include <mln/geometry/feature_index.hpp>
#include <mln/renderer/buckets/plugin_bucket.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/logging.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <set>

namespace mln {
namespace {

bool validRange(std::size_t offset, std::size_t length, std::size_t size) {
    return offset <= size && length <= size - offset;
}

void logFromPlugin(int32_t severity, mln_plugin_string message) {
    const std::string text = message.data ? std::string(message.data, message.size) : std::string{};
    if (severity >= 3) {
        Log::Error(Event::Style, text);
    } else if (severity == 2) {
        Log::Warning(Event::Style, text);
    } else {
        Log::Info(Event::Style, text);
    }
}

const mln_plugin_host_api_v1 hostAPI{sizeof(mln_plugin_host_api_v1), MLN_PLUGIN_ABI_VERSION_1, logFromPlugin};

bool usesResources(const plugin::RegisteredLayer& registration) {
    return std::any_of(registration.properties.begin(), registration.properties.end(), [](const auto& property) {
        return property.type == MLN_PLUGIN_VALUE_RESOURCE;
    });
}

// Whether the layer lays out the feature: a geometry it accepts, passing its filter.
bool laysOut(const plugin::RegisteredLayer& registration,
             const style::PluginStyleLayer::Impl& leader,
             const GeometryTileFeature& feature,
             float zoom,
             const CanonicalTileID& canonical) {
    const auto type = plugin::geometryType(feature.getType());
    return type != 0 && (registration.geometryTypeMask & type) != 0 &&
           leader.filter(style::expression::EvaluationContext(zoom, &feature).withCanonicalTileID(&canonical));
}

// The URL that a RESOURCE value names in the layer's RESOURCES property, or null.
const std::string* resourceURL(const style::PluginStyleLayer::Impl& leader,
                               const plugin::PropertyDefinition& definition,
                               const mln_plugin_value& value) {
    const auto& name = value.data.resource_value.name;
    const auto resources = leader.pluginResources.find(definition.resources);
    if (resources == leader.pluginResources.end() || !name.data) return nullptr;
    const auto url = resources->second.find(std::string(name.data, name.size));
    return url == resources->second.end() ? nullptr : &url->second;
}

// The layout-scope properties in registration order, RESOURCES left out: evaluated once for the tile, and
// again for each feature where they are data-driven. The C values point into this object, so it stays put.
class LayoutPropertyValues {
public:
    LayoutPropertyValues(const plugin::RegisteredLayer& registration,
                         const style::PluginStyleLayer::Impl& leader,
                         float zoom_)
        : zoom(zoom_) {
        for (const auto& definition : registration.properties) {
            if (!definition.layout || definition.type == MLN_PLUGIN_VALUE_RESOURCES) continue;
            const auto stored = leader.pluginProperties.find(definition.name);
            definitions.push_back(&definition);
            values.push_back(stored == leader.pluginProperties.end() ? style::defaultPluginPropertyValue(definition)
                                                                     : stored->second);
            mln_plugin_property_value_v1 property{};
            property.struct_size = sizeof(property);
            property.name = {definition.name.data(), definition.name.size()};
            property.explicitly_set = stored != leader.pluginProperties.end();
            tile.push_back(property);
        }
        tileStorage.resize(values.size());
        featureStorage.resize(values.size());
        for (std::size_t i = 0; i < values.size(); ++i) {
            tile[i].value = values[i].evaluate(zoom, *definitions[i], tileStorage[i]);
        }
        feature = tile;
    }
    LayoutPropertyValues(const LayoutPropertyValues&) = delete;
    LayoutPropertyValues& operator=(const LayoutPropertyValues&) = delete;

    // Updates `feature` for the given feature; values that are not data-driven keep their tile value.
    void evaluate(const GeometryTileFeature& input) {
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (!values[i].isDataDriven()) continue;
            feature[i].value = values[i].evaluate(zoom, input, noFeatureState, *definitions[i], featureStorage[i]);
        }
    }

    std::vector<const plugin::PropertyDefinition*> definitions;
    std::vector<mln_plugin_property_value_v1> tile;
    std::vector<mln_plugin_property_value_v1> feature;

private:
    const float zoom;
    std::vector<style::PluginPropertyValue> values;
    std::vector<style::PluginPropertyValue::EvaluationStorage> tileStorage;
    std::vector<style::PluginPropertyValue::EvaluationStorage> featureStorage;
    const FeatureState noFeatureState;
};

} // namespace

PluginLayout::PluginLayout(const BucketParameters& parameters,
                           std::vector<Immutable<style::LayerProperties>> layers_,
                           std::unique_ptr<GeometryTileLayer> sourceLayer_,
                           plugin::RegisteredLayerPtr registration_,
                           LayoutResourceDependencies* resourceDependencies)
    : zoom(parameters.tileID.overscaledZ),
      layers(std::move(layers_)),
      sourceLayer(std::move(sourceLayer_)),
      registration(std::move(registration_)) {
    if (!sourceLayer || layers.empty() || !usesResources(*registration)) return;

    // The resources the tile's features name, so the worker can load them before createBucket.
    const auto& leader = static_cast<const style::PluginStyleLayer::Impl&>(*layers.front()->baseImpl);
    LayoutPropertyValues properties(*registration, leader, zoom);
    for (std::size_t i = 0; i < sourceLayer->featureCount(); ++i) {
        const auto feature = sourceLayer->getFeature(i);
        if (!feature || !laysOut(*registration, leader, *feature, zoom, parameters.tileID.canonical)) continue;
        properties.evaluate(*feature);
        for (std::size_t p = 0; p < properties.definitions.size(); ++p) {
            if (properties.definitions[p]->type != MLN_PLUGIN_VALUE_RESOURCE) continue;
            if (const auto* url = resourceURL(leader, *properties.definitions[p], properties.feature[p].value)) {
                resourceURLs.insert(*url);
            }
        }
    }
    if (resourceDependencies) resourceDependencies->insert(resourceURLs.begin(), resourceURLs.end());
}

void PluginLayout::prepareResources(const LayoutResourceMap& available) {
    for (const auto& url : resourceURLs) {
        const auto found = available.find(url);
        if (found != available.end()) resources.insert_or_assign(url, found->second);
    }
}

void* PluginLayout::resourceHandle(const std::string& url) {
    const auto found = resources.find(url);
    if (found == resources.end() || !found->second) return nullptr;
    const auto made = found->second->getOrCreate(
        registration.get(), [&](const std::string& bytes) -> std::shared_ptr<void> {
            void* handle = nullptr;
            const auto status = registration->createResource(&hostAPI,
                                                             {url.data(), url.size()},
                                                             reinterpret_cast<const uint8_t*>(bytes.data()),
                                                             bytes.size(),
                                                             &handle);
            if (status != MLN_PLUGIN_STATUS_OK || !handle) {
                if (handle) registration->destroyResource(handle);
                Log::Error(Event::Style,
                           "Plugin layer type '" + registration->type + "' failed to create '" + url + "'");
                return nullptr;
            }
            return std::shared_ptr<void>(handle, registration->destroyResource);
        });
    return made.get();
}

void PluginLayout::createBucket(const ImagePositions&,
                                std::unique_ptr<FeatureIndex>& featureIndex,
                                mln::unordered_map<std::string, LayerRenderData>& renderData,
                                bool,
                                bool,
                                const CanonicalTileID& canonical) {
    if (!sourceLayer || layers.empty()) return;

    const auto& leader = static_cast<const style::PluginStyleLayer::Impl&>(*layers.front()->baseImpl);
    LayoutPropertyValues properties(*registration, leader, zoom);
    mln_plugin_layout_context_v1 context{};
    context.struct_size = sizeof(context);
    context.zoom = zoom;
    context.extent = util::EXTENT;
    context.properties = properties.tile.data();
    context.property_count = properties.tile.size();
    context.host = &hostAPI;

    void* layoutInstance = nullptr;
    const auto createStatus = registration->createLayout(&context, &layoutInstance);
    // Every returned handle transfers to the host, including on callback failure.
    const std::unique_ptr<void, mln_plugin_destroy_layout_fn> layoutOwner(layoutInstance, registration->destroyLayout);
    if (createStatus != MLN_PLUGIN_STATUS_OK || !layoutInstance) {
        Log::Error(Event::Style, "Plugin layer '" + leader.id + "' failed to create a layout instance");
        return;
    }

    // Handles by URL, so each resource is looked up once per tile.
    std::map<std::string, void*> handles;
    for (std::size_t i = 0; i < sourceLayer->featureCount(); ++i) {
        auto feature = sourceLayer->getFeature(i);
        if (!feature || !laysOut(*registration, leader, *feature, zoom, canonical)) continue;

        properties.evaluate(*feature);
        for (std::size_t p = 0; p < properties.definitions.size(); ++p) {
            if (properties.definitions[p]->type != MLN_PLUGIN_VALUE_RESOURCE) continue;
            auto& value = properties.feature[p].value;
            value.data.resource_value.handle = nullptr;
            if (const auto* url = resourceURL(leader, *properties.definitions[p], value)) {
                const auto [handle, added] = handles.emplace(*url, nullptr);
                if (added) handle->second = resourceHandle(*url);
                value.data.resource_value.handle = handle->second;
            }
        }
        plugin::FeatureView pluginFeature(*feature, i);
        pluginFeature.value.layout_properties = properties.feature.data();
        pluginFeature.value.layout_property_count = properties.feature.size();
        if (registration->layoutFeature(layoutInstance, &pluginFeature.value) != MLN_PLUGIN_STATUS_OK) {
            Log::Error(Event::Style, "Plugin layer '" + leader.id + "' failed while laying out a feature");
            return;
        }
        featureIndex->insert(feature->getGeometries(), i, leader.sourceLayer, leader.id);
    }

    mln_plugin_bucket_v1 output{};
    output.struct_size = sizeof(output);
    if (registration->finishLayout(layoutInstance, &output) != MLN_PLUGIN_STATUS_OK) {
        Log::Error(Event::Style, "Plugin layer '" + leader.id + "' failed to finish its bucket");
        return;
    }

    auto bucket = std::make_shared<PluginBucket>(registration);
    std::set<uint32_t> streamIDs;
    bool valid = output.struct_size >= sizeof(output) && std::isfinite(output.query_radius) &&
                 output.query_radius >= 0.0f && (output.vertex_stream_count == 0 || output.vertex_streams) &&
                 (output.index_count == 0 || output.indices) && (output.drawable_count == 0 || output.drawables) &&
                 (output.feature_vertex_range_count == 0 || output.feature_vertex_ranges);
    for (size_t i = 0; valid && i < output.vertex_stream_count; ++i) {
        const auto& stream = output.vertex_streams[i];
        valid = stream.struct_size >= sizeof(stream) && stream.stride > 0 && stream.vertex_count > 0 && stream.data &&
                stream.vertex_count <= std::numeric_limits<size_t>::max() / stream.stride &&
                stream.data_size == static_cast<size_t>(stream.stride) * stream.vertex_count &&
                streamIDs.emplace(stream.stream_id).second;
        if (valid) {
            std::vector<uint8_t> copied(stream.data_size);
            std::memcpy(copied.data(), stream.data, copied.size());
            bucket->vertexStreams.emplace(
                stream.stream_id,
                std::make_shared<PluginVertexVector>(std::move(copied), stream.vertex_count, stream.stride));
        }
    }
    if (valid) {
        std::vector<uint16_t> indices;
        if (output.index_count) indices.assign(output.indices, output.indices + output.index_count);
        bucket->indices = std::make_shared<gfx::IndexVectorBase>(std::move(indices));
    }

    std::set<uint64_t> drawableKeys;
    for (size_t i = 0; valid && i < output.drawable_count; ++i) {
        const auto& input = output.drawables[i];
        valid = input.struct_size >= sizeof(input) && input.shader_id.data && input.shader_id.size &&
                input.attribute_count && input.attributes && input.segment_count && input.segments &&
                drawableKeys.emplace(input.drawable_key).second;
        PluginDrawableDefinition drawable;
        if (!valid) break;
        drawable.key = input.drawable_key;
        drawable.shaderID.assign(input.shader_id.data, input.shader_id.size);
        drawable.depthMode = static_cast<PluginDrawableDepthMode>(input.depth_mode);
        drawable.enableStencilOverlap = input.enable_stencil_overlap != 0;
        drawable.cullBackFaces = input.cull_back_faces != 0;
        const auto shaderIt = std::find_if(registration->shaders.begin(),
                                           registration->shaders.end(),
                                           [&](const auto& shader) { return shader.id == drawable.shaderID; });
        std::set<uint32_t> hostAttributeIDs;
        if (shaderIt != registration->shaders.end()) {
            const auto referencesShaderAttribute = [&](uint32_t attributeID) {
                return std::any_of(shaderIt->attributes.begin(),
                                   shaderIt->attributes.end(),
                                   [&](const auto& attribute) { return attribute.id == attributeID; });
            };
            for (const auto& binding : shaderIt->propertyBindings) {
                if (!referencesShaderAttribute(binding.minimumAttributeID) &&
                    !referencesShaderAttribute(binding.maximumAttributeID)) {
                    continue;
                }
                hostAttributeIDs.emplace(binding.minimumAttributeID);
                hostAttributeIDs.emplace(binding.maximumAttributeID);
            }
        }
        valid = shaderIt != registration->shaders.end() &&
                input.attribute_count + hostAttributeIDs.size() == shaderIt->attributes.size();
        if (!valid) break;
        std::set<uint32_t> boundAttributeIDs;
        std::optional<std::size_t> drawableVertexCount;
        for (size_t bindingIndex = 0; valid && bindingIndex < input.attribute_count; ++bindingIndex) {
            const auto& binding = input.attributes[bindingIndex];
            valid = binding.struct_size >= sizeof(binding);
            if (!valid) break;
            const auto streamIt = bucket->vertexStreams.find(binding.stream_id);
            const auto attributeIt = std::find_if(
                shaderIt->attributes.begin(), shaderIt->attributes.end(), [&](const auto& attribute) {
                    return attribute.id == binding.attribute_id;
                });
            const auto type = attributeIt == shaderIt->attributes.end() ? gfx::AttributeDataType::Invalid
                                                                        : plugin::attributeType(attributeIt->type);
            const auto size = gfx::VertexAttribute::getStrideOf(type);
            valid = streamIt != bucket->vertexStreams.end() && attributeIt != shaderIt->attributes.end() && size > 0 &&
                    hostAttributeIDs.find(binding.attribute_id) == hostAttributeIDs.end() &&
                    binding.byte_offset <= streamIt->second->getRawSize() &&
                    size <= streamIt->second->getRawSize() - binding.byte_offset &&
                    boundAttributeIDs.emplace(binding.attribute_id).second &&
                    (!drawableVertexCount || *drawableVertexCount == streamIt->second->getRawCount());
            if (valid) {
                drawableVertexCount = streamIt->second->getRawCount();
                drawable.attributes.push_back({binding.attribute_id, binding.stream_id, binding.byte_offset, type});
            }
        }
        if (drawableVertexCount) drawable.vertexCount = *drawableVertexCount;
        for (size_t segmentIndex = 0; valid && segmentIndex < input.segment_count; ++segmentIndex) {
            const auto& segment = input.segments[segmentIndex];
            valid = segment.struct_size >= sizeof(segment) &&
                    validRange(segment.index_offset, segment.index_length, output.index_count) && drawableVertexCount &&
                    validRange(segment.vertex_offset, segment.vertex_length, *drawableVertexCount);
            if (valid) {
                for (size_t index = segment.index_offset; index < segment.index_offset + segment.index_length;
                     ++index) {
                    if (output.indices[index] >= segment.vertex_length) {
                        valid = false;
                        break;
                    }
                }
            }
            if (valid) {
                drawable.segments.emplace_back(
                    segment.vertex_offset, segment.index_offset, segment.vertex_length, segment.index_length);
            }
        }
        if (valid) bucket->drawables.push_back(std::move(drawable));
    }

    std::map<uint64_t, std::size_t> drawableVertexCounts;
    for (const auto& drawable : bucket->drawables) drawableVertexCounts.emplace(drawable.key, drawable.vertexCount);
    for (size_t rangeIndex = 0; valid && rangeIndex < output.feature_vertex_range_count; ++rangeIndex) {
        const auto& range = output.feature_vertex_ranges[rangeIndex];
        valid = range.struct_size >= sizeof(range);
        if (!valid) break;
        const auto drawable = drawableVertexCounts.find(range.drawable_key);
        valid = range.feature_index < sourceLayer->featureCount() && drawable != drawableVertexCounts.end() &&
                range.vertex_count > 0 && validRange(range.first_vertex, range.vertex_count, drawable->second);
        if (valid) {
            bucket->featureVertexRanges.push_back({static_cast<std::size_t>(range.feature_index),
                                                   range.drawable_key,
                                                   range.first_vertex,
                                                   range.vertex_count});
        }
    }
    for (const auto& drawable : bucket->drawables) {
        const auto shader = std::find_if(registration->shaders.begin(),
                                         registration->shaders.end(),
                                         [&](const auto& candidate) { return candidate.id == drawable.shaderID; });
        if (!valid || shader == registration->shaders.end() || shader->propertyBindings.empty()) continue;
        std::vector<uint8_t> coverage(drawable.vertexCount);
        for (const auto& range : bucket->featureVertexRanges) {
            if (range.drawableKey != drawable.key) continue;
            for (std::size_t vertex = range.firstVertex; vertex < range.firstVertex + range.vertexCount; ++vertex) {
                if (coverage[vertex] != 0) {
                    valid = false;
                    break;
                }
                coverage[vertex] = 1;
            }
            if (!valid) break;
        }
        if (valid && std::find(coverage.begin(), coverage.end(), 0) != coverage.end()) valid = false;
    }
    if (!valid) {
        Log::Error(Event::Style, "Plugin layer '" + leader.id + "' returned a malformed bucket");
        return;
    }
    bucket->queryRadius = output.query_radius;
    const auto features = std::make_shared<const PluginFeatureData>(bucket->featureVertexRanges,
                                                                    std::move(sourceLayer));
    for (const auto& layer : layers) {
        const auto& impl = static_cast<const style::PluginStyleLayer::Impl&>(*layer->baseImpl);
        auto& layerBinders = bucket->paintPropertyBinders[impl.id];
        for (const auto& drawable : bucket->drawables) {
            const auto shader = std::find_if(registration->shaders.begin(),
                                             registration->shaders.end(),
                                             [&](const auto& candidate) { return candidate.id == drawable.shaderID; });
            if (shader == registration->shaders.end() || shader->propertyBindings.empty()) continue;
            layerBinders.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(drawable.key),
                std::forward_as_tuple(
                    registration, *shader, drawable.key, drawable.vertexCount, zoom, impl.pluginProperties, features));
        }
        bucket->updateQueryRadius(impl.id, impl.pluginProperties, zoom);
    }
    if (!bucket->hasData()) return;

    for (const auto& layer : layers) {
        const auto& impl = static_cast<const style::PluginStyleLayer::Impl&>(*layer->baseImpl);
        renderData.emplace(impl.id, LayerRenderData{.bucket = bucket, .layerProperties = layer});
    }
}

} // namespace mln
