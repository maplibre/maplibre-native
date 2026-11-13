#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/bound_options.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/map/mode.hpp>
#include <mln/math/angles.hpp>
#include <mln/style/layers/fill_layer.hpp>
#include <mln/style/sources/geojson_source.hpp>
#include <mln/style/style.hpp>
#include <mln/style/terrain.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/image.hpp>
#include <mln/util/projection.hpp>
#include <mln/util/run_loop.hpp>

#include <mapbox/geojson.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <functional>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <vector>

using namespace mln;

namespace {

constexpr uint32_t demTileSize = 64;
// Terrain-RGB encodes elevation as -10000 + (R * 65536 + G * 256 + B) * 0.1, so a flat
// 1000 m plateau is RGB(1, 173, 176): 65536 + 44288 + 176 = 110000.
constexpr double plateauMeters = 1000.0;
// One metre in the Mercator units of the camera position, at the fixture's latitude.
const double mercatorMetre = 1.0 / (util::M2PI * util::EARTH_RADIUS_M * std::cos(util::deg2rad(47.2692)));

std::string makeFlatDEMTile() {
    PremultipliedImage image({demTileSize, demTileSize});
    for (uint32_t i = 0; i < demTileSize * demTileSize; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = 1;
        px[1] = 173;
        px[2] = 176;
        px[3] = 255;
    }
    return encodePNG(image);
}

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

struct TerrainCameraTest {
    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(),
                                                                                  ClientOptions());
    std::string tile = makeFlatDEMTile();
    HeadlessFrontend frontend{{256, 256}, 1};
    MapAdapter map;

    // Continuous runs camera transitions on the clock, for tests that do not render.
    explicit TerrainCameraTest(MapMode mode = MapMode::Static)
        : map(frontend,
              MapObserver::nullObserver(),
              fileSource,
              MapOptions().withMapMode(mode).withSize(frontend.getSize())) {
        fileSource->tileResponse = [this](const Resource&) {
            Response res;
            res.data = std::make_shared<std::string>(tile);
            return res;
        };
        map.getStyle().loadJSON(terrainStyle);
        map.jumpTo(CameraOptions().withCenter(LatLng{47.2692, 11.4041}).withZoom(2.0).withPitch(60.0));
    }

    double renderAndGetAltitude(int frames) {
        double altitude = 0.0;
        for (int i = 0; i < frames; ++i) {
            loop.runOnce();
            frontend.render(map);
            altitude = map.getCameraOptions({}).centerAltitude.value_or(0.0);
        }
        return altitude;
    }

    vec3 eye() const { return *map.getFreeCameraOptions().position; }

    // The camera is where it was, to within a metre.
    void expectEyeAt(const vec3& expected) const {
        const vec3 actual = eye();
        for (int i = 0; i < 3; ++i) {
            EXPECT_NEAR(actual[i], expected[i], mercatorMetre) << "axis " << i;
        }
    }
};

// A fill layer over a geojson source that covers far more tiles than the terrain mesh has,
// beside the DEM that gives the mesh.
const char* drapedFillStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 6,
      "tileSize": 64
    },
    "area": {
      "type": "geojson",
      "data": {
        "type": "Feature",
        "properties": {},
        "geometry": {"type": "Polygon", "coordinates": [[[-60, 10], [80, 10], [80, 75], [-60, 75], [-60, 10]]]}
      }
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [{"id": "area", "type": "fill", "source": "area", "paint": {"fill-color": "#c33"}}]
})STYLE";

struct DrapeMaskCounts {
    int targets = 0; // drape targets re-rendered
    int drawn = 0;   // masks drawn inside them
    int skipped = 0; // masks left out because the tile does not overlap the target
};

// Renders until the geojson tiles have arrived and drawn into the drape targets, then a few
// frames more, summing the counters the target passes add (RenderingStats accumulate across
// frames).
DrapeMaskCounts drapeMasksOverFrames(int maxFrames) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string tile = makeFlatDEMTile();
    fileSource->tileResponse = [&](const Resource&) {
        Response res;
        res.data = std::make_shared<std::string>(tile);
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.getStyle().loadJSON(drapedFillStyle);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.jumpTo(CameraOptions().withCenter(LatLng{47.2692, 11.4041}).withZoom(5.0).withPitch(75.0));

    gfx::RenderingStats before;
    DrapeMaskCounts counts;
    int settling = 0;
    for (int i = 0; i < maxFrames && settling < 4; ++i) {
        loop.runOnce();
        const auto stats = frontend.render(map).stats;
        counts.targets += stats.numDrapeTargetsRendered - before.numDrapeTargetsRendered;
        counts.drawn += stats.numDrapeMasksDrawn - before.numDrapeMasksDrawn;
        counts.skipped += stats.numDrapeMasksSkipped - before.numDrapeMasksSkipped;
        before = stats;
        if (counts.drawn > 0) ++settling;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return counts;
}

// A raster layer over a flat DEM at sea level, seen straight down: draped or not, it must look
// the same.
const char* drapedRasterStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 9,
      "tileSize": 256
    },
    "quadrants": {
      "type": "raster",
      "tiles": ["http://example.com/quadrants/{z}-{x}-{y}.png"],
      "tileSize": 256
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [{"id": "quadrants", "type": "raster", "source": "quadrants", "paint": {"raster-fade-duration": 0}}]
})STYLE";

// Each raster tile is red in its top half and green in its left half, so a draped map that is
// flipped or mirrored puts the colours in the wrong place.
std::string makeQuadrantsTile() {
    PremultipliedImage image({256, 256});
    for (uint32_t y = 0; y < 256; ++y) {
        for (uint32_t x = 0; x < 256; ++x) {
            uint8_t* px = image.data.get() + (y * 256 + x) * 4;
            px[0] = y < 128 ? 255 : 0;
            px[1] = x < 128 ? 255 : 0;
            px[2] = 0;
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

std::string makeSeaLevelDEMTile() {
    PremultipliedImage image({256, 256});
    for (uint32_t i = 0; i < 256 * 256; ++i) {
        // Sea level in Terrain-RGB: (0 + 10000) / 0.1 = 100000 = 1 * 65536 + 134 * 256 + 160
        image.data[i * 4 + 0] = 1;
        image.data[i * 4 + 1] = 134;
        image.data[i * 4 + 2] = 160;
        image.data[i * 4 + 3] = 255;
    }
    return encodePNG(image);
}

PremultipliedImage renderQuadrants(bool terrain) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string demTile = makeSeaLevelDEMTile();
    const std::string quadrantsTile = makeQuadrantsTile();
    fileSource->tileResponse = [&](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(resource.url.find("/dem/") != std::string::npos ? demTile
                                                                                                 : quadrantsTile);
        return res;
    };
    HeadlessFrontend frontend{{256, 256}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.getStyle().loadJSON(drapedRasterStyle);
    if (!terrain) {
        map.getStyle().setTerrain(nullptr);
    }
    map.jumpTo(CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(8.6).withPitch(0.0));
    PremultipliedImage image;
    for (int i = 0; i < 4; ++i) {
        loop.runOnce();
        image = frontend.render(map).image;
    }
    return image;
}

} // namespace

// With the clamp on (it is off by default), the centre rides the terrain instead of sea level,
// so pitching over high ground does not leave the camera inside the hillside.
TEST(TerrainCamera, CentreRidesTheTerrainSurface) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(true);
    EXPECT_TRUE(test.map.getCenterClampedToGround());

    const double settled = test.renderAndGetAltitude(8);
    EXPECT_NEAR(settled, plateauMeters, 5.0);
}

// A camera placed by its altitude above sea level over high ground starts inside the ground. The
// tile cover then found every tile above the camera and asked for none, the DEM that would lift the
// camera onto the ground included, so the clamp never learned of the ground and the camera stayed
// under it. The cover falls back to the flat footprint, the DEM stays, and the clamp lifts the camera.
TEST(TerrainCamera, ACameraStartingInsideTheGroundIsLiftedOntoIt) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(true);
    // Opened at sea-level altitude over the 1,000 m plateau, close in: the eye is a few hundred
    // meters up, under the ground.
    test.map.jumpTo(CameraOptions().withZoom(15.0).withPitch(60.0));
    const auto eyeMeters = [&] {
        const double mercatorToMeters = util::M2PI * util::EARTH_RADIUS_M * std::cos(util::deg2rad(47.2692));
        return test.eye()[2] * mercatorToMeters;
    };
    ASSERT_LT(eyeMeters(), plateauMeters) << "the camera starts inside the ground";

    test.renderAndGetAltitude(20);
    EXPECT_NEAR(*test.map.getCameraOptions({}).centerAltitude, plateauMeters, 5.0);
    EXPECT_GT(eyeMeters(), plateauMeters) << "the camera is above the ground";
}

// The historical worry about a terrain-anchored centre was that raising it re-samples a new
// elevation and runs away. Raising the centre moves the orbit plane, not the centre's lng/lat,
// so it must settle - and the centre must not drift while it does.
TEST(TerrainCamera, CentreElevationSettlesWithoutDrift) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(true);

    test.renderAndGetAltitude(8);
    const auto afterSettling = test.map.getCameraOptions({});
    const double settledAltitude = *afterSettling.centerAltitude;
    // Guard against settling "stably" at sea level, which would make the rest vacuous.
    ASSERT_NEAR(settledAltitude, plateauMeters, 5.0);

    // Ten more frames with nothing else changing.
    const double later = test.renderAndGetAltitude(10);
    const auto atEnd = test.map.getCameraOptions({});

    EXPECT_NEAR(later, settledAltitude, 0.5);
    EXPECT_NEAR(atEnd.center->latitude(), afterSettling.center->latitude(), 1e-6);
    EXPECT_NEAR(atEnd.center->longitude(), afterSettling.center->longitude(), 1e-6);
    EXPECT_NEAR(*atEnd.zoom, *afterSettling.zoom, 1e-6);
}

