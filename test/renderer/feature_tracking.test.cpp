#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/map/map_observer.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/style/expression/dsl.hpp>
#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/layers/circle_layer.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/layers/symbol_layer.hpp>
#include <mbgl/style/sources/custom_geometry_source.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/test/map_adapter.hpp>
#include <mbgl/test/stub_file_source.hpp>
#include <mbgl/test/util.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/run_loop.hpp>

#include <mapbox/geojson.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <optional>

using namespace mbgl;
using namespace mbgl::style;
using namespace mbgl::style::expression;
using namespace mbgl::util;

struct FeatureTrackingTest {
    util::RunLoop loop;
    HeadlessFrontend frontend;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>();
    MapAdapter map;

    FeatureTrackingTest(const CameraOptions& cameraOptions = CameraOptions())
        : FeatureTrackingTest(defaultGeometry(), cameraOptions) {}

    FeatureTrackingTest(const mapbox::geojson::geojson& geojson, const CameraOptions& cameraOptions = CameraOptions())
        : frontend(1.0f),
          map(frontend,
              MapObserver::nullObserver(),
              fileSource,
              MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()).withRenderedFeatureInfo(true)) {
        fileSource->glyphsResponse = [&](const Resource&) {
            Response response;
            response.data = std::make_shared<std::string>(util::read_file(glyphPath));
            return response;
        };

        auto source = std::make_unique<GeoJSONSource>(sourceName);
        source->setGeoJSON(geojson);

        map.getStyle().loadJSON(util::read_file(fixtureBase + "api/empty.json"));
        map.getStyle().addSource(std::move(source));

        auto bgLayer = std::make_unique<BackgroundLayer>("background");
        bgLayer->setBackgroundColor({{1, 1, 1, 1}});
        map.getStyle().addLayer(std::move(bgLayer));

        auto fillLayer = std::make_unique<FillLayer>(fillLayerName, sourceName);
        fillLayer->setVisibility(VisibilityType::None);
        fillLayer->setFillColor({{0, 0, 1, 1}});
        map.getStyle().addLayer(std::move(fillLayer));

        auto lineLayer = std::make_unique<LineLayer>(lineLayerName, sourceName);
        lineLayer->setVisibility(VisibilityType::None);
        lineLayer->setLineColor({{0, 0, 0, 1}});
        lineLayer->setLineWidth({2.0f});
        map.getStyle().addLayer(std::move(lineLayer));

        auto circleLayer = std::make_unique<CircleLayer>(circleLayerName, sourceName);
        circleLayer->setVisibility(VisibilityType::None);
        circleLayer->setCircleColor({{0, 0, 0, 1}});
        circleLayer->setCircleOpacity(0);
        circleLayer->setCircleRadius({30.0f});
        circleLayer->setCircleStrokeColor({{0, 0, 0, 1}});
        circleLayer->setCircleStrokeWidth({2.0f});
        map.getStyle().addLayer(std::move(circleLayer));

        auto symLayer = std::make_unique<SymbolLayer>(symbolLayerName, sourceName);
        symLayer->setVisibility(VisibilityType::None);
        symLayer->setIconAllowOverlap(false);
        symLayer->setSymbolPlacement(SymbolPlacementType::Point);
        symLayer->setIconImage({markerName});
        symLayer->setTextField({dsl::format(dsl::get(dsl::literal("name")))});
        symLayer->setTextSize({24.0f});
        symLayer->setTextColor({{0, 0, 0, 1}});
        symLayer->setTextAnchor(SymbolAnchorType::Right);
        symLayer->setTextOffset({{1, 0}});
        map.getStyle().addLayer(std::move(symLayer));

        auto image = decodeImage(util::read_file(markerPath));
        map.getStyle().addImage(std::make_unique<style::Image>(markerName, std::move(image), 1.0f));

        map.setDebug(MapDebugOptions::TileBorders);
        map.jumpTo(cameraOptions);
    }
    auto run(std::vector<std::string> images = {currentTestName()},
             double imageThreshold = 0.0,
             double pixelThreshold = 0.1) {
        auto result = frontend.render(map);

        if (!images.empty()) {
            std::ranges::transform(images, images.begin(), [&](const auto& img) { return fixturePath + img; });
#if !TEST_READ_ONLY
            std::ranges::for_each(images, [](const auto& img) { std::filesystem::create_directories(img); });
#endif
            test::checkImages(images, result.image, imageThreshold, pixelThreshold);
        }
        return result;
    }

    auto getLineLayer() { return static_cast<LineLayer*>(map.getStyle().getLayer(lineLayerName)); }
    auto getFillLayer() { return static_cast<FillLayer*>(map.getStyle().getLayer(fillLayerName)); }
    auto getCircleLayer() { return static_cast<CircleLayer*>(map.getStyle().getLayer(circleLayerName)); }
    auto getSymbolLayer() { return static_cast<SymbolLayer*>(map.getStyle().getLayer(symbolLayerName)); }

    static mapbox::geojson::feature_collection defaultGeometry() {
        return {
            {Geometry<double>{Point{0.01, 0.02}}, {{"name", "0"}, {"opacity", 0.5}}, "pt0"},
            {Geometry<double>{Point{0.012, 0.022}}, {{"name", "0"}}, "pt0_overlap"},
            {Geometry<double>{Point{0.03, -0.04}}, {{"name", "1"}, {"opacity", 0.0}}, "pt1"},
            {Geometry<double>{Point{0.1, 0.2}}, {{"name", "2"}}, "pt2"}, // outside the rendered area
            {Geometry<double>{LineString<double>{{0.01, 0.02}, {0.03, -0.04}}}, {{"name", "3"}}, "l1"},
            {Geometry<double>{Polygon<double>{{{0.01, 0.02}, {0.03, -0.04}, {-0.07, -0.08}, {-0.06, 0.04}}}},
             {{"name", "4"}},
             "poly0"},
        };
    }
    static std::string currentTestName() {
        auto testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
        return testInfo ? testInfo->name() : std::string{};
    }

    const std::string sourceName = "source";
    const std::string markerName = "marker";
    const std::string lineLayerName = "line";
    const std::string fillLayerName = "fill";
    const std::string circleLayerName = "circle";
    const std::string symbolLayerName = "symbol";

    const std::string fixtureBase = "test/fixtures/";
    const std::string fixturePath = fixtureBase + "feature_tracking/";
    const std::string glyphPath = fixtureBase + "resources/glyphs.pbf";
    const std::string markerPath = fixtureBase + "sprites/default_marker.png";
};

