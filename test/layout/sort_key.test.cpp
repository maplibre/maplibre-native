#include <mln/test/util.hpp>

#include <mln/geometry/feature_index.hpp>
#include <mln/layout/layout.hpp>
#include <mln/layermanager/circle_layer_factory.hpp>
#include <mln/layermanager/fill_layer_factory.hpp>
#include <mln/layermanager/line_layer_factory.hpp>
#include <mln/layermanager/symbol_layer_factory.hpp>
#include <mln/renderer/bucket_parameters.hpp>
#include <mln/renderer/buckets/circle_bucket.hpp>
#include <mln/renderer/buckets/fill_bucket.hpp>
#include <mln/renderer/buckets/line_bucket.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/expression/dsl.hpp>
#include <mln/style/layers/circle_layer_impl.hpp>
#include <mln/style/layers/fill_layer_impl.hpp>
#include <mln/style/layers/line_layer_impl.hpp>
#include <mln/style/layers/symbol_layer_impl.hpp>
#include <mln/tile/geojson_tile_data.hpp>

using namespace mln;
using namespace mln::style;

namespace {

enum class SortKeyType {
    Undefined,
    Constant,
    Camera,
    DataDriven
};

class SortKey : public ::testing::TestWithParam<SortKeyType> {
protected:
    PropertyValue<float> sortKey() const {
        using namespace expression::dsl;
        switch (GetParam()) {
            case SortKeyType::Undefined:
                return {};
            case SortKeyType::Constant:
                return 5.0f;
            case SortKeyType::Camera:
                return PropertyExpression<float>(step(zoom(), literal(5.0), 10.0, literal(10.0)));
            case SortKeyType::DataDriven:
                return PropertyExpression<float>(number(get("rank")));
        }
        return {};
    }

    std::vector<size_t> expectedOrder() const {
        switch (GetParam()) {
            case SortKeyType::Undefined:
                return {0, 1, 2};
            case SortKeyType::Constant:
            case SortKeyType::Camera:
                // Preserve the existing tie order of explicitly specified sort keys.
                return {2, 1, 0};
            case SortKeyType::DataDriven:
                return {1, 2, 0};
        }
        return {};
    }

    template <class Properties, class Layer>
    std::shared_ptr<Bucket> createBucket(const Layer& layer, LayerFactory&& factory, FeatureType type) {
        mapbox::feature::feature_collection<int16_t> features;
        const std::array<double, 3> ranks{2, 0, 1};
        for (size_t i = 0; i < ranks.size(); ++i) {
            const auto x = static_cast<int16_t>((i + 1) * 100);
            mapbox::geometry::geometry<int16_t> geometry;
            if (type == FeatureType::Polygon) {
                geometry = mapbox::geometry::polygon<int16_t>{
                    {{x, 10}, {x, 20}, {static_cast<int16_t>(x + 10), 20}, {x, 10}}};
            } else if (type == FeatureType::LineString) {
                geometry = mapbox::geometry::line_string<int16_t>{{x, 10}, {x, 20}};
            } else {
                geometry = mapbox::geometry::point<int16_t>{x, static_cast<int16_t>(300 - i * 100)};
            }
            features.emplace_back(std::move(geometry), PropertyMap{{"rank", ranks[i]}});
        }
        GeoJSONTileData data(std::move(features));
        const BucketParameters bucketParameters{.tileID = OverscaledTileID(0, 0, 0),
                                                .mode = MapMode::Continuous,
                                                .pixelRatio = 1.0f,
                                                .layerType = factory.getTypeInfo()};
        GlyphDependencies glyphDependencies;
        ImageDependencies imageDependencies;
        std::set<std::string> availableImages{"marker"};
        const LayoutParameters parameters{.bucketParameters = bucketParameters,
                                          .fontFaces = nullptr,
                                          .glyphDependencies = glyphDependencies,
                                          .imageDependencies = imageDependencies,
                                          .availableImages = availableImages};
        std::vector<Immutable<LayerProperties>> group{
            makeMutable<Properties>(staticImmutableCast<typename Layer::Impl>(layer.baseImpl))};
        auto layout = factory.createLayout(parameters, data.getLayer(""), group);

        ImageMap images{{"marker", makeMutable<style::Image::Impl>("marker", PremultipliedImage({16, 16}), 1.0f)}};
        ImagePositions imagePositions{{"marker", ImagePosition({0, 0, 18, 18}, *images.at("marker"))}};
        layout->prepareSymbols({}, {}, images, imagePositions);
        auto featureIndex = std::make_unique<FeatureIndex>(data.clone());
        mln::unordered_map<std::string, LayerRenderData> renderData;
        layout->createBucket(imagePositions, featureIndex, renderData, true, false, bucketParameters.tileID.canonical);
        return renderData.at(layer.getID()).bucket;
    }

