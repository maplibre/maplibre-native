#include <mln/renderer/buckets/plugin_bucket.hpp>

#include <mln/plugin/plugin_registry.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/plugin_property.hpp>
#include <mln/tile/geometry_tile_data.hpp>
#include <mln/util/logging.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>

namespace mln {
namespace {

class PluginFeatureSnapshot final : public GeometryTileFeature {
public:
    PluginFeatureSnapshot(FeatureType type_, FeatureIdentifier id_, PropertyMap properties_)
        : type(type_), id(std::move(id_)), properties(std::move(properties_)) {}

    FeatureType getType() const override { return type; }
    std::optional<Value> getValue(const std::string& key) const override {
        const auto it = properties.find(key);
        return it == properties.end() ? std::nullopt : std::optional<Value>{it->second};
    }
    const PropertyMap& getProperties() const override { return properties; }
    FeatureIdentifier getID() const override { return id; }

private:
    FeatureType type;
    FeatureIdentifier id;
    PropertyMap properties;
};

style::PluginPropertyValue propertyValue(const plugin::PropertyDefinition& definition,
                                         const style::PluginPropertyMap& properties) {
    const auto it = properties.find(definition.name);
    return it == properties.end() ? style::defaultPluginPropertyValue(definition) : it->second;
}

std::size_t componentCount(mln_plugin_property_encoding_v1 encoding) {
    switch (encoding) {
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT:
        case MLN_PLUGIN_PROPERTY_ENCODING_BOOLEAN_FLOAT:
            return 1;
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT2:
            return 2;
        case MLN_PLUGIN_PROPERTY_ENCODING_COLOR:
            return 4;
    }
    return 0;
}

void encodedValue(const mln_plugin_value& value,
                  mln_plugin_property_encoding_v1 encoding,
                  std::array<float, 4>& output) {
    output = {};
    switch (encoding) {
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT:
            output[0] = value.data.float_value;
            break;
        case MLN_PLUGIN_PROPERTY_ENCODING_BOOLEAN_FLOAT:
            output[0] = value.data.boolean_value ? 1.0f : 0.0f;
            break;
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT2:
            output[0] = value.data.float2_value.x;
            output[1] = value.data.float2_value.y;
            break;
        case MLN_PLUGIN_PROPERTY_ENCODING_COLOR:
            output[0] = value.data.color_value.r;
            output[1] = value.data.color_value.g;
            output[2] = value.data.color_value.b;
            output[3] = value.data.color_value.a;
            break;
    }
}

mln_plugin_value decodedValue(const std::array<float, 4>& input,
                              mln_plugin_value_type type) {
    mln_plugin_value value{};
    value.struct_size = sizeof(value);
    value.type = type;
    switch (type) {
        case MLN_PLUGIN_VALUE_BOOLEAN:
            value.data.boolean_value = input[0] != 0.0f;
            break;
        case MLN_PLUGIN_VALUE_FLOAT:
            value.data.float_value = input[0];
            break;
        case MLN_PLUGIN_VALUE_FLOAT2:
            value.data.float2_value = {input[0], input[1]};
            break;
        case MLN_PLUGIN_VALUE_COLOR:
            value.data.color_value = {input[0], input[1], input[2], input[3]};
            break;
        default:
            break;
    }
    return value;
}

} // namespace

void PluginPaintVertexVector::set(std::size_t first,
                                  std::size_t length,
                                  const float* minimum,
                                  const float* maximum) {
    if (first > count || length > count - first) return;
    for (std::size_t vertex = first; vertex < first + length; ++vertex) {
        auto* destination = data.data() + vertex * components * 2;
        std::copy_n(minimum, components, destination);
        std::copy_n(maximum, components, destination + components);
    }
    updateModified(true);
}

void PluginPaintVertexVector::bounds(std::array<float, 4>& minimum,
                                     std::array<float, 4>& maximum) const {
    minimum.fill(std::numeric_limits<float>::infinity());
    maximum.fill(-std::numeric_limits<float>::infinity());
    for (std::size_t vertex = 0; vertex < count; ++vertex) {
        const auto* values = data.data() + vertex * components * 2;
        for (std::size_t component = 0; component < components; ++component) {
            minimum[component] = std::min({minimum[component], values[component], values[components + component]});
            maximum[component] = std::max({maximum[component], values[component], values[components + component]});
        }
    }
    if (count == 0) {
        minimum.fill(0.0f);
        maximum.fill(0.0f);
    }
}

PluginPaintPropertyBinder::PluginPaintPropertyBinder(plugin::PropertyDefinition definition_,
                                                     plugin::ShaderPropertyBindingDefinition binding_,
                                                     style::PluginPropertyValue value_,
                                                     float bucketZoom_,
                                                     uint64_t drawableKey,
                                                     std::size_t vertexCount_,
                                                     const std::vector<PluginFeatureVertexRange>& featureRanges,
                                                     const GeometryTileLayer& layer)
    : definition(std::move(definition_)),
      binding(std::move(binding_)),
      value(std::move(value_)),
      bucketZoom(bucketZoom_),
      vertexCount(vertexCount_),
      dataDriven(value.isDataDriven()) {
    for (const auto& input : featureRanges) {
        if (input.drawableKey != drawableKey) continue;
        auto feature = layer.getFeature(input.featureIndex);
        if (!feature) continue;
        ranges.push_back({input.featureIndex,
                          featureIDtoString(feature->getID()).value_or(std::string{}),
                          feature->getType(),
                          feature->getID(),
                          feature->getProperties(),
                          input.firstVertex,
                          input.vertexCount});
    }
    if (dataDriven) {
        vertexVector = std::make_shared<PluginPaintVertexVector>(vertexCount, componentCount());
        refill(&layer);
    }
}

gfx::AttributeDataType PluginPaintPropertyBinder::attributeType() const noexcept {
    switch (binding.encoding) {
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT:
        case MLN_PLUGIN_PROPERTY_ENCODING_BOOLEAN_FLOAT:
            return gfx::AttributeDataType::Float;
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT2:
            return gfx::AttributeDataType::Float2;
        case MLN_PLUGIN_PROPERTY_ENCODING_COLOR:
            return gfx::AttributeDataType::Float4;
    }
    return gfx::AttributeDataType::Invalid;
}

std::size_t PluginPaintPropertyBinder::componentCount() const noexcept {
    return mln::componentCount(binding.encoding);
}

float PluginPaintPropertyBinder::interpolationFactor(float zoom) const noexcept {
    return dataDriven ? value.interpolationFactor(bucketZoom, zoom) : 0.0f;
}

void PluginPaintPropertyBinder::writeUniform(float zoom,
                                             uint32_t uniformID,
                                             uint8_t* output,
                                             std::size_t outputSize) const {
    if (!output) return;
    if (!dataDriven && uniformID == binding.uniformID) {
        style::PluginPropertyValue::EvaluationStorage storage;
        const auto evaluated = value.evaluate(zoom, definition, storage);
        std::array<float, 4> encoded{};
        encodedValue(evaluated, binding.encoding, encoded);
        const auto size = componentCount() * sizeof(float);
        if (binding.uniformByteOffset <= outputSize && size <= outputSize - binding.uniformByteOffset) {
            std::memcpy(output + binding.uniformByteOffset, encoded.data(), size);
        }
    }
    if (uniformID == binding.interpolationUniformID &&
        binding.interpolationUniformByteOffset <= outputSize &&
        sizeof(float) <= outputSize - binding.interpolationUniformByteOffset) {
        const auto factor = interpolationFactor(zoom);
        std::memcpy(output + binding.interpolationUniformByteOffset, &factor, sizeof(factor));
    }
}

bool PluginPaintPropertyBinder::synchronize(const style::PluginPropertyValue& replacement) {
    if (value == replacement) return false;
    const bool wasDataDriven = dataDriven;
    value = replacement;
    dataDriven = value.isDataDriven();
    if (dataDriven && !vertexVector) {
        vertexVector = std::make_shared<PluginPaintVertexVector>(vertexCount, componentCount());
    }
    if (dataDriven) refill();
    if (!dataDriven) vertexVector.reset();
    return wasDataDriven != dataDriven || dataDriven;
}

bool PluginPaintPropertyBinder::update(const FeatureStates& states, const GeometryTileLayer& layer) {
    if (!dataDriven || states.empty()) return false;
    bool changed = false;
    for (const auto& range : ranges) {
        const auto state = states.find(range.featureID);
        if (state == states.end()) continue;
        featureStates[range.featureID] = state->second;
        auto feature = layer.getFeature(range.featureIndex);
        if (!feature) continue;
        fillRange(range, *feature, state->second);
        changed = true;
    }
    if (changed) updateStatistics();
    return changed;
}

void PluginPaintPropertyBinder::statistics(float zoom,
                                           mln_plugin_value& minimum,
                                           mln_plugin_value& maximum) const {
    if (!dataDriven || !vertexVector) {
        style::PluginPropertyValue::EvaluationStorage storage;
        minimum = value.evaluate(zoom, definition, storage);
        maximum = minimum;
        return;
    }
    minimum = decodedValue(minimumValues, definition.type);
    maximum = decodedValue(maximumValues, definition.type);
}

void PluginPaintPropertyBinder::refill(const GeometryTileLayer* layer) {
    if (!vertexVector) return;
    for (const auto& range : ranges) {
        std::unique_ptr<GeometryTileFeature> feature;
        if (layer) feature = layer->getFeature(range.featureIndex);
        PluginFeatureSnapshot snapshot(range.featureType, range.featureIdentifier, range.properties);
        const GeometryTileFeature& sourceFeature = feature ? *feature : static_cast<const GeometryTileFeature&>(snapshot);
        const auto state = featureStates.find(range.featureID);
        const FeatureState empty;
        fillRange(range, sourceFeature, state == featureStates.end() ? empty : state->second);
    }
    updateStatistics();
}

void PluginPaintPropertyBinder::fillRange(const Range& range,
                                          const GeometryTileFeature& feature,
                                          const FeatureState& state) {
    style::PluginPropertyValue::EvaluationStorage minimumStorage;
    style::PluginPropertyValue::EvaluationStorage maximumStorage;
    const auto minimumValue = value.evaluate(bucketZoom, feature, state, definition, minimumStorage);
    const auto maximumValue = value.evaluate(
        value.isZoomConstant() ? bucketZoom : bucketZoom + 1.0f, feature, state, definition, maximumStorage);
    std::array<float, 4> minimum{};
    std::array<float, 4> maximum{};
    encodedValue(minimumValue, binding.encoding, minimum);
    encodedValue(maximumValue, binding.encoding, maximum);
    vertexVector->set(range.firstVertex, range.vertexCount, minimum.data(), maximum.data());
}

void PluginPaintPropertyBinder::updateStatistics() {
    if (vertexVector) vertexVector->bounds(minimumValues, maximumValues);
}

PluginPaintPropertyBinders::PluginPaintPropertyBinders(
    const plugin::LayerType& registration,
    const plugin::ShaderDefinition& shader,
    uint64_t drawableKey,
    std::size_t vertexCount,
    float bucketZoom,
    const style::PluginPropertyMap& properties,
    const std::vector<PluginFeatureVertexRange>& ranges,
    const GeometryTileLayer& layer) {
    const auto definitions = plugin::PluginRegistry::get().propertiesForLayer(registration.type);
    for (const auto& binding : shader.propertyBindings) {
        const auto definition = std::find_if(definitions.begin(), definitions.end(), [&](const auto& candidate) {
            return candidate.name == binding.propertyName;
        });
        if (definition == definitions.end()) continue;
        binders.emplace_back(*definition,
                             binding,
                             propertyValue(*definition, properties),
                             bucketZoom,
                             drawableKey,
                             vertexCount,
                             ranges,
                             layer);
    }
}

void PluginPaintPropertyBinders::populateVertexAttributes(gfx::VertexAttributeArray& attributes,
                                                          gfx::StringIDSetsPair& uniforms) const {
    for (const auto& binder : binders) {
        const auto& binding = binder.getBinding();
        if (!binder.isDataDriven()) {
            uniforms.first.emplace(binding.propertyName);
            uniforms.second.emplace(binding.minimumAttributeID);
            uniforms.second.emplace(binding.maximumAttributeID);
            continue;
        }
        const auto& vector = binder.getVertexVector();
        const auto components = binder.componentCount();
        if (const auto& minimum = attributes.set(binding.minimumAttributeID)) {
            minimum->setSharedRawData(vector,
                                      0,
                                      0,
                                      vector->getRawSize(),
                                      binder.attributeType());
        }
        if (const auto& maximum = attributes.set(binding.maximumAttributeID)) {
            maximum->setSharedRawData(vector,
                                      static_cast<uint32_t>(components * sizeof(float)),
                                      0,
                                      vector->getRawSize(),
                                      binder.attributeType());
        }
    }
}

void PluginPaintPropertyBinders::writeUniforms(float zoom,
                                               uint32_t uniformID,
                                               uint8_t* output,
                                               std::size_t outputSize) const {
    for (const auto& binder : binders) binder.writeUniform(zoom, uniformID, output, outputSize);
}

bool PluginPaintPropertyBinders::synchronize(const style::PluginPropertyMap& properties) {
    bool rebuildDrawable = false;
    for (auto& binder : binders) {
        const auto replacement = propertyValue(binder.getDefinition(), properties);
        rebuildDrawable = binder.synchronize(replacement) || rebuildDrawable;
    }
    return rebuildDrawable;
}

bool PluginPaintPropertyBinders::update(const FeatureStates& states, const GeometryTileLayer& layer) {
    bool changed = false;
    for (auto& binder : binders) changed = binder.update(states, layer) || changed;
    return changed;
}

void PluginPaintPropertyBinders::appendStatistics(
    float zoom,
    std::map<std::string, std::pair<mln_plugin_value, mln_plugin_value>>& output) const {
    for (const auto& binder : binders) {
        if (output.find(binder.getDefinition().name) != output.end()) continue;
        mln_plugin_value minimum{};
        mln_plugin_value maximum{};
        binder.statistics(zoom, minimum, maximum);
        output.emplace(binder.getDefinition().name, std::make_pair(minimum, maximum));
    }
}

void PluginBucket::update(const FeatureStates& states,
                          const GeometryTileLayer& layer,
                          const std::string& layerID,
                          const ImagePositions&) {
    const auto it = paintPropertyBinders.find(layerID);
    if (it == paintPropertyBinders.end()) return;
    bool changed = false;
    for (auto& [key, binders] : it->second) {
        (void)key;
        changed = binders.update(states, layer) || changed;
    }
    if (changed) {
        uploaded = false;
        const auto properties = latestPaintProperties.find(layerID);
        const auto zoom = latestZoom.find(layerID);
        if (properties != latestPaintProperties.end() && zoom != latestZoom.end()) {
            updateQueryRadius(layerID, properties->second, zoom->second);
        }
    }
}

float PluginBucket::getQueryRadius(const RenderLayer& layer) const {
    const auto it = queryRadii.find(layer.getID());
    return it == queryRadii.end() ? queryRadius : it->second;
}

bool PluginBucket::synchronizePaint(const std::string& layerID,
                                    const style::PluginPropertyMap& properties,
                                    float zoom) {
    const auto priorProperties = latestPaintProperties.find(layerID);
    const auto priorZoom = latestZoom.find(layerID);
    const bool propertiesChanged = priorProperties == latestPaintProperties.end() ||
                                   priorProperties->second != properties;
    const bool queryRadiusChanged = propertiesChanged || priorZoom == latestZoom.end() || priorZoom->second != zoom;
    latestPaintProperties.insert_or_assign(layerID, properties);
    latestZoom.insert_or_assign(layerID, zoom);
    const auto it = paintPropertyBinders.find(layerID);
    if (it == paintPropertyBinders.end()) {
        if (queryRadiusChanged) updateQueryRadius(layerID, properties, zoom);
        return false;
    }
    bool rebuildDrawable = false;
    if (propertiesChanged) {
        for (auto& [key, binders] : it->second) {
            (void)key;
            rebuildDrawable = binders.synchronize(properties) || rebuildDrawable;
        }
    }
    if (rebuildDrawable) uploaded = false;
    if (queryRadiusChanged) updateQueryRadius(layerID, properties, zoom);
    return rebuildDrawable;
}

void PluginBucket::updateQueryRadius(const std::string& layerID,
                                     const style::PluginPropertyMap& properties,
                                     float zoom) {
    if (!registration.queryRadius) return;
    std::map<std::string, std::pair<mln_plugin_value, mln_plugin_value>> values;
    if (const auto layer = paintPropertyBinders.find(layerID); layer != paintPropertyBinders.end()) {
        for (const auto& [key, binders] : layer->second) {
            (void)key;
            binders.appendStatistics(zoom, values);
        }
    }
    std::vector<mln_plugin_property_statistics_v1> statistics;
    statistics.reserve(values.size());
    for (const auto& [name, bounds] : values) {
        statistics.push_back({sizeof(mln_plugin_property_statistics_v1),
                              {name.data(), name.size()},
                              bounds.first,
                              bounds.second});
    }

    const auto definitions = plugin::PluginRegistry::get().propertiesForLayer(registration.type);
    std::vector<style::PluginPropertyValue::EvaluationStorage> storage(definitions.size());
    std::vector<mln_plugin_property_value_v1> cameraProperties;
    cameraProperties.reserve(definitions.size());
    for (std::size_t i = 0; i < definitions.size(); ++i) {
        const auto& definition = definitions[i];
        const auto current = propertyValue(definition, properties);
        cameraProperties.push_back({sizeof(mln_plugin_property_value_v1),
                                    {definition.name.data(), definition.name.size()},
                                    current.evaluate(zoom, definition, storage[i]),
                                    properties.find(definition.name) != properties.end()});
    }
    const auto radius = registration.queryRadius(statistics.data(),
                                                 statistics.size(),
                                                 cameraProperties.data(),
                                                 cameraProperties.size());
    if (std::isfinite(radius) && radius >= 0.0f) {
        queryRadii.insert_or_assign(layerID, radius);
        queryRadiusErrorsLogged.erase(layerID);
    } else {
        queryRadii.insert_or_assign(layerID, 0.0f);
        if (queryRadiusErrorsLogged.emplace(layerID).second) {
            Log::Warning(Event::Style,
                         "Plugin layer '" + registration.type + "' returned an invalid query radius");
        }
    }
}

PluginPaintPropertyBinders* PluginBucket::paintBinders(const std::string& layerID, uint64_t drawableKey) {
    const auto layer = paintPropertyBinders.find(layerID);
    if (layer == paintPropertyBinders.end()) return nullptr;
    const auto drawable = layer->second.find(drawableKey);
    return drawable == layer->second.end() ? nullptr : &drawable->second;
}

} // namespace mln
