#if MLN_RENDER_BACKEND_OPENGL

#include <mbgl/test/util.hpp>
#include <mbgl/test/stub_geometry_tile_feature.hpp>

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/renderer/buckets/circle_bucket.hpp>
#include <mbgl/renderer/buckets/fill_bucket.hpp>
#include <mbgl/renderer/buckets/line_bucket.hpp>
#include <mbgl/renderer/buckets/raster_bucket.hpp>
#include <mbgl/renderer/buckets/symbol_bucket.hpp>
#include <mbgl/renderer/bucket_parameters.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/headless_backend.hpp>

#include <mbgl/map/mode.hpp>

namespace mbgl {

bool operator==(const SegmentBase& lhs, const SegmentBase& rhs) {
    return std::tie(lhs.vertexOffset, lhs.indexOffset, lhs.vertexLength, lhs.indexLength) ==
           std::tie(rhs.vertexOffset, rhs.indexOffset, rhs.vertexLength, rhs.indexLength);
}

namespace gfx {
namespace detail {

template <class A1, class A2>
bool operator==(const VertexType<A1, A2>& lhs, const VertexType<A1, A2>& rhs) {
    return std::tie(lhs.a1, lhs.a2) == std::tie(rhs.a1, rhs.a2);
}

} // namespace detail
} // namespace gfx
} // namespace mbgl

using namespace mbgl;

namespace {

PropertyMap properties;

} // namespace

TEST(Buckets, CircleBucket) {
    gl::HeadlessBackend backend({512, 256});
    gfx::BackendScope scope{backend};

    gl::Context context{backend};
    CircleBucket bucket{{}, MapMode::Static, 1.0};
    ASSERT_FALSE(bucket.hasData());
    ASSERT_FALSE(bucket.needsUpload());

    // CircleBucket::addFeature() is a no-op.
    GeometryCollection point{{{0, 0}}};
    bucket.addFeature(StubGeometryTileFeature{{}, FeatureType::Point, point, properties},
                      point,
                      {},
                      PatternLayerMap(),
                      0,
                      CanonicalTileID(0, 0, 0));
    ASSERT_FALSE(bucket.hasData());
    ASSERT_FALSE(bucket.needsUpload());

    bucket.segments.emplace_back(0, 0);
    ASSERT_TRUE(bucket.hasData());
    ASSERT_TRUE(bucket.needsUpload());

    auto commandEncoder = context.createCommandEncoder();
    auto uploadPass = commandEncoder->createUploadPass("upload", backend.getDefaultRenderable());
    bucket.upload(*uploadPass);
    ASSERT_TRUE(bucket.hasData());
    ASSERT_FALSE(bucket.needsUpload());
}

TEST(Buckets, FillBucket) {
    gl::HeadlessBackend backend({512, 256});
    gfx::BackendScope scope{backend};
    FillBucket::PossiblyEvaluatedLayoutProperties layout;

    gl::Context context{backend};
    FillBucket bucket{layout, {}, 5.0f, 1};
    ASSERT_FALSE(bucket.hasData());
    ASSERT_FALSE(bucket.needsUpload());

    GeometryCollection polygon{{{0, 0}, {0, 1}, {1, 1}}};
    bucket.addFeature(StubGeometryTileFeature{{}, FeatureType::Polygon, polygon, properties},
                      polygon,
                      {},
                      PatternLayerMap(),
                      0,
                      CanonicalTileID(0, 0, 0));
    ASSERT_TRUE(bucket.hasData());
    ASSERT_TRUE(bucket.needsUpload());

    auto commandEncoder = context.createCommandEncoder();
    auto uploadPass = commandEncoder->createUploadPass("upload", backend.getDefaultRenderable());
    bucket.upload(*uploadPass);
    ASSERT_FALSE(bucket.needsUpload());
}

TEST(Buckets, LineBucket) {
    gl::HeadlessBackend backend({512, 256});
    gfx::BackendScope scope{backend};
    LineBucket::PossiblyEvaluatedLayoutProperties layout;

    gl::Context context{backend};
    LineBucket bucket{layout, {}, 10.0f, 1};
    ASSERT_FALSE(bucket.hasData());
    ASSERT_FALSE(bucket.needsUpload());

    // Ignore invalid feature type.
    GeometryCollection point{{{0, 0}}};
    bucket.addFeature(StubGeometryTileFeature{{}, FeatureType::Point, point, properties},
                      point,
                      {},
                      PatternLayerMap(),
                      0,
                      CanonicalTileID(0, 0, 0));
    ASSERT_FALSE(bucket.hasData());

    GeometryCollection line{{{0, 0}, {1, 1}}};
    bucket.addFeature(StubGeometryTileFeature{{}, FeatureType::LineString, line, properties},
                      line,
                      {},
                      PatternLayerMap(),
                      1,
                      CanonicalTileID(0, 0, 0));
    ASSERT_TRUE(bucket.hasData());
    ASSERT_TRUE(bucket.needsUpload());

    auto commandEncoder = context.createCommandEncoder();
    auto uploadPass = commandEncoder->createUploadPass("upload", backend.getDefaultRenderable());
    bucket.upload(*uploadPass);
    ASSERT_FALSE(bucket.needsUpload());
}

