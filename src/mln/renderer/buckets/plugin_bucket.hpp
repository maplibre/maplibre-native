#pragma once

#include <mln/gfx/index_vector.hpp>
#include <mln/gfx/vertex_vector.hpp>
#include <mln/plugin/plugin_registry.hpp>
#include <mln/renderer/bucket.hpp>
#include <mln/shaders/segment.hpp>

#include <map>

namespace mln {

class PluginVertexVector final : public gfx::VertexVectorBase {
public:
    PluginVertexVector(std::vector<uint8_t> data_, std::size_t count_, std::size_t stride_)
        : data(std::move(data_)), count(count_), stride(stride_) {}

    const void* getRawData() const override { return data.data(); }
    std::size_t getRawSize() const override { return stride; }
    std::size_t getRawCount() const override { return count; }

private:
    std::vector<uint8_t> data;
    std::size_t count;
    std::size_t stride;
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
};

class PluginBucket final : public Bucket {
public:
    explicit PluginBucket(plugin::LayerType registration_)
        : registration(std::move(registration_)) {}
    ~PluginBucket() override = default;

    void upload(gfx::UploadPass&) override { uploaded = true; }
    bool hasData() const override { return indices && !indices->empty() && !drawables.empty(); }
    float getQueryRadius(const RenderLayer&) const override { return queryRadius; }

    plugin::LayerType registration;
    std::map<uint32_t, std::shared_ptr<PluginVertexVector>> vertexStreams;
    std::shared_ptr<gfx::IndexVectorBase> indices;
    std::vector<PluginDrawableDefinition> drawables;
    float queryRadius = 0.0f;
};

} // namespace mln
