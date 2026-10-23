#if MLN_RENDER_BACKEND_OPENGL

#include <mln/test/util.hpp>
#include <mln/test/stub_geometry_tile_feature.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/drawable_builder.hpp>
#include <mln/gfx/index_vector.hpp>
#include <mln/renderer/buckets/circle_bucket.hpp>
#include <mln/renderer/buckets/fill_bucket.hpp>
#include <mln/renderer/buckets/line_bucket.hpp>
#include <mln/renderer/buckets/raster_bucket.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>
#include <mln/renderer/bucket_parameters.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/layer.hpp>
#include <mln/style/layers/fill_layer.hpp>
#include <mln/style/layers/fill_layer_impl.hpp>
#include <mln/style/layers/fill_layer_properties.hpp>
#include <mln/style/layers/symbol_layer_properties.hpp>
#include <mln/gl/context.hpp>
#include <mln/gl/headless_backend.hpp>
#include <mln/gl/upload_pass.hpp>

#include <mln/map/mode.hpp>

namespace mln {

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
} // namespace mln

using namespace mln;

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

namespace {

struct StubIndexBuffer final : gfx::IndexBufferBase {};

// Builds a fill bucket whose "fill-color" is the given data-driven expression, with one polygon.
std::unique_ptr<FillBucket> makeDataDrivenFillBucket(const std::string& fillColor) {
    style::conversion::Error error;
    auto layer = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"type": "fill", "id": "fill", "source": "s", "paint": {"fill-color": )" + fillColor + "}}", error);
    EXPECT_TRUE(layer) << error.message;

    style::FillPaintProperties::PossiblyEvaluated evaluated;
    evaluated.get<style::FillColor>() = PossiblyEvaluatedPropertyValue<Color>(
        static_cast<style::FillLayer&>(**layer).getFillColor().asExpression());
    Immutable<style::LayerProperties> layerProperties = makeMutable<style::FillLayerProperties>(
        staticImmutableCast<style::FillLayer::Impl>(layer.value()->baseImpl),
        CrossfadeParameters{},
        std::move(evaluated));
    auto bucket = std::make_unique<FillBucket>(
        FillBucket::PossiblyEvaluatedLayoutProperties{},
        std::map<std::string, Immutable<style::LayerProperties>>{{"fill", layerProperties}},
        5.0f,
        1);

    GeometryCollection polygon{{{0, 0}, {0, 1}, {1, 1}}};
    bucket->addFeature(StubGeometryTileFeature{{}, FeatureType::Polygon, polygon, PropertyMap{{"c", "red"}}},
                       polygon,
                       {},
                       PatternLayerMap(),
                       0,
                       CanonicalTileID(0, 0, 0));
    return bucket;
}

} // namespace

TEST(Buckets, FillBucketFreesCpuGeometryAfterUpload) {
    gl::HeadlessBackend backend({512, 256});
    gfx::BackendScope scope{backend};
    gl::Context context{backend};

    auto bucket = makeDataDrivenFillBucket(R"(["to-color", ["get", "c"]])");
    const auto vertexCount = bucket->vertices.elements();
    const auto indexCount = bucket->triangles.elements();
    ASSERT_GT(vertexCount, 0u);
    ASSERT_GT(indexCount, 0u);

    auto commandEncoder = context.createCommandEncoder();
    auto uploadPass = commandEncoder->createUploadPass("upload", backend.getDefaultRenderable());
    auto& glUploadPass = static_cast<gl::UploadPass&>(*uploadPass);
    bucket->upload(*uploadPass);

    // The bucket's layout is final, but nothing is in a GPU buffer yet, so the data stays.
    EXPECT_EQ(vertexCount, bucket->vertices.vector().size());

    // A drawable upload turns the shared vectors into buffers.
    const auto& buffer = glUploadPass.getBuffer(bucket->sharedVertices, gfx::BufferUsageType::StaticDraw);
    ASSERT_TRUE(buffer);
    bucket->triangles.setBuffer(std::make_unique<StubIndexBuffer>());
    bucket->triangles.setDirty(false);

    // The CPU copies are gone, the counts that drawables are built from are not.
    EXPECT_EQ(0u, bucket->vertices.vector().capacity());
    EXPECT_EQ(vertexCount, bucket->vertices.elements());
    EXPECT_EQ(0u, bucket->triangles.vector().capacity());
    EXPECT_EQ(indexCount, bucket->triangles.elements());

    // A second drawable over the same bucket reuses the buffer without reading the data.
    EXPECT_EQ(&buffer, &glUploadPass.getBuffer(bucket->sharedVertices, gfx::BufferUsageType::StaticDraw));

    // Data-driven paint values are freed too when nothing can rewrite them.
    const auto& paint = bucket->paintPropertyBinders.at("fill").interleavedVertexBuffer.sharedVertexVector;
    ASSERT_GT(paint->elements(), 0u);
    ASSERT_TRUE(glUploadPass.getBuffer(paint, gfx::BufferUsageType::StaticDraw));
    EXPECT_EQ(0u, paint->vector().capacity());
}

