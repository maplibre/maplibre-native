#pragma once

#include <mln/gfx/index_vector.hpp>
#include <mln/gfx/vertex_attribute.hpp>
#include <mln/gfx/vertex_vector.hpp>
#include <mln/plugin/plugin_registry.hpp>
#include <mln/renderer/bucket.hpp>
#include <mln/renderer/paint_property_binder.hpp>
#include <mln/shaders/segment.hpp>
#include <mln/style/plugin_property.hpp>

#include <map>
#include <set>
#include <unordered_map>

namespace mln {

class PluginVertexVector final : public gfx::VertexVectorBase {
public:
    PluginVertexVector(std::vector<uint8_t> data_, std::size_t count_, std::size_t stride_)
        : data(std::move(data_)),
          count(count_),
          stride(stride_) {}

    const void* getRawData() const override { return data.data(); }
    std::size_t getRawSize() const override { return stride; }
    std::size_t getRawCount() const override { return count; }

private:
    std::vector<uint8_t> data;
    std::size_t count;
    std::size_t stride;
};

class PluginPaintVertexVector final : public gfx::VertexVectorBase {
public:
    PluginPaintVertexVector(std::size_t count_, std::size_t components_)
        : data(count_ * components_ * 2),
          count(count_),
          components(components_) {}

    const void* getRawData() const override { return data.data(); }
    std::size_t getRawSize() const override { return components * 2 * sizeof(float); }
    std::size_t getRawCount() const override { return count; }

    void set(std::size_t first, std::size_t length, const float* minimum, const float* maximum);
    void bounds(std::array<float, 4>& minimum, std::array<float, 4>& maximum) const;

private:
    std::vector<float> data;
    std::size_t count;
    std::size_t components;
};

struct PluginFeatureVertexRange {
    std::size_t featureIndex = 0;
    uint64_t drawableKey = 0;
    std::size_t firstVertex = 0;
    std::size_t vertexCount = 0;
};

// One immutable snapshot per source feature, shared by all properties and layers
// using this bucket. Vertex ranges are shared per drawable too.
class PluginFeatureData {
public:
    struct Feature {
        std::string id;
        std::unique_ptr<const GeometryTileFeature> snapshot;
    };
    struct Range {
        std::size_t featureIndex;
        std::size_t firstVertex;
        std::size_t vertexCount;
    };
    struct Drawable {
        std::vector<Range> ranges;
        std::unordered_map<std::string, std::vector<std::size_t>> byID;
    };
    PluginFeatureData(const std::vector<PluginFeatureVertexRange>&, const GeometryTileLayer&);
    ~PluginFeatureData();
    const Drawable& drawable(uint64_t) const;
    std::vector<Feature> features;

private:
    std::map<uint64_t, Drawable> drawables;
};

class PluginPaintPropertyBinder {
public:
    PluginPaintPropertyBinder(plugin::PropertyDefinition,
                              plugin::ShaderPropertyBindingDefinition,
                              style::PluginPropertyValue,
                              float bucketZoom,
                              uint64_t drawableKey,
                              std::size_t vertexCount,
                              std::shared_ptr<const PluginFeatureData>,
                              std::shared_ptr<FeatureStates> = std::make_shared<FeatureStates>());

    bool isDataDriven() const noexcept { return dataDriven; }
    const plugin::ShaderPropertyBindingDefinition& getBinding() const noexcept { return binding; }
    const plugin::PropertyDefinition& getDefinition() const noexcept { return definition; }
    const std::shared_ptr<PluginPaintVertexVector>& getVertexVector() const noexcept { return vertexVector; }
    gfx::AttributeDataType attributeType() const noexcept;
    std::size_t componentCount() const noexcept;
    float interpolationFactor(float zoom) const noexcept;
    void writeUniform(float zoom, uint32_t uniformID, uint8_t* output, std::size_t outputSize) const;
    bool synchronize(const style::PluginPropertyValue&);
    bool update(const FeatureStates&, const GeometryTileLayer&);
    void statistics(float zoom, mln_plugin_value& minimum, mln_plugin_value& maximum) const;

private:
    friend class PluginPaintPropertyBinders;
    bool updateRanges(const FeatureStates&);
    void refill();
    void fillRange(const PluginFeatureData::Range&, const GeometryTileFeature&, const FeatureState&);
    void updateStatistics();