// Turning it off leaves the camera where the caller put it.
TEST(TerrainCamera, ClampingCanBeTurnedOff) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(false);
    EXPECT_FALSE(test.map.getCenterClampedToGround());

    EXPECT_NEAR(test.renderAndGetAltitude(8), 0.0, 0.001);
}

// An app can put the centre at any altitude without moving the camera, and restore a saved camera
// by setting its altitude before its centre and zoom: the same centre and zoom at another altitude
// are a different camera.
TEST(TerrainCamera, CentreAltitudeIsSetAndRestoredKeepingTheView) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(false);
    test.map.jumpTo(CameraOptions().withZoom(10.0));

    const vec3 before = test.eye();
    const double zoomAtSeaLevel = *test.map.getCameraOptions({}).zoom;

    EXPECT_TRUE(test.map.setCenterAltitudeKeepingView(1500.0));
    const CameraOptions saved = test.map.getCameraOptions({});
    EXPECT_NEAR(*saved.centerAltitude, 1500.0, 0.001);
    EXPECT_GT(*saved.zoom, zoomAtSeaLevel);
    test.expectEyeAt(before);
    // Already there: nothing to move.
    EXPECT_FALSE(test.map.setCenterAltitudeKeepingView(1500.0));

    // Somewhere else, at sea level.
    EXPECT_TRUE(test.map.setCenterAltitudeKeepingView(0.0));
    test.map.jumpTo(CameraOptions().withCenter(LatLng{47.0, 11.0}).withZoom(9.0));

    // Restore: the altitude first, then the centre and zoom.
    test.map.setCenterAltitudeKeepingView(*saved.centerAltitude);
    test.map.jumpTo(CameraOptions().withCenter(*saved.center).withZoom(*saved.zoom));
    test.expectEyeAt(before);

    // Back at sea level, the original centre and zoom.
    EXPECT_TRUE(test.map.setCenterAltitudeKeepingView(0.0));
    EXPECT_NEAR(*test.map.getCameraOptions({}).zoom, zoomAtSeaLevel, 1e-3);
    test.expectEyeAt(before);
}

// The same restore while a return to sea level still waits, here for the next render of a still
// map after the terrain went: setting the altitude runs the return first, so the jump that follows
// applies the saved centre and zoom at the saved altitude, not at sea level.
TEST(TerrainCamera, CameraRestoredWhileTheReturnWaitsIsKept) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(false);
    test.map.jumpTo(CameraOptions().withZoom(10.0));
    EXPECT_TRUE(test.map.setCenterAltitudeKeepingView(1500.0));
    const CameraOptions saved = test.map.getCameraOptions({});
    const vec3 savedEye = test.eye();

    // Somewhere else, anchored on the plateau, and then the terrain goes.
    EXPECT_TRUE(test.map.setCenterAltitudeKeepingView(0.0));
    test.map.jumpTo(CameraOptions().withCenter(LatLng{47.0, 11.0}).withZoom(9.0));
    // Lets the renderer report the plateau under the centre.
    ASSERT_NEAR(test.renderAndGetAltitude(8), 0.0, 0.001);
    test.map.anchorCenterOnTerrain();
    ASSERT_NEAR(*test.map.getCameraOptions({}).centerAltitude, plateauMeters, 0.5);
    test.map.getStyle().setTerrain(nullptr);

    test.map.setCenterAltitudeKeepingView(*saved.centerAltitude);
    test.map.jumpTo(CameraOptions().withCenter(*saved.center).withZoom(*saved.zoom));
    EXPECT_NEAR(*test.map.getCameraOptions({}).centerAltitude, *saved.centerAltitude, 0.001);
    test.expectEyeAt(savedEye);
}

// A gesture start anchors the centre on the ground being looked at without moving the camera,
// and removing the terrain brings it back to sea level, again without moving the camera:
// otherwise the flat map keeps orbiting a point up in the air.
TEST(TerrainCamera, AnchoredCentreReturnsToSeaLevelWhenTerrainIsRemoved) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(false);
    test.map.jumpTo(CameraOptions().withZoom(10.0));
    // Lets the renderer report the plateau under the centre.
    ASSERT_NEAR(test.renderAndGetAltitude(8), 0.0, 0.001);

    const vec3 before = test.eye();
    const double zoomBefore = *test.map.getCameraOptions({}).zoom;

    test.map.anchorCenterOnTerrain();
    EXPECT_NEAR(*test.map.getCameraOptions({}).centerAltitude, plateauMeters, 0.5);
    EXPECT_GT(*test.map.getCameraOptions({}).zoom, zoomBefore);
    test.expectEyeAt(before);

    test.map.getStyle().setTerrain(nullptr);
    EXPECT_NEAR(test.renderAndGetAltitude(1), 0.0, 0.001);
    EXPECT_NEAR(*test.map.getCameraOptions({}).zoom, zoomBefore, 1e-3);
    test.expectEyeAt(before);

    // Without terrain there is nothing to anchor on, whatever height was last reported.
    test.map.anchorCenterOnTerrain();
    EXPECT_NEAR(test.map.getCameraOptions({}).centerAltitude.value_or(0.0), 0.0, 0.001);
}

// Near grazing, the line of sight runs almost along the ground: keeping the view would slide the
// centre off towards the horizon, and close to 90 degrees past a pole. The anchor leaves the
// camera alone there instead.
TEST(TerrainCamera, AnchoringAtAGrazingPitchLeavesTheCameraAlone) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(false);
    test.map.jumpTo(CameraOptions().withZoom(9.0));
    // Lets the renderer report the plateau under the centre.
    ASSERT_NEAR(test.renderAndGetAltitude(8), 0.0, 0.001);
    test.map.setBounds(BoundOptions().withMaxPitch(89.0));
    test.map.jumpTo(CameraOptions().withPitch(89.0));
    const vec3 before = test.eye();

    test.map.anchorCenterOnTerrain();
    const auto camera = test.map.getCameraOptions({});
    EXPECT_NEAR(*camera.centerAltitude, 0.0, 0.001);
    EXPECT_NEAR(*camera.zoom, 9.0, 1e-9);
    test.expectEyeAt(before);
}

// Anchoring raises the zoom. Past the maximum zoom, jumpTo would clamp it and the view would step
// back, so the anchor leaves the camera alone there.
TEST(TerrainCamera, AnchoringPastMaxZoomLeavesTheCameraAlone) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(false);
    test.map.jumpTo(CameraOptions().withZoom(10.0));
    // Lets the renderer report the plateau under the centre.
    ASSERT_NEAR(test.renderAndGetAltitude(8), 0.0, 0.001);
    // Anchoring on the plateau raises the zoom by ~0.15.
    test.map.setBounds(BoundOptions().withMaxZoom(10.05));
    const vec3 before = test.eye();

    test.map.anchorCenterOnTerrain();
    const auto camera = test.map.getCameraOptions({});
    EXPECT_NEAR(*camera.centerAltitude, 0.0, 0.001);
    EXPECT_NEAR(*camera.zoom, 10.0, 1e-9);
    test.expectEyeAt(before);

    // With room to zoom in, the same anchor goes through.
    test.map.setBounds(BoundOptions().withMaxZoom(util::DEFAULT_MAX_ZOOM));
    test.map.anchorCenterOnTerrain();
    EXPECT_NEAR(*test.map.getCameraOptions({}).centerAltitude, plateauMeters, 0.5);
    test.expectEyeAt(before);
}

// The same for the bounds: they would hold the centre back from where the anchor moves it.
TEST(TerrainCamera, AnchoringPastTheBoundsLeavesTheCameraAlone) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(false);
    test.map.jumpTo(CameraOptions().withZoom(10.0));
    // Lets the renderer report the plateau under the centre.
    ASSERT_NEAR(test.renderAndGetAltitude(8), 0.0, 0.001);
    // The anchor moves the centre ~1.7 km towards the camera, south here; the bounds end ~500 m
    // south of it.
    const LatLng center = *test.map.getCameraOptions({}).center;
    test.map.setBounds(BoundOptions().withLatLngBounds(LatLngBounds::hull(
        {center.latitude() - 0.0045, center.longitude() - 1.0}, {center.latitude() + 1.0, center.longitude() + 1.0})));
    const vec3 before = test.eye();

    test.map.anchorCenterOnTerrain();
    const auto camera = test.map.getCameraOptions({});
    EXPECT_NEAR(*camera.centerAltitude, 0.0, 0.001);
    EXPECT_NEAR(*camera.zoom, 10.0, 1e-9);
    EXPECT_NEAR(camera.center->latitude(), center.latitude(), 1e-9);
    EXPECT_NEAR(camera.center->longitude(), center.longitude(), 1e-9);
    test.expectEyeAt(before);
}

// A move across the antimeridian wraps the centre's longitude, which is no limit cutting it short.
TEST(TerrainCamera, AnchoringAcrossTheAntimeridianGoesThrough) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(false);
    // Looking west from just west of the antimeridian: the camera, and the anchor's move towards
    // it, are east of the centre.
    test.map.jumpTo(CameraOptions().withCenter(LatLng{47.2692, 179.99}).withZoom(10.0).withBearing(270.0));
    // Lets the renderer report the plateau under the centre.
    ASSERT_NEAR(test.renderAndGetAltitude(8), 0.0, 0.001);

    test.map.anchorCenterOnTerrain();
    const auto camera = test.map.getCameraOptions({});
    EXPECT_NEAR(*camera.centerAltitude, plateauMeters, 0.5);
    EXPECT_LT(camera.center->longitude(), -179.9);
}

