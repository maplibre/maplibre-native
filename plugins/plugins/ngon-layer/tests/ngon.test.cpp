#include "ngon_layer.hpp"
#include <gtest/gtest.h>
#include <string_view>
#include <vector>

namespace {
const mln_plugin_descriptor_v1* registered = nullptr;
mln_plugin_status capture(const mln_plugin_descriptor_v1* d, char*, size_t) {
    registered = d;
    return MLN_PLUGIN_STATUS_OK;
}

mln_plugin_property_value_v1 number(const char* name, float value) {
    mln_plugin_property_value_v1 p{};
    p.struct_size = sizeof(p);
    p.name = {name, std::string_view(name).size()};
    p.value.struct_size = sizeof(p.value);
    p.value.type = MLN_PLUGIN_VALUE_FLOAT;
    p.value.data.float_value = value;
    return p;
}
} // namespace

class NgonLayer : public testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_ngon_layer_register(capture, nullptr, 0));
        ASSERT_TRUE(registered && registered->layer_type_count == 1);
        layer = registered->layer_types[0];
        context.struct_size = sizeof(context);
        context.extent = 8192;
        feature.struct_size = sizeof(feature);
        feature.geometry_type = MLN_PLUGIN_GEOMETRY_POINT;
        feature.feature_index = 3;
    }

    mln_plugin_layer_type_v1 layer{};
    mln_plugin_layout_context_v1 context{};
    mln_plugin_feature_v1 feature{};
};

TEST_F(NgonLayer, RegistersItsDescriptor) {
    EXPECT_EQ(MLN_PLUGIN_STATUS_NOT_FOUND, mln_ngon_layer_register(nullptr, nullptr, 0));
    EXPECT_EQ(13u, layer.property_count);
    ASSERT_EQ(1u, layer.shader_count);
    EXPECT_EQ(16u, layer.shaders[0].attribute_count);
    EXPECT_EQ(13u, layer.shaders[0].property_binding_count);
    for (size_t i = 0; i < layer.property_count; ++i) {
        EXPECT_TRUE(layer.properties[i].expression_capabilities & MLN_PLUGIN_EXPRESSION_FEATURE_STATE);
        EXPECT_TRUE(layer.properties[i].expression_capabilities & MLN_PLUGIN_EXPRESSION_COMPOSITE);
    }
}

TEST_F(NgonLayer, OwnsThePointsOfItsTile) {
    void* layout = nullptr;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, layer.create_layout(nullptr, &layout));
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, layer.create_layout(&context, &layout));
    const mln_plugin_tile_point_v1 points[] = {{0, 0}, {8191, 8191}, {8192, 0}, {-1, 0}};
    feature.points = points;
    feature.point_count = 4;
    EXPECT_EQ(MLN_PLUGIN_STATUS_OK, layer.layout_feature(layout, &feature));
    mln_plugin_bucket_v1 bucket{};
    bucket.struct_size = sizeof(bucket);
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, layer.finish_layout(layout, &bucket));
    ASSERT_EQ(1u, bucket.vertex_stream_count);
    EXPECT_EQ(8u, bucket.vertex_streams[0].vertex_count);
    EXPECT_EQ(12u, bucket.index_count);
    EXPECT_EQ(1u, bucket.drawable_count);
    ASSERT_EQ(1u, bucket.feature_vertex_range_count);
    EXPECT_EQ(3u, bucket.feature_vertex_ranges[0].feature_index);
    layer.destroy_layout(layout);
}

TEST_F(NgonLayer, SplitsABucketIntoSegmentsOf16BitIndices) {
    void* layout = nullptr;
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, layer.create_layout(&context, &layout));
    std::vector<mln_plugin_tile_point_v1> many(17000, {100, 100});
    feature.points = many.data();
    feature.point_count = many.size();
    EXPECT_EQ(MLN_PLUGIN_STATUS_OK, layer.layout_feature(layout, &feature));
    mln_plugin_bucket_v1 bucket{};
    bucket.struct_size = sizeof(bucket);
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, layer.finish_layout(layout, &bucket));
    ASSERT_EQ(2u, bucket.drawables[0].segment_count);
    for (size_t s = 0; s < bucket.drawables[0].segment_count; ++s) {
        const auto& segment = bucket.drawables[0].segments[s];
        EXPECT_LE(segment.vertex_length, 65535u);
        for (size_t i = 0; i < segment.index_length; ++i) {
            EXPECT_LT(bucket.indices[segment.index_offset + i], segment.vertex_length);
        }
    }
    layer.destroy_layout(layout);
}

TEST_F(NgonLayer, QueriesTheShapeAndNotItsQuad) {
    mln_plugin_query_context_v1 query{};
    query.struct_size = sizeof(query);
    query.pixels_to_tile_units = query.camera_to_center_distance = 1;
    query.viewport_width = query.viewport_height = 256;
    query.tile_matrix[0] = 2.0 / 256;
    query.tile_matrix[5] = -2.0 / 256;
    query.tile_matrix[12] = -1;
    query.tile_matrix[13] = query.tile_matrix[15] = 1;
    const mln_plugin_tile_point_v1 center{100, 100};
    feature.points = &center;
    feature.point_count = 1;
    auto properties = std::vector{number("ngon-radius", 20), number("ngon-corners", 3), number("ngon-rotate", 0)};
    const auto hit = [&](int16_t x, int16_t y) {
        const mln_plugin_tile_point_v1 point{x, y};
        return layer.query_feature(&feature, &point, 1, &query, properties.data(), properties.size());
    };
    EXPECT_TRUE(hit(100, 85));
    EXPECT_FALSE(hit(100, 115)); // Outside the triangle, but inside its bounding quad.
    properties[2] = number("ngon-rotate", 180);
    EXPECT_TRUE(hit(100, 115));
    EXPECT_FALSE(hit(100, 85));
    properties[0] = number("ngon-radius", 0);
    EXPECT_FALSE(hit(100, 100));
}
