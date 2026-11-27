#include <mln/test/crs.hpp>
#include <mln/test/util.hpp>

#include <mln/map/transform.hpp>
#include <mln/map/world_coordinate_helper.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/crs.hpp>
#include <mln/util/tile_cover.hpp>

#include <algorithm>

using namespace mln;

namespace {

void useProjection(Transform& transform, const std::string& name) {
    transform.setRegisteredProjection(getRegisteredProjection(name));
}

} // namespace

TEST(TransformInTheIdentityProjection, PlacesTheCenterInItsSquare) {
    Transform transform;
    transform.resize({512, 512});
    useProjection(transform, "identity");
    // Tile 1/1/0 of the identity spans lng 0..90 and lat 0..90, and fills the view.
    transform.jumpTo(CameraOptions().withCenter(LatLng{45, 45}).withZoom(1));

    EXPECT_EQ(LatLng(45, 45), transform.getLatLng());
    EXPECT_EQ(ScreenCoordinate(256, 256), transform.latLngToScreenCoordinate({45, 45}));
    const LatLng topLeft = transform.screenCoordinateToLatLng({0, 0});
    EXPECT_NEAR(90, topLeft.latitude(), 1e-6);
    EXPECT_NEAR(0, topLeft.longitude(), 1e-6);
    const LatLng bottomRight = transform.screenCoordinateToLatLng({512, 512});
    EXPECT_NEAR(0, bottomRight.latitude(), 1e-6);
    EXPECT_NEAR(90, bottomRight.longitude(), 1e-6);
}

TEST(TransformInTheIdentityProjection, StopsTheCenterAtTheEdgeOfItsSquare) {
    Transform transform;
    transform.resize({512, 512});
    useProjection(transform, "identity");
    transform.jumpTo(CameraOptions().withCenter(LatLng{0, 120}).withZoom(2));

    EXPECT_EQ(LatLng(0, 90), transform.getLatLng());
}

TEST(TransformInTheIdentityProjection, LeavesTheZoomToTheMinimumZoom) {
    Transform transform;
    transform.resize({1024, 1024});
    useProjection(transform, "identity");
    transform.jumpTo(CameraOptions().withCenter(LatLng{0, 0}).withZoom(0));

    // The square is half the height of the view; Mercator would zoom in until it filled it.
    EXPECT_EQ(0, transform.getZoom());
}

TEST(TransformInTheIdentityProjection, KeepsALatitudePastTheMercatorLimit) {
    Transform transform;
    transform.resize({512, 512});
    useProjection(transform, "identity");
    transform.jumpTo(CameraOptions().withCenter(LatLng{88, 10}).withZoom(4));

    EXPECT_DOUBLE_EQ(88, transform.getLatLng().latitude());
    EXPECT_DOUBLE_EQ(10, transform.getLatLng().longitude());
}