TEST(FeatureTracking, ApiQuery) {
    FeatureTrackingTest test(CameraOptions().withZoom(10));
    test.getSymbolLayer()->setVisibility(VisibilityType::Visible);
    test.run();

    // Positive and negative results on each filter criterion
    const std::optional<std::string> any;
    EXPECT_EQ(3, test.map.getRenderedFeatureCount());
    EXPECT_EQ(3, test.map.getRenderedFeatureCount(any, any, any));
    EXPECT_EQ(1, test.map.getRenderedFeatureCount("pt0"));
    EXPECT_EQ(0, test.map.getRenderedFeatureCount("pt2"));
    EXPECT_EQ(1, test.map.getRenderedFeatureCount("pt0", "symbol"));
    EXPECT_EQ(0, test.map.getRenderedFeatureCount("pt0", "symbol0"));
    EXPECT_EQ(3, test.map.getRenderedFeatureCount(any, "symbol"));
    EXPECT_EQ(0, test.map.getRenderedFeatureCount(any, "symbol0"));
    EXPECT_EQ(3, test.map.getRenderedFeatureCount(any, any, test.sourceName));
    EXPECT_EQ(0, test.map.getRenderedFeatureCount(any, any, test.sourceName + "x"));
    EXPECT_EQ(1, test.map.getRenderedFeatureCount("pt0", "symbol", test.sourceName));
    EXPECT_EQ(0, test.map.getRenderedFeatureCount("pt0", "symbol", test.sourceName + "x"));
}