    plugin::PropertyDefinition definition;
    plugin::ShaderPropertyBindingDefinition binding;
    style::PluginPropertyValue value;
    float bucketZoom;
    std::size_t vertexCount;
    bool dataDriven = false;
    const uint64_t drawableKey;
    std::shared_ptr<const PluginFeatureData> features;
    std::shared_ptr<FeatureStates> featureStates;
    std::shared_ptr<PluginPaintVertexVector> vertexVector;
    std::array<float, 4> minimumValues{};
    std::array<float, 4> maximumValues{};
    mutable std::optional<float> uniformZoom;
    mutable std::array<float, 4> uniformValue{};
};

class PluginPaintPropertyBinders final : public PaintPropertyBindersBase {
public:
    PluginPaintPropertyBinders(const plugin::LayerType&,
                               const plugin::ShaderDefinition&,
                               uint64_t drawableKey,
                               std::size_t vertexCount,
                               float bucketZoom,
                               const style::PluginPropertyMap&,
                               std::shared_ptr<const PluginFeatureData>);

    void populateVertexAttributes(gfx::VertexAttributeArray&, gfx::StringIDSetsPair&) const;
    void writeUniforms(float zoom, uint32_t uniformID, uint8_t* output, std::size_t outputSize) const;
    bool synchronize(const style::PluginPropertyMap&);
    bool update(const FeatureStates&, const GeometryTileLayer&);
    void appendStatistics(float, std::map<std::string, std::pair<mln_plugin_value, mln_plugin_value>>&) const;

private:
    std::vector<PluginPaintPropertyBinder> binders;
    std::shared_ptr<FeatureStates> featureStates = std::make_shared<FeatureStates>();
};

struct PluginAttributeBinding {
    uint32_t attributeID = 0;
    uint32_t streamID = 0;
    uint32_t byteOffset = 0;
    mln_plugin_vertex_attribute_type type = MLN_PLUGIN_VERTEX_FLOAT;
};

struct PluginDrawableDefinition {
    uint64_t key = 0;
    std::string shaderID;
    std::vector<PluginAttributeBinding> attributes;
    SegmentVector segments;
    std::size_t vertexCount = 0;
};

class PluginBucket final : public Bucket {
public:
    explicit PluginBucket(plugin::LayerType registration_)
        : registration(std::move(registration_)) {}
    ~PluginBucket() override = default;

    void upload(gfx::UploadPass&) override { uploaded = true; }
    bool hasData() const override { return indices && !indices->empty() && !drawables.empty(); }
    float getQueryRadius(const RenderLayer&) const override;
    void update(const FeatureStates&, const GeometryTileLayer&, const std::string&, const ImagePositions&) override;

    // The render layer retains one immutable snapshot until its evaluated paint
    // changes. Tiles compare identity instead of copying/comparing every property.
    bool synchronizePaint(const std::string& layerID, const std::shared_ptr<const style::PluginPropertyMap>&, float zoom);
    void updateQueryRadius(const std::string& layerID, const style::PluginPropertyMap&, float zoom);
    PluginPaintPropertyBinders* paintBinders(const std::string& layerID, uint64_t drawableKey);

    plugin::LayerType registration;
    std::map<uint32_t, std::shared_ptr<PluginVertexVector>> vertexStreams;
    std::shared_ptr<gfx::IndexVectorBase> indices;
    std::vector<PluginDrawableDefinition> drawables;
    std::vector<PluginFeatureVertexRange> featureVertexRanges;
    std::map<std::string, std::map<uint64_t, PluginPaintPropertyBinders>> paintPropertyBinders;
    std::map<std::string, std::shared_ptr<const style::PluginPropertyMap>> latestPaintProperties;
    std::map<std::string, float> latestZoom;
    float queryRadius = 0.0f;
    std::map<std::string, float> queryRadii;
    std::set<std::string> queryRadiusErrorsLogged;
};

} // namespace mln
