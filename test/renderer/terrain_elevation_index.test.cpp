#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/geometry/dem_data.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/renderer/terrain_elevation_index.hpp>
#include <mln/style/style.hpp>
#include <mln/style/terrain.hpp>
#include <mln/util/image.hpp>
#include <mln/util/projection.hpp>
#include <mln/util/run_loop.hpp>

#include <cmath>

using namespace mln;

namespace {

constexpr uint32_t demTileSize = 64;

// Terrain-RGB ("mapbox" encoding): -10000 + (R * 65536 + G * 256 + B) * 0.1
PremultipliedImage makeDEMImage(double metres) {
    const auto encoded = static_cast<uint32_t>(std::lround((metres + 10000.0) * 10.0));
    PremultipliedImage image({demTileSize, demTileSize});
    for (uint32_t i = 0; i < demTileSize * demTileSize; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = static_cast<uint8_t>((encoded >> 16) & 0xFF);
        px[1] = static_cast<uint8_t>((encoded >> 8) & 0xFF);
        px[2] = static_cast<uint8_t>(encoded & 0xFF);
        px[3] = 255;
    }
    return image;
}

std::shared_ptr<const TerrainElevationIndex::Grid> makeGrid(double metres) {
    const DEMData dem(makeDEMImage(metres), Tileset::RasterEncoding::Mapbox);
    return std::make_shared<const TerrainElevationIndex::Grid>(dem);
}

// A 1000 m plateau, as the terrain camera tests use
constexpr double plateauMeters = 1000.0;

const char* terrainStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 2,
      "tileSize": 64
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": []
})STYLE";

const LatLng innsbruck{47.2692, 11.4041};

struct TerrainProjectionTest {
    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(),
                                                                                  ClientOptions());
    std::string tile = encodePNG(makeDEMImage(plateauMeters));
    HeadlessFrontend frontend{{256, 256}, 1};
    MapAdapter map;

    TerrainProjectionTest()
        : map(frontend,
              MapObserver::nullObserver(),
              fileSource,
              MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize())) {
        fileSource->tileResponse = [this](const Resource&) {
            Response res;
            res.data = std::make_shared<std::string>(tile);
            return res;
        };
        map.setCenterClampedToGround(false);
        map.getStyle().loadJSON(terrainStyle);
        map.jumpTo(CameraOptions().withCenter(innsbruck).withZoom(2.0).withPitch(60.0));
    }

    void render(int frames) {
        for (int i = 0; i < frames; ++i) {
            loop.runOnce();
            frontend.render(map);
        }
    }
};

} // namespace

// The copy samples like the renderer: whole metres, scaled by the exaggeration.
TEST(TerrainElevationIndex, SamplesTheTileUnderTheLocation) {
    const UnwrappedTileID id(2, 2, 1); // covers Innsbruck at z2
    TerrainElevationIndex index({{id, makeGrid(1234.0)}}, 1.5f);

    const auto elevation = index.getElevation(innsbruck);
    ASSERT_TRUE(elevation);
    EXPECT_NEAR(*elevation, 1234.0 * 1.5, 0.01);

    // A location no tile covers has no height, rather than sea level.
    EXPECT_FALSE(index.getElevation(LatLng{-33.87, 151.21}));
}

// Where a deeper tile is loaded it wins over its ancestors, as in RenderTerrain.
TEST(TerrainElevationIndex, DeepestTileWins) {
    const auto world = Projection::project(innsbruck, 4);
    const UnwrappedTileID child(4, static_cast<uint32_t>(world.x), static_cast<uint32_t>(world.y));
    TerrainElevationIndex index({{UnwrappedTileID(2, 2, 1), makeGrid(500.0)}, {child, makeGrid(900.0)}}, 1.0f);

    EXPECT_NEAR(*index.getElevation(innsbruck), 900.0, 0.01);
    // Beside the child, only the ancestor covers it.
    const LatLng neighbourCorner(CanonicalTileID(4, child.canonical.x + 1, child.canonical.y));
    const LatLng besideChild{neighbourCorner.latitude() - 0.5, neighbourCorner.longitude() + 0.5};
    ASSERT_TRUE(index.getElevation(besideChild));
    EXPECT_NEAR(*index.getElevation(besideChild), 500.0, 0.01);
}