TEST(Buckets, SymbolBucket) {
    gl::HeadlessBackend backend({512, 256});
    gfx::BackendScope scope{backend};

    auto layout = makeMutable<style::SymbolLayoutProperties::PossiblyEvaluated>();
    bool iconsNeedLinear = false;
    bool sortFeaturesByY = false;
    std::string bucketLeaderID = "test";
    std::vector<SymbolInstance> symbolInstances;
    std::vector<SortKeyRange> symbolRanges;

    gl::Context context{backend};
    SymbolBucket bucket{std::move(layout),
                        {},
                        16.0f,
                        1.0f,
                        0,
                        iconsNeedLinear,
                        sortFeaturesByY,
                        bucketLeaderID,
                        std::move(symbolInstances),
                        std::move(symbolRanges),
                        1.0f,
                        false,
                        {},
                        false /*iconsInText*/};
    ASSERT_FALSE(bucket.hasIconData());
    ASSERT_FALSE(bucket.hasSdfIconData());
    ASSERT_FALSE(bucket.hasTextData());
    ASSERT_FALSE(bucket.hasIconCollisionBoxData());
    ASSERT_FALSE(bucket.hasTextCollisionBoxData());
    ASSERT_FALSE(bucket.hasIconCollisionCircleData());
    ASSERT_FALSE(bucket.hasTextCollisionCircleData());
    ASSERT_FALSE(bucket.hasData());
    ASSERT_FALSE(bucket.needsUpload());

    // SymbolBucket::addFeature() is a no-op.
    GeometryCollection point{{{0, 0}}};
    bucket.addFeature(StubGeometryTileFeature{{}, FeatureType::Point, std::move(point), properties},
                      point,
                      {},
                      PatternLayerMap(),
                      0,
                      CanonicalTileID(0, 0, 0));
    ASSERT_FALSE(bucket.hasData());
    ASSERT_FALSE(bucket.needsUpload());

    bucket.text.segments.emplace_back(0, 0);
    ASSERT_TRUE(bucket.hasTextData());
    ASSERT_TRUE(bucket.hasData());
    ASSERT_TRUE(bucket.needsUpload());

    auto commandEncoder = context.createCommandEncoder();
    auto uploadPass = commandEncoder->createUploadPass("upload", backend.getDefaultRenderable());
    bucket.upload(*uploadPass);
    ASSERT_FALSE(bucket.needsUpload());
}

TEST(Buckets, RasterBucket) {
    gl::HeadlessBackend backend({512, 256});
    gfx::BackendScope scope{backend};

    gl::Context context{backend};
    PremultipliedImage rgba({1, 1});

    // RasterBucket::hasData() is always true.
    RasterBucket bucket = {std::move(rgba)};
    ASSERT_TRUE(bucket.hasData());
    ASSERT_TRUE(bucket.needsUpload());

    auto commandEncoder = context.createCommandEncoder();
    auto uploadPass = commandEncoder->createUploadPass("upload", backend.getDefaultRenderable());
    bucket.upload(*uploadPass);
    ASSERT_FALSE(bucket.needsUpload());

    bucket.clear();
    ASSERT_TRUE(bucket.needsUpload());
}

TEST(Buckets, RasterBucketMaskEmpty) {
    RasterBucket bucket{nullptr};
    bucket.setMask({});
    EXPECT_EQ((std::vector<RasterLayoutVertex>{}), bucket.vertices.vector());
    EXPECT_EQ((std::vector<uint16_t>{}), bucket.indices.vector());
    SegmentVector expectedSegments;
    expectedSegments.emplace_back(0, 0, 0, 0);
    EXPECT_EQ(expectedSegments, bucket.segments);
}

TEST(Buckets, RasterBucketMaskNoChildren) {
    RasterBucket bucket{nullptr};
    bucket.setMask({CanonicalTileID{0, 0, 0}});

    // A mask of 0/0/0 doesn't produce buffers since we're instead using the global shared buffers.
    EXPECT_EQ((std::vector<RasterLayoutVertex>{}), bucket.vertices.vector());
    EXPECT_EQ((std::vector<uint16_t>{}), bucket.indices.vector());
    EXPECT_EQ((SegmentVector{}), bucket.segments);
}

