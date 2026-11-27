#include <mln/test/crs.hpp>
#include <mln/test/util.hpp>

#include <mln/map/map.hpp>
#include <mln/map/map_options.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/renderer/renderer.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/style/projection.hpp>
#include <mln/style/style.hpp>
#include <mln/style/sources/custom_geometry_source.hpp>
#include <mln/style/layers/fill_layer.hpp>
#include <mln/style/layers/line_layer.hpp>
#include <mln/util/crs.hpp>
#include <mln/util/geojson.hpp>
#include <mln/util/io.hpp>
#include <mln/util/mat4.hpp>
#include <mln/util/run_loop.hpp>

using namespace mln;
using namespace mln::style;

TEST(CustomGeometrySource, Grid) {
    util::RunLoop loop;

    HeadlessFrontend frontend{1};
    Map map(frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
            ResourceOptions().withCachePath(":memory:").withAssetPath("test/fixtures/api/assets"));
    map.getStyle().loadJSON(util::read_file("test/fixtures/api/water.json"));
    map.jumpTo(CameraOptions().withCenter(LatLng{37.8, -122.5}).withZoom(10.0));

    CustomGeometrySource::Options options;
    options.fetchTileFunction = [&map](const mln::CanonicalTileID& tileID) {
        double gridSpacing = 0.1;
        FeatureCollection features;
        const LatLngBounds bounds(tileID);
        for (double y = ceil(bounds.north() / gridSpacing) * gridSpacing;
             y >= floor(bounds.south() / gridSpacing) * gridSpacing;
             y -= gridSpacing) {
            mapbox::geojson::line_string gridLine;
            gridLine.emplace_back(bounds.west(), y);
            gridLine.emplace_back(bounds.east(), y);

            features.emplace_back(gridLine);
        }

        for (double x = floor(bounds.west() / gridSpacing) * gridSpacing;
             x <= ceil(bounds.east() / gridSpacing) * gridSpacing;
             x += gridSpacing) {
            mapbox::geojson::line_string gridLine;
            gridLine.emplace_back(x, bounds.south());
            gridLine.emplace_back(x, bounds.north());

            features.emplace_back(gridLine);
        }
        auto source = static_cast<CustomGeometrySource*>(map.getStyle().getSource("custom"));
        if (source) {
            source->setTileData(tileID, features);
        }
    };

    map.getStyle().addSource(std::make_unique<CustomGeometrySource>("custom", options));

    auto fillLayer = std::make_unique<FillLayer>("landcover", "mapbox");
    fillLayer->setSourceLayer("landcover");
    fillLayer->setFillColor(Color{1.0, 1.0, 0.0, 1.0});
    map.getStyle().addLayer(std::move(fillLayer));

    auto layer = std::make_unique<LineLayer>("grid", "custom");
    layer->setLineColor(Color{1.0, 1.0, 1.0, 1.0});
    map.getStyle().addLayer(std::move(layer));

    test::checkImage("test/fixtures/custom_geometry_source/grid", frontend.render(map).image, 0.0006, 0.1);
}

TEST(CustomGeometrySource, InARegisteredProjection) {
    util::RunLoop loop;

    HeadlessFrontend frontend{1};
    Map map(frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
            ResourceOptions().withCachePath(":memory:").withAssetPath("test/fixtures/api/assets"));
    map.getStyle().loadJSON(R"({"version": 8, "projection": {"type": "identity"}, "sources": {}, "layers": []})");

    CustomGeometrySource::Options options;
    options.fetchTileFunction = [&map](const CanonicalTileID& tileID) {
        // Every tile gets the box from 0 to 20 E and 20 to 40 N, in lat/lng as the source's data always is.
        const mapbox::geojson::polygon box{{{0, 20}, {20, 20}, {20, 40}, {0, 40}, {0, 20}}};
        if (auto* source = static_cast<CustomGeometrySource*>(map.getStyle().getSource("custom"))) {
            source->setTileData(tileID, FeatureCollection{Feature{box}});
        }
    };
    options.tileOptions.clip = true;
    map.getStyle().addSource(std::make_unique<CustomGeometrySource>("custom", options));

    auto layer = std::make_unique<FillLayer>("box", "custom");
    layer->setFillColor(Color::red());
    map.getStyle().addLayer(std::move(layer));

    frontend.render(map);

    // 10 E, 30 N is inside the box in the identity projection; in Mercator the whole box is drawn below that point.
    EXPECT_EQ(frontend.getRenderer()->queryRenderedFeatures(map.pixelForLatLng({30, 10})).size(), 1u);
}