// Going back to sea level lowers the zoom. Past the minimum zoom the centre comes down where it is,
// at its zoom, as in gl-js, rather than stay up in the air over the flat map.
TEST(TerrainCamera, ReturnToSeaLevelAtMinZoomKeepsTheCentre) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(false);
    test.map.jumpTo(CameraOptions().withZoom(10.0));
    // Lets the renderer report the plateau under the centre.
    ASSERT_NEAR(test.renderAndGetAltitude(8), 0.0, 0.001);
    test.map.anchorCenterOnTerrain();
    const auto anchored = test.map.getCameraOptions({});
    ASSERT_NEAR(*anchored.centerAltitude, plateauMeters, 0.5);
    // Keeping the view would take the zoom back to 10.
    test.map.setBounds(BoundOptions().withMinZoom(10.05));

    test.map.getStyle().setTerrain(nullptr);
    EXPECT_NEAR(test.renderAndGetAltitude(1), 0.0, 0.001);
    const auto camera = test.map.getCameraOptions({});
    EXPECT_NEAR(*camera.zoom, *anchored.zoom, 1e-9);
    EXPECT_NEAR(camera.center->latitude(), anchored.center->latitude(), 1e-9);
    EXPECT_NEAR(camera.center->longitude(), anchored.center->longitude(), 1e-9);
}

// A running transition re-applies the altitude it started with on every frame, so the return to
// sea level waits for it to end (here, to be cancelled), then keeps the view.
TEST(TerrainCamera, RemovingTerrainMidTransitionReturnsAfterIt) {
    TerrainCameraTest test{MapMode::Continuous};
    test.map.setCenterClampedToGround(false);
    test.map.jumpTo(CameraOptions().withZoom(10.0).withCenterAltitude(plateauMeters));
    test.map.easeTo(CameraOptions().withBearing(90.0), AnimationOptions(Seconds(10)));

    test.map.getStyle().setTerrain(nullptr);
    EXPECT_NEAR(*test.map.getCameraOptions({}).centerAltitude, plateauMeters, 0.001);

    const vec3 before = test.eye();
    test.map.cancelTransitions();
    EXPECT_NEAR(*test.map.getCameraOptions({}).centerAltitude, 0.0, 0.001);
    test.expectEyeAt(before);
}

// A camera the app sets while the return to sea level waits is the app's to keep: the return
// comes first, from the camera being replaced, and must not move the new one afterwards.
TEST(TerrainCamera, CameraSetWhileTheReturnWaitsIsKept) {
    TerrainCameraTest test{MapMode::Continuous};
    test.map.setCenterClampedToGround(false);
    test.map.jumpTo(CameraOptions().withZoom(10.0).withCenterAltitude(plateauMeters));
    test.map.easeTo(CameraOptions().withBearing(90.0), AnimationOptions(Seconds(10)));
    // The return waits for the ease.
    test.map.getStyle().setTerrain(nullptr);

    const LatLng target{47.3, 11.5};
    test.map.jumpTo(CameraOptions().withCenter(target).withZoom(12.0));
    const auto camera = test.map.getCameraOptions({});
    EXPECT_NEAR(*camera.centerAltitude, 0.0, 0.001);
    EXPECT_NEAR(*camera.zoom, 12.0, 1e-9);
    EXPECT_NEAR(camera.center->latitude(), target.latitude(), 1e-9);
    EXPECT_NEAR(camera.center->longitude(), target.longitude(), 1e-9);
}

// The same on a still map, where the return waits for the next render.
TEST(TerrainCamera, CameraSetBeforeTheNextStillRenderIsKept) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(false);
    test.map.jumpTo(CameraOptions().withZoom(10.0));
    // Lets the renderer report the plateau under the centre.
    ASSERT_NEAR(test.renderAndGetAltitude(8), 0.0, 0.001);
    test.map.anchorCenterOnTerrain();
    test.map.getStyle().setTerrain(nullptr);

    const LatLng target{47.3, 11.5};
    test.map.jumpTo(CameraOptions().withCenter(target).withZoom(12.0));
    EXPECT_NEAR(test.renderAndGetAltitude(1), 0.0, 0.001);
    const auto camera = test.map.getCameraOptions({});
    EXPECT_NEAR(*camera.zoom, 12.0, 1e-9);
    EXPECT_NEAR(camera.center->latitude(), target.latitude(), 1e-9);
    EXPECT_NEAR(camera.center->longitude(), target.longitude(), 1e-9);
}

namespace {

// A flat Terrain-RGB tile at `meters`, a multiple of 0.1 m.
std::string makeDEMTile(double meters) {
    const auto value = static_cast<uint32_t>(std::lround((meters + 10000.0) * 10.0));
    PremultipliedImage image({demTileSize, demTileSize});
    for (uint32_t i = 0; i < demTileSize * demTileSize; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = static_cast<uint8_t>(value >> 16);
        px[1] = static_cast<uint8_t>(value >> 8);
        px[2] = static_cast<uint8_t>(value);
        px[3] = 255;
    }
    return encodePNG(image);
}

constexpr double westPlateauMeters = 2000.0;

// Continuous mode, so camera transitions run on the clock while frames render, with the
// clamp on. The z2 DEM tiles between 90°W and the prime meridian (x = 1) are a 2000 m
// plateau, the rest 1000 m, so a move west from Innsbruck crosses a step in the terrain.
struct TerrainTransitionTest {
    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(),
                                                                                  ClientOptions());
    std::string lowTile = makeDEMTile(plateauMeters);
    std::string highTile = makeDEMTile(westPlateauMeters);
    HeadlessFrontend frontend{{256, 256}, 1};
    MapAdapter map;

    TerrainTransitionTest()
        : map(frontend,
              MapObserver::nullObserver(),
              fileSource,
              MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize())) {
        fileSource->tileResponse = [this](const Resource& resource) {
            const bool west = resource.tileData && resource.tileData->z == 2 && resource.tileData->x == 1;
            Response res;
            res.data = std::make_shared<std::string>(west ? highTile : lowTile);
            return res;
        };
        map.getStyle().loadJSON(terrainStyle);
        map.setCenterClampedToGround(true);
        map.jumpTo(CameraOptions().withCenter(LatLng{47.2692, 11.4041}).withZoom(2.0).withPitch(60.0));
    }

    void renderFrame() {
        loop.runOnce();
        frontend.renderFrame();
    }

    double altitude() { return map.getCameraOptions({}).centerAltitude.value_or(0.0); }

    // Renders until the clamp has put the centre on the 1000 m plateau under Innsbruck.
    bool settleOnPlateau() {
        const auto deadline = Clock::now() + Seconds(10);
        while (Clock::now() < deadline) {
            renderFrame();
            if (std::abs(altitude() - plateauMeters) < 5.0) {
                return true;
            }
        }
        return false;
    }

    // Eases west onto the 2000 m plateau; `whileRunning` is called after every frame of the
    // transition. Returns once the transition has finished and a few more frames rendered.
    template <typename Fn>
    bool easeWest(Fn whileRunning) {
        bool finished = false;
        AnimationOptions animation(Milliseconds(1500));
        animation.transitionFinishFn = [&] {
            finished = true;
        };
        map.easeTo(CameraOptions().withCenter(westTarget), animation);

        const auto deadline = Clock::now() + Seconds(20);
        while (!finished && Clock::now() < deadline) {
            renderFrame();
            whileRunning(finished);
        }
        for (int i = 0; i < 10; ++i) {
            renderFrame();
        }
        return finished;
    }

    const LatLng westTarget{47.2692, -30.0};
};

} // namespace

// Across relief the height under the centre changes every frame of a camera transition. The
// clamp waits for the transition to end instead of cancelling it with a jump, then puts the
// centre on the ground it arrived over.
TEST(TerrainCamera, ClampWaitsForTransitionToEnd) {
    TerrainTransitionTest test;
    ASSERT_TRUE(test.settleOnPlateau());

    ASSERT_TRUE(test.easeWest([](bool) {}));

    const auto camera = test.map.getCameraOptions({});
    EXPECT_NEAR(camera.center->latitude(), test.westTarget.latitude(), 1e-6);
    EXPECT_NEAR(camera.center->longitude(), test.westTarget.longitude(), 1e-6);
    EXPECT_NEAR(*camera.centerAltitude, westPlateauMeters, 5.0);
}

// A terrain removed while the clamp waits for a transition takes the waiting height with it:
// once the transition ends, the centre returns to sea level and the clamp does not raise it
// back onto ground that is no longer drawn.
TEST(TerrainCamera, ClampDoesNotApplyAfterTerrainRemoval) {
    TerrainTransitionTest test;
    ASSERT_TRUE(test.settleOnPlateau());

    bool removedMidTransition = false;
    ASSERT_TRUE(test.easeWest([&](bool finished) {
        // Well over the 2000 m plateau, so the clamp has a new height waiting.
        if (!finished && !removedMidTransition && test.map.getCameraOptions({}).center->longitude() < -5.0) {
            test.map.getStyle().setTerrain(nullptr);
            removedMidTransition = true;
        }
    }));
    ASSERT_TRUE(removedMidTransition);

    EXPECT_NEAR(test.altitude(), 0.0, 0.001);
}

// A drape target is drawn into only by the source tiles that overlap its own tile, so the
// masks it draws follow those tiles and not every tile the source covers.
TEST(TerrainDrape, MasksAreDrawnOnlyForTilesOverlappingTheTarget) {
    const auto counts = drapeMasksOverFrames(60);
    ASSERT_GT(counts.targets, 0);
    EXPECT_GT(counts.drawn, 0);
    EXPECT_GT(counts.skipped, 0);
    // The source's tiles are coarser than the mesh or the same size, so a target overlaps a
    // handful of them at most.
    EXPECT_LE(counts.drawn, 4 * counts.targets)
        << counts.drawn << " masks over " << counts.targets << " targets, " << counts.skipped << " skipped";
}

