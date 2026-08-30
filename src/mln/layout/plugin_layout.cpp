#include <mln/layout/plugin_layout.hpp>

#include <mln/geometry/feature_index.hpp>
#include <mln/renderer/buckets/plugin_bucket.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/storage/file_source.hpp>
#include <mln/storage/resource.hpp>
#include <mln/style/conversion/stringify.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/rapidjson.hpp>
#include <mln/util/run_loop.hpp>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <cstring>
#include <limits>
#include <set>
#include <thread>

namespace mln {
namespace {

mln_plugin_string borrowed(const std::string& value) {
    return {value.data(), value.size()};
}

std::string featurePropertiesJSON(const GeometryTileFeature& feature) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    style::conversion::stringify(writer, feature.getProperties());
    return {buffer.GetString(), buffer.GetSize()};
}

std::string valueJSON(const Value& value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    style::conversion::stringify(writer, value);
    return {buffer.GetString(), buffer.GetSize()};
}

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

bool validDrawMode(mln_plugin_draw_mode mode) {
    return mode == MLN_PLUGIN_DRAW_MODE_TRIANGLES || mode == MLN_PLUGIN_DRAW_MODE_LINES ||
           mode == MLN_PLUGIN_DRAW_MODE_POINTS;
}

bool validDepthMode(mln_plugin_depth_mode mode) {
    return mode >= MLN_PLUGIN_DEPTH_DISABLED && mode <= MLN_PLUGIN_DEPTH_READ_WRITE;
}

bool validBlendMode(mln_plugin_blend_mode mode) {
    return mode >= MLN_PLUGIN_BLEND_REPLACE && mode <= MLN_PLUGIN_BLEND_ADDITIVE;
}

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
        api.request_repaint = requestRepaint;
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
        auto fileSource = self->fileSource;
        // FileSource delivers onto the scheduler that starts a request. Layout
        // callbacks already run on a tile worker, so waiting for a request
        // started there would deadlock that scheduler. Give the resource its
        // own short-lived run loop and return its result on the layout thread.
        std::thread loader([&] {
            // A layout request runs on its own worker thread. On libuv
            // platforms the default RunLoop is process-global and may already
            // be active on the renderer thread, so this bridge needs an
            // independent loop to receive the FileSource callback.
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

    static void requestRepaint(void*) {}

    mln_plugin_host_api_v1 api{};
    std::shared_ptr<FileSource> fileSource;
    uint64_t nextRequestID = 1;
};

} // namespace

PluginLayout::PluginLayout(const BucketParameters& parameters,
                           std::vector<Immutable<style::LayerProperties>> layers_,
                           std::unique_ptr<GeometryTileLayer> sourceLayer_,
                           plugin::LayerType registration_)
    : tileID(parameters.tileID),
      zoom(parameters.tileID.overscaledZ),
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
    const auto propertyDefinitions = plugin::PluginRegistry::get().propertiesForLayer(registration.type);
    std::vector<mln_plugin_property_value_v1> properties;
    std::vector<std::string> propertyStrings(propertyDefinitions.size());
    std::vector<std::vector<float>> propertyFloatArrays(propertyDefinitions.size());
    std::vector<std::vector<mln_plugin_color>> propertyColorArrays(propertyDefinitions.size());
    properties.reserve(propertyDefinitions.size());
    for (size_t propertyIndex = 0; propertyIndex < propertyDefinitions.size(); ++propertyIndex) {
        const auto& definition = propertyDefinitions[propertyIndex];
        mln_plugin_property_value_v1 property{};
        property.struct_size = sizeof(property);
        property.name = borrowed(definition.name);
        property.value.struct_size = sizeof(property.value);
        property.value.type = definition.type;
        const auto it = leader.pluginProperties.find(definition.name);
        const auto styleProperty = it == leader.pluginProperties.end() ? style::StyleProperty{}
                                                                       : it->second.toStyleProperty();
        const auto& value = styleProperty.getKind() == style::StyleProperty::Kind::Constant ? styleProperty.getValue()
                                                                                            : definition.defaultValue;
        property.explicitly_set = it != leader.pluginProperties.end();
        switch (definition.type) {
            case MLN_PLUGIN_VALUE_BOOLEAN:
                property.value.data.boolean_value = value.getBool() && *value.getBool();
                break;
            case MLN_PLUGIN_VALUE_FLOAT:
                property.value.data.float_value = static_cast<float>(numericValue<double>(value).value_or(0.0));
                break;
            case MLN_PLUGIN_VALUE_FLOAT2:
            case MLN_PLUGIN_VALUE_COLOR: {
                const auto* array = value.getArray();
                const auto number = [&](size_t index) {
                    return array && index < array->size() ? numericValue<float>((*array)[index]).value_or(0.0f) : 0.0f;
                };
                if (definition.type == MLN_PLUGIN_VALUE_FLOAT2) {
                    property.value.data.float2_value = {number(0), number(1)};
                } else {
                    property.value.data.color_value = {number(0), number(1), number(2), number(3)};
                }
                break;
            }
            case MLN_PLUGIN_VALUE_STRING: {
                const auto* string = value.getString();
                if (string) propertyStrings[propertyIndex] = *string;
                property.value.data.string_value = borrowed(propertyStrings[propertyIndex]);
                break;
            }
            case MLN_PLUGIN_VALUE_FLOAT_ARRAY: {
                const auto* array = value.getArray();
                auto& output = propertyFloatArrays[propertyIndex];
                if (array) {
                    output.reserve(array->size());
                    for (const auto& item : *array) {
                        output.push_back(numericValue<float>(item).value_or(0.0f));
                    }
                }
                property.value.data.float_array_value = {output.data(), output.size()};
                break;
            }
            case MLN_PLUGIN_VALUE_COLOR_ARRAY: {
                const auto* array = value.getArray();
                auto& output = propertyColorArrays[propertyIndex];
                if (array) {
                    output.reserve(array->size());
                    for (const auto& item : *array) {
                        const auto* components = item.getArray();
                        if (!components || components->size() != 4) continue;
                        output.push_back({numericValue<float>((*components)[0]).value_or(0.0f),
                                          numericValue<float>((*components)[1]).value_or(0.0f),
                                          numericValue<float>((*components)[2]).value_or(0.0f),
                                          numericValue<float>((*components)[3]).value_or(0.0f)});
                    }
                }
                property.value.data.color_array_value = {output.data(), output.size()};
                break;
            }
            case MLN_PLUGIN_VALUE_COLOR_RAMP:
                propertyStrings[propertyIndex] = valueJSON(value);
                property.value.data.color_ramp_json = borrowed(propertyStrings[propertyIndex]);
                break;
        }
        properties.push_back(property);
    }