    template <class Vertices>
    static std::vector<size_t> featureOrder(const Vertices& vertices, int coordinateScale) {
        std::vector<size_t> order;
        for (const auto& vertex : vertices.vector()) {
            const size_t index = vertex.a1[0] / coordinateScale / 100 - 1;
            if (order.empty() || order.back() != index) {
                order.push_back(index);
            }
        }
        return order;
    }
};

TEST_P(SortKey, FillOrder) {
    FillLayer layer("fill", "source");
    layer.setFillSortKey(sortKey());
    auto bucket = std::static_pointer_cast<FillBucket>(
        createBucket<FillLayerProperties>(layer, FillLayerFactory{}, FeatureType::Polygon));
    EXPECT_EQ(expectedOrder(), featureOrder(bucket->vertices, 1));
}

TEST_P(SortKey, LineOrder) {
    LineLayer layer("line", "source");
    layer.setLineSortKey(sortKey());
    auto bucket = std::static_pointer_cast<LineBucket>(
        createBucket<LineLayerProperties>(layer, LineLayerFactory{}, FeatureType::LineString));
    EXPECT_EQ(expectedOrder(), featureOrder(bucket->vertices, 2));
}

TEST_P(SortKey, CircleOrder) {
    CircleLayer layer("circle", "source");
    layer.setCircleSortKey(sortKey());
    auto bucket = std::static_pointer_cast<CircleBucket>(
        createBucket<CircleLayerProperties>(layer, CircleLayerFactory{}, FeatureType::Point));
    EXPECT_EQ(expectedOrder(), featureOrder(bucket->vertices, 2));
    ASSERT_FALSE(bucket->segments.empty());
    EXPECT_FLOAT_EQ(GetParam() == SortKeyType::Constant || GetParam() == SortKeyType::Camera ? 5.0f : 0.0f,
                    bucket->segments.front().sortKey);
}

TEST_P(SortKey, SymbolOrder) {
    SymbolLayer layer("symbol", "source");
    layer.setIconImage(expression::Image("marker"));
    layer.setIconAllowOverlap(true);
    layer.setSymbolSortKey(sortKey());
    auto bucket = std::static_pointer_cast<SymbolBucket>(
        createBucket<SymbolLayerProperties>(layer, SymbolLayerFactory{}, FeatureType::Point));
    std::vector<size_t> order;
    for (const auto& symbol : bucket->symbolInstances) {
        order.push_back(symbol.getDataFeatureIndex());
    }
    EXPECT_EQ(expectedOrder(), order);
    // An explicit constant key still disables viewport-Y sorting for symbol-z-order:auto.
    EXPECT_EQ(GetParam() == SortKeyType::Undefined, bucket->sortFeaturesByY);
    if (GetParam() == SortKeyType::Undefined) {
        EXPECT_TRUE(bucket->sortKeyRanges.empty());
    } else if (GetParam() == SortKeyType::Constant || GetParam() == SortKeyType::Camera) {
        ASSERT_EQ(1u, bucket->sortKeyRanges.size());
        EXPECT_FLOAT_EQ(5.0f, bucket->sortKeyRanges.front().sortKey);
        EXPECT_EQ(0u, bucket->sortKeyRanges.front().start);
        EXPECT_EQ(3u, bucket->sortKeyRanges.front().end);
    } else {
        EXPECT_EQ(3u, bucket->sortKeyRanges.size());
    }
}

TEST_P(SortKey, SymbolViewportY) {
    SymbolLayer layer("symbol", "source");
    layer.setIconImage(expression::Image("marker"));
    layer.setIconAllowOverlap(true);
    layer.setSymbolSortKey(sortKey());
    layer.setSymbolZOrder(SymbolZOrderType::ViewportY);
    auto bucket = std::static_pointer_cast<SymbolBucket>(
        createBucket<SymbolLayerProperties>(layer, SymbolLayerFactory{}, FeatureType::Point));
    EXPECT_TRUE(bucket->sortFeaturesByY);
    EXPECT_TRUE(bucket->sortKeyRanges.empty());
    std::vector<size_t> order;
    for (const auto& symbol : bucket->symbolInstances) {
        order.push_back(symbol.getDataFeatureIndex());
    }
    EXPECT_EQ((std::vector<size_t>{0, 1, 2}), order);
}

INSTANTIATE_TEST_SUITE_P(
    Layout,
    SortKey,
    ::testing::Values(SortKeyType::Undefined, SortKeyType::Constant, SortKeyType::Camera, SortKeyType::DataDriven));

} // namespace