TEST(FeatureTracking, NDCBoundSymbol) {
    FeatureTrackingTest test(CameraOptions().withZoom(10.0));
    test.getSymbolLayer()->setVisibility(VisibilityType::Visible);
    test.getLineLayer()->setVisibility(VisibilityType::Visible);
    test.run();

    EXPECT_EQ(1, test.map.getRenderedFeatureCount("pt0"));
    test.map.getRenderedFeatures("pt0", std::nullopt, std::nullopt, [](const auto&, const auto& info) -> bool {
        EXPECT_GT(info.ndcBound.minX, 0.0);
        EXPECT_LT(info.ndcBound.maxX, 0.5);
        EXPECT_GT(info.ndcBound.minY, 0.0);
        EXPECT_LT(info.ndcBound.maxY, 0.5);
        return true;
    });

    EXPECT_EQ(1, test.map.getRenderedFeatureCount("poly0", test.symbolLayerName));
    test.map.getRenderedFeatures(
        "poly0", test.symbolLayerName, std::nullopt, [](const auto&, const auto& info) -> bool {
            EXPECT_LT(info.ndcBound.maxX, 0.0);
            EXPECT_LT(info.ndcBound.maxY, 0.1);
            return true;
        });

    EXPECT_EQ(1, test.map.getRenderedFeatureCount("l1", test.lineLayerName));
    test.map.getRenderedFeatures("l1", test.lineLayerName, std::nullopt, [](const auto&, const auto& info) -> bool {
        EXPECT_GT(info.ndcBound.minX, 0.1);
        EXPECT_LT(info.ndcBound.maxX, 0.5);
        EXPECT_GT(info.ndcBound.minY, -0.5);
        EXPECT_LT(info.ndcBound.maxY, 0.5);
        return true;
    });
}

TEST(FeatureTracking, NDCBoundSymbolOffset) {
    FeatureTrackingTest test(CameraOptions().withZoom(10.0));
    test.getSymbolLayer()->setVisibility(VisibilityType::Visible);
    test.getSymbolLayer()->setIconTranslate({{-140, -80}});
    test.getSymbolLayer()->setTextTranslate({{-140, -80}});
    test.run();

    EXPECT_EQ(1, test.map.getRenderedFeatureCount("pt0"));
    test.map.getRenderedFeatures("pt0", std::nullopt, std::nullopt, [](const auto&, const auto& info) -> bool {
        EXPECT_LT(info.ndcBound.minX, -1.0);
        EXPECT_LT(info.ndcBound.maxX, -0.5);
        EXPECT_GT(info.ndcBound.minY, 0.5);
        EXPECT_GT(info.ndcBound.maxY, 1.0);
        return true;
    });
}

TEST(FeatureTracking, NDCBoundCircle) {
    FeatureTrackingTest test(CameraOptions().withZoom(10.0));
    test.getCircleLayer()->setVisibility(VisibilityType::Visible);
    test.getCircleLayer()->setCircleTranslate({{-100, -20}});
    test.getSymbolLayer()->setVisibility(VisibilityType::Visible);
    test.run();

    EXPECT_EQ(1, test.map.getRenderedFeatureCount("pt0", test.circleLayerName));
    test.map.getRenderedFeatures("pt0", test.circleLayerName, std::nullopt, [](const auto&, const auto& info) -> bool {
        EXPECT_LT(info.ndcBound.maxX, 0.0);
        EXPECT_GT(info.ndcBound.minY, 0.0);
        return true;
    });
    EXPECT_EQ(1, test.map.getRenderedFeatureCount("pt1", test.circleLayerName));
    test.map.getRenderedFeatures("pt1", test.circleLayerName, std::nullopt, [](const auto&, const auto& info) -> bool {
        EXPECT_LT(info.ndcBound.maxX, 0.0);
        EXPECT_LT(info.ndcBound.maxY, 0.0);
        return true;
    });
}