// Flat terrain at sea level seen straight down covers the map exactly, so the draped frame must
// match the frame without terrain, colour for colour. On WebGPU the terrain surface drew nothing
// (its layer group never set the 3D depth and stencil state), and the drape texture was sampled
// upside down.
TEST(TerrainDrape, FlatTerrainLooksLikeTheFlatMap) {
    const auto flat = renderQuadrants(false);
    const auto draped = renderQuadrants(true);
    ASSERT_EQ(flat.size, draped.size);
    const size_t pixels = static_cast<size_t>(flat.size.width) * flat.size.height;
    size_t same = 0;
    for (size_t i = 0; i < pixels; ++i) {
        const bool red = (flat.data[i * 4] > 127) == (draped.data[i * 4] > 127);
        const bool green = (flat.data[i * 4 + 1] > 127) == (draped.data[i * 4 + 1] > 127);
        const bool opaque = draped.data[i * 4 + 3] > 127;
        same += red && green && opaque ? 1 : 0;
    }
    // Only pixels on a colour edge may differ.
    EXPECT_GT(same, pixels * 95 / 100) << same << " of " << pixels << " pixels match";
}

// The depth pass (the symbol-occlusion depth texture) is a function of the camera and the mesh
// set. With both unchanged the previous texture still holds, so still frames run no pass.
// RenderingStats counters accumulate across frames, so the test reads differences.
TEST(TerrainDepthPass, StillCameraSkipsThePass) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(false);
    test.renderAndGetAltitude(8);
    const auto depthPassesAfterFrame = [&] {
        test.loop.runOnce();
        return test.frontend.render(test.map).stats.numDepthPasses;
    };

    // Tiles and DEM data still arriving mark the pass dirty, so wait for three frames in a row
    // without a pass. Before the pass was skipped on still frames it ran on every frame.
    int passes = depthPassesAfterFrame();
    int stillFrames = 0;
    for (int frame = 0; frame < 60 && stillFrames < 3; ++frame) {
        const int next = depthPassesAfterFrame();
        stillFrames = next == passes ? stillFrames + 1 : 0;
        passes = next;
    }
    ASSERT_EQ(stillFrames, 3) << "the depth pass ran on still frames";

    // Tilt one degree less. The maximum pitch defaults to 60 degrees, so asking for 61 clamps back to
    // 60 and leaves the camera where it was. A small tilt also keeps the tile cover the same, so the
    // pass can only run because the gate saw the camera move, not because the mesh set changed.
    const double pitch = test.map.getCameraOptions({}).pitch.value_or(0.0);
    test.map.jumpTo(CameraOptions().withPitch(pitch - 1.0));
    ASSERT_NEAR(test.map.getCameraOptions({}).pitch.value_or(0.0), pitch - 1.0, 1e-6) << "the camera did not move";
    EXPECT_GT(depthPassesAfterFrame(), passes) << "a camera move must redraw the depth texture";
}

namespace {

// Ground that rises to the south: nothing north of the ridge line, then a steep climb to 6,000 m,
// as a function of latitude alone so the test knows the height anywhere.
constexpr double rampStartLatitude = 47.30;
double rampMeters(double latitude) {
    return std::clamp((rampStartLatitude - latitude) * 100000.0, 0.0, 6000.0);
}

std::string makeRampDEMTile(const std::string& url) {
    // ".../{z}-{x}-{y}.png"
    int z = 0, x = 0, y = 0;
    const auto name = url.substr(url.rfind('/') + 1);
    std::sscanf(name.c_str(), "%d-%d-%d.png", &z, &x, &y);
    const double n = std::pow(2.0, z);
    PremultipliedImage image({demTileSize, demTileSize});
    for (uint32_t row = 0; row < demTileSize; ++row) {
        const double mercatorY = (y + (row + 0.5) / demTileSize) / n;
        const double latitude = util::rad2deg(std::atan(std::sinh(M_PI * (1.0 - 2.0 * mercatorY))));
        const auto encoded = static_cast<uint32_t>(std::lround((rampMeters(latitude) + 10000.0) * 10.0));
        for (uint32_t col = 0; col < demTileSize; ++col) {
            uint8_t* px = image.data.get() + (row * demTileSize + col) * 4;
            px[0] = static_cast<uint8_t>(encoded >> 16);
            px[1] = static_cast<uint8_t>((encoded >> 8) & 0xFF);
            px[2] = static_cast<uint8_t>(encoded & 0xFF);
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

const char* rampStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 9,
      "tileSize": 64
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": []
})STYLE";

} // namespace

// A pan that drags the eye towards a slope rising in front of it must not leave the eye under
// the ground: at a steep pitch the eye sits only a little above the center, so the ground under
// the center being the camera's floor is not enough. The camera is lifted until the eye clears
// the ground under it.
TEST(TerrainCamera, EyeStaysAboveTheSlopeRisingTowardsTheCamera) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(makeRampDEMTile(resource.url));
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.setCenterClampedToGround(true);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.getStyle().loadJSON(rampStyle);
    // The center on the ridge line, looking north from the climb behind it.
    map.jumpTo(
        CameraOptions().withCenter(LatLng{rampStartLatitude, 11.4041}).withZoom(12.0).withPitch(80.0).withBearing(0.0));

    for (int i = 0; i < 24; ++i) {
        loop.runOnce();
        frontend.render(map);
    }

    const auto location = map.getFreeCameraOptions().getLocation();
    ASSERT_TRUE(location.has_value());
    const double ground = rampMeters(location->location.latitude());
    ASSERT_GT(ground, 100.0) << "the eye is over the climb";
    EXPECT_GE(location->altitude, ground + 5.0) << "eye altitude " << location->altitude << " over ground " << ground;
}

// A drag over relief keeps the camera at the height it started at, as gl-js's elevationFreeze
// does: the clamp following the ground under the center moved the camera up and down with every
// ridge and valley the drag passed. When the gesture ends, the center goes onto the ground
// without moving the camera.
TEST(TerrainCamera, AGestureKeepsTheCameraHeightAndItsEndKeepsTheView) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(makeRampDEMTile(resource.url));
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.setCenterClampedToGround(true);
    map.getStyle().loadJSON(rampStyle);
    const auto settle = [&] {
        for (int i = 0; i < 24; ++i) {
            loop.runOnce();
            frontend.render(map);
        }
    };
    const auto eye = [&] {
        const auto location = map.getFreeCameraOptions().getLocation();
        EXPECT_TRUE(location.has_value());
        return location.value_or(LatLngAltitude{});
    };
    // High enough that the eye clears the 6,000 m plateau by a wide margin, so only the hold
    // decides the camera's height.
    map.jumpTo(CameraOptions().withCenter(LatLng{47.40, 11.4041}).withZoom(10.0).withPitch(45.0).withBearing(0.0));
    settle();
    ASSERT_NEAR(*map.getCameraOptions().centerAltitude, 0.0, 1.0) << "the center starts on flat ground";
    const double eyeAltitudeBefore = eye().altitude;

    // Drag the center from the flat ground onto the plateau.
    map.setGestureInProgress(true);
    map.jumpTo(CameraOptions().withCenter(LatLng{47.20, 11.4041}));
    settle();
    EXPECT_NEAR(*map.getCameraOptions().centerAltitude, 0.0, 1.0) << "the gesture holds the center's height";
    // The eye sits a fixed number of pixels from the center, so its height above the held center
    // scales with the meters per pixel at the center's latitude; without the hold it rose 6,000 m.
    const double scaleChange = std::cos(util::deg2rad(47.20)) / std::cos(util::deg2rad(47.40));
    EXPECT_NEAR(eye().altitude, eyeAltitudeBefore * scaleChange, 2.0)
        << "the camera keeps its height during the gesture";

    const auto eyeAtRelease = eye();
    map.setGestureInProgress(false);
    settle();
    EXPECT_NEAR(*map.getCameraOptions().centerAltitude, rampMeters(map.getCameraOptions().center->latitude()), 10.0)
        << "the end anchors the center on the ground";
    const auto eyeAfter = eye();
    // The anchor takes the meters per pixel at the old center, so with the camera 47 km out it
    // lands within meters of where it was; following the ground instead would lift it 6,000 m.
    EXPECT_NEAR(eyeAfter.altitude, eyeAtRelease.altitude, 10.0) << "the anchor keeps the camera where it is";
    EXPECT_NEAR(eyeAfter.location.latitude(), eyeAtRelease.location.latitude(), 1e-4);
    EXPECT_NEAR(eyeAfter.location.longitude(), eyeAtRelease.location.longitude(), 1e-4);
}

