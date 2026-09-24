#include "fill_extrusion_plugin.hpp"
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

#define CHECK(condition)                                   \
    do {                                                   \
        if (!(condition)) {                                \
            std::cerr << __LINE__ << ": " #condition "\n"; \
            std::abort();                                  \
        }                                                  \
    } while (false)

namespace {
const mln_plugin_descriptor_v1* registered = nullptr;
mln_plugin_status capture(const mln_plugin_descriptor_v1* d, char*, size_t) {
    registered = d;
    return MLN_PLUGIN_STATUS_OK;
}

mln_plugin_feature_v1 makeFeature(const std::vector<mln_plugin_tile_point_v1>& points,
                                  const uint32_t* pathOffsets,
                                  size_t pathCount,
                                  uint64_t featureIndex) {
    mln_plugin_feature_v1 f{};
    f.struct_size = sizeof(f);
    f.geometry_type = MLN_PLUGIN_GEOMETRY_POLYGON;
    f.feature_index = featureIndex;
    f.points = points.data();
    f.point_count = points.size();
    f.path_offsets = pathOffsets;
    f.path_count = pathCount;
    return f;
}

const mln_plugin_shader_descriptor_v1* findShader(const mln_plugin_layer_type_v1& layer, const char* id, size_t idLen) {
    for (size_t i = 0; i < layer.shader_count; ++i) {
        const auto& shader = layer.shaders[i];
        if (shader.shader_id.size == idLen &&
            std::string_view(shader.shader_id.data, idLen) == std::string_view(id, idLen)) {
            return &shader;
        }
    }
    return nullptr;
}
} // namespace

