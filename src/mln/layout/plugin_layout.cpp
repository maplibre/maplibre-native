#include <mln/layout/plugin_layout.hpp>

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

mln_plugin_geometry_type geometryType(FeatureType type) {
    switch (type) {
        case FeatureType::Point:
            return MLN_PLUGIN_GEOMETRY_POINT;
        case FeatureType::LineString:
            return MLN_PLUGIN_GEOMETRY_LINESTRING;
        case FeatureType::Polygon:
            return MLN_PLUGIN_GEOMETRY_POLYGON;
        case FeatureType::Unknown:
            return static_cast<mln_plugin_geometry_type>(0);
    }
    return static_cast<mln_plugin_geometry_type>(0);
}

bool validRange(std::size_t offset, std::size_t length, std::size_t size) {
    return offset <= size && length <= size - offset;
}

std::size_t attributeSize(mln_plugin_vertex_attribute_type type) {
    switch (type) {
        case MLN_PLUGIN_VERTEX_INT16:
        case MLN_PLUGIN_VERTEX_UINT16:
            return 2;
        case MLN_PLUGIN_VERTEX_INT16_X2:
        case MLN_PLUGIN_VERTEX_UINT16_X2:
        case MLN_PLUGIN_VERTEX_FLOAT:
        case MLN_PLUGIN_VERTEX_UINT8_X4_NORMALIZED:
            return 4;
        case MLN_PLUGIN_VERTEX_FLOAT_X2:
            return 8;
        case MLN_PLUGIN_VERTEX_FLOAT_X3:
            return 12;
        case MLN_PLUGIN_VERTEX_FLOAT_X4:
            return 16;
    }
    return 0;
}

} // namespace

PluginLayout::PluginLayout(const BucketParameters& parameters,
                           std::vector<Immutable<style::LayerProperties>> layers_,
                           std::unique_ptr<GeometryTileLayer> sourceLayer_,
                           plugin::LayerType registration_)
    : tileID(parameters.tileID),
      zoom(parameters.tileID.overscaledZ),
      layers(std::move(layers_)),
      sourceLayer(std::move(sourceLayer_)),
      registration(std::move(registration_)) {}

void PluginLayout::createBucket(const ImagePositions&,
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

    void* layoutInstance = nullptr;
    const auto createStatus = registration.createLayout(&context, &layoutInstance);
    // Every returned handle transfers to the host, including on callback failure.
    const std::unique_ptr<void, mln_plugin_destroy_layout_fn> layoutOwner(layoutInstance, registration.destroyLayout);
    if (createStatus != MLN_PLUGIN_STATUS_OK || !layoutInstance) {
        Log::Error(Event::Style, "Plugin layer '" + leader.id + "' failed to create a layout instance");
        return;
    }

    for (std::size_t i = 0; i < sourceLayer->featureCount(); ++i) {
        auto feature = sourceLayer->getFeature(i);
        if (!feature) continue;
        const auto type = geometryType(feature->getType());
        if (type == 0 || (registration.geometryTypeMask & type) == 0) continue;
        if (!leader.filter(style::expression::EvaluationContext(zoom, feature.get()).withCanonicalTileID(&canonical))) {
            continue;
        }

        const auto& geometry = feature->getGeometries();
        std::vector<mln_plugin_tile_point_v1> points;
        std::vector<uint32_t> offsets;
        offsets.reserve(geometry.size() + 1);
        offsets.push_back(0);
        for (const auto& path : geometry) {
            points.reserve(points.size() + path.size());
            for (const auto& point : path) points.push_back({point.x, point.y});
            offsets.push_back(static_cast<uint32_t>(points.size()));
        }

        mln_plugin_feature_v1 pluginFeature{};
        pluginFeature.struct_size = sizeof(pluginFeature);
        pluginFeature.geometry_type = type;
        pluginFeature.feature_index = i;
        pluginFeature.points = points.data();
        pluginFeature.point_count = points.size();
        pluginFeature.path_offsets = offsets.data();
        pluginFeature.path_count = geometry.size();
        if (registration.layoutFeature(layoutInstance, &pluginFeature) != MLN_PLUGIN_STATUS_OK) {
            Log::Error(Event::Style, "Plugin layer '" + leader.id + "' failed while laying out a feature");
            return;
        }
        featureIndex->insert(geometry, i, leader.sourceLayer, leader.id);
    }

    mln_plugin_bucket_v1 output{};
    output.struct_size = sizeof(output);
    if (registration.finishLayout(layoutInstance, &output) != MLN_PLUGIN_STATUS_OK) {
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
        const auto shaderIt = std::find_if(registration.shaders.begin(),
                                           registration.shaders.end(),
                                           [&](const auto& shader) { return shader.id == drawable.shaderID; });
        std::set<uint32_t> hostAttributeIDs;
        if (shaderIt != registration.shaders.end()) {
            for (const auto& binding : shaderIt->propertyBindings) {
                hostAttributeIDs.emplace(binding.minimumAttributeID);
                hostAttributeIDs.emplace(binding.maximumAttributeID);
            }
        }
        valid = shaderIt != registration.shaders.end() &&
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
            const auto size = attributeSize(binding.type);
            valid = streamIt != bucket->vertexStreams.end() && attributeIt != shaderIt->attributes.end() &&
                    attributeIt->type == binding.type && size > 0 &&
                    hostAttributeIDs.find(binding.attribute_id) == hostAttributeIDs.end() &&
                    binding.byte_offset <= streamIt->second->getRawSize() &&
                    size <= streamIt->second->getRawSize() - binding.byte_offset &&
                    boundAttributeIDs.emplace(binding.attribute_id).second &&
                    (!drawableVertexCount || *drawableVertexCount == streamIt->second->getRawCount());
            if (valid) {
                drawableVertexCount = streamIt->second->getRawCount();
                drawable.attributes.push_back(
                    {binding.attribute_id, binding.stream_id, binding.byte_offset, binding.type});
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
        const auto shader = std::find_if(registration.shaders.begin(),
                                         registration.shaders.end(),
                                         [&](const auto& candidate) { return candidate.id == drawable.shaderID; });
        if (!valid || shader == registration.shaders.end() || shader->propertyBindings.empty()) continue;
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
    for (const auto& layer : layers) {
        const auto& impl = static_cast<const style::PluginStyleLayer::Impl&>(*layer->baseImpl);
        auto& layerBinders = bucket->paintPropertyBinders[impl.id];
        for (const auto& drawable : bucket->drawables) {
            const auto shader = std::find_if(registration.shaders.begin(),
                                             registration.shaders.end(),
                                             [&](const auto& candidate) { return candidate.id == drawable.shaderID; });
            if (shader == registration.shaders.end() || shader->propertyBindings.empty()) continue;
            layerBinders.emplace(std::piecewise_construct,
                                 std::forward_as_tuple(drawable.key),
                                 std::forward_as_tuple(registration,
                                                       *shader,
                                                       drawable.key,
                                                       drawable.vertexCount,
                                                       zoom,
                                                       impl.pluginProperties,
                                                       bucket->featureVertexRanges,
                                                       *sourceLayer));
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
