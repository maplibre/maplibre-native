#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/geometry/dem_data.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/map/transform.hpp>
#include <mln/renderer/terrain_elevation_index.hpp>
#include <mln/style/style.hpp>
#include <mln/style/image.hpp>
#include <mln/renderer/renderer.hpp>
#include <mln/style/terrain.hpp>
#include <mln/util/image.hpp>
#include <mln/util/projection.hpp>
#include <mln/util/run_loop.hpp>

#include <cmath>
#include <limits>
#include <set>

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

// Camera code that turns a screen point into a camera centre (iOS panning and moveIntoView, Android
// zoomBy around a point) unprojects on the plane the centre sits on: the centre pixel maps back to
// the centre, and a camera moved to a point unprojected this way puts that point where it was asked.
// The terrain pick answers on the plateau in front of that plane, so a centre taken from it moves
// the view.
TEST(TerrainElevationIndex, CameraCodeUnprojectsOnTheCentresPlane) {
    for (const double centerAltitude : {0.0, 400.0}) {
        TerrainProjectionTest test;
        test.map.jumpTo(CameraOptions().withZoom(10.0).withCenterAltitude(centerAltitude));
        test.render(4);
        ASSERT_TRUE(test.map.getTerrainElevation(innsbruck));

        const ScreenCoordinate centre{128.0, 128.0};
        const auto camera = test.map.getCameraOptions({});
        ASSERT_NEAR(*camera.centerAltitude, centerAltitude, 1e-6);
        const LatLng onPlane = test.map.latLngForPixelAtCenterAltitude(centre);
        // To within a tenth of a pixel (~5 m at z10); the pick below is off by over 100 m
        EXPECT_NEAR(onPlane.latitude(), camera.center->latitude(), 1e-4) << centerAltitude;
        EXPECT_NEAR(onPlane.longitude(), camera.center->longitude(), 1e-4) << centerAltitude;

        // The plateau is above the centre's plane, so the pick lands well short of the centre
        const LatLng picked = test.map.latLngForPixelOnTerrain(centre);
        EXPECT_GT(std::abs(picked.latitude() - camera.center->latitude()), 1e-3) << centerAltitude;

        // A pan: the point 40 px below the centre becomes the centre
        const LatLng target = test.map.latLngForPixelAtCenterAltitude({128.0, 168.0});
        test.map.jumpTo(CameraOptions().withCenter(target));
        const ScreenCoordinate landed = test.map.pixelForLatLng(target, centerAltitude);
        EXPECT_NEAR(landed.x, centre.x, 0.1) << centerAltitude;
        EXPECT_NEAR(landed.y, centre.y, 0.1) << centerAltitude;
    }
}

// The highest point of the copied tiles, exaggerated, is where the pick starts its march.
TEST(TerrainElevationIndex, KnowsItsHighestPoint) {
    const auto world = Projection::project(innsbruck, 4);
    const UnwrappedTileID child(4, static_cast<uint32_t>(world.x), static_cast<uint32_t>(world.y));
    TerrainElevationIndex index({{UnwrappedTileID(2, 2, 1), makeGrid(500.0)}, {child, makeGrid(900.0)}}, 1.5f);
    EXPECT_NEAR(index.getMaxElevation(), 900.0 * 1.5, 0.01);
}

namespace {

// North-up and steeply pitched, so a point farther up the screen ray is farther north.
TransformState pitchedNorthUp(double zoom, double pitch) {
    Transform transform;
    transform.resize({256, 256});
    transform.jumpTo(CameraOptions().withCenter(innsbruck).withZoom(zoom).withBearing(0.0).withPitch(pitch));
    return transform.getState();
}

// Latitude where the ray through a y-down view pixel crosses the plane `height` metres up.
double latitudeAt(const TransformState& state, const ScreenCoordinate& pixel, double height) {
    return state.screenCoordinateToLatLng({pixel.x, state.getSize().height - pixel.y}, height).latitude();
}

} // namespace

// Heights that stop before the ray's sea-level point (a pixel high on a pitched screen, past the
// loaded tiles) still catch the ray on the ground in front of it.
TEST(TerrainElevationIndex, PickFindsTheGroundWhenSeaLevelIsPastTheHeights) {
    const TransformState state = pitchedNorthUp(10.0, 70.0);
    const ScreenCoordinate pixel{128.0, 40.0};
    const double plateau = 1000.0;
    // Heights stop halfway between where the ray reaches the plateau and where it reaches sea level
    const double edge = 0.5 * (latitudeAt(state, pixel, plateau) + latitudeAt(state, pixel, 0.0));
    const auto elevationAt = [&](const LatLng& latLng) -> std::optional<double> {
        return latLng.latitude() <= edge ? std::optional<double>(plateau) : std::nullopt;
    };
    const auto picked = pickTerrainSurface(state, pixel, elevationAt, plateau);
    ASSERT_TRUE(picked);
    EXPECT_NEAR(picked->latitude(), latitudeAt(state, pixel, plateau), 1e-4);
}