int main() {
    CHECK(mln_fill_extrusion_plugin_register(nullptr, nullptr, 0) == MLN_PLUGIN_STATUS_NOT_FOUND);
    CHECK(mln_fill_extrusion_plugin_register(capture, nullptr, 0) == MLN_PLUGIN_STATUS_OK);
    CHECK(registered && registered->layer_type_count == 1);
    const auto& layer = registered->layer_types[0];
    CHECK(layer.property_count == 11);
    CHECK(layer.shader_count == 2);
    CHECK(layer.geometry_type_mask == MLN_PLUGIN_GEOMETRY_POLYGON);
    const auto* buildingShader = findShader(layer, "building", 8);
    const auto* shadowShader = findShader(layer, "shadow", 6);
    CHECK(buildingShader != nullptr);
    CHECK(shadowShader != nullptr);
    // Option 2: the building shader opts into real depth test+write (correct occlusion); the
    // shadow shader does not, so it keeps the layer-wide stencil-overlap-dedup mode.
    // Stencil overlap dedup for both shaders (not real depth-write): see the enable_depth_write
    // doc comment in fill_extrusion_plugin.cpp for why.
    CHECK(buildingShader->enable_depth_write == 0);
    CHECK(shadowShader->enable_depth_write == 0);

    for (size_t i = 0; i < layer.shader_count; ++i) {
        const auto& shader = layer.shaders[i];
        const std::string_view id(shader.shader_id.data, shader.shader_id.size);
        if (id == "building") {
            CHECK(shader.attribute_count == 9);
            CHECK(shader.property_binding_count == 5);
        } else if (id == "shadow") {
            CHECK(shader.attribute_count == 9);
            CHECK(shader.property_binding_count == 6);
        } else {
            CHECK(false);
        }
    }

    mln_plugin_layout_context_v1 context{};
    context.struct_size = sizeof(context);
    context.extent = 8192;
    void* layoutInstance = nullptr;
    CHECK(layer.create_layout(nullptr, &layoutInstance) == MLN_PLUGIN_STATUS_INVALID_ARGUMENT);
    CHECK(layer.create_layout(&context, &layoutInstance) == MLN_PLUGIN_STATUS_OK);

    const std::vector<mln_plugin_tile_point_v1> square = {
        {1000, 1000}, {1200, 1000}, {1200, 1200}, {1000, 1200}, {1000, 1000}};
    const uint32_t squareOffsets[] = {0, static_cast<uint32_t>(square.size())};
    auto feature = makeFeature(square, squareOffsets, 1, 7);
    CHECK(layer.layout_feature(layoutInstance, &feature) == MLN_PLUGIN_STATUS_OK);

    mln_plugin_bucket_v1 bucket{};
    bucket.struct_size = sizeof(bucket);
    CHECK(layer.finish_layout(layoutInstance, &bucket) == MLN_PLUGIN_STATUS_OK);

    // Corner rounding replaces each sharp corner with a short arc, so exact vertex/index counts
    // depend on the rounding algorithm; assert structural correctness instead.
    CHECK(bucket.vertex_stream_count == 1);
    CHECK(bucket.vertex_streams[0].vertex_count > 8); // more than the unrounded square's 8 wall verts
    CHECK(bucket.index_count > 0 && bucket.index_count % 3 == 0);
    CHECK(bucket.drawable_count == 2);
    for (size_t i = 0; i < bucket.index_count; ++i) {
        CHECK(bucket.indices[i] < bucket.vertex_streams[0].vertex_count);
    }

    // Both drawables share the exact same geometry range, only their drawable_key differs.
    CHECK(bucket.feature_vertex_range_count == 2);
    const auto& rangeA = bucket.feature_vertex_ranges[0];
    const auto& rangeB = bucket.feature_vertex_ranges[1];
    CHECK(rangeA.feature_index == 7 && rangeB.feature_index == 7);
    CHECK(rangeA.first_vertex == rangeB.first_vertex);
    CHECK(rangeA.vertex_count == rangeB.vertex_count);
    CHECK(rangeA.vertex_count == bucket.vertex_streams[0].vertex_count);
    CHECK((rangeA.drawable_key == 1 && rangeB.drawable_key == 2) ||
          (rangeA.drawable_key == 2 && rangeB.drawable_key == 1));

    layer.destroy_layout(layoutInstance);

    // A concave (L-shaped) footprint should still triangulate without crashing.
    CHECK(layer.create_layout(&context, &layoutInstance) == MLN_PLUGIN_STATUS_OK);
    const std::vector<mln_plugin_tile_point_v1> lshape = {
        {0, 0}, {200, 0}, {200, 100}, {100, 100}, {100, 200}, {0, 200}, {0, 0}};
    const uint32_t lshapeOffsets[] = {0, static_cast<uint32_t>(lshape.size())};
    auto lFeature = makeFeature(lshape, lshapeOffsets, 1, 9);
    CHECK(layer.layout_feature(layoutInstance, &lFeature) == MLN_PLUGIN_STATUS_OK);
    CHECK(layer.finish_layout(layoutInstance, &bucket) == MLN_PLUGIN_STATUS_OK);
    CHECK(bucket.index_count > 0 && bucket.index_count % 3 == 0);
    CHECK(bucket.drawable_count == 2);
    for (size_t i = 0; i < bucket.index_count; ++i) {
        CHECK(bucket.indices[i] < bucket.vertex_streams[0].vertex_count);
    }
    layer.destroy_layout(layoutInstance);

    // Wrong geometry type is rejected.
    CHECK(layer.create_layout(&context, &layoutInstance) == MLN_PLUGIN_STATUS_OK);
    mln_plugin_feature_v1 pointFeature{};
    pointFeature.struct_size = sizeof(pointFeature);
    pointFeature.geometry_type = MLN_PLUGIN_GEOMETRY_POINT;
    const mln_plugin_tile_point_v1 onePoint{0, 0};
    pointFeature.points = &onePoint;
    pointFeature.point_count = 1;
    CHECK(layer.layout_feature(layoutInstance, &pointFeature) == MLN_PLUGIN_STATUS_INVALID_ARGUMENT);
    layer.destroy_layout(layoutInstance);

    std::cout << "fill-extrusion-plugin descriptor, dual-drawable layout, and rounding tests passed\n";
}