TEST(Buckets, RasterBucketMaskTwoChildren) {
    RasterBucket bucket{nullptr};
    bucket.setMask({CanonicalTileID{1, 0, 0}, CanonicalTileID{1, 1, 1}});

    EXPECT_EQ((std::vector<RasterLayoutVertex>{
                  // 1/0/1
                  RasterBucket::layoutVertex({0, 0}, {0, 0}),
                  RasterBucket::layoutVertex({4096, 0}, {4096, 0}),
                  RasterBucket::layoutVertex({0, 4096}, {0, 4096}),
                  RasterBucket::layoutVertex({4096, 4096}, {4096, 4096}),

                  // 1/1/1
                  RasterBucket::layoutVertex({4096, 4096}, {4096, 4096}),
                  RasterBucket::layoutVertex({8192, 4096}, {8192, 4096}),
                  RasterBucket::layoutVertex({4096, 8192}, {4096, 8192}),
                  RasterBucket::layoutVertex({8192, 8192}, {8192, 8192}),
              }),
              bucket.vertices.vector());

    EXPECT_EQ((std::vector<uint16_t>{
                  // 1/0/1
                  0,
                  1,
                  2,
                  1,
                  2,
                  3,

                  // 1/1/1
                  4,
                  5,
                  6,
                  5,
                  6,
                  7,
              }),
              bucket.indices.vector());

    SegmentVector expectedSegments;
    expectedSegments.emplace_back(0, 0, 8, 12);
    EXPECT_EQ(expectedSegments, bucket.segments);
}

TEST(Buckets, RasterBucketMaskComplex) {
    RasterBucket bucket{nullptr};
    bucket.setMask({CanonicalTileID{1, 0, 1},
                    CanonicalTileID{1, 1, 0},
                    CanonicalTileID{2, 2, 3},
                    CanonicalTileID{2, 3, 2},
                    CanonicalTileID{3, 6, 7},
                    CanonicalTileID{3, 7, 6}});

    EXPECT_EQ((std::vector<RasterLayoutVertex>{
                  // 1/0/1
                  RasterBucket::layoutVertex({0, 4096}, {0, 4096}),
                  RasterBucket::layoutVertex({4096, 4096}, {4096, 4096}),
                  RasterBucket::layoutVertex({0, 8192}, {0, 8192}),
                  RasterBucket::layoutVertex({4096, 8192}, {4096, 8192}),

                  // 1/1/0
                  RasterBucket::layoutVertex({4096, 0}, {4096, 0}),
                  RasterBucket::layoutVertex({8192, 0}, {8192, 0}),
                  RasterBucket::layoutVertex({4096, 4096}, {4096, 4096}),
                  RasterBucket::layoutVertex({8192, 4096}, {8192, 4096}),

                  // 2/2/3
                  RasterBucket::layoutVertex({4096, 6144}, {4096, 6144}),
                  RasterBucket::layoutVertex({6144, 6144}, {6144, 6144}),
                  RasterBucket::layoutVertex({4096, 8192}, {4096, 8192}),
                  RasterBucket::layoutVertex({6144, 8192}, {6144, 8192}),

                  // 2/3/2
                  RasterBucket::layoutVertex({6144, 4096}, {6144, 4096}),
                  RasterBucket::layoutVertex({8192, 4096}, {8192, 4096}),
                  RasterBucket::layoutVertex({6144, 6144}, {6144, 6144}),
                  RasterBucket::layoutVertex({8192, 6144}, {8192, 6144}),

                  // 3/6/7
                  RasterBucket::layoutVertex({6144, 7168}, {6144, 7168}),
                  RasterBucket::layoutVertex({7168, 7168}, {7168, 7168}),
                  RasterBucket::layoutVertex({6144, 8192}, {6144, 8192}),
                  RasterBucket::layoutVertex({7168, 8192}, {7168, 8192}),

                  // 3/7/6
                  RasterBucket::layoutVertex({7168, 6144}, {7168, 6144}),
                  RasterBucket::layoutVertex({8192, 6144}, {8192, 6144}),
                  RasterBucket::layoutVertex({7168, 7168}, {7168, 7168}),
                  RasterBucket::layoutVertex({8192, 7168}, {8192, 7168}),
              }),
              bucket.vertices.vector());

    EXPECT_EQ((std::vector<uint16_t>{
                  // 1/0/1
                  0,
                  1,
                  2,
                  1,
                  2,
                  3,

                  // 1/1/0
                  4,
                  5,
                  6,
                  5,
                  6,
                  7,

                  // 2/2/3
                  8,
                  9,
                  10,
                  9,
                  10,
                  11,

                  // 2/3/2
                  12,
                  13,
                  14,
                  13,
                  14,
                  15,

                  // 3/6/7
                  16,
                  17,
                  18,
                  17,
                  18,
                  19,

                  // 3/7/6
                  20,
                  21,
                  22,
                  21,
                  22,
                  23,
              }),
              bucket.indices.vector());

    SegmentVector expectedSegments;
    expectedSegments.emplace_back(0, 0, 24, 36);
    EXPECT_EQ(expectedSegments, bucket.segments);
}

#endif
