#include <mln/layout/plugin_layout.hpp>
#include <mln/plugin/plugin_conversion.hpp>

#include <mln/geometry/feature_index.hpp>
#include <mln/renderer/buckets/plugin_bucket.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/storage/file_source.hpp>
#include <mln/storage/resource.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/run_loop.hpp>

#include <cmath>
#include <cstring>
#include <limits>
#include <set>
#include <thread>

namespace mln {
namespace {

bool validRange(std::size_t offset, std::size_t length, std::size_t size) {
    return offset <= size && length <= size - offset;
}

// Services for layout callbacks. Layout runs on a tile worker, so a FileSource
// request started there would wait on that same scheduler. Each request gets a
// short-lived thread with its own run loop, and its result is delivered to the
// callback on the layout thread before request_resource returns.
class LayoutResourceHost {
public:
    explicit LayoutResourceHost(std::shared_ptr<FileSource> fileSource_)
        : fileSource(std::move(fileSource_)) {
        api.struct_size = sizeof(api);
        api.abi_version = MLN_PLUGIN_ABI_VERSION_1;
        api.log = log;
        api.context = this;
        api.request_resource = requestResource;
        api.cancel_resource_request = cancelResourceRequest;
    }

    const mln_plugin_host_api_v1* getAPI() const noexcept { return &api; }

private:
    static void log(int32_t severity, mln_plugin_string message) {
        const std::string text = message.data ? std::string(message.data, message.size) : std::string{};
        if (severity >= 3) {
            Log::Error(Event::Style, text);
        } else if (severity == 2) {
            Log::Warning(Event::Style, text);
        } else {
            Log::Info(Event::Style, text);
        }
    }

    static mln_plugin_status requestResource(void* context,
                                             mln_plugin_string url,
                                             mln_plugin_resource_callback_fn callback,
                                             void* callbackContext,
                                             uint64_t* requestID) {
        auto* self = static_cast<LayoutResourceHost*>(context);
        if (!self || !self->fileSource || !url.data || !url.size || !callback || !requestID) {
            return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
        }
        const auto id = self->nextRequestID++;
        *requestID = id;
        const std::string resourceURL(url.data, url.size);
        Response response;
        bool receivedResponse = false;
        const auto fileSource = self->fileSource;
        std::thread loader([&] {
            util::RunLoop runLoop(util::RunLoop::Type::New);
            auto request = fileSource->request(Resource(Resource::Kind::Unknown, resourceURL), [&](Response result) {
                response = std::move(result);
                receivedResponse = true;
                runLoop.stop();
            });
            if (!request) return;
            runLoop.run();
        });
        loader.join();
        if (!receivedResponse) return MLN_PLUGIN_STATUS_CALLBACK_ERROR;

        std::string error;
        if (response.error) error = response.error->message;
        mln_plugin_resource_response_v1 result{};
        result.struct_size = sizeof(result);
        result.request_id = id;
        if (response.data) {
            result.data = reinterpret_cast<const uint8_t*>(response.data->data());
            result.data_size = response.data->size();
        }
        result.error_message = {error.data(), error.size()};
        callback(callbackContext, &result);
        return MLN_PLUGIN_STATUS_OK;
    }

    static void cancelResourceRequest(void*, uint64_t) {}

    mln_plugin_host_api_v1 api{};
    std::shared_ptr<FileSource> fileSource;
    uint64_t nextRequestID = 1;
};

} // namespace

PluginLayout::PluginLayout(const BucketParameters& parameters,
                           std::vector<Immutable<style::LayerProperties>> layers_,
                           std::unique_ptr<GeometryTileLayer> sourceLayer_,
                           plugin::RegisteredLayerPtr registration_)
    : zoom(parameters.tileID.overscaledZ),
      fileSource(parameters.fileSource),
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

    // Layout-scope properties are constants; expressions are not evaluated here.
    LayoutResourceHost resourceHost(fileSource);
    std::vector<style::PluginPropertyValue> layoutValues;
    std::vector<style::PluginPropertyValue::EvaluationStorage> layoutStorage(registration->properties.size());
    std::vector<mln_plugin_property_value_v1> layoutProperties;
    layoutValues.reserve(registration->properties.size());
    layoutProperties.reserve(registration->properties.size());
    for (std::size_t i = 0; i < registration->properties.size(); ++i) {
        const auto& definition = registration->properties[i];
        if (!definition.layout) continue;
        const auto stored = leader.pluginProperties.find(definition.name);
        layoutValues.push_back(stored == leader.pluginProperties.end() ? style::defaultPluginPropertyValue(definition)
                                                                       : stored->second);
        mln_plugin_property_value_v1 property{};
        property.struct_size = sizeof(property);
        property.name = {definition.name.data(), definition.name.size()};
        property.value = layoutValues.back().evaluate(zoom, definition, layoutStorage[i]);
        property.explicitly_set = stored != leader.pluginProperties.end();
        layoutProperties.push_back(property);
    }
    context.properties = layoutProperties.data();
    context.property_count = layoutProperties.size();
    context.host = resourceHost.getAPI();

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