// A gesture that ends over a slope: the end anchors the center at the height of the ground under
// the old center, which puts it on ground that is higher or lower than that. Correcting the
// center's height in place then moves the camera by the difference, the jump the hold was meant
// to remove. As gl-js's recalculateZoomAndCenter does in passes, each correction slides the
// center along the line of sight instead, so the camera stays where the gesture left it.
TEST(TerrainCamera, AGestureEndingOverASlopeKeepsTheCameraWhereItIs) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(makeRampDEMTile(resource.url));
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.setCenterClampedToGround(true);
    map.getStyle().loadJSON(rampStyle);
    const auto settle = [&] {
        for (int i = 0; i < 24; ++i) {
            loop.runOnce();
            frontend.render(map);
        }
    };
    const auto eye = [&] {
        const auto location = map.getFreeCameraOptions().getLocation();
        EXPECT_TRUE(location.has_value());
        return location.value_or(LatLngAltitude{});
    };
    // Looking north, so the camera sits south of the center, up the slope.
    map.jumpTo(CameraOptions().withCenter(LatLng{47.40, 11.4041}).withZoom(12.0).withPitch(60.0).withBearing(0.0));
    settle();

    // Drag the center onto the middle of the slope, where the ground is 3,000 m.
    map.setGestureInProgress(true);
    map.jumpTo(CameraOptions().withCenter(LatLng{47.27, 11.4041}));
    settle();
    const auto eyeAtRelease = eye();
    map.setGestureInProgress(false);
    settle();

    const auto eyeAfter = eye();
    EXPECT_NEAR(eyeAfter.altitude, eyeAtRelease.altitude, 10.0) << "the camera stays where the gesture left it";
    EXPECT_NEAR(eyeAfter.location.latitude(), eyeAtRelease.location.latitude(), 1e-4);
    const auto camera = map.getCameraOptions();
    EXPECT_NEAR(*camera.centerAltitude, rampMeters(camera.center->latitude()), 30.0)
        << "the center ends on the ground it looks at";
}

namespace {

// Ground in east-west bands 0.01 degrees wide, alternately 2,000 m and 2,600 m, south of the
// ridge line; flat at 0 north of it. At pitch 60 a 600 m change in the center's height moves the
// center about a band along the line of sight, so taking the height of the ground under the new
// center each time flips between the two.
double bandMeters(double latitude) {
    if (latitude > rampStartLatitude) return 0.0;
    return (static_cast<int>(std::floor((rampStartLatitude - latitude) / 0.01)) % 2 == 0) ? 2000.0 : 2600.0;
}

std::string makeBandDEMTile(const std::string& url) {
    int z = 0, x = 0, y = 0;
    const auto name = url.substr(url.rfind('/') + 1);
    std::sscanf(name.c_str(), "%d-%d-%d.png", &z, &x, &y);
    const double n = std::pow(2.0, z);
    PremultipliedImage image({demTileSize, demTileSize});
    for (uint32_t row = 0; row < demTileSize; ++row) {
        const double mercatorY = (y + (row + 0.5) / demTileSize) / n;
        const double latitude = util::rad2deg(std::atan(std::sinh(M_PI * (1.0 - 2.0 * mercatorY))));
        const auto encoded = static_cast<uint32_t>(std::lround((bandMeters(latitude) + 10000.0) * 10.0));
        for (uint32_t col = 0; col < demTileSize; ++col) {
            uint8_t* px = image.data.get() + (row * demTileSize + col) * 4;
            px[0] = static_cast<uint8_t>(encoded >> 16);
            px[1] = static_cast<uint8_t>((encoded >> 8) & 0xFF);
            px[2] = static_cast<uint8_t>(encoded & 0xFF);
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

} // namespace

// Over broken ground the height under the center can flip between two values as the center slides
// along the line of sight, so the passes after an anchor never agree with it. They must still not
// fall back to moving the camera: the center is narrowed onto where the line of sight meets the
// ground, and left there.
TEST(TerrainCamera, AGestureEndingOverBrokenGroundKeepsTheCameraWhereItIs) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(makeBandDEMTile(resource.url));
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.setCenterClampedToGround(true);
    map.getStyle().loadJSON(rampStyle);
    const auto settle = [&] {
        for (int i = 0; i < 40; ++i) {
            loop.runOnce();
            frontend.render(map);
        }
    };
    const auto eye = [&] {
        const auto location = map.getFreeCameraOptions().getLocation();
        EXPECT_TRUE(location.has_value());
        return location.value_or(LatLngAltitude{});
    };
    map.jumpTo(CameraOptions().withCenter(LatLng{47.40, 11.4041}).withZoom(12.0).withPitch(60.0).withBearing(0.0));
    settle();

    for (const double latitude : {47.255, 47.245, 47.235, 47.225}) {
        map.setGestureInProgress(true);
        map.jumpTo(CameraOptions().withCenter(LatLng{latitude, 11.4041}));
        settle();
        const auto eyeAtRelease = eye();
        map.setGestureInProgress(false);
        settle();
        const auto eyeAfter = eye();
        EXPECT_NEAR(eyeAfter.altitude, eyeAtRelease.altitude, 10.0)
            << "released over " << latitude << ": the camera stays where the gesture left it";
    }
}

// Looking from flat ground at a wall of terrain, the line of sight meets the wall almost at once, so
// the center cannot slide onto the height of the ground under it with the camera kept where it is.
// The end of the gesture must then leave the center, not move the camera onto that height.
TEST(TerrainCamera, AGestureEndingWhereTheCenterCannotSlideKeepsTheCamera) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(makeRampDEMTile(resource.url));
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.setCenterClampedToGround(true);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.getStyle().loadJSON(rampStyle);
    const auto settle = [&] {
        for (int i = 0; i < 40; ++i) {
            loop.runOnce();
            frontend.render(map);
        }
    };
    const auto eye = [&] {
        const auto location = map.getFreeCameraOptions().getLocation();
        EXPECT_TRUE(location.has_value());
        return location.value_or(LatLngAltitude{});
    };
    // Looking south from the flat ground at the climb.
    map.jumpTo(CameraOptions().withCenter(LatLng{47.40, 11.4041}).withZoom(13.0).withPitch(80.0).withBearing(180.0));
    settle();

    map.setGestureInProgress(true);
    map.jumpTo(CameraOptions().withCenter(LatLng{47.25, 11.4041}));
    settle();
    const auto eyeAtRelease = eye();
    ASSERT_LT(eyeAtRelease.altitude, 4000.0) << "the eye is still low over the flat ground, below the plateau";
    map.setGestureInProgress(false);
    settle();
    EXPECT_NEAR(eye().altitude, eyeAtRelease.altitude, 10.0) << "the camera stays where the gesture left it";
}

// A fling at a steep pitch towards rising ground carried the eye under the ground: the clamp stands
// aside while a transition runs. As gl-js keeps the camera above the terrain during an animation,
// the eye clearance still lifts a fling's camera, which ends the fling at the slope.
//
// The fling here is long and short in distance, and the eye starts under the ground it is over, so
// the outcome does not depend on how fast the frames run: the camera barely moves while the report
// of the ground under it arrives, however late that is, and only a lift can bring the eye above the
// ground before the fling is over.
TEST(TerrainCamera, AFlingDoesNotCarryTheEyeUnderTheGround) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(makeRampDEMTile(resource.url));
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize()));
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.getStyle().loadJSON(rampStyle);
    const auto pump = [&](std::chrono::milliseconds duration, const std::function<bool()>& done = nullptr) {
        const auto end = std::chrono::steady_clock::now() + duration;
        while (std::chrono::steady_clock::now() < end && !(done && done())) {
            loop.runOnce();
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
    };
    const auto clearance = [&] {
        const auto location = map.getFreeCameraOptions().getLocation();
        EXPECT_TRUE(location.has_value());
        return location ? location->altitude - rampMeters(location->location.latitude()) : 0.0;
    };

    // With the clamp off, put the center half way up the climb at sea level, looking south: the eye
    // sits a little north of it, on the slope, with the ground a couple of thousand meters above it. The renderer
    // reports that ground whatever the clamp does.
    map.setCenterClampedToGround(false);
    map.jumpTo(CameraOptions().withCenter(LatLng{47.289, 11.4041}).withZoom(16.0).withPitch(80.0).withBearing(180.0));
    pump(std::chrono::milliseconds(2000));
    map.jumpTo(CameraOptions().withCenter(LatLng{47.27, 11.4041}));
    pump(std::chrono::milliseconds(3000));
    ASSERT_LT(clearance(), -1000.0) << "the eye starts inside the climb";

    // Turn the clamp on and start a fling that moves the camera a few tens of meters in 30 s. Its
    // first frames move the eye enough for the renderer to report the ground under it again.
    map.setCenterClampedToGround(true);
    map.moveBy({0, 40}, AnimationOptions(Milliseconds(30000)));
    pump(std::chrono::milliseconds(10000), [&] { return clearance() > 0.0; });
    EXPECT_GT(clearance(), 0.0) << "the eye was lifted out of the ground while the fling ran";
}

// An animation that is not a fling (a pinch's drift, a double-tap zoom, a recenter) zooming out
// backs the eye up a climb that rises faster than the eye does. The eye was lifted only for a fling,
// and for every other animation only once it had ended, so the camera was inside the mountain for
// the whole of it (up to a second after a pinch). Each frame of the animation keeps the eye over the
// ground under it.
TEST(TerrainCamera, AnAnimatedZoomDoesNotCarryTheEyeUnderTheGround) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(makeRampDEMTile(resource.url));
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize()));
    map.setCenterClampedToGround(true);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.getStyle().loadJSON(rampStyle);
    const auto pump = [&](std::chrono::milliseconds duration, const std::function<bool()>& done = nullptr) {
        const auto end = std::chrono::steady_clock::now() + duration;
        while (std::chrono::steady_clock::now() < end && !(done && done())) {
            loop.runOnce();
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
    };
    const auto clearance = [&] {
        const auto location = map.getFreeCameraOptions().getLocation();
        EXPECT_TRUE(location.has_value());
        return location ? location->altitude - rampMeters(location->location.latitude()) : 0.0;
    };

    // The center on the ridge line looking north, the climb behind the camera, the eye settled over it.
    map.jumpTo(
        CameraOptions().withCenter(LatLng{rampStartLatitude, 11.4041}).withZoom(14.0).withPitch(80.0).withBearing(0.0));
    pump(std::chrono::milliseconds(30000), [&] { return clearance() > 0.0; });
    ASSERT_GT(clearance(), 0.0) << "the eye starts over the ground";
    pump(std::chrono::milliseconds(500));

    bool finished = false;
    AnimationOptions animation(Milliseconds(3000));
    animation.transitionFinishFn = [&] {
        finished = true;
    };
    map.easeTo(CameraOptions().withZoom(12.0), animation);
    double lowest = std::numeric_limits<double>::infinity();
    pump(std::chrono::milliseconds(15000), [&] {
        if (!finished) {
            lowest = std::min(lowest, clearance());
        }
        return finished;
    });
    ASSERT_TRUE(finished);
    EXPECT_GE(lowest, 0.0) << "the eye was " << -lowest << " m under the ground during the animation";
}

