#include "ngon_layer.hpp"
#include <cmath>
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

int main() {
    CHECK(mln_ngon_layer_register(nullptr, nullptr, 0) == MLN_PLUGIN_STATUS_NOT_FOUND);
    CHECK(mln_ngon_layer_register(capture, nullptr, 0) == MLN_PLUGIN_STATUS_OK);
    CHECK(registered && registered->layer_type_count == 1);
    const auto& layer = registered->layer_types[0];
    CHECK(layer.property_count == 13 && layer.shader_count == 1);
    CHECK(layer.shaders[0].attribute_count == 16);
    CHECK(layer.shaders[0].property_binding_count == 13);
    for (size_t i = 0; i < layer.property_count; ++i) {
        CHECK(layer.properties[i].expression_capabilities & MLN_PLUGIN_EXPRESSION_FEATURE_STATE);
        CHECK(layer.properties[i].expression_capabilities & MLN_PLUGIN_EXPRESSION_COMPOSITE);
    }

    mln_plugin_layout_context_v1 context{};
    context.struct_size = sizeof(context);
    context.extent = 8192;
    void* layout = nullptr;
    CHECK(layer.create_layout(nullptr, &layout) == MLN_PLUGIN_STATUS_INVALID_ARGUMENT);
    CHECK(layer.create_layout(&context, &layout) == MLN_PLUGIN_STATUS_OK);
    const mln_plugin_tile_point_v1 points[] = {{0, 0}, {8191, 8191}, {8192, 0}, {-1, 0}};
    mln_plugin_feature_v1 feature{};
    feature.struct_size = sizeof(feature);
    feature.geometry_type = MLN_PLUGIN_GEOMETRY_POINT;
    feature.feature_index = 3;
    feature.points = points;
    feature.point_count = 4;
    CHECK(layer.layout_feature(layout, &feature) == MLN_PLUGIN_STATUS_OK);
    mln_plugin_bucket_v1 bucket{};
    bucket.struct_size = sizeof(bucket);
    CHECK(layer.finish_layout(layout, &bucket) == MLN_PLUGIN_STATUS_OK);
    CHECK(bucket.vertex_stream_count == 1 && bucket.vertex_streams[0].vertex_count == 8);
    CHECK(bucket.index_count == 12 && bucket.drawable_count == 1);
    CHECK(bucket.feature_vertex_range_count == 1);
    CHECK(bucket.feature_vertex_ranges[0].feature_index == 3);
    layer.destroy_layout(layout);

    // Force more than one 16-bit index segment without duplicating tile geometry.
    CHECK(layer.create_layout(&context, &layout) == MLN_PLUGIN_STATUS_OK);
    std::vector<mln_plugin_tile_point_v1> many(17000, {100, 100});
    feature.points = many.data();
    feature.point_count = many.size();
    CHECK(layer.layout_feature(layout, &feature) == MLN_PLUGIN_STATUS_OK);
    CHECK(layer.finish_layout(layout, &bucket) == MLN_PLUGIN_STATUS_OK);
    CHECK(bucket.drawables[0].segment_count == 2);
    for (size_t s = 0; s < bucket.drawables[0].segment_count; ++s) {
        const auto& segment = bucket.drawables[0].segments[s];
        CHECK(segment.vertex_length <= 65535);
        for (size_t i = 0; i < segment.index_length; ++i)
            CHECK(bucket.indices[segment.index_offset + i] < segment.vertex_length);
    }
    layer.destroy_layout(layout);

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
    CHECK(hit(100, 85));
    CHECK(!hit(100, 115)); // Outside the triangle, but inside its bounding quad.
    properties[2] = number("ngon-rotate", 180);
    CHECK(hit(100, 115));
    CHECK(!hit(100, 85));
    properties[0] = number("ngon-radius", 0);
    CHECK(!hit(100, 100));
    std::cout << "n-gon descriptor, ownership, segmentation and query tests passed\n";
}