TEST(TransformInTheIdentityProjection, KeepsTheCenterAcrossAProjectionChange) {
    Transform transform;
    transform.resize({512, 512});
    transform.jumpTo(CameraOptions().withCenter(LatLng{30, 40}).withZoom(3));
    useProjection(transform, "identity");

    EXPECT_NEAR(30, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(40, transform.getLatLng().longitude(), 1e-9);

    transform.setRegisteredProjection({});
    EXPECT_NEAR(30, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(40, transform.getLatLng().longitude(), 1e-9);
}

TEST(TransformInTheIdentityProjection, KeepsTheCenterInsideMaxBounds) {
    Transform transform;
    transform.resize({512, 512});
    useProjection(transform, "identity");
    transform.setLatLngBounds(LatLngBounds::hull({0, 0}, {45, 45}));
    transform.jumpTo(CameraOptions().withCenter(LatLng{80, -60}).withZoom(3));

    EXPECT_NEAR(45, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(0, transform.getLatLng().longitude(), 1e-9);
}

TEST(TransformInTheIdentityProjection, RoundTripsTheFreeCamera) {
    Transform transform;
    transform.resize({512, 512});
    useProjection(transform, "identity");
    transform.jumpTo(CameraOptions().withCenter(LatLng{20, -30}).withZoom(5).withPitch(40).withBearing(25));

    const auto camera = transform.getFreeCameraOptions();
    Transform roundTrip;
    roundTrip.resize({512, 512});
    useProjection(roundTrip, "identity");
    roundTrip.setFreeCameraOptions(camera);

    EXPECT_NEAR(20, roundTrip.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(-30, roundTrip.getLatLng().longitude(), 1e-9);
    EXPECT_NEAR(5, roundTrip.getZoom(), 1e-9);
}

TEST(TransformInTheIdentityProjection, CoversItsSquareWithoutWorldCopies) {
    Transform transform;
    transform.resize({1024, 1024});
    useProjection(transform, "identity");
    transform.jumpTo(CameraOptions().withCenter(LatLng{0, 0}).withZoom(0));

    // The view is twice as wide as the square: Mercator would add the world copies on both sides.
    EXPECT_EQ((std::vector<OverscaledTileID>{{0, 0, 0}}),
              util::tileCover({.transformState = transform.getState()}, 0, {0, 22}));
}

TEST(TransformInTheIdentityProjection, CoversTheTileThatFillsTheView) {
    Transform transform;
    transform.resize({512, 512});
    useProjection(transform, "identity");
    transform.jumpTo(CameraOptions().withCenter(LatLng{45, 45}).withZoom(1));

    EXPECT_EQ((std::vector<OverscaledTileID>{{1, 1, 0}}),
              util::tileCover({.transformState = transform.getState()}, 1, {0, 22}));
}

TEST(TransformInARegisteredProjection, FliesPastTheAntimeridianWithoutWrapping) {
    const test::ScopedProjection registration(test::wideIdentity("test:wide"));
    Transform transform;
    transform.resize({512, 512});
    useProjection(transform, "test:wide");
    transform.jumpTo(CameraOptions().withCenter(LatLng{0, 170}).withZoom(4));

    transform.flyTo(CameraOptions().withCenter(LatLng{0, 200}), AnimationOptions(Seconds(1)));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(500));
    // Halfway, eastward: a wrapping world would have taken the target as -160 and gone the other way around.
    EXPECT_GT(transform.getLatLng().longitude(), 170);
    transform.updateTransitions(transform.getTransitionStart() + transform.getTransitionDuration());
    EXPECT_NEAR(200, transform.getLatLng().longitude(), 1e-9);
}

TEST(TransformInARegisteredProjection, FallsBackToMercatorForAnUnknownName) {
    Transform transform;
    transform.resize({512, 512});
    useProjection(transform, "test:unknown");
    transform.jumpTo(CameraOptions().withCenter(LatLng{60, 10}).withZoom(3));

    EXPECT_TRUE(transform.getState().getProjection().worldCoordinateHelper().wraps());
    EXPECT_NEAR(60, transform.getLatLng().latitude(), 1e-9);
}

TEST(TransformInARegisteredProjection, KeepsACenterInsideTheMaxBoundsOfAPolarCap) {
    const test::ScopedProjection registration(test::northPolarStereographic("test:polar-cap"));
    Transform transform;
    transform.resize({512, 512});
    useProjection(transform, "test:polar-cap");
    transform.setLatLngBounds(LatLngBounds::hull({60, -180}, {90, 180}));
    transform.jumpTo(CameraOptions().withCenter(LatLng{70, 45}).withZoom(3));

    // The corners of the cap are the pole and a point on its edge; the cap's world box keeps the center where it is.
    EXPECT_NEAR(70, transform.getLatLng().latitude(), 1e-6);
    EXPECT_NEAR(45, transform.getLatLng().longitude(), 1e-6);
}

TEST(TransformInARegisteredProjection, CoversTheSameTilesWhenTheCenterCrossesTheAntimeridian) {
    const test::ScopedProjection registration(test::polarStereographic("test:polar-antimeridian"));
    Transform transform;
    transform.resize({512, 512});
    useProjection(transform, "test:polar-antimeridian");
    const auto coverAt = [&](const LatLng& center) {
        transform.jumpTo(CameraOptions().withCenter(center).withZoom(2));
        auto tiles = util::tileCover({.transformState = transform.getState()}, 2, {0, 22});
        std::sort(tiles.begin(), tiles.end());
        return tiles;
    };

    // The world does not wrap, so 179.9 E and 179.9 W at 72 N are a few hundred meters apart and see the same four
    // tiles, which GL JS checks by not loading any tile again.
    const auto east = coverAt({72, 179.9});
    EXPECT_EQ(4u, east.size());
    EXPECT_EQ(east, coverAt({72, -179.9}));
}

TEST(TransformInARegisteredProjection, KeepsTheViewInsideThePolarCapInScreenMode) {
    const test::ScopedProjection registration(test::northPolarStereographic("test:polar-screen"));
    Transform transform;
    transform.resize({512, 512});
    useProjection(transform, "test:polar-screen");
    transform.setLatLngBounds(LatLngBounds::hull({60, -180}, {90, 180}));
    transform.setConstrainMode(ConstrainMode::Screen);
    transform.easeTo(CameraOptions().withCenter(LatLng{61, 0}).withZoom(3));

    // At 61N the bottom of the view would reach past the cap's world box, so the center moves toward the pole until the
    // bottom of the view is on the box's edge.
    EXPECT_GT(transform.getLatLng().latitude(), 61);
    EXPECT_NEAR(0, transform.getLatLng().longitude(), 1e-6);
    const auto& helper = transform.getState().getProjection().worldCoordinateHelper();
    const auto box = latLngBoxToWorldBox(helper, LatLngBounds::hull({60, -180}, {90, 180}));
    EXPECT_NEAR(box[3], helper.worldFromLatLng(transform.screenCoordinateToLatLng({256, 512})).y, 1e-6);
}
