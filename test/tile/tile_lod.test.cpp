#include <gmock/gmock.h>

#include <algorithm>
#include <mbgl/map/map.hpp>
#include <mbgl/map/map_observer.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/layers/circle_layer.hpp>
#include <mbgl/style/sources/custom_geometry_source.hpp>
#include <mbgl/test/util.hpp>

#include <filesystem>

using namespace mbgl;
using namespace mbgl::util;
using namespace ::testing;

class TileLODTest {
public:
    util::RunLoop loop;
    HeadlessFrontend frontend;
    Map map;

    uint32_t tileCount = 0;

public:
    TileLODTest(const CameraOptions& cameraOptions = CameraOptions())
        : frontend(1.0f),
          map(frontend,
              MapObserver::nullObserver(),
              MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
              ResourceOptions().withCachePath(":memory:").withAssetPath("test/fixtures/api/assets")) {
        map.jumpTo(cameraOptions);

        map.setDebug(MapDebugOptions::TileBorders);
        map.getStyle().loadJSON(util::read_file("test/fixtures/api/empty.json"));

        style::CustomGeometrySource::Options options;
        options.fetchTileFunction = [&](const mbgl::CanonicalTileID& tileID) {
            auto source = static_cast<style::CustomGeometrySource*>(map.getStyle().getSource("custom"));
            if (!source) {
                return;
            }

            const LatLngBounds bounds(tileID);
            const LatLng& center = bounds.center();

            mapbox::geojson::feature feature;
            feature.geometry = mapbox::geometry::geometry<double>(Point<double>{center.longitude(), center.latitude()});

            FeatureCollection features;
            features.emplace_back(feature);

            source->setTileData(tileID, features);

            ++tileCount;
        };

        map.getStyle().addSource(std::make_unique<style::CustomGeometrySource>("custom", options));

        auto layer = std::make_unique<style::CircleLayer>("circles", "custom");
        layer->setCircleColor(Color{0.0, 0.0, 1.0, 0.0});
        map.getStyle().addLayer(std::move(layer));
    }

    void checkImage(std::string image, uint32_t expectedTileCount, double imageThreshold = 0.0002) {
        checkImages(std::vector<std::string>{image}, expectedTileCount, imageThreshold);
    }

    void checkImages(std::vector<std::string> images, uint32_t expectedTileCount, double imageThreshold) {
        const auto result = frontend.render(map);

        // check this after frontend.render()
        EXPECT_EQ(tileCount, expectedTileCount);

        std::ranges::transform(images, images.begin(), [](const auto& img) { return "test/fixtures/tile_lod/" + img; });

        test::checkImages(images, result.image, imageThreshold, 0.01);
    }
};

TEST(TileLOD, disabled) {
    TileLODTest test(CameraOptions().withZoom(10.0));

    test.map.setTileLodPitchThreshold(std::numbers::pi);

    test.checkImage("disabled", 4);
}

TEST(TileLOD, zoomShift) {
    TileLODTest test(CameraOptions().withZoom(12.0));

    test.map.setTileLodZoomShift(4.0);
    test.frontend.render(test.map);
    EXPECT_EQ(test.tileCount, 64);

    test.tileCount = 0;

    test.map.setTileLodZoomShift(-4.0);
    test.frontend.render(test.map);
    EXPECT_EQ(test.tileCount, 4);
}

TEST(TileLOD, pitchThreshold) {
    TileLODTest test(CameraOptions().withZoom(10.0));

    constexpr double pitch = 45.0;
    test.map.setTileLodZoomShift(4.0);
    test.map.setTileLodMinRadius(1.0);
    test.map.setTileLodScale(1.0);
    test.map.setTileLodPitchThreshold(pitch / 180.0 * std::numbers::pi);

    test.map.jumpTo(CameraOptions().withPitch(pitch - 1.0));
    test.frontend.render(test.map);
    EXPECT_EQ(test.tileCount, 140);
    test.tileCount = 0;

    test.map.jumpTo(CameraOptions().withPitch(pitch + 1.0));
    test.checkImages(
        {
            "pitchThreshold-line",
            "pitchThreshold-polyline",
        },
        22,
        0.0015);
}

TEST(TileLOD, scale) {
    TileLODTest test(CameraOptions().withZoom(0.0));

    test.map.setTileLodMinRadius(1.0);
    test.map.setTileLodScale(0.4);
    test.map.setTileLodPitchThreshold(-1.0);

    test.map.setTileLodZoomShift(5.0);

    test.checkImage("scale", 112);
}

TEST(TileLOD, radius) {
    TileLODTest test(CameraOptions().withZoom(0.0));

    test.map.setTileLodMinRadius(5.0);
    test.map.setTileLodScale(1.0);
    test.map.setTileLodPitchThreshold(-1.0);

    test.map.setTileLodZoomShift(5.0);

    test.checkImage("radius", 172);
}
