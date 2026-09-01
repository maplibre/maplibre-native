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

class PluginPaintPropertyBinder {
public:
    PluginPaintPropertyBinder(plugin::PropertyDefinition,
                              plugin::ShaderPropertyBindingDefinition,
                              style::PluginPropertyValue,
                              float bucketZoom,
                              uint64_t drawableKey,
                              std::size_t vertexCount,
                              const std::vector<PluginFeatureVertexRange>&,
                              const GeometryTileLayer&);

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
    struct Range {
        std::size_t featureIndex = 0;
        std::string featureID;
        FeatureType featureType = FeatureType::Unknown;
        FeatureIdentifier featureIdentifier;
        PropertyMap properties;
        std::size_t firstVertex = 0;
        std::size_t vertexCount = 0;
    };

    void refill(const GeometryTileLayer* = nullptr);
    void fillRange(const Range&, const GeometryTileFeature&, const FeatureState&);
    void updateStatistics();

    plugin::PropertyDefinition definition;
    plugin::ShaderPropertyBindingDefinition binding;
    style::PluginPropertyValue value;
    float bucketZoom;
    std::size_t vertexCount;
    bool dataDriven = false;
    std::vector<Range> ranges;
    std::map<std::string, FeatureState> featureStates;
    std::shared_ptr<PluginPaintVertexVector> vertexVector;
    std::array<float, 4> minimumValues{};
    std::array<float, 4> maximumValues{};
};

class PluginPaintPropertyBinders final : public PaintPropertyBindersBase {
public:
    PluginPaintPropertyBinders(const plugin::LayerType&,
                               const plugin::ShaderDefinition&,
                               uint64_t drawableKey,
                               std::size_t vertexCount,
                               float bucketZoom,
                               const style::PluginPropertyMap&,
                               const std::vector<PluginFeatureVertexRange>&,
                               const GeometryTileLayer&);

    void populateVertexAttributes(gfx::VertexAttributeArray&, gfx::StringIDSetsPair&) const;
    void writeUniforms(float zoom, uint32_t uniformID, uint8_t* output, std::size_t outputSize) const;
    bool synchronize(const style::PluginPropertyMap&);
    bool update(const FeatureStates&, const GeometryTileLayer&);
    void appendStatistics(float,
                          std::map<std::string, std::pair<mln_plugin_value, mln_plugin_value>>&) const;

private:
    std::vector<PluginPaintPropertyBinder> binders;
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
    mln_plugin_draw_mode drawMode = MLN_PLUGIN_DRAW_MODE_TRIANGLES;
    mln_plugin_render_stage renderStage = MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT;
    mln_plugin_depth_mode depthMode = MLN_PLUGIN_DEPTH_READ_ONLY;
    mln_plugin_blend_mode blendMode = MLN_PLUGIN_BLEND_ALPHA;
    bool enableStencil = false;
    bool enableCullFace = false;
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

    bool synchronizePaint(const std::string& layerID, const style::PluginPropertyMap&, float zoom);
    void updateQueryRadius(const std::string& layerID, const style::PluginPropertyMap&, float zoom);
    PluginPaintPropertyBinders* paintBinders(const std::string& layerID, uint64_t drawableKey);

    plugin::LayerType registration;
    std::map<uint32_t, std::shared_ptr<PluginVertexVector>> vertexStreams;
    std::shared_ptr<gfx::IndexVectorBase> indices;
    std::vector<PluginDrawableDefinition> drawables;
    std::vector<PluginFeatureVertexRange> featureVertexRanges;
    std::map<std::string, std::map<uint64_t, PluginPaintPropertyBinders>> paintPropertyBinders;
    std::map<std::string, style::PluginPropertyMap> latestPaintProperties;
    std::map<std::string, float> latestZoom;
    float queryRadius = 0.0f;
    std::map<std::string, float> queryRadii;
    std::set<std::string> queryRadiusErrorsLogged;
};

} // namespace mln