// Cover the combinations of pitch options, which each project the vertices differently
TEST(FeatureTracking, NDCBoundCirclePitch1) {
    FeatureTrackingTest test(CameraOptions().withZoom(10.0).withPitch(60));
    test.getCircleLayer()->setVisibility(VisibilityType::Visible);
    test.getCircleLayer()->setCirclePitchAlignment(AlignmentType::Map);
    test.getCircleLayer()->setCirclePitchScale(CirclePitchScaleType::Map);
    test.run();

    EXPECT_EQ(1, test.map.getRenderedFeatureCount("pt2", test.circleLayerName));
    test.map.getRenderedFeatures("pt2", test.circleLayerName, std::nullopt, [](const auto&, const auto& info) -> bool {
        EXPECT_LT(info.ndcBound.maxX, 0.9);
        EXPECT_LT(info.ndcBound.minX, 0.6);
        EXPECT_LT(info.ndcBound.maxY, 0.8);
        EXPECT_GT(info.ndcBound.minY, 0.6);
        return true;
    });
}

TEST(FeatureTracking, NDCBoundCirclePitch2) {
    FeatureTrackingTest test(CameraOptions().withZoom(10.0).withPitch(60));
    test.getCircleLayer()->setVisibility(VisibilityType::Visible);
    test.getCircleLayer()->setCirclePitchAlignment(AlignmentType::Map);
    test.getCircleLayer()->setCirclePitchScale(CirclePitchScaleType::Viewport);
    test.run();
    EXPECT_EQ(1, test.map.getRenderedFeatureCount("pt2", test.circleLayerName));
}

TEST(FeatureTracking, NDCBoundCirclePitch3) {
    FeatureTrackingTest test(CameraOptions().withZoom(10.0).withPitch(60));
    test.getCircleLayer()->setVisibility(VisibilityType::Visible);
    test.getCircleLayer()->setCirclePitchAlignment(AlignmentType::Viewport);
    test.getCircleLayer()->setCirclePitchScale(CirclePitchScaleType::Map);
    test.run();
    EXPECT_EQ(1, test.map.getRenderedFeatureCount("pt2", test.circleLayerName));
}

TEST(FeatureTracking, NDCBoundCirclePitch4) {
    FeatureTrackingTest test(CameraOptions().withZoom(10.0).withPitch(60));
    test.getCircleLayer()->setVisibility(VisibilityType::Visible);
    test.getCircleLayer()->setCirclePitchAlignment(AlignmentType::Viewport);
    test.getCircleLayer()->setCirclePitchScale(CirclePitchScaleType::Viewport);
    test.run();
    EXPECT_EQ(1, test.map.getRenderedFeatureCount("pt2", test.circleLayerName));
}

TEST(FeatureTracking, NDCBoundFill) {
    FeatureTrackingTest test(CameraOptions().withZoom(10.0));
    test.getFillLayer()->setVisibility(VisibilityType::Visible);
    test.run();

    EXPECT_EQ(1, test.map.getRenderedFeatureCount("poly0", test.fillLayerName));
    test.map.getRenderedFeatures("poly0", test.fillLayerName, std::nullopt, [](const auto&, const auto& info) -> bool {
        EXPECT_GT(info.ndcBound.minX, -1.0);
        EXPECT_LT(info.ndcBound.minX, -0.75);
        EXPECT_GT(info.ndcBound.maxX, 0.0);
        EXPECT_LT(info.ndcBound.maxX, 0.5);
        EXPECT_GT(info.ndcBound.minY, -1.0);
        EXPECT_LT(info.ndcBound.minY, -0.9);
        EXPECT_GT(info.ndcBound.maxY, 0.0);
        EXPECT_LT(info.ndcBound.maxY, 0.5);
        return true;
    });
}