// A camera animation the app asks for (a recenter on the user, a flight to a place) from high
// ground to low kept the center at the height it left, and only when it ended did the clamp jump
// the center down to the ground: the camera dropped thousands of meters in one frame. As gl-js
// does, the center's height eases to the ground under the target once it is known, so the
// animation ends with the center on the ground it was headed for.
TEST(TerrainCamera, AFlightEasesTheCenterHeightToTheGroundItIsHeadedFor) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(makeRampDEMTile(resource.url));
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize()));
    map.setCenterClampedToGround(true);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.getStyle().loadJSON(rampStyle);
    const auto pump = [&](std::chrono::milliseconds duration, const std::function<bool()>& done = nullptr) {
        const auto end = std::chrono::steady_clock::now() + duration;
        while (std::chrono::steady_clock::now() < end && !(done && done())) {
            loop.runOnce();
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
    };
    // On the 6,000 m plateau, from far enough above it that the renderer reports the ground under
    // the center, then tilted and close.
    map.jumpTo(CameraOptions().withCenter(LatLng{47.20, 11.4041}).withZoom(10.0).withPitch(0.0).withBearing(0.0));
    pump(std::chrono::milliseconds(2000));
    map.jumpTo(CameraOptions().withZoom(14.0).withPitch(60.0));
    pump(std::chrono::milliseconds(2000));
    ASSERT_NEAR(*map.getCameraOptions().centerAltitude, 6000.0, 30.0) << "the center starts on the plateau";

    // Fly to the flat ground north of the ridge, as a recenter does, and read the center's height at
    // the moment the animation ends, before anything that follows it can move it.
    const LatLng target{47.40, 11.4041};
    bool finished = false;
    double altitudeAtTheEnd = std::numeric_limits<double>::quiet_NaN();
    AnimationOptions animation(Milliseconds(3000));
    animation.transitionFinishFn = [&] {
        finished = true;
        altitudeAtTheEnd = map.getCameraOptions().centerAltitude.value_or(0.0);
    };
    map.easeTo(CameraOptions().withCenter(target).withZoom(14.0), animation);
    pump(std::chrono::milliseconds(15000), [&] { return finished; });
    ASSERT_TRUE(finished);
    EXPECT_NEAR(altitudeAtTheEnd, 0.0, 30.0) << "the center's height reached the ground it was headed for";

    pump(std::chrono::milliseconds(1000));
    const auto camera = map.getCameraOptions();
    EXPECT_NEAR(camera.center->latitude(), target.latitude(), 1e-3) << "the flight lands on the center asked for";
    EXPECT_NEAR(camera.center->longitude(), target.longitude(), 1e-3);
    EXPECT_NEAR(*camera.zoom, 14.0, 0.05) << "at the zoom asked for";
    EXPECT_NEAR(*camera.centerAltitude, 0.0, 30.0) << "with the center on the ground there";
}

namespace {

// A DEM tile of one height everywhere, in Terrain-RGB: -10000 + (R * 65536 + G * 256 + B) * 0.1.
std::string makeUniformDEMTile(double meters) {
    const auto encoded = static_cast<uint32_t>(std::lround((meters + 10000.0) * 10.0));
    PremultipliedImage image({demTileSize, demTileSize});
    for (uint32_t i = 0; i < demTileSize * demTileSize; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = static_cast<uint8_t>(encoded >> 16);
        px[1] = static_cast<uint8_t>((encoded >> 8) & 0xFF);
        px[2] = static_cast<uint8_t>(encoded & 0xFF);
        px[3] = 255;
    }
    return encodePNG(image);
}

// ".../{z}-{x}-{y}.png" holds the position, at its zoom, the tile covers.
bool tileUrlCovers(const std::string& url, const LatLng& position) {
    int z = 0, x = 0, y = 0;
    const auto name = url.substr(url.rfind('/') + 1);
    if (std::sscanf(name.c_str(), "%d-%d-%d.png", &z, &x, &y) != 3) {
        return false;
    }
    const auto at = Projection::project(position, z);
    return std::floor(at.x) == x && std::floor(at.y) == y;
}

// The ramp's ground again, with tiles fine enough that the one under a camera a few kilometers
// from the center is not the center's.
const char* deepRampStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 14,
      "tileSize": 64
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": []
})STYLE";

} // namespace

// Ground that has not loaded is not sea level. Close to the ground in a basin below sea level
// (the Dead Sea, -430 m), the eye check read the DEM tile under the eye that had not arrived as
// 0 m, took the ground to be 430 m higher than it is, and lifted the camera to 10 m over it.
TEST(TerrainCamera, EyeOverUnloadedGroundBelowSeaLevelIsNotLifted) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string basin = makeUniformDEMTile(-430.0);
    std::optional<LatLng> eyeGround;
    // The server has no DEM tile for the ground under the eye, of any zoom: nothing covers it.
    fileSource->tileResponse = [&](const Resource& resource) {
        Response res;
        if (eyeGround && tileUrlCovers(resource.url, *eyeGround)) {
            res.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound);
        } else {
            res.data = std::make_shared<std::string>(basin);
        }
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.setCenterClampedToGround(true);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.getStyle().loadJSON(deepRampStyle);
    // Looking north from close to the water: the eye sits a couple of hundred meters above the
    // center and a few kilometers south of it.
    map.jumpTo(CameraOptions().withCenter(LatLng{31.5, 35.5}).withZoom(15.0).withPitch(85.0).withBearing(0.0));
    const auto start = map.getFreeCameraOptions().getLocation();
    ASSERT_TRUE(start.has_value());
    eyeGround = start->location;

    for (int i = 0; i < 24; ++i) {
        loop.runOnce();
        frontend.render(map);
    }

    ASSERT_NEAR(*map.getCameraOptions().centerAltitude, -430.0, 1.0) << "the center is on the basin floor";
    const auto eye = map.getFreeCameraOptions().getLocation();
    ASSERT_TRUE(eye.has_value());
    EXPECT_LT(eye->altitude, 0.0) << "the eye was lifted over ground that is not known";
}

// The ground under the eye is often outside the view, behind or below it: the tile cover drops its
// tile once the DEM is known to lie below the frustum, and a camera backed towards a mountain behind
// it ended up inside it. The tile under the eye is always asked for, and the eye clears the ground
// it holds.
TEST(TerrainCamera, TheTileUnderTheEyeKeepsTheEyeOutOfTheMountainBehind) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(makeRampDEMTile(resource.url));
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.setCenterClampedToGround(true);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.getStyle().loadJSON(deepRampStyle);
    // Looking north from the foot of the climb, which rises to the south, behind the camera: the
    // center is on ground 1,000 m high and the eye a few kilometers up the slope behind it.
    map.jumpTo(CameraOptions().withCenter(LatLng{47.29, 11.4041}).withZoom(14.0).withPitch(85.0).withBearing(0.0));
    const auto start = map.getFreeCameraOptions().getLocation();
    ASSERT_TRUE(start.has_value());
    ASSERT_GT(rampMeters(start->location.latitude()), 3000.0) << "the eye starts well inside the mountain";

    for (int i = 0; i < 24; ++i) {
        loop.runOnce();
        frontend.render(map);
    }

    const auto eye = map.getFreeCameraOptions().getLocation();
    ASSERT_TRUE(eye.has_value());
    const double ground = rampMeters(eye->location.latitude());
    EXPECT_GE(eye->altitude, ground + 5.0) << "eye altitude " << eye->altitude << " over ground " << ground;
}

// A center on ground at exactly 0 m is ground like any other: the renderer's last reported height
// started at 0, so the height under the center was never reported, the clamp never ran, and the
// eye check with it.
TEST(TerrainCamera, TheEyeIsLiftedWhenTheGroundUnderTheCenterIsAtSeaLevel) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(makeRampDEMTile(resource.url));
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.setCenterClampedToGround(true);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.getStyle().loadJSON(rampStyle);
    // The center on the flat ground north of the ridge line, the climb behind the camera.
    map.jumpTo(CameraOptions().withCenter(LatLng{47.34, 11.4041}).withZoom(12.0).withPitch(80.0).withBearing(0.0));

    for (int i = 0; i < 24; ++i) {
        loop.runOnce();
        frontend.render(map);
    }

    const auto location = map.getFreeCameraOptions().getLocation();
    ASSERT_TRUE(location.has_value());
    const double ground = rampMeters(location->location.latitude());
    ASSERT_GT(ground, 100.0) << "the eye is over the climb";
    EXPECT_GE(location->altitude, ground + 5.0) << "eye altitude " << location->altitude << " over ground " << ground;
}