// A wrapped copy of the world is found from a longitude past the antimeridian.
TEST(TerrainElevationIndex, WrappedTiles) {
    // Innsbruck is z1 tile (1, 0); one world east it is unwrapped x 3
    TerrainElevationIndex index({{UnwrappedTileID(1, 3, 0), makeGrid(700.0)}}, 1.0f);
    const auto elevation = index.getElevation(LatLng{47.2692, 11.4041 + 360.0, LatLng::Unwrapped});
    ASSERT_TRUE(elevation);
    EXPECT_NEAR(*elevation, 700.0, 0.01);
    // The unwrapped copy is not the original world.
    EXPECT_FALSE(index.getElevation(innsbruck));
}

// The map gets the renderer's heights once tiles load, without asking the render side, and loses
// them when terrain goes.
TEST(TerrainElevationIndex, MapAnswersFromTheRenderersHeights) {
    TerrainProjectionTest test;
    EXPECT_FALSE(test.map.getTerrainElevation(innsbruck));

    test.render(4);
    const auto elevation = test.map.getTerrainElevation(innsbruck);
    ASSERT_TRUE(elevation);
    EXPECT_NEAR(*elevation, plateauMeters, 1.0);

    test.map.getStyle().setTerrain(nullptr);
    test.render(1);
    EXPECT_FALSE(test.map.getTerrainElevation(innsbruck));
}

// Projection onto the terrain lands on the plateau, and a screen point picked off it maps back to
// the same place.
TEST(TerrainElevationIndex, ProjectionRoundTripsOnTheSurface) {
    TerrainProjectionTest test;
    test.map.jumpTo(CameraOptions().withZoom(10.0));
    test.render(4);
    ASSERT_TRUE(test.map.getTerrainElevation(innsbruck));

    const LatLng target{47.28, 11.42};
    const ScreenCoordinate onSurface = test.map.pixelForLatLngOnTerrain(target);
    const ScreenCoordinate atHeight = test.map.pixelForLatLng(target, plateauMeters);
    EXPECT_NEAR(onSurface.x, atHeight.x, 0.5);
    EXPECT_NEAR(onSurface.y, atHeight.y, 0.5);
    // Pitched, the plateau sits visibly off the sea-level position.
    const ScreenCoordinate atSeaLevel = test.map.pixelForLatLng(target);
    EXPECT_GT(std::abs(onSurface.y - atSeaLevel.y), 5.0);

    const LatLng picked = test.map.latLngForPixelOnTerrain(onSurface);
    EXPECT_NEAR(picked.latitude(), target.latitude(), 1e-4);
    EXPECT_NEAR(picked.longitude(), target.longitude(), 1e-4);

    // The batch versions answer the same way.
    const auto pixels = test.map.pixelsForLatLngsOnTerrain({target});
    ASSERT_EQ(pixels.size(), 1u);
    EXPECT_NEAR(pixels[0].x, onSurface.x, 1e-9);
    const auto latLngs = test.map.latLngsForPixelsOnTerrain({onSurface});
    ASSERT_EQ(latLngs.size(), 1u);
    EXPECT_NEAR(latLngs[0].latitude(), picked.latitude(), 1e-9);
}

// Without terrain the terrain versions are the sea-level ones.
TEST(TerrainElevationIndex, WithoutTerrainProjectionIsSeaLevel) {
    TerrainProjectionTest test;
    test.map.getStyle().setTerrain(nullptr);
    test.map.jumpTo(CameraOptions().withZoom(10.0));
    test.render(2);

    const LatLng target{47.28, 11.42};
    const ScreenCoordinate onSurface = test.map.pixelForLatLngOnTerrain(target);
    const ScreenCoordinate atSeaLevel = test.map.pixelForLatLng(target);
    EXPECT_DOUBLE_EQ(onSurface.x, atSeaLevel.x);
    EXPECT_DOUBLE_EQ(onSurface.y, atSeaLevel.y);
    const LatLng picked = test.map.latLngForPixelOnTerrain(onSurface);
    EXPECT_NEAR(picked.latitude(), target.latitude(), 1e-6);
}