TEST(Buckets, FillBucketKeepsPaintDataThatFeatureStateRewrites) {
    gl::HeadlessBackend backend({512, 256});
    gfx::BackendScope scope{backend};
    gl::Context context{backend};

    auto bucket = makeDataDrivenFillBucket(
        R"(["to-color", ["case", ["boolean", ["feature-state", "hover"], false], "red", "blue"]])");

    auto commandEncoder = context.createCommandEncoder();
    auto uploadPass = commandEncoder->createUploadPass("upload", backend.getDefaultRenderable());
    bucket->upload(*uploadPass);

    const auto& paint = bucket->paintPropertyBinders.at("fill").interleavedVertexBuffer.sharedVertexVector;
    ASSERT_GT(paint->elements(), 0u);
    ASSERT_TRUE(static_cast<gl::UploadPass&>(*uploadPass).getBuffer(paint, gfx::BufferUsageType::StaticDraw));
    EXPECT_EQ(paint->elements(), paint->vector().size());
}

TEST(Buckets, FillBucketKeepsCpuGeometryWhenRenderedFeaturesAreRetained) {
    gl::HeadlessBackend backend({512, 256});
    gfx::BackendScope scope{backend};
    gl::Context context{backend};

    auto bucket = makeDataDrivenFillBucket(R"(["to-color", ["get", "c"]])");
    bucket->setRetainFeaturesById(true);
    const auto vertexCount = bucket->vertices.elements();

    auto commandEncoder = context.createCommandEncoder();
    auto uploadPass = commandEncoder->createUploadPass("upload", backend.getDefaultRenderable());
    auto& glUploadPass = static_cast<gl::UploadPass&>(*uploadPass);
    bucket->upload(*uploadPass);
    ASSERT_TRUE(glUploadPass.getBuffer(bucket->sharedVertices, gfx::BufferUsageType::StaticDraw));

    // Reading rendered features back needs the vertices.
    EXPECT_EQ(vertexCount, bucket->vertices.vector().size());
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

TEST(Buckets, LineBucketDrawablesBuildFromFreedIndexes) {
    gl::HeadlessBackend backend({512, 256});
    gfx::BackendScope scope{backend};
    gl::Context context{backend};

    LineBucket bucket{LineBucket::PossiblyEvaluatedLayoutProperties{}, {}, 10.0f, 1};
    GeometryCollection line{{{0, 0}, {100, 100}, {200, 0}}};
    bucket.addFeature(StubGeometryTileFeature{{}, FeatureType::LineString, line, properties},
                      line,
                      {},
                      PatternLayerMap(),
                      0,
                      CanonicalTileID(0, 0, 0));
    ASSERT_TRUE(bucket.hasData());

    auto commandEncoder = context.createCommandEncoder();
    auto uploadPass = commandEncoder->createUploadPass("upload", backend.getDefaultRenderable());
    bucket.upload(*uploadPass);

    // An earlier drawable put the indexes in a buffer, which frees their CPU copy.
    const auto vertexCount = bucket.vertices.elements();
    const auto indexCount = bucket.triangles.elements();
    bucket.triangles.setBuffer(std::make_unique<StubIndexBuffer>());
    bucket.triangles.setDirty(false);
    ASSERT_EQ(0u, bucket.triangles.vector().size());

    // A layer that rebuilds its drawables for the tile builds the segments from the counts.
    auto builder = context.createDrawableBuilder("line");
    builder->setRawVertices(std::vector<uint8_t>(vertexCount * 4), vertexCount, gfx::AttributeDataType::Short2);
    builder->setSegments(gfx::Triangles(), bucket.sharedTriangles, bucket.segments.data(), bucket.segments.size());
    EXPECT_EQ(vertexCount, builder->curVertexCount());
    EXPECT_EQ(indexCount, bucket.triangles.elements());
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