namespace {

// A camera over a high mountain with its eye height read, then thrown onto flat ground far away.
// The ground under the new eye is a tile of its own, which the test holds back, or never sends.
struct InspectableMap : MapAdapter {
    using MapAdapter::MapAdapter;
    double reportedCenterGround() const { return impl->terrainCenterElevation; }
    bool centerGroundLost() const { return impl->centerGroundLost; }
    void removeTerrain() { getStyle().setTerrain(nullptr); }
};

struct EyeLeavesTheMountain {
    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(),
                                                                                  ClientOptions());
    HeadlessFrontend frontend{{390, 844}, 1};
    InspectableMap map;
    std::optional<LatLng> newEyeGround;
    std::optional<LatLng> newCenterGround;
    enum class NewEyeTile {
        Held,
        Missing,
        Sent
    };
    NewEyeTile newEyeTile = NewEyeTile::Held;
    NewEyeTile newCenterTile = NewEyeTile::Sent;
    const std::string shelf = makeUniformDEMTile(2000.0);

    EyeLeavesTheMountain()
        : map(frontend,
              MapObserver::nullObserver(),
              fileSource,
              MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize())) {
        fileSource->tileResponse = [this](const Resource& resource) -> std::optional<Response> {
            Response res;
            for (const auto& [ground, tile] :
                 {std::pair{newEyeGround, newEyeTile}, std::pair{newCenterGround, newCenterTile}}) {
                if (!ground || !tileUrlCovers(resource.url, *ground)) {
                    continue;
                }
                if (tile == NewEyeTile::Held) {
                    return std::nullopt;
                }
                if (tile == NewEyeTile::Missing) {
                    res.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound);
                    return res;
                }
                // Sent: 2,000 m ground under it.
                res.data = std::make_shared<std::string>(shelf);
                return res;
            }
            res.data = std::make_shared<std::string>(makeRampDEMTile(resource.url));
            return res;
        };
        map.setCenterClampedToGround(true);
        map.setBounds(BoundOptions().withMaxPitch(85.0));
        map.getStyle().loadJSON(deepRampStyle);
    }

    // Continuous mode renders on its own as the loop runs; each step is a few milliseconds of it.
    // Runs until the condition holds, or for as long as the steps last.
    void render(int steps, const std::function<bool()>& done = nullptr) {
        for (int i = 0; i < steps && !(done && done()); ++i) {
            loop.runOnce();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

    double centerAltitude() const { return map.getCameraOptions().centerAltitude.value_or(0.0); }
    // The ground height under the center the renderer last reported to the map; NaN is "not known".
    double reportedCenterGround() const { return map.reportedCenterGround(); }
    bool centerGroundLost() const { return map.centerGroundLost(); }
    double eyeAltitude() const {
        const auto eye = map.getFreeCameraOptions().getLocation();
        return eye ? eye->altitude : 0.0;
    }

    // The flat ground the camera is thrown onto.
    static LatLng flatCenter() { return LatLng{47.60, 51.4041}; }

    // On the mountain: the eye a few kilometers up the slope behind the center, the camera lifted
    // out of it, then flat ground 40 degrees of longitude east, where the eye is over ground that is
    // not the mountain's. The center's own ground is never known there (the camera is too high over
    // flat ground for the cover to take its tile), so it keeps the height it last had, 1,000 m. With
    // `overview` the camera is also lifted to zoom 10, looking straight down, where the cover does
    // take the tile under the center.
    void climbThenJump(bool overview = false) {
        map.jumpTo(CameraOptions().withCenter(LatLng{47.29, 11.4041}).withZoom(15.0).withPitch(85.0).withBearing(0.0));
        render(1500, [&] { return centerAltitude() > 2500.0; });
        const auto onMountain = map.getFreeCameraOptions().getLocation();
        ASSERT_TRUE(onMountain.has_value());
        ASSERT_GT(rampMeters(onMountain->location.latitude()), 2500.0) << "the eye starts inside the mountain";
        ASSERT_GT(onMountain->altitude, 2500.0) << "and the camera is lifted out of it";
        ASSERT_GT(reportedCenterGround(), 500.0) << "the ground under the center is known there";

        map.jumpTo(overview ? CameraOptions().withCenter(flatCenter()).withZoom(10.0).withPitch(0.0)
                            : CameraOptions().withCenter(flatCenter()));
        const auto flat = map.getFreeCameraOptions().getLocation();
        ASSERT_TRUE(flat.has_value());
        ASSERT_EQ(rampMeters(flat->location.latitude()), 0.0) << "the new eye is over flat ground";
        if (!overview) {
            newEyeGround = flat->location;
        }
    }
};

} // namespace

// After a jump from a mountain to flat ground, the height read under the old eye is not the height
// under the new one. Where the new eye's tile never arrives, the eye is lifted by nothing: the camera
// keeps the center's last height, 1,000 m, and the eye sits over that, not over the mountain's 2,800 m.
TEST(TerrainCamera, EyeFarFromWhereItsHeightWasReadDoesNotKeepTheMountainClearance) {
    EyeLeavesTheMountain test;
    test.newEyeTile = EyeLeavesTheMountain::NewEyeTile::Missing;
    test.climbThenJump();
    test.render(400);

    EXPECT_LT(test.centerAltitude(), 1050.0) << "center altitude " << test.centerAltitude();
    EXPECT_LT(test.eyeAltitude(), 1300.0) << "eye altitude " << test.eyeAltitude() << " over flat ground";
}

// Until the tile under the new eye gives a height the eye's height is unknown, not the mountain's;
// once it does, the eye clears that ground, here 2,000 m.
TEST(TerrainCamera, EyeHeightIsUnknownUntilTheTileUnderTheNewEyeArrives) {
    EyeLeavesTheMountain test;
    test.climbThenJump();
    test.render(400);

    EXPECT_LT(test.centerAltitude(), 1050.0) << "center altitude " << test.centerAltitude();
    EXPECT_LT(test.eyeAltitude(), 1300.0) << "eye altitude " << test.eyeAltitude() << " while its tile is on the way";

    test.newEyeTile = EyeLeavesTheMountain::NewEyeTile::Sent;
    test.render(1500, [&] { return test.eyeAltitude() >= 2005.0; });

    EXPECT_GE(test.eyeAltitude(), 2005.0) << "eye altitude " << test.eyeAltitude() << " over ground 2000";
    EXPECT_LT(test.eyeAltitude(), 2500.0) << "eye altitude " << test.eyeAltitude() << " over ground 2000";
}

// The same for the height under the center, which is not the eye's: after a jump from the mountain to
// flat ground far away, the 1,000 m read under the old center is not the ground under the new one.
// Where the tile under the new center never arrives, the map is told the height is not known, not
// left with the mountain's, and the camera comes down to sea level as for one not told yet.
TEST(TerrainCamera, CenterFarFromWhereItsHeightWasReadDoesNotKeepTheMountainHeight) {
    EyeLeavesTheMountain test;
    test.newCenterGround = EyeLeavesTheMountain::flatCenter();
    test.newCenterTile = EyeLeavesTheMountain::NewEyeTile::Missing;
    test.climbThenJump(true);
    test.render(400);

    EXPECT_TRUE(std::isnan(test.reportedCenterGround())) << "reported center ground " << test.reportedCenterGround();
    EXPECT_LT(test.centerAltitude(), 100.0) << "center altitude " << test.centerAltitude() << " over unknown ground";
}

// A report that the ground under the center is lost belongs to the terrain it was reported for: with
// the terrain removed it no longer holds, and it must not meet a new terrain that comes back.
TEST(TerrainCamera, TheLostCenterGroundIsForgottenWhenTheTerrainIsRemoved) {
    EyeLeavesTheMountain test;
    test.newCenterGround = EyeLeavesTheMountain::flatCenter();
    test.newCenterTile = EyeLeavesTheMountain::NewEyeTile::Missing;
    test.climbThenJump(true);
    test.render(400);
    ASSERT_TRUE(test.centerGroundLost()) << "the renderer lost the ground under the center";

    test.map.removeTerrain();
    test.render(100);
    EXPECT_FALSE(test.centerGroundLost()) << "still lost with no terrain";
}

// Until the tile under the new center gives a height the center's height is unknown, not the
// mountain's; once it does, it is that ground's, here 2,000 m, and the center rides it.
TEST(TerrainCamera, CenterHeightIsUnknownUntilTheTileUnderTheNewCenterArrives) {
    EyeLeavesTheMountain test;
    test.newCenterGround = EyeLeavesTheMountain::flatCenter();
    test.newCenterTile = EyeLeavesTheMountain::NewEyeTile::Held;
    test.climbThenJump(true);
    test.render(400);

    EXPECT_TRUE(std::isnan(test.reportedCenterGround()))
        << "reported center ground " << test.reportedCenterGround() << " while its tile is on the way";

    test.newCenterTile = EyeLeavesTheMountain::NewEyeTile::Sent;
    test.render(1500, [&] { return test.reportedCenterGround() > 1500.0; });

    EXPECT_NEAR(test.reportedCenterGround(), 2000.0, 5.0);
    test.render(100);
    EXPECT_NEAR(test.centerAltitude(), 2000.0, 5.0)
        << "center altitude " << test.centerAltitude() << " over ground 2000";
}

