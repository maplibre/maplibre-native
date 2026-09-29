#include <mln/layout/plugin_layout.hpp>
#include <mln/plugin/plugin_conversion.hpp>

#include <mln/geometry/feature_index.hpp>
#include <mln/renderer/buckets/plugin_bucket.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/logging.hpp>

#include <cmath>
#include <cstring>
#include <limits>
#include <set>

namespace mln {
namespace {

bool validRange(std::size_t offset, std::size_t length, std::size_t size) {
    return offset <= size && length <= size - offset;
}

} // namespace

PluginLayout::PluginLayout(const BucketParameters& parameters,
                           std::vector<Immutable<style::LayerProperties>> layers_,
                           std::unique_ptr<GeometryTileLayer> sourceLayer_,
                           plugin::RegisteredLayerPtr registration_,
                           const LayoutParameters* layoutParameters)
    : zoom(parameters.tileID.overscaledZ),
      layers(std::move(layers_)),
      sourceLayer(std::move(sourceLayer_)),
      registration(std::move(registration_)) {
    if (!layoutParameters || !sourceLayer) return;
    for (const auto& layer : layers) {
        const auto& impl = static_cast<const style::PluginStyleLayer::Impl&>(*layer->baseImpl);
        for (const auto& definition : registration->properties) {
            if (definition.type != MLN_PLUGIN_VALUE_IMAGE) continue;
            const auto it = impl.pluginProperties.find(definition.name);
            if (it == impl.pluginProperties.end()) continue;
            needsImages = true;
            const auto& value = it->second;
            style::PluginPropertyValue::EvaluationStorage storage;
            const auto add = [&](const mln_plugin_value& image) {
                if (image.data.string_value.size)
                    layoutParameters->imageDependencies.emplace(
                        std::string(image.data.string_value.data, image.data.string_value.size), ImageType::Pattern);
            };
            for (float z : {zoom - 1, zoom, zoom + 1}) {
                if (!value.isDataDriven())
                    add(value.evaluate(z, definition, storage, &layoutParameters->availableImages));
                else
                    for (size_t i = 0; i < sourceLayer->featureCount(); ++i) {
                        const auto feature = sourceLayer->getFeature(i);
                        if (feature)
                            add(value.evaluate(z + (value.isZoomConstant() ? 0 : 1),
                                               *feature,
                                               {},
                                               definition,
                                               storage,
                                               &layoutParameters->availableImages));
                    }
            }
        }
    }
}

void PluginLayout::createBucket(const ImagePositions& imagePositions,
                                std::unique_ptr<FeatureIndex>& featureIndex,
                                mln::unordered_map<std::string, LayerRenderData>& renderData,
                                bool,
                                bool,
                                const CanonicalTileID& canonical) {
    if (!sourceLayer || layers.empty()) return;

    const auto& leader = static_cast<const style::PluginStyleLayer::Impl&>(*layers.front()->baseImpl);
    mln_plugin_layout_context_v1 context{};
    context.struct_size = sizeof(context);
    context.zoom = zoom;
    context.extent = util::EXTENT;
    std::vector<mln_plugin_property_value_v1> layoutProperties;
    std::vector<style::PluginPropertyValue::EvaluationStorage> storage(registration->properties.size());
    for (size_t i = 0; i < registration->properties.size(); ++i) {
        const auto& definition = registration->properties[i];
        if (!definition.isLayout) continue;
        const auto it = leader.pluginProperties.find(definition.name);
        const auto value = it == leader.pluginProperties.end() ? style::defaultPluginPropertyValue(definition)
                                                               : it->second;
        layoutProperties.push_back({sizeof(mln_plugin_property_value_v1),
                                    {definition.name.data(), definition.name.size()},
                                    value.evaluate(zoom, definition, storage[i]),
                                    static_cast<uint8_t>(it != leader.pluginProperties.end())});
    }
    context.properties = layoutProperties.data();
    context.property_count = layoutProperties.size();

    void* layoutInstance = nullptr;
    const auto createStatus = registration->createLayout(&context, &layoutInstance);
    // Every returned handle transfers to the host, including on callback failure.
    const std::unique_ptr<void, mln_plugin_destroy_layout_fn> layoutOwner(layoutInstance, registration->destroyLayout);
    if (createStatus != MLN_PLUGIN_STATUS_OK || !layoutInstance) {
        Log::Error(Event::Style, "Plugin layer '" + leader.id + "' failed to create a layout instance");
        return;
    }

    for (std::size_t i = 0; i < sourceLayer->featureCount(); ++i) {
        auto feature = sourceLayer->getFeature(i);
        if (!feature) continue;
        const auto type = plugin::geometryType(feature->getType());
        if (type == 0 || (registration->geometryTypeMask & type) == 0) continue;
        if (!leader.filter(style::expression::EvaluationContext(zoom, feature.get()).withCanonicalTileID(&canonical))) {
            continue;
        }

        const plugin::FeatureView pluginFeature(*feature, i);
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
    const auto positions = std::make_shared<const ImagePositions>(imagePositions);
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
        const auto shaderIt = std::find_if(registration->shaders.begin(),
                                           registration->shaders.end(),
                                           [&](const auto& shader) { return shader.id == drawable.shaderID; });
        std::set<uint32_t> hostAttributeIDs;
        if (shaderIt != registration->shaders.end()) {
            for (const auto& binding : shaderIt->propertyBindings) {
                hostAttributeIDs.emplace(binding.minimumAttributeID);
                hostAttributeIDs.emplace(binding.maximumAttributeID);
            }
        }
        valid = shaderIt != registration->shaders.end() &&
                input.attribute_count + hostAttributeIDs.size() == shaderIt->attributes.size();
        if (!valid) break;
        std::set<uint32_t> boundAttributeIDs;
        std::optional<std::size_t> drawableVertexCount;
        std::size_t availableRecords = std::numeric_limits<std::size_t>::max();
        drawable.instanced = shaderIt->instanced;
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
                    binding.element_offset < streamIt->second->getRawCount() &&
                    boundAttributeIDs.emplace(binding.attribute_id).second &&
                    (!drawableVertexCount || *drawableVertexCount == streamIt->second->getRawCount());
            if (valid) {
                drawableVertexCount = streamIt->second->getRawCount();
                availableRecords = std::min(availableRecords, *drawableVertexCount - binding.element_offset);
                drawable.attributes.push_back(
                    {binding.attribute_id, binding.stream_id, binding.byte_offset, type, binding.element_offset});
            }
        }
        if (drawableVertexCount) drawable.vertexCount = *drawableVertexCount;
        for (size_t segmentIndex = 0; valid && segmentIndex < input.segment_count; ++segmentIndex) {
            const auto& segment = input.segments[segmentIndex];
            valid = segment.struct_size >= sizeof(segment) &&
                    validRange(segment.index_offset, segment.index_length, output.index_count) && drawableVertexCount &&
                    segment.vertex_length > 0 && segment.vertex_offset <= INT32_MAX &&
                    segment.vertex_length <= uint32_t(INT32_MAX) - segment.vertex_offset &&
                    (drawable.instanced
                         ? segment.instance_count > 0 &&
                               validRange(segment.first_instance, segment.instance_count, availableRecords)
                         : segment.first_instance == 0 && segment.instance_count == 0 &&
                               validRange(segment.vertex_offset, segment.vertex_length, availableRecords));
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
                drawable.primitiveVertexCount = std::max(drawable.primitiveVertexCount,
                                                         size_t(segment.vertex_offset) + segment.vertex_length);
                drawable.segments.emplace_back(segment.vertex_offset,
                                               segment.index_offset,
                                               segment.vertex_length,
                                               segment.index_length,
                                               segment.first_instance,
                                               drawable.instanced ? segment.instance_count : 1);
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
            // Roof and wall drawables can address identical feature records at
            // different input rates. Share their paint buffers and update once.
            std::shared_ptr<PluginPaintPropertyBinders> binders;
            for (const auto& candidate : bucket->drawables) {
                const auto existing = layerBinders.find(candidate.key);
                if (existing == layerBinders.end() || candidate.vertexCount != drawable.vertexCount) continue;
                const auto candidateShader = std::find_if(
                    registration->shaders.begin(), registration->shaders.end(), [&](const auto& item) {
                        return item.id == candidate.shaderID;
                    });
                if (candidateShader != registration->shaders.end() &&
                    candidateShader->propertyBindings == shader->propertyBindings &&
                    features->drawable(candidate.key).ranges == features->drawable(drawable.key).ranges) {
                    binders = existing->second;
                    break;
                }
            }
            if (!binders) {
                binders = std::make_shared<PluginPaintPropertyBinders>(
                    registration, *shader, drawable.key, drawable.vertexCount, zoom, impl.pluginProperties, features);
                binders->setPatternPositions(positions);
            }
            layerBinders.emplace(drawable.key, std::move(binders));
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