// A ridge the ray passes through only within one of the march's steps is found when the march
// starts at the highest terrain. Started from just below the camera, ~10 km up, the 64 steps are
// ~160 m apart: one lands in front of the ridge and the next behind it, and the pick lands on the
// sea-level ground beyond.
TEST(TerrainElevationIndex, PickDoesNotStepOverAThinRidge) {
    const TransformState state = pitchedNorthUp(9.0, 75.0);
    const ScreenCoordinate pixel{128.0, 60.0};
    // The heights the march from the camera samples are top * (1 - i / 64); put the ray's stretch
    // inside the ridge between two of them, around 1000 m up
    const double top = state.getFreeCameraOptions().getLocation()->altitude * 0.999;
    const double step = top / 64.0;
    const double above = std::ceil(1000.0 / step) * step;
    const double windowTop = above - 0.25 * step;
    const double windowBottom = above - 0.75 * step;
    const double ridge = windowTop + 10.0;
    const double nearEdge = latitudeAt(state, pixel, windowTop);
    const double farEdge = latitudeAt(state, pixel, windowBottom);
    ASSERT_LT(nearEdge, farEdge);
    const auto elevationAt = [&](const LatLng& latLng) -> std::optional<double> {
        return latLng.latitude() >= nearEdge && latLng.latitude() <= farEdge ? ridge : 0.0;
    };

    const auto picked = pickTerrainSurface(state, pixel, elevationAt, ridge);
    ASSERT_TRUE(picked);
    EXPECT_NEAR(picked->latitude(), nearEdge, 0.25 * (farEdge - nearEdge));

    const auto fromTheCamera = pickTerrainSurface(state, pixel, elevationAt, std::numeric_limits<double>::infinity());
    ASSERT_TRUE(fromTheCamera);
    EXPECT_GT(fromTheCamera->latitude(), farEdge);
}

// The DEM source keeps the coarser ancestors of its cover that have already loaded, for layers that
// draw from shallower tiles, but never requests them: that fetched up to eight extra levels and made
// still renders wait for them. Over a source with maxzoom 2, only z2 tiles are asked for.
TEST(TerrainElevationIndex, DEMAncestorsAreNotRequested) {
    TerrainProjectionTest test;
    std::set<uint8_t> requested;
    test.fileSource->tileResponse = [&](const Resource& resource) {
        if (resource.tileData) {
            requested.insert(resource.tileData->z);
        }
        Response res;
        res.data = std::make_shared<std::string>(test.tile);
        return res;
    };
    test.map.jumpTo(CameraOptions().withZoom(10.0));
    test.render(4);
    ASSERT_TRUE(test.map.getTerrainElevation(innsbruck));
    EXPECT_EQ(std::set<uint8_t>{2}, requested);
}

// Across the antimeridian a point is projected unwrapped for the shortest path to the centre, so its
// height has to come from the DEM tiles drawn there, the next world's (wrap 1), not the wrapped
// longitude's tile in this world, which is not loaded.
TEST(TerrainElevationIndex, ProjectionOnTerrainAcrossTheAntimeridian) {
    TerrainProjectionTest test;
    const LatLng centre{-16.5, 179.95};
    test.map.jumpTo(CameraOptions().withCenter(centre).withZoom(10.0));
    test.render(4);
    ASSERT_TRUE(test.map.getTerrainElevation(centre));

    const LatLng east{-16.5, -179.95}; // just across the line, as a wrapped longitude
    LatLng unwrapped = east;
    unwrapped.unwrapForShortestPath(centre);
    const ScreenCoordinate onSurface = test.map.pixelForLatLngOnTerrain(east);
    const ScreenCoordinate atHeight = test.map.pixelForLatLng(unwrapped, plateauMeters);
    EXPECT_NEAR(onSurface.x, atHeight.x, 0.5);
    EXPECT_NEAR(onSurface.y, atHeight.y, 0.5);
}

namespace {

// A plateau DEM to z8 under a point symbol whose GeoJSON source stops at z6, so at map zoom 11 the
// symbol tile (z6) is shallower than the DEM cover (z8) and, since ancestors are never requested, no
// DEM tile covers it whole. (z6 keeps the point's tile quantisation to ~2 px at z11.)
const char* symbolOnTerrainStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 8,
      "tileSize": 64
    },
    "pins": {
      "type": "geojson",
      "maxzoom": 6,
      "data": {"type": "Feature", "properties": {}, "geometry": {"type": "Point", "coordinates": [11.42, 47.28]}}
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [{"id": "pin", "type": "symbol", "source": "pins", "layout": {"icon-image": "dot"}}]
})STYLE";

} // namespace

// Placement projects the collision boxes at the terrain height, so a symbol over the plateau is
// found where it is drawn on the surface and not at its sea-level position. The symbol's tile has
// no DEM tile covering it whole, so this also needs the sampler's per-point fallback to the deeper
// DEM tiles.
TEST(TerrainElevationIndex, SymbolCollisionBoxesSitOnTheTerrain) {
    TerrainProjectionTest test;
    test.map.getStyle().loadJSON(symbolOnTerrainStyle);
    test.map.getStyle().addImage(std::make_unique<style::Image>("dot", PremultipliedImage({8, 8}), 1.0f));
    test.map.jumpTo(CameraOptions().withCenter(innsbruck).withZoom(11.0).withPitch(60.0));
    test.render(6);
    ASSERT_TRUE(test.map.getTerrainElevation(innsbruck));

    const LatLng pin{47.28, 11.42};
    const ScreenCoordinate onSurface = test.map.pixelForLatLngOnTerrain(pin);
    const ScreenCoordinate atSeaLevel = test.map.pixelForLatLng(pin);
    // Far enough apart that an 8 px icon (with its collision padding) cannot cover both
    ASSERT_GT(std::hypot(onSurface.x - atSeaLevel.x, onSurface.y - atSeaLevel.y), 20.0);

    const RenderedQueryOptions options{std::vector<std::string>{"pin"}};
    EXPECT_EQ(1u, test.frontend.getRenderer()->queryRenderedFeatures(onSurface, options).size());
    EXPECT_EQ(0u, test.frontend.getRenderer()->queryRenderedFeatures(atSeaLevel, options).size());
}