TEST(CustomGeometrySource, TileBoundsInARegisteredProjection) {
    addProjection(test::polarStereographic("test:polar-tile-bounds"));
    {
        util::RunLoop loop;

        HeadlessFrontend frontend{1};
        Map map(frontend,
                MapObserver::nullObserver(),
                MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
                ResourceOptions().withCachePath(":memory:").withAssetPath("test/fixtures/api/assets"));
        map.getStyle().loadJSON(
            R"({"version": 8, "projection": {"type": "test:polar-tile-bounds"}, "sources": {}, "layers": []})");
        map.getStyle().addSource(std::make_unique<CustomGeometrySource>("custom", CustomGeometrySource::Options()));
        const auto& source = *map.getStyle().getSource("custom")->as<CustomGeometrySource>();

        struct Tile {
            CanonicalTileID id;
            double west;
            double south;
            double east;
            double north;
        };
        // GL JS's `tileIdToLngLatBounds` for the same tiles in the same projection, its box of the four corners. The
        // pole is inside tile 0/0/0, so that box misses it, and it is the corner of each tile at zoom 1 and of 2/1/1.
        const std::array tiles = {
            Tile{{0, 0, 0}, -180, 38.76872677332668, 90, 38.76872677332668},
            Tile{{1, 0, 0}, -180, 38.76872677332668, 135, 90},
            Tile{{1, 1, 1}, -45, 38.76872677332668, 135, 90},
            Tile{{2, 1, 1}, -180, 63.03804324805077, 135, 90},
            Tile{{3, 4, 5}, -45, 68.53700782445765, 0, 80.31077447189598},
            Tile{{5, 17, 20}, -33.690067525979785, 76.98061569283365, -18.43494882292201, 80.01406000287281},
            Tile{{8, 130, 140}, -36.25383773744479, 85.95232966804826, -30.96375653207352, 86.30889476739338},
        };
        constexpr double toleranceDegrees = 1e-12;
        for (const auto& tile : tiles) {
            const LatLngBounds bounds = source.getTileBounds(tile.id);
            EXPECT_NEAR(tile.west, bounds.west(), toleranceDegrees) << tile.id;
            EXPECT_NEAR(tile.south, bounds.south(), toleranceDegrees) << tile.id;
            EXPECT_NEAR(tile.east, bounds.east(), toleranceDegrees) << tile.id;
            EXPECT_NEAR(tile.north, bounds.north(), toleranceDegrees) << tile.id;
        }

        // In Mercator, the style's default projection, the box is the tile's, as before.
        map.getStyle().setProjection(std::make_unique<Projection>());
        for (const auto& tile : tiles) {
            EXPECT_EQ(LatLngBounds(tile.id), source.getTileBounds(tile.id)) << tile.id;
        }
    }
    removeProjection("test:polar-tile-bounds");
}

TEST(CustomGeometrySource, TileBoundsOutsideAStyleAreTheMercatorTiles) {
    const CustomGeometrySource source("custom", CustomGeometrySource::Options());
    for (const CanonicalTileID& id :
         {CanonicalTileID{0, 0, 0}, CanonicalTileID{3, 4, 5}, CanonicalTileID{12, 655, 1583}}) {
        EXPECT_EQ(LatLngBounds(id), source.getTileBounds(id)) << id;
    }
}
