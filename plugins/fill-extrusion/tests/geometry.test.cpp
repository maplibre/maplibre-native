#include <fill_extrusion.hpp>
#include <cmath>
#include <cstring>
#include <gtest/gtest.h>
#include <vector>
namespace {
const mln_plugin_descriptor_v1* descriptor;
mln_plugin_status capture(const mln_plugin_descriptor_v1* d, char*, size_t) {
    descriptor = d;
    return MLN_PLUGIN_STATUS_OK;
}
struct Geometry {
    const mln_plugin_layer_type_v1* layer;
    void* layout = nullptr;
    mln_plugin_bucket_v1 bucket{};
    Geometry() {
        bucket.struct_size = sizeof(bucket);
        EXPECT_EQ(MLN_PLUGIN_STATUS_OK, mln_fill_extrusion_register(capture, nullptr, 0));
        layer = descriptor->layer_types;
        mln_plugin_layout_context_v1 context{sizeof(context), 15, 8192, nullptr, 0};
        EXPECT_EQ(MLN_PLUGIN_STATUS_OK, layer->create_layout(&context, &layout));
    }
    ~Geometry() { layer->destroy_layout(layout); }
    void finish() { ASSERT_EQ(MLN_PLUGIN_STATUS_OK, layer->finish_layout(layout, &bucket)); }
};
TEST(FillExtrusionGeometry, RoofHolesWallsRangesAndQueries) {
    Geometry g;
    const mln_plugin_tile_point_v1 points[] = {
        {0, 0}, {100, 0}, {100, 100}, {0, 100}, {0, 0}, {25, 25}, {25, 75}, {75, 75}, {75, 25}, {25, 25}};
    const uint32_t paths[] = {0, 5, 10};
    mln_plugin_feature_v1 f{sizeof(f), MLN_PLUGIN_GEOMETRY_POLYGON, 42, points, 10, paths, 2};
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, g.layer->layout_feature(g.layout, &f));
    g.finish();
    ASSERT_EQ(2u, g.bucket.feature_vertex_range_count);
    EXPECT_EQ(42u, g.bucket.feature_vertex_ranges[0].feature_index);
    const auto& stream = g.bucket.vertex_streams[0];
    EXPECT_EQ(stream.vertex_count, g.bucket.feature_vertex_ranges[0].vertex_count);
    double roofArea = 0;
    size_t walls = 0;
    struct Position {
        double x, y;
    };
    const auto position = [&](size_t index) {
        int16_t packed[4];
        std::memcpy(packed, stream.data + index * stream.stride, sizeof(packed));
        const auto decimals = uint16_t(packed[2]);
        return Position{packed[0] + (decimals >> 9) / 128.0, packed[1] + ((decimals >> 1) & 127) / 128.0};
    };
    for (const auto& seg : std::vector<mln_plugin_segment_v1>(
             g.bucket.drawables[0].segments, g.bucket.drawables[0].segments + g.bucket.drawables[0].segment_count)) {
        for (size_t i = seg.index_offset; i < seg.index_offset + seg.index_length; i += 3) {
            const auto a = position(seg.vertex_offset + g.bucket.indices[i]);
            const auto b = position(seg.vertex_offset + g.bucket.indices[i + 1]);
            const auto c = position(seg.vertex_offset + g.bucket.indices[i + 2]);
            roofArea += std::abs((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) / 2;
        }
    }
    ASSERT_EQ(2u, g.bucket.drawable_count);
    const auto& wall = g.bucket.drawables[1];
    ASSERT_EQ(1u, wall.segment_count);
    EXPECT_EQ(4u, wall.segments[0].vertex_length);
    EXPECT_EQ(6u, wall.segments[0].index_length);
    EXPECT_EQ(stream.vertex_count - 1, wall.segments[0].instance_count);
    for (size_t i = 0; i < wall.segments[0].instance_count; ++i) {
        uint16_t packed;
        std::memcpy(&packed, stream.data + i * stream.stride + 4, sizeof(packed));
        if (!(packed & 1)) walls += 2;
    }
    EXPECT_DOUBLE_EQ(7500, roofArea); // Hole must not be roofed over.
    EXPECT_EQ(16u, walls);
    mln_plugin_query_context_v1 context{};
    context.struct_size = sizeof(context);
    context.pixels_to_tile_units = 1;
    const mln_plugin_tile_point_v1 hole{50, 50}, solid{10, 10};
    EXPECT_FALSE(g.layer->query_feature(&f, &hole, 1, &context, nullptr, 0));
    EXPECT_TRUE(g.layer->query_feature(&f, &solid, 1, &context, nullptr, 0));
    const uint32_t invalidPaths[] = {0, 11, 10};
    f.path_offsets = invalidPaths;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, g.layer->layout_feature(g.layout, &f));
}
TEST(FillExtrusionGeometry, SegmentsNeverOverflow16BitIndices) {
    Geometry g;
    std::vector<mln_plugin_tile_point_v1> points;
    std::vector<uint32_t> paths{0};
    for (int i = 0; i < 16000; ++i) {
        const auto x = int16_t(i % 100 * 70), y = int16_t(i / 100 * 70);
        points.insert(points.end(),
                      {{x, y}, {int16_t(x + 30), y}, {int16_t(x + 30), int16_t(y + 30)}, {x, int16_t(y + 30)}, {x, y}});
        paths.push_back(uint32_t(points.size()));
    }
    mln_plugin_feature_v1 f{
        sizeof(f), MLN_PLUGIN_GEOMETRY_POLYGON, 7, points.data(), points.size(), paths.data(), paths.size() - 1};
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, g.layer->layout_feature(g.layout, &f));
    g.finish();
    const auto& drawable = g.bucket.drawables[0];
    ASSERT_GT(drawable.segment_count, 1u);
    uint32_t total = 0;
    for (size_t s = 0; s < drawable.segment_count; ++s) {
        const auto& seg = drawable.segments[s];
        EXPECT_LE(seg.vertex_length, UINT16_MAX);
        EXPECT_EQ(total, seg.vertex_offset);
        for (size_t i = seg.index_offset; i < seg.index_offset + seg.index_length; ++i)
            ASSERT_LT(g.bucket.indices[i], seg.vertex_length);
        total += seg.vertex_length;
    }
    EXPECT_EQ(total, g.bucket.feature_vertex_ranges[0].vertex_count);
    EXPECT_EQ(total, g.bucket.vertex_streams[0].vertex_count);
}
TEST(FillExtrusionGeometry, OversizedOutlineUsesInstancedWallsAndSegmentedRoofs) {
    Geometry g;
    std::vector<mln_plugin_tile_point_v1> points;
    constexpr size_t count = 70000;
    for (size_t i = 0; i < count; ++i) {
        const double angle = double(i) * 2 * std::acos(-1) / count;
        points.push_back({int16_t(16000 * std::cos(angle)), int16_t(16000 * std::sin(angle))});
    }
    points.push_back(points.front());
    const uint32_t paths[] = {0, uint32_t(points.size())};
    mln_plugin_feature_v1 f{sizeof(f), MLN_PLUGIN_GEOMETRY_POLYGON, 9, points.data(), points.size(), paths, 1};
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, g.layer->layout_feature(g.layout, &f));
    g.finish();
    ASSERT_EQ(2u, g.bucket.drawable_count);
    const auto& stream = g.bucket.vertex_streams[0];
    const auto& roof = g.bucket.drawables[0];
    EXPECT_GT(stream.vertex_count, points.size()); // Roof copies avoid 16-bit overflow.
    for (size_t s = 0; s < roof.segment_count; ++s) {
        const auto& segment = roof.segments[s];
        EXPECT_LE(segment.vertex_length, UINT16_MAX);
        EXPECT_LE(size_t(segment.vertex_offset) + segment.vertex_length, stream.vertex_count);
        for (size_t i = segment.index_offset; i < segment.index_offset + segment.index_length; ++i)
            ASSERT_LT(g.bucket.indices[i], segment.vertex_length);
    }
    const auto& wall = g.bucket.drawables[1].segments[0];
    EXPECT_EQ(stream.vertex_count - 1, wall.instance_count);
    EXPECT_GT(wall.instance_count, UINT16_MAX);
    for (size_t i = points.size() - 1; i < stream.vertex_count; ++i) {
        uint16_t packed;
        std::memcpy(&packed, stream.data + i * stream.stride + 4, sizeof(packed));
        ASSERT_TRUE(packed & 1) << "Roof copies must not become walls";
    }
}
} // namespace