TEST(FeatureTracking, NDCBoundTilt) {
    const mapbox::geojson::feature_collection geom = {
        {Geometry<double>{Point{0.02, 0.25}}, {{"name", "0"}}, "pt0"},
    };

    FeatureTrackingTest test(geom, CameraOptions().withZoom(10.0));
    test.getSymbolLayer()->setVisibility(VisibilityType::Visible);
    test.run({});
    EXPECT_EQ(0, test.map.getRenderedFeatureCount("pt0")); // out of view

    test.map.jumpTo(test.map.getCameraOptions().withPitch(60)); // tilt and the feature appears
    test.run();

    test.map.getRenderedFeatures("pt0", std::nullopt, std::nullopt, [](const auto&, const auto& info) -> bool {
        EXPECT_GT(info.ndcBound.minX, 0.0);
        EXPECT_LT(info.ndcBound.maxX, 0.5);
        EXPECT_GT(info.ndcBound.minY, 0.6);
        EXPECT_LT(info.ndcBound.minY, 0.7);
        EXPECT_GT(info.ndcBound.maxY, 0.9);
        EXPECT_LT(info.ndcBound.maxY, 1.0);
        return true;
    });
}

// Features hidden by `icon-allow-overlap:false` do not appear in the results
TEST(FeatureTracking, Overlap) {
    FeatureTrackingTest test(CameraOptions().withZoom(10.0));
    test.getSymbolLayer()->setVisibility(VisibilityType::Visible);
    test.run({});
    ASSERT_EQ(0, test.map.getRenderedFeatureCount("pt0_overlap"));
    test.getSymbolLayer()->setIconAllowOverlap(true);
    test.getSymbolLayer()->setTextAllowOverlap(true);
    test.run();
    ASSERT_EQ(1, test.map.getRenderedFeatureCount("pt0_overlap"));
}

// Features appear when either icon or text opacity is non-zero (constant)
TEST(FeatureTracking, ZeroOpacityConstant) {
    FeatureTrackingTest test(CameraOptions().withZoom(10.0));
    test.getSymbolLayer()->setVisibility(VisibilityType::Visible);
    test.run({});
    ASSERT_EQ(1, test.map.getRenderedFeatureCount("pt0"));

    test.getSymbolLayer()->setIconOpacity(0.0);
    test.getSymbolLayer()->setTextOpacity(0.0);
    test.run({});
    ASSERT_EQ(0, test.map.getRenderedFeatureCount("pt0"));

    test.getSymbolLayer()->setIconOpacity(0.5);
    test.getSymbolLayer()->setTextOpacity(0.0);
    test.run({});
    ASSERT_EQ(1, test.map.getRenderedFeatureCount("pt0"));

    test.getSymbolLayer()->setIconOpacity(0.0);
    test.getSymbolLayer()->setTextOpacity(0.5);
    test.run({});
    ASSERT_EQ(1, test.map.getRenderedFeatureCount("pt0"));
}

// Features appear when either icon or text opacity is non-zero (zoom expression)
TEST(FeatureTracking, ZeroOpacityExpression) {
    FeatureTrackingTest test({}, CameraOptions().withZoom(10.0));
    test.getSymbolLayer()->setVisibility(VisibilityType::Visible);

    using namespace dsl;
    const auto expr = []() {
        return interpolate(linear(), zoom(), 9, literal(1.0), 10, literal(0.0), 11, literal(1.0));
    };
    test.getSymbolLayer()->setIconOpacity({expr()});
    test.getSymbolLayer()->setTextOpacity({expr()});
    test.run({});
    ASSERT_EQ(0, test.map.getRenderedFeatureCount());
}

// Symbols appear when either icon or text opacity is non-zero (data-driven)
TEST(FeatureTracking, ZeroOpacityDataDriven) {
    FeatureTrackingTest test(CameraOptions().withZoom(10.0));
    test.getSymbolLayer()->setVisibility(VisibilityType::Visible);

    using namespace dsl;
    test.getSymbolLayer()->setIconOpacity({get(literal("opacity"))});
    test.getSymbolLayer()->setTextOpacity({get(literal("opacity"))});
    test.run({});
    ASSERT_EQ(1, test.map.getRenderedFeatureCount("pt0"));
    ASSERT_EQ(0, test.map.getRenderedFeatureCount("pt1"));
}
