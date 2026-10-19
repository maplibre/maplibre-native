#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/bound_options.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/math/angles.hpp>
#include <mln/style/style.hpp>
#include <mln/style/terrain.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <cmath>

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

} // namespace

// The centre rides the terrain instead of sea level, so pitching over high ground does not
// leave the camera inside the hillside.
TEST(TerrainCamera, CentreRidesTheTerrainSurface) {
    TerrainCameraTest test;
    EXPECT_TRUE(test.map.getCenterClampedToGround());

    const double settled = test.renderAndGetAltitude(8);
    EXPECT_NEAR(settled, plateauMeters, 5.0);
}

// The historical worry about a terrain-anchored centre was that raising it re-samples a new
// elevation and runs away. Raising the centre moves the orbit plane, not the centre's lng/lat,
// so it must settle - and the centre must not drift while it does.
TEST(TerrainCamera, CentreElevationSettlesWithoutDrift) {
    TerrainCameraTest test;

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