    LayoutResourceHost resourceHost(fileSource);
    mln_plugin_layout_context_v1 context{};
    context.struct_size = sizeof(context);
    context.zoom = zoom;
    context.canonical_z = canonical.z;
    context.canonical_x = canonical.x;
    context.canonical_y = canonical.y;
    context.extent = util::EXTENT;
    context.layer_id = borrowed(leader.id);
    context.source_layer_id = borrowed(leader.sourceLayer);
    context.properties = properties.data();
    context.property_count = properties.size();
    context.host = resourceHost.getAPI();

    void* layoutInstance = nullptr;
    if (registration.createLayout(&context, &layoutInstance) != MLN_PLUGIN_STATUS_OK || !layoutInstance) {
        Log::Error(Event::Style, "Plugin layer '" + leader.id + "' failed to create a layout instance");
        return;
    }
    const auto destroyLayout = [&] {
        registration.destroyLayout(layoutInstance);
    };

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

        const auto id = featureIDtoString(feature->getID()).value_or(std::string{});
        const auto propertiesJSON = featurePropertiesJSON(*feature);
        std::vector<mln_plugin_property_value_v1> evaluatedProperties;
        std::vector<style::PluginPropertyValue::EvaluationStorage> evaluatedStorage(propertyDefinitions.size());
        evaluatedProperties.reserve(propertyDefinitions.size());
        const FeatureState emptyState;
        for (size_t propertyIndex = 0; propertyIndex < propertyDefinitions.size(); ++propertyIndex) {
            const auto& definition = propertyDefinitions[propertyIndex];
            const auto propertyIt = leader.pluginProperties.find(definition.name);
            const auto value = propertyIt == leader.pluginProperties.end()
                                   ? style::defaultPluginPropertyValue(definition)
                                   : propertyIt->second;
            mln_plugin_property_value_v1 evaluated{};
            evaluated.struct_size = sizeof(evaluated);
            evaluated.name = borrowed(definition.name);
            evaluated.value = value.evaluate(zoom, *feature, emptyState, definition, evaluatedStorage[propertyIndex]);
            evaluated.explicitly_set = propertyIt != leader.pluginProperties.end();
            evaluatedProperties.push_back(evaluated);
        }
        mln_plugin_feature_v1 pluginFeature{};
        pluginFeature.struct_size = sizeof(pluginFeature);
        pluginFeature.geometry_type = type;
        pluginFeature.feature_index = i;
        pluginFeature.feature_id = borrowed(id);
        pluginFeature.points = points.data();
        pluginFeature.point_count = points.size();
        pluginFeature.path_offsets = offsets.data();
        pluginFeature.path_count = geometry.size();
        pluginFeature.properties_json = borrowed(propertiesJSON);
        pluginFeature.evaluated_properties = evaluatedProperties.data();
        pluginFeature.evaluated_property_count = evaluatedProperties.size();
        if (registration.layoutFeature(layoutInstance, &pluginFeature) != MLN_PLUGIN_STATUS_OK) {
            Log::Error(Event::Style, "Plugin layer '" + leader.id + "' failed while laying out a feature");
            destroyLayout();
            return;
        }
        featureIndex->insert(geometry, i, leader.sourceLayer, leader.id);
    }

    mln_plugin_bucket_v1 output{};
    output.struct_size = sizeof(output);
    if (registration.finishLayout(layoutInstance, &output) != MLN_PLUGIN_STATUS_OK) {
        Log::Error(Event::Style, "Plugin layer '" + leader.id + "' failed to finish its bucket");
        destroyLayout();
        return;
    }

    auto bucket = std::make_shared<PluginBucket>(registration);
    std::set<uint32_t> streamIDs;
    bool valid = output.query_radius >= 0.0f && (output.vertex_stream_count == 0 || output.vertex_streams) &&
                 (output.index_count == 0 || output.indices) && (output.drawable_count == 0 || output.drawables);
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
                input.render_stage == registration.renderStage && validDrawMode(input.draw_mode) &&
                validDepthMode(input.depth_mode) && validBlendMode(input.blend_mode) &&
                drawableKeys.emplace(input.drawable_key).second;
        PluginDrawableDefinition drawable;
        if (!valid) break;
        drawable.key = input.drawable_key;
        drawable.shaderID.assign(input.shader_id.data, input.shader_id.size);
        const auto shaderIt = std::find_if(registration.shaders.begin(),
                                           registration.shaders.end(),
                                           [&](const auto& shader) { return shader.id == drawable.shaderID; });
        valid = shaderIt != registration.shaders.end() && input.attribute_count == shaderIt->attributes.size();
        if (!valid) break;
        drawable.drawMode = input.draw_mode;
        drawable.renderStage = input.render_stage;
        drawable.depthMode = input.depth_mode;
        drawable.blendMode = input.blend_mode;
        drawable.enableStencil = input.enable_stencil != 0;
        drawable.enableCullFace = input.enable_cull_face != 0;
        std::set<uint32_t> boundAttributeIDs;
        std::optional<std::size_t> drawableVertexCount;
        for (size_t bindingIndex = 0; valid && bindingIndex < input.attribute_count; ++bindingIndex) {
            const auto& binding = input.attributes[bindingIndex];
            const auto streamIt = bucket->vertexStreams.find(binding.stream_id);
            const auto attributeIt = std::find_if(
                shaderIt->attributes.begin(), shaderIt->attributes.end(), [&](const auto& attribute) {
                    return attribute.id == binding.attribute_id;
                });
            const auto size = attributeSize(binding.type);
            valid = binding.struct_size >= sizeof(binding) && streamIt != bucket->vertexStreams.end() &&
                    attributeIt != shaderIt->attributes.end() && attributeIt->type == binding.type && size > 0 &&
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
    bucket->queryRadius = output.query_radius;
    destroyLayout();

    if (!valid) {
        Log::Error(Event::Style, "Plugin layer '" + leader.id + "' returned a malformed bucket");
        return;
    }
    if (!bucket->hasData()) return;

    for (const auto& layer : layers) {
        const auto& impl = static_cast<const style::PluginStyleLayer::Impl&>(*layer->baseImpl);
        renderData.emplace(impl.id, LayerRenderData{.bucket = bucket, .layerProperties = layer});
    }
}

} // namespace mln