namespace {

// What loadBudgetStats renders: a fill over a large area, under terrain in the given load mode.
struct LoadBudgetScene {
    TerrainLoadMode mode;
    int fillLayers = 1;       // fill layers stacked on the area
    bool replaceArea = false; // a second still follows a change of the area's shape
};

// The counters after the still(s) of the scene (they accumulate over frames). The changed area
// gives the drape targets under the tiles it rebuilds new content to bake.
gfx::RenderingStats loadBudgetStats(const LoadBudgetScene& scene) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string tile = makeFlatDEMTile();
    fileSource->tileResponse = [&](const Resource&) {
        Response res;
        res.data = std::make_shared<std::string>(tile);
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.getStyle().loadJSON(drapedFillStyle);
    for (int i = 1; i < scene.fillLayers; ++i) {
        map.getStyle().addLayer(std::make_unique<style::FillLayer>("area" + std::to_string(i), "area"));
    }
    map.setTerrainLoadMode(scene.mode);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.jumpTo(CameraOptions().withCenter(LatLng{47.2692, 11.4041}).withZoom(5.0).withPitch(80.0));

    auto stats = frontend.render(map).stats;
    if (scene.replaceArea) {
        static_cast<style::GeoJSONSource*>(map.getStyle().getSource("area"))->setGeoJSON(mapbox::geojson::parse(R"({
                "type": "Feature",
                "properties": {},
                "geometry": {"type": "Polygon", "coordinates": [[[-60, 11], [80, 11], [80, 75], [-60, 75], [-60, 11]]]}
            })"));
        stats = frontend.render(map).stats;
    }
    return stats;
}

// A terrain whose DEM source does not exist: the drape cover stays empty.
const char* terrainWithoutDEMStyle = R"STYLE({
  "version": 8,
  "sources": {},
  "terrain": {"source": "missing", "exaggeration": 1.0},
  "layers": []
})STYLE";

// A DEM to zoom 17 under a zoom 13 view: the DEM source covers the ground to the horizon, while
// the mesh keeps only the tiles nearest the camera, so the source holds tiles the mesh has no use for.
const char* deepDEMStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 17,
      "tileSize": 64
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": []
})STYLE";

} // namespace

// The tests read the counters of a still, not a second frame: a Static map ignores the request
// for a follow-up frame (Map::Impl::onDidFinishRenderingFrame acts on it only in Continuous).
// So the request itself, and the recount of deferred tiles each frame, are not tested.

// The load budget defers new tile builds in the modes that have one, and the count is of
// tiles: a second layer of the same tiles is refused the same builds and adds none.
TEST(TerrainLoadBudget, DeferredNewTileBuildsAreCountedOncePerTile) {
    const auto performance = TerrainLoadMode::Performance;
    const int deferred = loadBudgetStats({.mode = performance}).numNewTileBuildsDeferred;
    EXPECT_GT(deferred, 0) << "Performance deferred no new tile build";
    const int deferredTwoLayers = loadBudgetStats({.mode = performance, .fillLayers = 2}).numNewTileBuildsDeferred;
    EXPECT_EQ(deferred, deferredTwoLayers) << "a second layer of the same tiles changed the count of deferred tiles";
    EXPECT_EQ(0, loadBudgetStats({.mode = TerrainLoadMode::Quality}).numNewTileBuildsDeferred)
        << "Quality has no budget to defer by";
}

// Drape targets that already hold a texture are re-rendered only up to the budget when their
// content changes; the rest wait for a later frame and are counted.
TEST(TerrainLoadBudget, DeferredDrapeTargetsAreCounted) {
    const auto performance = TerrainLoadMode::Performance;
    EXPECT_GT(loadBudgetStats({.mode = performance, .replaceArea = true}).numDrapeTargetsDeferred, 0)
        << "Performance deferred no drape re-render after the area changed";
    EXPECT_EQ(0, loadBudgetStats({.mode = TerrainLoadMode::Quality, .replaceArea = true}).numDrapeTargetsDeferred)
        << "Quality has no budget to defer by";
}

// The retry for an empty drape cover is bounded, so a cover that never fills counts four
// follow-up frames and no more.
TEST(TerrainLoadBudget, AnEmptyDrapeCoverIsCountedUpToItsRetryBound) {
    TerrainCameraTest test;
    test.map.getStyle().loadJSON(terrainWithoutDEMStyle);
    gfx::RenderingStats stats;
    for (int i = 0; i < 6; ++i) {
        stats = test.frontend.render(test.map).stats;
    }
    EXPECT_EQ(4, stats.numTerrainCoverPendingFrames) << "after 6 stills of a terrain without a DEM source";
}

// A DEM texture is made, once, for a loaded tile related to the mesh tile set (a mesh tile, an
// ancestor or a descendant of one), and not for the others. The DEM source's cover is not capped
// where the mesh is, so a steep view loads DEM tiles the mesh has no use for.
TEST(TerrainDEMTextures, ATextureIsMadeOnlyForTilesRelatedToTheMeshTileSet) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string tile = makeFlatDEMTile();
    std::set<std::string> demTiles;
    fileSource->tileResponse = [&](const Resource& resource) {
        demTiles.insert(resource.url);
        Response res;
        res.data = std::make_shared<std::string>(tile);
        return res;
    };
    HeadlessFrontend frontend{{1200, 800}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.getStyle().loadJSON(deepDEMStyle);
    map.setTerrainLoadMode(TerrainLoadMode::Performance);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.jumpTo(CameraOptions().withCenter(LatLng{47.2692, 11.4041}).withZoom(13.0).withPitch(75.0));

    // Render until the DEM has stopped arriving and no frame makes a texture: the elevated cover
    // shifts as the DEM lands, so a pause of a frame or two does not mean it is in.
    constexpr int maxFrames = 200;
    constexpr int quietFrames = 10;
    gfx::RenderingStats stats;
    std::size_t requested = 0;
    int quiet = 0;
    for (int frame = 0; frame < maxFrames && quiet < quietFrames; ++frame) {
        loop.runOnce();
        const int before = stats.numDEMTexturesCreated;
        stats = frontend.render(map).stats;
        quiet = demTiles.size() == requested && stats.numDEMTexturesCreated == before ? quiet + 1 : 0;
        requested = demTiles.size();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    ASSERT_EQ(quiet, quietFrames) << "tiles or DEM textures were still arriving after " << maxFrames << " frames";
    const std::size_t cap = terrainLoadBudget(TerrainLoadMode::Performance).maxMeshTiles;
    ASSERT_GT(demTiles.size(), 2 * cap)
        << "the DEM source should hold more than twice the mesh limit, or the bound below proves nothing";

    // In this view the tiles related to the mesh are its own, a texture each. The bound is a margin,
    // not a rule: a mesh tile can have up to four loaded children, and a tile that leaves the related
    // set and returns is made twice. It stays well below the tiles the source holds, which the guard
    // above makes sure of.
    EXPECT_GT(stats.numDEMTexturesCreated, 0) << "the mesh tiles have no DEM texture";
    EXPECT_LE(stats.numDEMTexturesCreated, static_cast<int>(2 * cap))
        << "a texture was made for a tile unrelated to the mesh";
}

namespace {

// A red fill over a geojson source on a flat terrain, seen from straight above, on a white
// background: the pixel at the centre of the view shows whether the drape draws the fill there.
const char* drapedFillUpdateStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 6,
      "tileSize": 64
    },
    "area": {"type": "geojson", "data": {"type": "FeatureCollection", "features": []}}
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [
    {"id": "ground", "type": "background", "paint": {"background-color": "#fff"}},
    {"id": "area", "type": "fill", "source": "area", "paint": {"fill-color": "#f00"}}
  ]
})STYLE";

// Around the centre of the view, well beyond its edges
const char* areaGeoJSON = R"JSON({"type": "Polygon",
  "coordinates": [[[2, 42], [21, 42], [21, 52], [2, 52], [2, 42]]]})JSON";
// The same area with a hole at the centre of the view: new data on the same tiles
const char* areaWithHoleGeoJSON = R"JSON({"type": "Polygon",
  "coordinates": [[[2, 42], [21, 42], [21, 52], [2, 52], [2, 42]],
                  [[10.9, 46.9], [10.9, 47.6], [11.9, 47.6], [11.9, 46.9], [10.9, 46.9]]]})JSON";
const char* noFeaturesGeoJSON = R"JSON({"type": "FeatureCollection", "features": []})JSON";

struct DrapedFillTest {
    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(),
                                                                                  ClientOptions());
    std::string tile = makeFlatDEMTile();
    HeadlessFrontend frontend{{256, 256}, 1};
    MapAdapter map{frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize())};

    DrapedFillTest() {
        fileSource->tileResponse = [this](const Resource&) {
            Response res;
            res.data = std::make_shared<std::string>(tile);
            return res;
        };
        map.getStyle().loadJSON(drapedFillUpdateStyle);
        map.jumpTo(CameraOptions().withCenter(LatLng{47.2692, 11.4041}).withZoom(5.0));
    }

    void setArea(const char* geojson) {
        map.getStyle().getSource("area")->as<style::GeoJSONSource>()->setGeoJSON(mapbox::geojson::parse(geojson));
    }

    // Whether the fill is drawn at the centre of the view
    bool fillAtCentre() {
        const auto image = frontend.render(map).image;
        const uint8_t* px = image.data.get() + (image.size.height / 2 * image.size.width + image.size.width / 2) * 4;
        return px[0] > 200 && px[1] < 100 && px[2] < 100;
    }
};

} // namespace

// New data on the tiles the drape already shows - a geojson source updated with setGeoJSON -
// is drawn at once, not when the next zoom level changes the tiles.
TEST(TerrainDrape, NewDataOnTheSameTilesIsDrawn) {
    DrapedFillTest test;
    test.setArea(areaGeoJSON);
    ASSERT_TRUE(test.fillAtCentre());

    test.setArea(areaWithHoleGeoJSON);
    EXPECT_FALSE(test.fillAtCentre());
}

// A source emptied of its features clears the drape instead of leaving the old fill on the
// terrain.
TEST(TerrainDrape, AnEmptiedSourceIsCleared) {
    DrapedFillTest test;
    test.setArea(areaGeoJSON);
    ASSERT_TRUE(test.fillAtCentre());

    test.setArea(noFeaturesGeoJSON);
    EXPECT_FALSE(test.fillAtCentre());
}
