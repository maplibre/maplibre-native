#include <mln/test/util.hpp>

#include <gmock/gmock.h>
#include <cmath>
#include <mln/map/map_impl.hpp>
#include <mln/map/mercator_projection.hpp>
#include <mln/map/tile_projector.hpp>
#include <mln/map/transform.hpp>
#include <mln/map/vertical_perspective_projection.hpp>
#include <mln/math/angles.hpp>
#include <mln/util/geo.hpp>
#include <mln/util/projection.hpp>
#include <mln/util/quaternion.hpp>

#include <numbers>

using namespace std::numbers;
using namespace mln;

TEST(Transform, InvalidZoom) {
    Transform transform;
    transform.resize({1, 1});

    ASSERT_DOUBLE_EQ(0, transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform.getLatLng().longitude());
    ASSERT_DOUBLE_EQ(0, transform.getZoom());

    transform.jumpTo(CameraOptions().withZoom(1.0));

    ASSERT_DOUBLE_EQ(1, transform.getZoom());

    const double invalid = NAN;

    transform.jumpTo(CameraOptions().withZoom(invalid));

    ASSERT_DOUBLE_EQ(0, transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform.getLatLng().longitude());
    ASSERT_DOUBLE_EQ(1, transform.getZoom());

    transform.jumpTo(CameraOptions().withCenter(LatLng()).withZoom(invalid));

    ASSERT_DOUBLE_EQ(0, transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform.getLatLng().longitude());
    ASSERT_DOUBLE_EQ(1, transform.getZoom());

    transform.jumpTo(CameraOptions().withZoom(transform.getState().getMaxZoom() + 0.1));
    ASSERT_DOUBLE_EQ(transform.getZoom(), transform.getState().getMaxZoom());

    // Executing flyTo with an empty size causes frameZoom to be NaN.
    transform.flyTo(CameraOptions()
                        .withCenter(LatLng{util::LATITUDE_MAX, util::LONGITUDE_MAX})
                        .withZoom(transform.getState().getMaxZoom()));
    transform.updateTransitions(transform.getTransitionStart() + transform.getTransitionDuration());
    ASSERT_DOUBLE_EQ(transform.getZoom(), transform.getState().getMaxZoom());

    // Executing flyTo with maximum zoom level to the same zoom level causes
    // frameZoom to be bigger than maximum zoom.
    transform.resize(Size{100, 100});
    transform.flyTo(CameraOptions()
                        .withCenter(LatLng{util::LATITUDE_MAX, util::LONGITUDE_MAX})
                        .withZoom(transform.getState().getMaxZoom()));
    transform.updateTransitions(transform.getTransitionStart() + transform.getTransitionDuration());

    ASSERT_TRUE(transform.getState().valid());
    ASSERT_DOUBLE_EQ(transform.getState().getMaxZoom(), transform.getZoom());
}

TEST(Transform, InvalidBearing) {
    Transform transform;
    transform.resize({1, 1});

    ASSERT_DOUBLE_EQ(0, transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform.getLatLng().longitude());
    ASSERT_DOUBLE_EQ(0, transform.getZoom());

    transform.jumpTo(CameraOptions().withZoom(1.0).withBearing(2.0));
    ASSERT_DOUBLE_EQ(0, transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform.getLatLng().longitude());
    ASSERT_DOUBLE_EQ(1, transform.getZoom());
    ASSERT_DOUBLE_EQ(util::deg2rad(-2.0), transform.getBearing());

    const double invalid = NAN;

    transform.jumpTo(CameraOptions().withBearing(invalid));
    ASSERT_DOUBLE_EQ(0, transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform.getLatLng().longitude());
    ASSERT_DOUBLE_EQ(1, transform.getZoom());
    ASSERT_DOUBLE_EQ(util::deg2rad(-2.0), transform.getBearing());
}

TEST(Transform, IntegerZoom) {
    Transform transform;
    transform.resize({1, 1});

    auto checkIntegerZoom = [&transform](uint8_t zoomInt, double zoom) {
        transform.jumpTo(CameraOptions().withZoom(zoom));
        ASSERT_NEAR(transform.getZoom(), zoom, 1e-8);
        ASSERT_EQ(transform.getState().getIntegerZoom(), zoomInt);
        ASSERT_NEAR(transform.getState().getZoomFraction(), zoom - zoomInt, 1e-8);
    };

    for (uint8_t zoomInt = 0; zoomInt < 20; ++zoomInt) {
        for (uint32_t percent = 0; percent < 100; ++percent) {
            double zoom = zoomInt + (0.01 * percent);
            checkIntegerZoom(zoomInt, zoom);
        }
    }

    // Special case zoom 20.
    checkIntegerZoom(20, 20.0);
}

TEST(Transform, PerspectiveProjection) {
    LatLng loc;

    Transform transform;
    transform.resize({1000, 1000});

    // 0.9 rad ~ 51.56620156 deg
    transform.jumpTo(CameraOptions().withCenter(LatLng{38.0, -77.0}).withZoom(10.0).withPitch(51.56620156));

    // expected values are from maplibre-gl-js

    loc = transform.getLatLng();
    ASSERT_DOUBLE_EQ(-77, loc.longitude());
    ASSERT_DOUBLE_EQ(38, loc.latitude());

    loc = transform.getState().screenCoordinateToLatLng({0, 1000});
    ASSERT_NEAR(-77.59198961199148, loc.longitude(), 1e-6);
    ASSERT_NEAR(38.74661326302018, loc.latitude(), 1e-6);

    loc = transform.getState().screenCoordinateToLatLng({1000, 0});
    ASSERT_NEAR(-76.75823239205641, loc.longitude(), 1e-6);
    ASSERT_NEAR(37.692872969426375, loc.latitude(), 1e-6);

    ScreenCoordinate point = transform.getState().latLngToScreenCoordinate({38.74661326302018, -77.59198961199148});
    ASSERT_NEAR(point.x, 0.0, 1e-5);
    ASSERT_NEAR(point.y, 1000.0, 1e-4);

    point = transform.getState().latLngToScreenCoordinate({37.692872969426375, -76.75823239205641});
    ASSERT_NEAR(point.x, 1000.0, 1e-5);
    ASSERT_NEAR(point.y, 0.0, 1e-4);

    mln::vec4 p;
    point = transform.getState().latLngToScreenCoordinate({37.692872969426375, -76.75823239205641}, p);
    ASSERT_NEAR(point.x, 1000.0, 1e-5);
    ASSERT_NEAR(point.y, 0.0, 1e-4);
    ASSERT_GT(p[3], 0.0);

    transform.jumpTo(CameraOptions().withCenter(LatLng{38.0, -77.0}).withZoom(18.0).withPitch(51.56620156));
    point = transform.getState().latLngToScreenCoordinate({7.692872969426375, -76.75823239205641}, p);
    ASSERT_LT(p[3], 0.0);
}

TEST(Transform, UnwrappedLatLng) {
    Transform transform;
    transform.resize({1000, 1000});

    // 0.9 rad ~ 51.56620156 deg
    transform.jumpTo(CameraOptions().withCenter(LatLng{38.0, -77.0}).withZoom(10.0).withPitch(51.56620156));

    const TransformState& state = transform.getState();

    LatLng fromGetLatLng = state.getLatLng();
    ASSERT_DOUBLE_EQ(fromGetLatLng.latitude(), 38.0);
    ASSERT_DOUBLE_EQ(fromGetLatLng.longitude(), -77.0);

    LatLng fromScreenCoordinate = state.screenCoordinateToLatLng({500, 500});
    ASSERT_NEAR(fromScreenCoordinate.latitude(), 38.0, 1e-8);
    ASSERT_NEAR(fromScreenCoordinate.longitude(), -77.0, 1e-8);

    LatLng wrappedRightwards = state.screenCoordinateToLatLng(state.latLngToScreenCoordinate({38, 283}));
    ASSERT_NEAR(wrappedRightwards.latitude(), 38.0, 1e-8);
    ASSERT_NEAR(wrappedRightwards.longitude(), 283.0, 1e-8);
    wrappedRightwards.wrap();
    ASSERT_NEAR(wrappedRightwards.longitude(), -77.0, 1e-8);

    LatLng wrappedLeftwards = state.screenCoordinateToLatLng(state.latLngToScreenCoordinate({38, -437}));
    ASSERT_DOUBLE_EQ(wrappedLeftwards.latitude(), wrappedRightwards.latitude());
    ASSERT_NEAR(wrappedLeftwards.longitude(), -437.0, 1e-8);
    wrappedLeftwards.wrap();
    ASSERT_NEAR(wrappedLeftwards.longitude(), -77.0, 1e-8);
}

TEST(Transform, ConstrainHeightOnly) {
    Transform transform(TransformObserver::nullObserver(), ConstrainMode::HeightOnly);
    transform.resize({2, 2});

    transform.jumpTo(CameraOptions().withCenter(LatLngBounds::world().southwest()).withZoom(util::MAX_ZOOM));
    ASSERT_NEAR(-util::LATITUDE_MAX, transform.getLatLng().latitude(), 1e-7);
    ASSERT_NEAR(-util::LONGITUDE_MAX, transform.getLatLng().longitude(), 1e-7);

    transform.jumpTo(CameraOptions().withCenter(LatLngBounds::world().northeast()));
    ASSERT_NEAR(util::LATITUDE_MAX, transform.getLatLng().latitude(), 1e-7);
    ASSERT_NEAR(-util::LONGITUDE_MAX, transform.getLatLng().longitude(), 1e-7);
}

TEST(Transform, ConstrainWidthAndHeight) {
    Transform transform(TransformObserver::nullObserver(), ConstrainMode::WidthAndHeight);
    transform.resize({2, 2});

    transform.jumpTo(CameraOptions().withCenter(LatLngBounds::world().southwest()).withZoom(util::MAX_ZOOM));
    ASSERT_NEAR(-util::LATITUDE_MAX, transform.getLatLng().latitude(), 1e-7);
    ASSERT_NEAR(-util::LONGITUDE_MAX, transform.getLatLng().longitude(), 1e-6);

    transform.jumpTo(CameraOptions().withCenter(LatLngBounds::world().northeast()));
    ASSERT_NEAR(util::LATITUDE_MAX, transform.getLatLng().latitude(), 1e-7);
    ASSERT_NEAR(-util::LONGITUDE_MAX, transform.getLatLng().longitude(), 1e-6);
}

TEST(Transform, Anchor) {
    Transform transform;
    transform.resize({1000, 1000});

    const LatLng latLng{10, -100};
    const ScreenCoordinate anchorPoint = {150, 150};

    transform.jumpTo(CameraOptions().withCenter(latLng).withZoom(10.0));
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform.getLatLng().longitude());
    ASSERT_DOUBLE_EQ(10, transform.getZoom());
    ASSERT_DOUBLE_EQ(0, transform.getBearing());

    const LatLng anchorLatLng = transform.getState().screenCoordinateToLatLng(anchorPoint);
    ASSERT_NE(latLng.latitude(), anchorLatLng.latitude());
    ASSERT_NE(latLng.longitude(), anchorLatLng.longitude());

    transform.jumpTo(CameraOptions().withCenter(latLng).withZoom(3.0));
    ASSERT_DOUBLE_EQ(3, transform.getZoom());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withZoom(3.5));
    ASSERT_DOUBLE_EQ(3.5, transform.getZoom());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withZoom(5.5).withAnchor(anchorPoint));
    ASSERT_DOUBLE_EQ(5.5, transform.getZoom());
    ASSERT_NE(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_NE(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withCenter(latLng).withZoom(3.0));
    ASSERT_DOUBLE_EQ(3, transform.getZoom());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withZoom(5.0));
    ASSERT_DOUBLE_EQ(5, transform.getZoom());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withZoom(7.0).withAnchor(anchorPoint));
    ASSERT_DOUBLE_EQ(7, transform.getZoom());
    ASSERT_NE(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_NE(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withCenter(latLng).withZoom(2.0));
    ASSERT_DOUBLE_EQ(2, transform.getZoom());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withZoom(4.0));
    ASSERT_DOUBLE_EQ(4, transform.getZoom());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withZoom(8.0).withAnchor(anchorPoint));
    ASSERT_DOUBLE_EQ(8, transform.getZoom());
    ASSERT_NE(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_NE(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withCenter(latLng).withZoom(10.0).withBearing(-45.0));
    ASSERT_DOUBLE_EQ(pi / 4, transform.getBearing());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withBearing(0.0));
    ASSERT_DOUBLE_EQ(0, transform.getBearing());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withBearing(45.0).withAnchor(anchorPoint));
    ASSERT_DOUBLE_EQ(util::deg2rad(-45.0), transform.getBearing());

    // Anchor coordinates are imprecise because we are converting from an integer pixel.
    ASSERT_NEAR(anchorLatLng.latitude(), transform.getLatLng().latitude(), 0.5);
    ASSERT_NEAR(anchorLatLng.longitude(), transform.getLatLng().longitude(), 0.5);

    transform.jumpTo(CameraOptions().withCenter(latLng).withZoom(10.0).withPitch(10.0));
    ASSERT_DOUBLE_EQ(util::deg2rad(10.0), transform.getPitch());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withPitch(15.0));
    ASSERT_DOUBLE_EQ(util::deg2rad(15.0), transform.getPitch());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform.getLatLng().longitude());

    transform.jumpTo(CameraOptions().withPitch(20.0).withAnchor(anchorPoint));
    ASSERT_DOUBLE_EQ(util::deg2rad(20.0), transform.getPitch());

    // Anchor coordinates are imprecise because we are converting from an integer pixel.
    ASSERT_NEAR(anchorLatLng.latitude(), transform.getLatLng().latitude(), 0.5);
    ASSERT_NEAR(anchorLatLng.longitude(), transform.getLatLng().longitude(), 0.5);
}

TEST(Transform, Padding) {
    Transform transform;
    transform.resize({1000, 1000});

    ASSERT_DOUBLE_EQ(0, transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform.getLatLng().longitude());
    CameraOptions nonPaddedCameraOptions = CameraOptions().withCenter(LatLng{10, -100}).withZoom(10.0);
    transform.jumpTo(nonPaddedCameraOptions);

    const LatLng trueCenter = transform.getLatLng();
    ASSERT_DOUBLE_EQ(10, trueCenter.latitude());
    ASSERT_DOUBLE_EQ(-100, trueCenter.longitude());
    ASSERT_DOUBLE_EQ(10, transform.getZoom());

    const LatLng screenCenter = transform.screenCoordinateToLatLng({
        1000.0 / 2.0,
        1000.0 / 2.0,
    });
    const LatLng upperHalfCenter = transform.screenCoordinateToLatLng({
        1000.0 / 2.0,
        1000.0 * 0.25,
    });

    EdgeInsets padding(1000.0 / 2.0, 0, 0, 0);
    // CameraOption center and zoom don't change when padding changes: center of
    // viewport remains the same as padding defines viwport center offset in rendering.
    CameraOptions paddedOptions = CameraOptions().withPadding(padding);
    transform.jumpTo(paddedOptions);
    const LatLng theSameCenter = transform.getLatLng();
    ASSERT_DOUBLE_EQ(trueCenter.latitude(), theSameCenter.latitude());
    ASSERT_DOUBLE_EQ(trueCenter.longitude(), theSameCenter.longitude());

    // However, LatLng is now at the center of lower half - verify conversion
    // from screen coordinate to LatLng.
    const LatLng paddedLowerHalfScreenCenter = transform.screenCoordinateToLatLng({
        1000.0 / 2.0,
        1000.0 * 0.75,
    });
    ASSERT_NEAR(screenCenter.latitude(), paddedLowerHalfScreenCenter.latitude(), 1e-10);
    ASSERT_NEAR(screenCenter.longitude(), paddedLowerHalfScreenCenter.longitude(), 1e-10);

    // LatLng previously in upper half center, should now be under screen center.
    const LatLng paddedScreenCenter = transform.screenCoordinateToLatLng({
        1000.0 / 2.0,
        1000.0 / 2.0,
    });
    ASSERT_NEAR(upperHalfCenter.latitude(), paddedScreenCenter.latitude(), 1e-10);
    ASSERT_NEAR(upperHalfCenter.longitude(), paddedScreenCenter.longitude(), 1e-10);
}

TEST(Transform, MoveBy) {
    Transform transform;
    transform.resize({1000, 1000});

    transform.jumpTo(CameraOptions().withCenter(LatLng()).withZoom(10.0));

    LatLng trueCenter = transform.getLatLng();
    ASSERT_DOUBLE_EQ(0, trueCenter.latitude());
    ASSERT_DOUBLE_EQ(0, trueCenter.longitude());
    ASSERT_DOUBLE_EQ(10, transform.getZoom());

    for (uint8_t x = 0; x < 20; ++x) {
        bool odd = x % 2;
        bool forward = x % 10;

        LatLng coordinate = transform.screenCoordinateToLatLng({odd ? 400. : 600., forward ? 400. : 600});
        transform.moveBy({odd ? 100. : -100., forward ? 100. : -100});

        trueCenter = transform.getLatLng();
        ASSERT_NEAR(coordinate.latitude(), trueCenter.latitude(), 1e-8);
        ASSERT_NEAR(coordinate.longitude(), trueCenter.longitude(), 1e-8);
    }

    // We have ~1.1 precision loss for each coordinate for 20 rounds of moveBy.
    ASSERT_NEAR(0.0, trueCenter.latitude(), 1.1);
    ASSERT_NEAR(0.0, trueCenter.longitude(), 1.1);
}

// On a pitched map a large (fling-sized) pan offset must be reduced so it can
// never reach the horizon -- otherwise the unprojected pan direction flips and the
// pan reverses (or lands in the tile-less void). See maplibre-native#3105.
TEST(Transform, MoveByPitchedDoesNotCrossHorizon) {
    Transform transform;
    transform.resize({1000, 1000});
    const auto start = CameraOptions().withCenter(LatLng{0.0, 0.0}).withZoom(10.0).withPitch(60.0);

    // Panning "up" the screen on a north-up pitched map moves the center north.
    transform.jumpTo(start);
    transform.moveBy({0.0, 300.0});
    const double moderateLat = transform.getLatLng().latitude();

    // A fling-sized offset toward the horizon must keep moving the center the same
    // way (north) -- before the fix it reversed south and stay bounded (clamped
    // short of the horizon).
    transform.jumpTo(start);
    transform.moveBy({0.0, 100000.0});
    const double flingLat = transform.getLatLng().latitude();

    EXPECT_GT(moderateLat, 0.0);             // moved north
    EXPECT_GT(flingLat, 0.0);                // still north; not reversed
    EXPECT_GE(flingLat, moderateLat);        // fling goes at least as far as the moderate pan
    EXPECT_LT(flingLat, util::LATITUDE_MAX); // but is bounded, not off near the horizon
}

TEST(Transform, Antimeridian) {
    Transform transform;
    transform.resize({1000, 1000});

    transform.jumpTo(CameraOptions().withCenter(LatLng()).withZoom(1.0));

    // San Francisco
    const LatLng coordinateSanFrancisco{37.7833, -122.4167};
    ScreenCoordinate pixelSF = transform.latLngToScreenCoordinate(coordinateSanFrancisco);
    ASSERT_DOUBLE_EQ(151.79249437176432, pixelSF.x);
    ASSERT_DOUBLE_EQ(383.76720782527661, pixelSF.y);

    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, -181.0}));

    ScreenCoordinate pixelSFLongest = transform.latLngToScreenCoordinate(coordinateSanFrancisco);
    ASSERT_DOUBLE_EQ(-357.36306616412816, pixelSFLongest.x);
    ASSERT_DOUBLE_EQ(pixelSF.y, pixelSFLongest.y);
    LatLng unwrappedSF = coordinateSanFrancisco.wrapped();
    unwrappedSF.unwrapForShortestPath(transform.getLatLng());

    ScreenCoordinate pixelSFShortest = transform.latLngToScreenCoordinate(unwrappedSF);
    ASSERT_DOUBLE_EQ(666.63694385219173, pixelSFShortest.x);
    ASSERT_DOUBLE_EQ(pixelSF.y, pixelSFShortest.y);

    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 179.0}));
    pixelSFShortest = transform.latLngToScreenCoordinate(coordinateSanFrancisco);
    ASSERT_DOUBLE_EQ(pixelSFLongest.x, pixelSFShortest.x);
    ASSERT_DOUBLE_EQ(pixelSFLongest.y, pixelSFShortest.y);

    // Waikiri
    const LatLng coordinateWaikiri{-16.9310, 179.9787};
    transform.jumpTo(CameraOptions().withCenter(coordinateWaikiri).withZoom(10.0));
    ScreenCoordinate pixelWaikiri = transform.latLngToScreenCoordinate(coordinateWaikiri);
    ASSERT_DOUBLE_EQ(500, pixelWaikiri.x);
    ASSERT_DOUBLE_EQ(500, pixelWaikiri.y);

    transform.jumpTo(CameraOptions().withCenter(LatLng{coordinateWaikiri.latitude(), 180.0213}));
    ScreenCoordinate pixelWaikiriLongest = transform.latLngToScreenCoordinate(coordinateWaikiri);
    ASSERT_DOUBLE_EQ(524725.96438108233, pixelWaikiriLongest.x);
    ASSERT_DOUBLE_EQ(pixelWaikiri.y, pixelWaikiriLongest.y);

    LatLng unwrappedWaikiri = coordinateWaikiri.wrapped();
    unwrappedWaikiri.unwrapForShortestPath(transform.getLatLng());
    ScreenCoordinate pixelWaikiriShortest = transform.latLngToScreenCoordinate(unwrappedWaikiri);
    ASSERT_DOUBLE_EQ(437.95925272648344, pixelWaikiriShortest.x);
    ASSERT_DOUBLE_EQ(pixelWaikiri.y, pixelWaikiriShortest.y);

    LatLng coordinateFromPixel = transform.screenCoordinateToLatLng(pixelWaikiriLongest);
    ASSERT_NEAR(coordinateWaikiri.latitude(), coordinateFromPixel.latitude(), 1e-4);
    ASSERT_NEAR(coordinateWaikiri.longitude(), coordinateFromPixel.longitude(), 1e-4);

    transform.jumpTo(CameraOptions().withCenter(LatLng{coordinateWaikiri.latitude(), 180.0213}));
    pixelWaikiriShortest = transform.latLngToScreenCoordinate(coordinateWaikiri);
    ASSERT_DOUBLE_EQ(pixelWaikiriLongest.x, pixelWaikiriShortest.x);
    ASSERT_DOUBLE_EQ(pixelWaikiriLongest.y, pixelWaikiriShortest.y);

    coordinateFromPixel = transform.screenCoordinateToLatLng(pixelWaikiriShortest);
    ASSERT_NEAR(coordinateWaikiri.latitude(), coordinateFromPixel.latitude(), 1e-4);
    ASSERT_NEAR(coordinateWaikiri.longitude(), coordinateFromPixel.longitude(), 1e-4);
}

TEST(Transform, Camera) {
    Transform transform;
    transform.resize({1000, 1000});

    const LatLng latLng1{45, 135};
    CameraOptions cameraOptions1 = CameraOptions().withCenter(latLng1).withZoom(20.0);
    transform.jumpTo(cameraOptions1);
    ASSERT_DOUBLE_EQ(latLng1.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng1.longitude(), transform.getLatLng().longitude());
    ASSERT_DOUBLE_EQ(20, transform.getZoom());

    const LatLng latLng2{-45, -135};
    CameraOptions cameraOptions2 = CameraOptions().withCenter(latLng2).withZoom(10.0);
    transform.jumpTo(cameraOptions2);
    ASSERT_DOUBLE_EQ(latLng2.latitude(), transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng2.longitude(), transform.getLatLng().longitude());
    ASSERT_DOUBLE_EQ(10, transform.getZoom());

    AnimationOptions easeOptions(Seconds(1));
    easeOptions.transitionFrameFn = [&](double t) {
        ASSERT_TRUE(t >= 0 && t <= 1);
        ASSERT_GE(latLng1.latitude(), transform.getLatLng().latitude());
        ASSERT_LE(latLng1.longitude(), transform.getLatLng().longitude());
    };
    easeOptions.transitionFinishFn = [&]() {
        ASSERT_DOUBLE_EQ(latLng1.latitude(), transform.getLatLng().latitude());
        ASSERT_DOUBLE_EQ(latLng1.longitude(), transform.getLatLng().longitude());
        ASSERT_DOUBLE_EQ(20, transform.getZoom());
    };

    transform.easeTo(cameraOptions1, easeOptions);
    ASSERT_TRUE(transform.inTransition());
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(250));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(500));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(750));
    transform.updateTransitions(transform.getTransitionStart() + transform.getTransitionDuration());
    ASSERT_FALSE(transform.inTransition());

    AnimationOptions flyOptions(Seconds(1));
    flyOptions.transitionFrameFn = [&](double t) {
        ASSERT_TRUE(t >= 0 && t <= 1);
        ASSERT_LE(latLng2.latitude(), transform.getLatLng().latitude());
        ASSERT_GE(latLng2.longitude(), transform.getLatLng(LatLng::Unwrapped).longitude());
    };
    flyOptions.transitionFinishFn = [&]() {
        // XXX Fix precision loss in flyTo:
        // https://github.com/mapbox/mapbox-gl-native/issues/4298
        ASSERT_DOUBLE_EQ(latLng2.latitude(), transform.getLatLng().latitude());
        ASSERT_DOUBLE_EQ(latLng2.longitude(), transform.getLatLng().longitude());
        ASSERT_NEAR(10.0, transform.getZoom(), 1e-5);
    };

    transform.flyTo(cameraOptions2, flyOptions);
    ASSERT_TRUE(transform.inTransition());
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(250));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(500));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(750));
    transform.updateTransitions(transform.getTransitionStart() + transform.getTransitionDuration());
    ASSERT_FALSE(transform.inTransition());

    // Anchor and center points are mutually exclusive.
    CameraOptions camera;
    camera.center = LatLng{0, 0};
    camera.anchor = ScreenCoordinate{0, 0}; // top-left
    camera.zoom = transform.getState().getMaxZoom();
    transform.easeTo(camera, AnimationOptions(Seconds(1)));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(250));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(500));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(750));
    transform.updateTransitions(transform.getTransitionStart() + transform.getTransitionDuration());
    ASSERT_DOUBLE_EQ(transform.getLatLng().latitude(), 0);
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), 0);
}

TEST(Transform, ProjectionMode) {
    Transform transform;

    transform.setProjectionMode(ProjectionMode().withAxonometric(true).withXSkew(1.0).withYSkew(0.0));
    auto options = transform.getProjectionMode();

    EXPECT_TRUE(*options.axonometric);
    EXPECT_EQ(*options.xSkew, 1.0);
    EXPECT_EQ(*options.ySkew, 0.0);
}

TEST(Transform, IsPanning) {
    Transform transform;

    AnimationOptions easeOptions(Seconds(1));
    easeOptions.transitionFrameFn = [&transform](double) {
        ASSERT_TRUE(transform.getState().isPanning());
    };

    transform.resize({1000, 1000});
    transform.easeTo(CameraOptions().withCenter(LatLng(0, 360.0)), easeOptions);
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(250));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(500));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(750));
    transform.updateTransitions(transform.getTransitionStart() + transform.getTransitionDuration());
}

TEST(Transform, DefaultTransform) {
    struct TransformObserver : public mln::TransformObserver {
        void onCameraWillChange(MapObserver::CameraChangeMode) final { cameraWillChangeCallback(); };

        void onCameraDidChange(MapObserver::CameraChangeMode) final { cameraDidChangeCallback(); };

        std::function<void()> cameraWillChangeCallback;
        std::function<void()> cameraDidChangeCallback;
    };

    uint32_t cameraWillChangeCount = 0;
    uint32_t cameraDidChangeCount = 0;

    TransformObserver observer;
    observer.cameraWillChangeCallback = [&cameraWillChangeCount]() {
        cameraWillChangeCount++;
    };
    observer.cameraDidChangeCallback = [&cameraDidChangeCount]() {
        cameraDidChangeCount++;
    };

    Transform transform(observer);
    const TransformState& state = transform.getState();
    ASSERT_FALSE(state.valid());

    LatLng nullIsland, latLng = {};
    ScreenCoordinate center, point = {};
    const uint32_t min = 0;
    const uint32_t max = 65535;

    // Cannot assign invalid sizes.
    std::vector<Size> invalidSizes = {{}, {min, max}, {max, min}};
    for (const Size& size : invalidSizes) {
        try {
            transform.resize(size);
            ASSERT_TRUE(false) << "Should throw";
        } catch (...) {
            ASSERT_TRUE(size.isEmpty());
        }
    }

    Size validSize{max, max};
    ASSERT_FALSE(validSize.isEmpty());

    try {
        transform.resize(validSize);
        ASSERT_EQ(cameraWillChangeCount, 1u);
        ASSERT_EQ(cameraDidChangeCount, 1u);
    } catch (...) {
        ASSERT_TRUE(false) << "Should not throw";
    }

    ASSERT_TRUE(state.valid());

    // Double resize
    try {
        transform.resize(validSize);
        ASSERT_EQ(cameraWillChangeCount, 1u);
        ASSERT_EQ(cameraDidChangeCount, 1u);
    } catch (...) {
        ASSERT_TRUE(false) << "Should not throw";
    }

    center = {max / 2., max / 2.};
    latLng = state.screenCoordinateToLatLng(center);
    ASSERT_NEAR(latLng.latitude(), nullIsland.latitude(), 1e-8);
    ASSERT_NEAR(latLng.longitude(), nullIsland.longitude(), 1e-8);

    point = state.latLngToScreenCoordinate(nullIsland);
    ASSERT_DOUBLE_EQ(point.x, center.x);
    ASSERT_DOUBLE_EQ(point.y, center.y);

    // Constrain to screen while resizing
    transform.resize({1000, 500});
    transform.setLatLngBounds(LatLngBounds::hull({40.0, -10.0}, {70.0, 40.0}));
    transform.setConstrainMode(ConstrainMode::Screen);

    // Request impossible zoom
    AnimationOptions easeOptions(Seconds(1));
    transform.easeTo(CameraOptions().withCenter(LatLng{56, 11}).withZoom(1), easeOptions);
    ASSERT_TRUE(transform.inTransition());
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(250));

    // Rotate the screen during a transition (resize it)
    transform.resize({500, 1000});

    // The resize while constraining to screen should have stopped the transition and updated the state
    ASSERT_FALSE(transform.inTransition());
    ASSERT_NEAR(transform.getLatLng().longitude(), 8.22103, 1e-4);
    ASSERT_NEAR(transform.getLatLng().latitude(), 46.6905, 1e-4);
    ASSERT_NEAR(transform.getState().getScale(), 38.1529, 1e-4);
}

TEST(Transform, LatLngBounds) {
    const LatLng nullIsland{};
    const LatLng sanFrancisco{37.7749, -122.4194};

    Transform transform;
    transform.resize({1000, 1000});

    transform.jumpTo(CameraOptions().withCenter(LatLng()).withZoom(transform.getState().getMaxZoom()));

    // Default bounds.
    ASSERT_EQ(transform.getState().getLatLngBounds(), LatLngBounds());
    ASSERT_EQ(transform.getLatLng(), nullIsland);

    // Invalid bounds.
    try {
        transform.setLatLngBounds(LatLngBounds::empty());
        ASSERT_TRUE(false) << "Should throw";
    } catch (...) {
        ASSERT_EQ(transform.getState().getLatLngBounds(), LatLngBounds());
    }

    transform.jumpTo(CameraOptions().withCenter(sanFrancisco));
    ASSERT_NEAR(transform.getLatLng().latitude(), sanFrancisco.latitude(), 1e-8);
    ASSERT_NEAR(transform.getLatLng().longitude(), sanFrancisco.longitude(), 1e-8);

    // Single location.
    transform.setLatLngBounds(LatLngBounds::singleton(sanFrancisco));
    ASSERT_EQ(transform.getLatLng(), sanFrancisco);

    //    -1   |   0   |  +1
    // ┌───┬───┰───┬───┰───┬───┐
    // │   │   ┃•  │   ┃   │   │
    // ├───┼───╂───┼───╂───┼───┤
    // │   │   ┃▓▓▓│▓▓▓┃   │   │
    // └───┴───┸───┴───┸───┴───┘
    transform.setLatLngBounds(LatLngBounds::hull({-90.0, -180.0}, {0.0, 180.0}));
    transform.jumpTo(CameraOptions().withCenter(sanFrancisco));
    ASSERT_EQ(transform.getLatLng().latitude(), 0.0);
    ASSERT_EQ(transform.getLatLng().longitude(), sanFrancisco.longitude());

    // Try crossing the antimeridian from the left.
    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, -200.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), -180.0);

    // Try crossing the antimeridian from the right.
    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 200.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng(LatLng::Unwrapped).longitude(), 180.0);
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), -180.0);

    //    -1   |   0   |  +1
    // ┌───┬───┰───┬───┰───┬───┐
    // │   │   ┃•  │▓▓▓┃   │   │
    // ├───┼───╂───┼───╂───┼───┤
    // │   │   ┃   │▓▓▓┃   │   │
    // └───┴───┸───┴───┸───┴───┘
    transform.setLatLngBounds(LatLngBounds::hull({-90.0, 0.0}, {90.0, 180.0}));
    transform.jumpTo(CameraOptions().withCenter(sanFrancisco));
    ASSERT_NEAR(transform.getLatLng().latitude(), sanFrancisco.latitude(), 1e-8);
    ASSERT_EQ(transform.getLatLng().longitude(), 0.0);

    //    -1   |   0   |  +1
    // ┌───┬───┰───┬───┰───┬───┐
    // │   │   ┃•  │   ┃   │   │
    // ├───┼───╂───┼───╂───┼───┤
    // │   │   ┃   │▓▓▓┃   │   │
    // └───┴───┸───┴───┸───┴───┘
    transform.setLatLngBounds(LatLngBounds::hull({-90.0, 0.0}, {0.0, 180.0}));
    transform.jumpTo(CameraOptions().withCenter(sanFrancisco));
    ASSERT_EQ(transform.getLatLng().latitude(), 0.0);
    ASSERT_EQ(transform.getLatLng().longitude(), 0.0);

    //    -1   |   0   |  +1
    // ┌───┬───┰───┬───┰───┬───┐
    // │   │   ┃   │  ▓┃▓  │   │
    // ├───┼───╂───┼───╂───┼───┤
    // │   │   ┃   │   ┃   │   │
    // └───┴───┸───┴───┸───┴───┘
    LatLng inside{45.0, 150.0};
    transform.setLatLngBounds(LatLngBounds::hull({0.0, 120.0}, {90.0, 240.0}));
    transform.jumpTo(CameraOptions().withCenter(inside));
    ASSERT_EQ(transform.getLatLng().latitude(), inside.latitude());
    ASSERT_EQ(transform.getLatLng().longitude(), inside.longitude());

    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 140.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), 140.0);

    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 160.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), 160.0);

    // Constrain latitude only.
    transform.jumpTo(CameraOptions().withCenter(LatLng{-45.0, inside.longitude()}));
    ASSERT_EQ(transform.getLatLng().latitude(), 0.0);
    ASSERT_EQ(transform.getLatLng().longitude(), inside.longitude());

    // Crossing the antimeridian, within bounds.
    transform.jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), 181.0}));
    ASSERT_EQ(transform.getLatLng().longitude(), -179.0);

    // Crossing the antimeridian, outside bounds.
    transform.jumpTo(CameraOptions().withCenter(inside));
    transform.jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), 250.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), -120.0);

    // Constrain to the left edge.
    transform.jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), 119.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), 120.0);

    // Simulate swipe to the left.
    mln::AnimationOptions easeOptions(mln::Seconds(1));
    easeOptions.transitionFrameFn = [&](double /* t */) {
        ASSERT_NEAR(transform.getLatLng().longitude(), 120.0, 1e-4);
    };
    easeOptions.transitionFinishFn = [&]() {
        ASSERT_NEAR(transform.getLatLng().longitude(), 120.0, 1e-4);
    };
    transform.moveBy(ScreenCoordinate{-500, -500}, easeOptions);

    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(0));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(250));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(500));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(750));
    transform.updateTransitions(transform.getTransitionStart() + transform.getTransitionDuration());

    // Constrain to the right edge.
    transform.jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), 241.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), -120.0);

    // Simulate swipe to the right.
    easeOptions.transitionFrameFn = [&](double /* t */) {
        ASSERT_NEAR(transform.getLatLng().longitude(), -120.0, 1e-4);
    };
    easeOptions.transitionFinishFn = [&]() {
        ASSERT_NEAR(transform.getLatLng().longitude(), -120.0, 1e-4);
    };
    transform.moveBy(ScreenCoordinate{500, 500}, easeOptions);

    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(0));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(250));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(500));
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(750));
    transform.updateTransitions(transform.getTransitionStart() + transform.getTransitionDuration());

    //    -1   |   0   |  +1
    // ┌───┬───┰───┬───┰───┬───┐
    // │   │   ┃   │   ┃   │   │
    // ├───┼───╂───┼───╂───┼───┤
    // │   │  ▓┃▓  │   ┃   │   │
    // └───┴───┸───┴───┸───┴───┘
    inside = LatLng{-45.0, -150.0};
    transform.setLatLngBounds(LatLngBounds::hull({-90.0, -240.0}, {0.0, -120.0}));
    transform.jumpTo(CameraOptions().withCenter(inside));
    ASSERT_DOUBLE_EQ(transform.getLatLng().latitude(), inside.latitude());
    ASSERT_EQ(transform.getLatLng().longitude(), inside.longitude());

    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, -140.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), -140.0);

    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, -160.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), -160.0);

    // Constrain latitude only.
    transform.jumpTo(CameraOptions().withCenter(LatLng{45.0, inside.longitude()}));
    ASSERT_EQ(transform.getLatLng().latitude(), 0.0);
    ASSERT_EQ(transform.getLatLng().longitude(), inside.longitude());

    // Crossing the antimeridian, within bounds.
    transform.jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), -181.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng().latitude(), inside.latitude());
    ASSERT_EQ(transform.getLatLng().longitude(), 179.0);

    // Crossing the antimeridian, outside bounds.
    transform.jumpTo(CameraOptions().withCenter(inside));
    transform.jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), -250.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), 120.0);

    // Constrain to the left edge.
    transform.jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), -119.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), -120.0);

    transform.moveBy(ScreenCoordinate{-500, 0});
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), -120.0);

    // Constrain to the right edge.
    transform.jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), -241.0}));
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), 120.0);

    transform.moveBy(ScreenCoordinate{500, 0});
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), 120.0);
}

TEST(Transform, ConstrainScreenToBounds) {
    Transform transform;

    transform.resize({500, 500});
    transform.setLatLngBounds(LatLngBounds::hull({40.0, -10.0}, {70.0, 40.0}));
    transform.setConstrainMode(ConstrainMode::Screen);

    // Request impossible zoom
    transform.easeTo(CameraOptions().withCenter(LatLng{56, 11}).withZoom(1));
    ASSERT_NEAR(transform.getZoom(), 2.81378, 1e-4);

    // Request impossible center left
    transform.easeTo(CameraOptions().withCenter(LatLng{56, -65}).withZoom(4));
    ASSERT_NEAR(transform.getLatLng().longitude(), 0.98632, 1e-4);
    ASSERT_NEAR(transform.getLatLng().latitude(), 56.0, 1e-4);

    // Request impossible center top
    transform.easeTo(CameraOptions().withCenter(LatLng{80, 11}).withZoom(4));
    ASSERT_NEAR(transform.getLatLng().longitude(), 11.0, 1e-4);
    ASSERT_NEAR(transform.getLatLng().latitude(), 65.88603, 1e-4);

    // Request impossible center right
    transform.easeTo(CameraOptions().withCenter(LatLng{56, 50}).withZoom(4));
    ASSERT_NEAR(transform.getLatLng().longitude(), 29.01367, 1e-4);
    ASSERT_NEAR(transform.getLatLng().latitude(), 56.0, 1e-4);

    // Request impossible center bottom
    transform.easeTo(CameraOptions().withCenter(LatLng{30, 11}).withZoom(4));
    ASSERT_NEAR(transform.getLatLng().longitude(), 11.0, 1e-4);
    ASSERT_NEAR(transform.getLatLng().latitude(), 47.89217, 1e-4);

    // Request impossible center with anchor
    transform.easeTo(CameraOptions().withAnchor(ScreenCoordinate{250, 250}).withCenter(LatLng{56, -65}).withZoom(4));
    ASSERT_NEAR(transform.getLatLng().longitude(), 0.98632, 1e-4);
    ASSERT_NEAR(transform.getLatLng().latitude(), 56.0, 1e-4);

    // Request impossible center (anchor)
    transform.easeTo(CameraOptions().withAnchor(ScreenCoordinate{250, 250}).withZoom(4));
    ASSERT_NEAR(transform.getLatLng().longitude(), 0.98632, 1e-4);
    ASSERT_NEAR(transform.getLatLng().latitude(), 56.0, 1e-4);

    // Fly to impossible center
    transform.flyTo(CameraOptions().withCenter(LatLng{56, -65}).withZoom(4));
    ASSERT_NEAR(transform.getZoom(), 4.0, 1e-4);
    ASSERT_NEAR(transform.getLatLng().longitude(), 0.98632, 1e-4);
    ASSERT_NEAR(transform.getLatLng().latitude(), 56.0, 1e-4);

    // Fly to impossible center and zoom
    transform.flyTo(CameraOptions().withCenter(LatLng{56, -65}).withZoom(2));
    ASSERT_NEAR(transform.getZoom(), 4.0, 1e-4);
    ASSERT_NEAR(transform.getLatLng().longitude(), 0.98632, 1e-4);
    ASSERT_NEAR(transform.getLatLng().latitude(), 56.0, 1e-4);
}

TEST(Transform, InvalidPitch) {
    Transform transform;
    transform.resize({1, 1});

    ASSERT_DOUBLE_EQ(0, transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform.getLatLng().longitude());
    ASSERT_DOUBLE_EQ(0, transform.getZoom());
    ASSERT_DOUBLE_EQ(0, transform.getPitch());

    transform.jumpTo(CameraOptions().withZoom(1.0).withPitch(45));
    ASSERT_DOUBLE_EQ(1, transform.getZoom());
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform.getPitch());

    const double invalid = NAN;

    transform.jumpTo(CameraOptions().withPitch(invalid));
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform.getPitch());

    transform.jumpTo(CameraOptions().withPitch(60));
    ASSERT_DOUBLE_EQ(util::deg2rad(60), transform.getPitch());
}

TEST(Transform, MinMaxPitch) {
    Transform transform;
    transform.resize({1, 1});

    ASSERT_DOUBLE_EQ(0, transform.getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform.getLatLng().longitude());
    ASSERT_DOUBLE_EQ(0, transform.getZoom());
    ASSERT_DOUBLE_EQ(0, transform.getPitch());

    transform.jumpTo(CameraOptions().withZoom(1.0).withPitch(60));
    ASSERT_DOUBLE_EQ(1, transform.getZoom());
    ASSERT_DOUBLE_EQ(transform.getState().getMaxPitch(), transform.getPitch());
    ASSERT_DOUBLE_EQ(util::deg2rad(60), transform.getPitch());

    transform.setMaxPitch(70);
    transform.jumpTo(CameraOptions().withPitch(70));
    ASSERT_DOUBLE_EQ(transform.getState().getMaxPitch(), transform.getPitch());

    transform.setMaxPitch(45);
    transform.jumpTo(CameraOptions().withPitch(60));
    ASSERT_DOUBLE_EQ(transform.getState().getMaxPitch(), transform.getPitch());
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform.getPitch());

    transform.jumpTo(CameraOptions().withPitch(0));
    ASSERT_DOUBLE_EQ(transform.getState().getMinPitch(), transform.getPitch());
    ASSERT_DOUBLE_EQ(0, transform.getPitch());

    transform.setMinPitch(-10);
    transform.jumpTo(CameraOptions().withPitch(-10));
    ASSERT_DOUBLE_EQ(transform.getState().getMinPitch(), transform.getPitch());
    ASSERT_DOUBLE_EQ(0, transform.getPitch());

    transform.setMinPitch(15);
    transform.jumpTo(CameraOptions().withPitch(0));
    ASSERT_DOUBLE_EQ(transform.getState().getMinPitch(), transform.getPitch());
    ASSERT_DOUBLE_EQ(util::deg2rad(15), transform.getPitch());

    transform.setMinPitch(45);
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform.getState().getMinPitch());
    transform.setMaxPitch(45);
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform.getState().getMaxPitch());

    transform.setMaxPitch(10);
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform.getState().getMaxPitch());

    transform.setMinPitch(60);
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform.getState().getMinPitch());
}

static const double abs_double_error = 1e-5;

MATCHER_P(Vec3NearEquals1E5, vec, "") {
    return std::fabs(vec[0] - arg[0]) <= abs_double_error && std::fabs(vec[1] - arg[1]) <= abs_double_error &&
           std::fabs(vec[2] - arg[2]) <= abs_double_error;
}

TEST(Transform, FreeCameraOptionsInvalidSize) {
    Transform transform;
    FreeCameraOptions options;

    options.orientation = vec4{{1.0, 1.0, 1.0, 1.0}};
    options.position = vec3{{0.1, 0.2, 0.3}};
    transform.setFreeCameraOptions(options);

    const auto updatedOrientation = transform.getFreeCameraOptions().orientation.value();
    const auto updatedPosition = transform.getFreeCameraOptions().position.value();

    EXPECT_DOUBLE_EQ(0.0, updatedOrientation[0]);
    EXPECT_DOUBLE_EQ(0.0, updatedOrientation[1]);
    EXPECT_DOUBLE_EQ(0.0, updatedOrientation[2]);
    EXPECT_DOUBLE_EQ(1.0, updatedOrientation[3]);

    EXPECT_THAT(updatedPosition, Vec3NearEquals1E5(vec3{{0.0, 0.0, 0.0}}));
}

TEST(Transform, FreeCameraOptionsNanInput) {
    Transform transform;
    transform.resize({100, 100});
    FreeCameraOptions options;

    options.position = vec3{{0.5, 0.5, 0.25}};
    transform.setFreeCameraOptions(options);

    options.position = vec3{{0.0, 0.0, NAN}};
    transform.setFreeCameraOptions(options);
    EXPECT_EQ((vec3{{0.5, 0.5, 0.25}}), transform.getFreeCameraOptions().position.value());

    // Only the invalid parameter should be discarded
    options.position = vec3{{0.3, 0.1, 0.2}};
    options.orientation = vec4{{NAN, 0.0, NAN, 0.0}};
    transform.setFreeCameraOptions(options);
    EXPECT_THAT(transform.getFreeCameraOptions().position.value(), Vec3NearEquals1E5(vec3{{0.3, 0.1, 0.2}}));
    EXPECT_EQ(Quaternion::identity.m, transform.getFreeCameraOptions().orientation.value());
}

TEST(Transform, FreeCameraOptionsInvalidZ) {
    Transform transform;
    transform.resize({100, 100});
    FreeCameraOptions options;

    // Invalid z-value (<= 0.0 || > 1) should be clamped to respect both min&max zoom values
    options.position = vec3{{0.1, 0.1, 0.0}};
    transform.setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(transform.getState().getMaxZoom(), transform.getState().getZoom());
    EXPECT_GT(transform.getFreeCameraOptions().position.value()[2], 0.0);

    options.position = vec3{{0.5, 0.2, 123.456}};
    transform.setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(transform.getState().getMinZoom(), transform.getState().getZoom());
    EXPECT_LE(transform.getFreeCameraOptions().position.value()[2], 1.0);
}

TEST(Transform, FreeCameraOptionsHighPitch) {
    Transform transform;
    transform.resize({100, 100});
    transform.setMaxPitch(180.0);
    FreeCameraOptions options;

    options.position = vec3{{0.1, 0.1, 0.1}};
    options.orientation = Quaternion::fromAxisAngle(vec3{{1.0, 0.0, 0.0}}, util::deg2rad(-90.0)).m;
    transform.setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(util::deg2rad(90.0), transform.getState().getPitch());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getBearing());
    EXPECT_THAT(transform.getFreeCameraOptions().position.value(), Vec3NearEquals1E5(vec3{{0.1, 0.1, 0.1}}));

    options.position = vec3{{0.1, 0.1, 0.1}};
    options.orientation = Quaternion::fromAxisAngle(vec3{{1.0, 0.0, 0.0}}, util::deg2rad(-135.0)).m;
    transform.setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(util::deg2rad(135.0), transform.getState().getPitch());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getBearing());
    EXPECT_THAT(transform.getFreeCameraOptions().position.value(), Vec3NearEquals1E5(vec3{{0.1, 0.1, 0.1}}));
}

TEST(Transform, FreeCameraOptionsInvalidOrientation) {
    // Invalid orientations that cannot be clamped into a valid range
    Transform transform;
    transform.resize({100, 100});

    FreeCameraOptions options;
    options.orientation = vec4{{0.0, 0.0, 0.0, 0.0}};
    transform.setFreeCameraOptions(options);
    EXPECT_EQ(Quaternion::identity.m, transform.getFreeCameraOptions().orientation);

    // Gimbal lock. Both forward and up vectors are on xy-plane
    transform.setMaxPitch(180.0);
    options.orientation = Quaternion::fromAxisAngle(vec3{{0.0, 1.0, 0.0}}, pi / 2).m;
    transform.setFreeCameraOptions(options);
    for (int i = 0; i < 4; i++) {
        EXPECT_NEAR(Quaternion::fromEulerAngles(pi / 2, pi / 2, pi / 2).m[i],
                    transform.getFreeCameraOptions().orientation.value()[i],
                    1.0e-12);
    }

    // Camera is upside down
    options.orientation = Quaternion::fromAxisAngle(vec3{{1.0, 0.0, 0.0}}, pi / 2 + pi / 4).m;
    transform.setFreeCameraOptions(options);
    for (int i = 0; i < 4; i++) {
        EXPECT_NEAR(options.orientation.value()[i], transform.getFreeCameraOptions().orientation.value()[i], 1.0e-12);
    }
}

TEST(Transform, FreeCameraOptionsSetOrientation) {
    Transform transform;
    transform.resize({100, 100});
    FreeCameraOptions options;

    options.orientation = Quaternion::identity.m;
    transform.setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getBearing());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getPitch());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getX());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getY());

    options.orientation = Quaternion::fromAxisAngle(vec3{{1.0, 0.0, 0.0}}, util::deg2rad(-60.0)).m;
    transform.setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getBearing());
    EXPECT_DOUBLE_EQ(util::deg2rad(60.0), transform.getState().getPitch());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getX());
    EXPECT_DOUBLE_EQ(206.0, transform.getState().getY());

    options.orientation = Quaternion::fromAxisAngle(vec3{{0.0, 0.0, 1.0}}, util::deg2rad(56.0)).m;
    transform.setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(util::deg2rad(-56.0), transform.getState().getBearing());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getPitch());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getRoll());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getX());
    EXPECT_NEAR(152.192378, transform.getState().getY(), 1e-6);

    options.orientation = Quaternion::fromEulerAngles(0.0, 0.0, util::deg2rad(-179.0))
                              .multiply(Quaternion::fromEulerAngles(util::deg2rad(-30.0), 0.0, 0.0))
                              .m;
    transform.setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(util::deg2rad(179.0), transform.getState().getBearing());
    EXPECT_DOUBLE_EQ(util::deg2rad(30.0), transform.getState().getPitch());
    EXPECT_NEAR(1.308930, transform.getState().getX(), 1e-6);
    EXPECT_NEAR(56.813889, transform.getState().getY(), 1e-6);

    options.orientation = Quaternion::fromAxisAngle(vec3{{0.0, 0.0, 1.0}}, util::deg2rad(-33.0))
                              .multiply(Quaternion::fromAxisAngle(vec3{{1.0, 0.0, 0.0}}, util::deg2rad(-22.0)))
                              .multiply(Quaternion::fromAxisAngle(vec3{{0.0, 0.0, 1.0}}, util::deg2rad(11.0)))
                              .m;
    transform.setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(util::deg2rad(33.0), transform.getState().getBearing());
    EXPECT_DOUBLE_EQ(util::deg2rad(22.0), transform.getState().getPitch());
    EXPECT_FLOAT_EQ(util::deg2rad(11.0), transform.getState().getRoll());
}

static std::tuple<vec3, vec3, vec3> rotatedFrame(const std::array<double, 4>& quaternion) {
    Quaternion q(quaternion);
    return std::make_tuple(
        q.transform({{1.0, 0.0, 0.0}}), q.transform({{0.0, -1.0, 0.0}}), q.transform({{0.0, 0.0, -1.0}}));
}

TEST(Transform, FreeCameraOptionsClampToBounds) {
    Transform transform;
    transform.resize({100, 100});
    transform.setConstrainMode(ConstrainMode::WidthAndHeight);
    transform.jumpTo(CameraOptions().withZoom(8.56));
    FreeCameraOptions options;

    // Place camera to an arbitrary position looking away from the map
    options.position = vec3{{-100.0, -10000.0, 1000.0}};
    options.orientation = Quaternion::fromEulerAngles(util::deg2rad(-45.0), 0.0, 0.0).m;
    transform.setFreeCameraOptions(options);

    // Map center should be clamped to width/2 pixels away from map borders
    EXPECT_DOUBLE_EQ(206.0, transform.getState().getX());
    EXPECT_DOUBLE_EQ(206.0, transform.getState().getY());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getBearing());
    EXPECT_DOUBLE_EQ(util::deg2rad(45.0), transform.getState().getPitch());

    vec3 right, up, forward;
    std::tie(right, up, forward) = rotatedFrame(transform.getFreeCameraOptions().orientation.value());
    EXPECT_THAT(transform.getFreeCameraOptions().position.value(),
                Vec3NearEquals1E5(vec3{{0.0976562, 0.304816, 0.20716}}));
    EXPECT_THAT(right, Vec3NearEquals1E5(vec3{{1.0, 0.0, 0.0}}));
    EXPECT_THAT(up, Vec3NearEquals1E5(vec3{{0, -0.707107, 0.707107}}));
    EXPECT_THAT(forward, Vec3NearEquals1E5(vec3{{0, -0.707107, -0.707107}}));
}

TEST(Transform, FreeCameraOptionsInvalidState) {
    Transform transform;

    // Invalid size
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getX());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getY());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getBearing());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getPitch());

    const auto options = transform.getFreeCameraOptions();
    EXPECT_THAT(options.position.value(), Vec3NearEquals1E5(vec3{{0.0, 0.0, 0.0}}));
}

TEST(Transform, FreeCameraOptionsStateSynchronization) {
    Transform transform;
    transform.resize({100, 100});
    vec3 right, up, forward;

    transform.jumpTo(CameraOptions().withPitch(0.0).withBearing(0.0));
    std::tie(right, up, forward) = rotatedFrame(transform.getFreeCameraOptions().orientation.value());
    EXPECT_THAT(transform.getFreeCameraOptions().position.value(), Vec3NearEquals1E5(vec3{{0.5, 0.5, 0.29296875}}));
    EXPECT_THAT(right, Vec3NearEquals1E5(vec3{{1.0, 0.0, 0.0}}));
    EXPECT_THAT(up, Vec3NearEquals1E5(vec3{{0.0, -1.0, 0.0}}));
    EXPECT_THAT(forward, Vec3NearEquals1E5(vec3{{0.0, 0.0, -1.0}}));

    transform.jumpTo(CameraOptions().withCenter(LatLng{60.1699, 24.9384}));
    EXPECT_THAT(transform.getFreeCameraOptions().position.value(),
                Vec3NearEquals1E5(vec3{{0.569273, 0.289453, 0.292969}}));

    transform.jumpTo(CameraOptions().withPitch(20.0).withBearing(77.0).withCenter(LatLng{-20.0, 20.0}));
    EXPECT_THAT(transform.getFreeCameraOptions().position.value(),
                Vec3NearEquals1E5(vec3{{0.457922, 0.57926, 0.275301}}));
}

TEST(Camera, SetOrientation) {
    util::Camera camera;
    const double bearing = 0.22;
    const double pitch = 0.34;
    const double roll = 0.0;
    double bearing_ = 0;
    double pitch_ = 0;
    double roll_ = 0;
    camera.setOrientation(roll, pitch, bearing);
    camera.getOrientation(roll_, pitch_, bearing_);
    EXPECT_NEAR(bearing, bearing_, 1.0e-9);
    EXPECT_NEAR(pitch, pitch_, 1.0e-9);
    EXPECT_NEAR(roll, roll_, 1.0e-9);
}

TEST(Camera, SetOrientationWithRoll) {
    util::Camera camera;
    const double bearing = 0.22;
    const double pitch = 0.34;
    const double roll = 0.45;
    double bearing_ = 0;
    double pitch_ = 0;
    double roll_ = 0;
    camera.setOrientation(roll, pitch, bearing);
    camera.getOrientation(roll_, pitch_, bearing_);
    EXPECT_NEAR(bearing, bearing_, 1.0e-9);
    EXPECT_NEAR(pitch, pitch_, 1.0e-9);
    EXPECT_NEAR(roll, roll_, 1.0e-9);
}

TEST(Camera, SetOrientationWithRollNoPitch) {
    util::Camera camera;
    const double bearing = 0.22;
    const double pitch = 0.0;
    const double roll = 0.45;
    double bearing_ = 0;
    double pitch_ = 0;
    double roll_ = 0;
    camera.setOrientation(roll, pitch, bearing);
    camera.getOrientation(roll_, pitch_, bearing_);
    EXPECT_NEAR(bearing - roll, bearing_, 1.0e-9);
    EXPECT_NEAR(pitch, pitch_, 1.0e-9);
    EXPECT_NEAR(0.0, roll_, 1.0e-9);
}

namespace {

// The tile matrix as TransformState::matrixFor computed it before the projection seam.
mat4 referenceTileMatrix(double scale, const UnwrappedTileID& tileID) {
    const uint64_t tileScale = 1ull << tileID.canonical.z;
    const double s = Projection::worldSize(scale) / tileScale;

    mat4 matrix;
    matrix::identity(matrix);
    matrix::translate(matrix,
                      matrix,
                      static_cast<double>(tileID.canonical.x + tileID.wrap * static_cast<int64_t>(tileScale)) * s,
                      static_cast<double>(tileID.canonical.y) * s,
                      0);
    matrix::scale(matrix, matrix, s / util::EXTENT, s / util::EXTENT, 1);
    return matrix;
}

} // namespace

TEST(MercatorProjection, MatchesStaticProjection) {
    const MercatorProjection projection;
    for (const double zoom : {0.0, 1.0, 3.5, 7.25, 12.0, 18.0}) {
        const double scale = std::pow(2.0, zoom);
        for (const double lat : {-85.0, -45.0, 0.0, 30.0, 80.0}) {
            for (const double lon : {-180.0, -90.0, 0.0, 45.0, 179.0}) {
                const LatLng latLng{lat, lon};
                const Point<double> expected = Projection::project(latLng, scale);
                const Point<double> actual = projection.project(latLng, scale);
                EXPECT_EQ(expected.x, actual.x);
                EXPECT_EQ(expected.y, actual.y);

                const LatLng unprojected = projection.unproject(actual, scale, LatLng::Wrapped);
                const LatLng expectedUnprojected = Projection::unproject(expected, scale, LatLng::Wrapped);
                EXPECT_EQ(expectedUnprojected.latitude(), unprojected.latitude());
                EXPECT_EQ(expectedUnprojected.longitude(), unprojected.longitude());
            }
        }
    }
}

TEST(TransformState, ProjectionDataMatchesTileMatrix) {
    Transform transform;
    const std::vector<UnwrappedTileID> tileIDs{
        {0, 0, 0}, {1, 0, 1}, {3, 5, 2}, {10, 512, 340}, {14, 16000, 8000}, {-1, CanonicalTileID{2, 1, 1}}};

    for (const Size size : {Size{1, 1}, Size{512, 512}, Size{1024, 768}, Size{333, 777}}) {
        transform.resize(size);
        for (const double zoom : {0.0, 2.5, 6.0, 11.75, 16.0}) {
            for (const double pitch : {0.0, 30.0, 60.0}) {
                for (const double bearing : {0.0, 45.0, 210.0}) {
                    for (const double roll : {0.0, 15.0}) {
                        transform.jumpTo(CameraOptions()
                                             .withCenter(LatLng{37.7749, -122.4194})
                                             .withZoom(zoom)
                                             .withPitch(pitch)
                                             .withBearing(bearing)
                                             .withRoll(roll));
                        const TransformState& state = transform.getState();
                        const double scale = std::pow(2.0, state.getZoom());

                        for (const bool aligned : {false, true}) {
                            mat4 projMatrix;
                            state.getProjMatrix(projMatrix, 1, aligned);

                            for (const auto& tileID : tileIDs) {
                                const mat4 tileMatrix = referenceTileMatrix(scale, tileID);
                                mat4 actualTileMatrix;
                                state.matrixFor(actualTileMatrix, tileID);
                                EXPECT_EQ(tileMatrix, actualTileMatrix);

                                mat4 expected;
                                matrix::multiply(expected, projMatrix, tileMatrix);
                                EXPECT_EQ(expected, state.getProjectionData(tileID, projMatrix).mainMatrix);
                            }
                        }

                        for (const auto& tileID : tileIDs) {
                            mat4 expected;
                            matrix::multiply(expected, state.getProjectionMatrix(), referenceTileMatrix(scale, tileID));
                            EXPECT_EQ(expected, state.getProjectionData(tileID).mainMatrix);
                        }
                    }
                }
            }
        }
    }
}

TEST(TransformState, ProjectionDataMercatorFields) {
    Transform transform;
    transform.resize({512, 512});
    transform.jumpTo(CameraOptions().withCenter(LatLng{10, 20}).withZoom(5.0).withPitch(20.0).withBearing(30.0));
    const TransformState& state = transform.getState();

    for (const auto& tileID :
         {UnwrappedTileID{0, 0, 0}, UnwrappedTileID{3, 5, 2}, UnwrappedTileID{-1, CanonicalTileID{2, 1, 1}}}) {
        const ProjectionData data = state.getProjectionData(tileID);
        const double tileScale = static_cast<double>(1u << tileID.canonical.z);
        EXPECT_EQ(data.mainMatrix, data.fallbackMatrix);
        EXPECT_EQ(tileID.canonical.x / tileScale, data.tileMercatorCoords[0]);
        EXPECT_EQ(tileID.canonical.y / tileScale, data.tileMercatorCoords[1]);
        EXPECT_EQ(1.0 / tileScale / util::EXTENT, data.tileMercatorCoords[2]);
        EXPECT_EQ(1.0 / tileScale / util::EXTENT, data.tileMercatorCoords[3]);
        EXPECT_EQ((vec4{{0, 0, 0, 0}}), data.clippingPlane);
        EXPECT_EQ(0.0, data.projectionTransition);
    }
}

TEST(Transform, ProjectionDefinition) {
    Transform transform;
    ASSERT_FALSE(transform.getState().isGlobeRendering());
    ASSERT_DOUBLE_EQ(0.0, transform.getState().getProjectionTransition());
    transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective", "mercator", 0.5));
    ASSERT_TRUE(transform.getState().isGlobeRendering());
    ASSERT_DOUBLE_EQ(0.5, transform.getState().getProjectionTransition());
    // The transition is how far the globe has gone, whichever way the definition reads.
    transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective", "mercator", 0.25));
    ASSERT_DOUBLE_EQ(0.75, transform.getState().getProjectionTransition());
    transform.setProjectionDefinition(ProjectionDefinition("mercator", "vertical-perspective", 0.25));
    ASSERT_DOUBLE_EQ(0.25, transform.getState().getProjectionTransition());
    transform.setProjectionDefinition(ProjectionDefinition("mercator"));
    ASSERT_FALSE(transform.getState().isGlobeRendering());
    ASSERT_DOUBLE_EQ(0.0, transform.getState().getProjectionTransition());
}

TEST(VerticalPerspectiveProjection, TileCoordinatesToSphere) {
    const auto near = [](const vec3& a, const vec3& b) {
        return std::abs(a[0] - b[0]) < 1e-9 && std::abs(a[1] - b[1]) < 1e-9 && std::abs(a[2] - b[2]) < 1e-9;
    };
    const UnwrappedTileID world{0, 0, 0};
    const double half = util::EXTENT / 2.0;
    // (0°, 0°) is the +Z axis; the equator runs through ±X; latitude is Y.
    EXPECT_TRUE(near({{0, 0, 1}}, VerticalPerspectiveProjection::tileCoordinatesToSphere({half, half}, world)));
    EXPECT_TRUE(near({{1, 0, 0}}, VerticalPerspectiveProjection::tileCoordinatesToSphere({half * 1.5, half}, world)));
    EXPECT_TRUE(near({{-1, 0, 0}}, VerticalPerspectiveProjection::tileCoordinatesToSphere({half * 0.5, half}, world)));
    const vec3 north = VerticalPerspectiveProjection::tileCoordinatesToSphere({half, 0}, world);
    EXPECT_GT(north[1], 0.99);
    const vec3 south = VerticalPerspectiveProjection::tileCoordinatesToSphere({half, util::EXTENT}, world);
    EXPECT_LT(south[1], -0.99);
}

namespace {

void setUpGlobe(Transform& transform, const LatLng& center, double zoom, double bearing = 0, double pitch = 0) {
    transform.resize({800, 600});
    transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective"));
    transform.jumpTo(CameraOptions().withCenter(center).withZoom(zoom).withBearing(bearing).withPitch(pitch));
}

// The GL JS camera tests run on a 512 by 512 globe.
void setUpGlobeCamera(Transform& transform, const LatLng& center, double zoom) {
    transform.resize({512, 512});
    transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective"));
    transform.jumpTo(CameraOptions().withCenter(center).withZoom(zoom));
}

// One second with a linear easing, so a frame at half time is the frame at k = 0.5.
AnimationOptions linearSecond() {
    AnimationOptions animation;
    animation.duration = Milliseconds(1000);
    animation.easing.emplace(0, 0, 1, 1);
    return animation;
}

void runTo(Transform& transform, double k) {
    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(static_cast<int64_t>(k * 1000)));
}

} // namespace

TEST(VerticalPerspectiveProjection, CenterProjectsToScreenCenter) {
    Transform transform;
    setUpGlobe(transform, {37.0, -122.0}, 3.0, 20.0, 30.0);
    const TransformState& state = transform.getState();
    ASSERT_TRUE(state.isGlobeRendering());

    const double radius = VerticalPerspectiveProjection::globeRadiusPixels(Projection::worldSize(state.getScale()),
                                                                           state.getLatLng().latitude());
    const mat4 matrix = VerticalPerspectiveProjection::globeViewProjectionMatrix(state, radius);
    const vec3 center = VerticalPerspectiveProjection::surfaceVector(state.getLatLng());
    const vec4 surface = {{center[0], center[1], center[2], 1}};
    vec4 clip;
    matrix::transformMat4(clip, surface, matrix);
    EXPECT_NEAR(0.0, clip[0] / clip[3], 1e-6);
    EXPECT_NEAR(0.0, clip[1] / clip[3], 1e-6);

    const vec4 plane = VerticalPerspectiveProjection::clippingPlane(state, radius);
    EXPECT_GT(plane[0] * surface[0] + plane[1] * surface[1] + plane[2] * surface[2] + plane[3], 0.0);
    const vec4 antipode = {{-surface[0], -surface[1], -surface[2], 1}};
    EXPECT_LT(plane[0] * antipode[0] + plane[1] * antipode[1] + plane[2] * antipode[2] + plane[3], 0.0);
}

TEST(GlobeTransform, CachedViewStateFollowsTheCamera) {
    Transform transform;
    setUpGlobe(transform, {20.0, -40.0}, 2.5, 35.0, 25.0);
    const TransformState& state = transform.getState();
    const auto check = [&] {
        const double radius = VerticalPerspectiveProjection::globeRadiusPixels(Projection::worldSize(state.getScale()),
                                                                               state.getLatLng().latitude());
        EXPECT_DOUBLE_EQ(radius, state.getGlobeRadiusPixels());
        const mat4 expected = VerticalPerspectiveProjection::globeViewProjectionMatrix(state, radius);
        const vec4 plane = VerticalPerspectiveProjection::clippingPlane(state, radius);
        const vec3 camera = VerticalPerspectiveProjection::cameraPosition(state, radius);
        for (std::size_t i = 0; i < 16; ++i) {
            EXPECT_DOUBLE_EQ(expected[i], state.getGlobeViewProjectionMatrix()[i]) << "element " << i;
        }
        for (std::size_t i = 0; i < 4; ++i) {
            EXPECT_DOUBLE_EQ(plane[i], state.getGlobeClippingPlane()[i]) << "plane " << i;
        }
        for (std::size_t i = 0; i < 3; ++i) {
            EXPECT_DOUBLE_EQ(camera[i], state.getGlobeCameraPosition()[i]) << "camera " << i;
        }
        mat4 roundTrip;
        matrix::multiply(roundTrip, state.getGlobeViewProjectionMatrix(), state.getInverseGlobeViewProjectionMatrix());
        for (std::size_t i = 0; i < 16; ++i) {
            EXPECT_NEAR(i % 5 == 0 ? 1.0 : 0.0, roundTrip[i], 1e-9) << "inverse element " << i;
        }
    };
    check();
    transform.jumpTo(CameraOptions().withCenter(LatLng{-30.0, 100.0}).withZoom(4.0).withPitch(50.0).withBearing(-80.0));
    check();
    transform.resize({1024, 300});
    check();
}

TEST(GlobeTransform, UnprojectRoundTrip) {
    Transform transform;
    setUpGlobe(transform, {20.0, -40.0}, 2.5, 35.0, 25.0);
    for (const ScreenCoordinate& point :
         {ScreenCoordinate{400, 300}, ScreenCoordinate{300, 200}, ScreenCoordinate{500, 420}}) {
        const LatLng latLng = transform.screenCoordinateToLatLng(point);
        const ScreenCoordinate back = transform.latLngToScreenCoordinate(latLng);
        EXPECT_NEAR(point.x, back.x, 1e-6);
        EXPECT_NEAR(point.y, back.y, 1e-6);
    }
    const LatLng center = transform.screenCoordinateToLatLng({400, 300});
    EXPECT_NEAR(20.0, center.latitude(), 1e-9);
    EXPECT_NEAR(-40.0, center.longitude(), 1e-9);
}

TEST(GlobeTransform, OffGlobePixelsSnapToTheHorizon) {
    Transform transform;
    setUpGlobe(transform, {0.0, 0.0}, 0.5);
    // Zoom 0.5 shows the whole globe with room around it; a corner pixel is off the planet.
    const LatLng corner = transform.screenCoordinateToLatLng({0, 0});
    EXPECT_LT(corner.latitude(), 90.0);
    EXPECT_GT(corner.latitude(), 0.0);
    EXPECT_LT(corner.longitude(), 0.0);
    // The snap lands on the horizon: from a camera `d` radii out, that circle is acos(1 / d) from the center.
    const vec3& camera = transform.getState().getGlobeCameraPosition();
    const double horizon = std::acos(1.0 / std::hypot(camera[0], camera[1], camera[2]));
    const double angle = std::acos(std::cos(util::deg2rad(corner.latitude())) *
                                   std::cos(util::deg2rad(corner.longitude())));
    EXPECT_NEAR(horizon, angle, 1e-6);
}

TEST(GlobeTransform, MoveByPutsTheDraggedPointAtTheCenter) {
    Transform transform;
    setUpGlobe(transform, {10.0, 20.0}, 3.0);
    for (int i = 0; i < 8; i++) {
        const ScreenCoordinate offset{i % 2 ? 60.0 : -45.0, i % 3 ? 30.0 : -50.0};
        const LatLng expected = transform.screenCoordinateToLatLng({400 - offset.x, 300 - offset.y});
        const double zoomBefore = transform.getZoom();
        const double latitudeBefore = transform.getLatLng().latitude();
        transform.moveBy(offset);
        EXPECT_NEAR(expected.latitude(), transform.getLatLng().latitude(), 1e-6);
        EXPECT_NEAR(expected.longitude(), transform.getLatLng().longitude(), 1e-6);
        // The planet keeps its apparent size: zoom follows the latitude.
        EXPECT_NEAR(zoomBefore + VerticalPerspectiveProjection::zoomAdjustment(latitudeBefore, expected.latitude()),
                    transform.getZoom(),
                    1e-6);
    }
}

TEST(GlobeTransform, AnchoredZoomKeepsTheAnchorInPlace) {
    Transform transform;
    setUpGlobe(transform, {30.0, 15.0}, 3.0);
    const ScreenCoordinate anchor{250, 180};
    const LatLng anchorLatLng = transform.screenCoordinateToLatLng(anchor);
    transform.jumpTo(CameraOptions().withZoom(4.5).withAnchor(anchor));
    EXPECT_NE(30.0, transform.getLatLng().latitude());
    const ScreenCoordinate after = transform.latLngToScreenCoordinate(anchorLatLng);
    EXPECT_NEAR(anchor.x, after.x, 0.5);
    EXPECT_NEAR(anchor.y, after.y, 0.5);
}

TEST(GlobeTransform, ZoomingOutAroundAnAnchorNearTheHorizonStaysContinuous) {
    // A fast pinch-out with the focal point drifting off the planet: the exact re-anchoring jumped the center by
    // ten degrees of latitude and the zoom by half a level for one frame (a much larger globe); GL JS blends in a
    // damped heuristic near the horizon.
    Transform transform;
    setUpGlobe(transform, {48.0, 10.0}, 6.0);
    const ScreenCoordinate anchor{600, 200};
    double lastRadius = transform.getState().getGlobeRadiusPixels();
    LatLng lastCenter = transform.getLatLng();
    for (int i = 1; i <= 60; ++i) {
        transform.easeTo(CameraOptions().withZoom(6.0 - i * 0.1).withAnchor(anchor));
        if (i % 3 == 0) transform.moveBy({-7, 4});
        const double radius = transform.getState().getGlobeRadiusPixels();
        const LatLng center = transform.getLatLng();
        EXPECT_LE(radius, lastRadius * 1.005) << "step " << i;
        EXPECT_LT(std::abs(center.latitude() - lastCenter.latitude()), 5.0) << "step " << i;
        lastRadius = radius;
        lastCenter = center;
    }
}

TEST(GlobeTransform, PolesAreReachableAndZoomFollowsLatitude) {
    Transform transform;
    setUpGlobe(transform, {0.0, 0.0}, 0.0);
    const double radiusAtEquator = transform.getState().getGlobeRadiusPixels();
    transform.jumpTo(CameraOptions().withCenter(LatLng{80.0, 0.0}));
    EXPECT_NEAR(80.0, transform.getLatLng().latitude(), 1e-9);
    // Moving the center towards the pole lowers the zoom by log2(cos 80), below the map's minimum zoom of 0, so the
    // planet keeps its size on screen.
    EXPECT_NEAR(VerticalPerspectiveProjection::zoomAdjustment(0.0, 80.0), transform.getZoom(), 1e-9);
    EXPECT_NEAR(radiusAtEquator, transform.getState().getGlobeRadiusPixels(), 1e-6);
    transform.jumpTo(CameraOptions().withCenter(LatLng{80.0, 0.0}).withZoom(3.0));
    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 0.0}));
    EXPECT_NEAR(3.0 + VerticalPerspectiveProjection::zoomAdjustment(80.0, 0.0), transform.getZoom(), 1e-9);

    // Past the Mercator limit the center clamps, and the zoom follows the clamped center: the planet keeps its size.
    const double radiusBefore = transform.getState().getGlobeRadiusPixels();
    transform.jumpTo(CameraOptions().withCenter(LatLng{89.0, 0.0}));
    EXPECT_NEAR(util::LATITUDE_MAX, transform.getLatLng().latitude(), 1e-6);
    EXPECT_NEAR(radiusBefore, transform.getState().getGlobeRadiusPixels(), 1e-6);
    transform.moveBy({0.0, 300.0});
    EXPECT_NEAR(util::LATITUDE_MAX, transform.getLatLng().latitude(), 1e-6);
    EXPECT_NEAR(radiusBefore, transform.getState().getGlobeRadiusPixels(), 1e-6);
}

TEST(GlobeTransform, BoundsKeepTheCenterInside) {
    // GL JS ignores `maxBounds` on the globe (its TODO); native keeps honoring them, in every constrain mode.
    const LatLngBounds bounds = LatLngBounds::hull({40.0, -10.0}, {70.0, 40.0});
    for (const auto mode : {ConstrainMode::HeightOnly, ConstrainMode::WidthAndHeight, ConstrainMode::Screen}) {
        Transform transform;
        setUpGlobe(transform, {56.0, 11.0}, 4.0);
        transform.setConstrainMode(mode);
        transform.setLatLngBounds(bounds);
        EXPECT_NEAR(56.0, transform.getLatLng().latitude(), 1e-9);
        EXPECT_NEAR(11.0, transform.getLatLng().longitude(), 1e-9);
        for (const LatLng& outside :
             {LatLng{56.0, -65.0}, LatLng{80.0, 11.0}, LatLng{56.0, 50.0}, LatLng{30.0, 11.0}}) {
            transform.jumpTo(CameraOptions().withCenter(outside).withZoom(4.0));
            EXPECT_TRUE(bounds.contains(transform.getLatLng()))
                << transform.getLatLng().latitude() << ", " << transform.getLatLng().longitude();
        }
        // A center the whole screen fits around stays where it was asked for.
        transform.jumpTo(CameraOptions().withCenter(LatLng{55.0, 15.0}).withZoom(4.0));
        EXPECT_NEAR(55.0, transform.getLatLng().latitude(), 1e-9);
        EXPECT_NEAR(15.0, transform.getLatLng().longitude(), 1e-9);
        if (mode == ConstrainMode::Screen) {
            // The screen has to fit inside the bounds, so zooming out is stopped short.
            transform.jumpTo(CameraOptions().withCenter(LatLng{56.0, 11.0}).withZoom(1.0));
            EXPECT_GT(transform.getZoom(), 1.0);
            EXPECT_TRUE(bounds.contains(transform.getLatLng()));
        }
    }
}

TEST(GlobeTransform, MinZoomFollowsLatitude) {
    Transform transform;
    setUpGlobe(transform, {0.0, 0.0}, 2.0);
    const TransformState& state = transform.getState();
    EXPECT_DOUBLE_EQ(0.0, state.getMinZoom());
    EXPECT_DOUBLE_EQ(0.0, state.getMinZoomAtLatitude(0.0));
    EXPECT_NEAR(-1.0, state.getMinZoomAtLatitude(60.0), 1e-12);
    EXPECT_NEAR(std::log2(std::cos(util::deg2rad(80.0))), state.getMinZoomAtLatitude(80.0), 1e-12);

    // Zoom requests clamp to the floor at the requested center, not at the equator's.
    transform.jumpTo(CameraOptions().withCenter(LatLng{80.0, 0.0}).withZoom(-5.0));
    EXPECT_NEAR(state.getMinZoomAtLatitude(80.0), transform.getZoom(), 1e-9);
    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 0.0}).withZoom(-1.0));
    EXPECT_DOUBLE_EQ(0.0, transform.getZoom());
    EXPECT_EQ(0, state.getIntegerZoom());

    // Integer zoom never wraps below zero.
    transform.jumpTo(CameraOptions().withCenter(LatLng{80.0, 0.0}).withZoom(-2.0));
    EXPECT_NEAR(-2.0, transform.getZoom(), 1e-9);
    EXPECT_EQ(0, state.getIntegerZoom());

    // Mercator keeps the plain minimum.
    transform.setProjectionDefinition(ProjectionDefinition("mercator"));
    EXPECT_DOUBLE_EQ(state.getMinZoom(), state.getMinZoomAtLatitude(80.0));
    transform.jumpTo(CameraOptions().withCenter(LatLng{80.0, 0.0}).withZoom(-2.0));
    EXPECT_GE(transform.getZoom(), 0.0);
}

TEST(GlobeTransform, ZoomAdjustment) {
    EXPECT_DOUBLE_EQ(0.0, VerticalPerspectiveProjection::zoomAdjustment(20.0, 20.0));
    EXPECT_NEAR(-1.0, VerticalPerspectiveProjection::zoomAdjustment(0.0, 60.0), 1e-12);
    EXPECT_NEAR(1.0, VerticalPerspectiveProjection::zoomAdjustment(60.0, 0.0), 1e-12);
}

// The expected numbers below come from GL JS's own camera (`camera.test.ts`, "globe projection" suites, and its
// `VerticalPerspectiveCameraHelper`) run on the same 512 by 512 viewport with a linear easing.

TEST(GlobeTransform, EaseAdjustsTheZoomToTheNewLatitude) {
    Transform transform;
    setUpGlobeCamera(transform, {0.0, 0.0}, 1.0);
    transform.easeTo(CameraOptions().withCenter(LatLng{40.0, 0.0}), linearSecond());
    runTo(transform, 1.0);
    EXPECT_FALSE(transform.inTransition());
    EXPECT_NEAR(40.0, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(0.61549999962236379, transform.getZoom(), 1e-9);
}

TEST(GlobeTransform, FlyAdjustsTheZoomToTheNewLatitude) {
    Transform transform;
    setUpGlobeCamera(transform, {0.0, 0.0}, 1.0);
    transform.flyTo(CameraOptions().withCenter(LatLng{40.0, 0.0}), linearSecond());
    runTo(transform, 0.5);
    EXPECT_NEAR(20.0, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(0.87493158146277261, transform.getZoom(), 1e-9);
    runTo(transform, 1.0);
    EXPECT_FALSE(transform.inTransition());
    EXPECT_NEAR(40.0, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(0.61549999962236379, transform.getZoom(), 1e-9);
}

TEST(GlobeTransform, EaseKeepsThePlanetSizeAcrossLatitudes) {
    Transform transform;
    setUpGlobeCamera(transform, {0.0, 0.0}, 3.0);
    const double radius = transform.getState().getGlobeRadiusPixels();
    transform.easeTo(CameraOptions().withCenter(LatLng{60.0, 0.0}), linearSecond());
    runTo(transform, 0.5);
    EXPECT_NEAR(30.0, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(0.0, transform.getLatLng().longitude(), 1e-9);
    EXPECT_NEAR(2.7924812503605780, transform.getZoom(), 1e-9);
    EXPECT_NEAR(radius, transform.getState().getGlobeRadiusPixels(), 1e-6);
    runTo(transform, 1.0);
    EXPECT_NEAR(60.0, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(2.0, transform.getZoom(), 1e-9);
    EXPECT_NEAR(radius, transform.getState().getGlobeRadiusPixels(), 1e-6);
}

TEST(GlobeTransform, EaseFollowsTheLongitudeSpeedCurve) {
    Transform transform;
    setUpGlobeCamera(transform, {0.0, 0.0}, 3.0);
    transform.easeTo(CameraOptions().withCenter(LatLng{60.0, 60.0}), linearSecond());
    runTo(transform, 0.5);
    EXPECT_NEAR(25.026136930423718, transform.getLatLng().longitude(), 1e-9);
    EXPECT_NEAR(30.0, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(2.7924812503605780, transform.getZoom(), 1e-9);
    runTo(transform, 1.0);
    EXPECT_NEAR(60.0, transform.getLatLng().longitude(), 1e-9);
    EXPECT_NEAR(60.0, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(2.0, transform.getZoom(), 1e-9);
}

TEST(GlobeTransform, EaseWithZoomSpeedsUpTheCenter) {
    Transform transform;
    setUpGlobeCamera(transform, {0.0, 0.0}, 3.0);
    transform.easeTo(CameraOptions().withCenter(LatLng{60.0, 0.0}).withZoom(5.0), linearSecond());
    runTo(transform, 0.5);
    EXPECT_NEAR(42.426406871192853, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(4.0619751435424885, transform.getZoom(), 1e-9);
    runTo(transform, 1.0);
    EXPECT_NEAR(60.0, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(5.0, transform.getZoom(), 1e-9);
}

TEST(GlobeTransform, FlyFollowsTheGlobePath) {
    {
        Transform transform;
        setUpGlobeCamera(transform, {0.0, 0.0}, 3.0);
        transform.flyTo(CameraOptions().withCenter(LatLng{60.0, 60.0}), linearSecond());
        runTo(transform, 0.5);
        EXPECT_NEAR(25.026136930425196, transform.getLatLng().longitude(), 1e-9);
        EXPECT_NEAR(30.0, transform.getLatLng().latitude(), 1e-8);
        EXPECT_NEAR(1.8176209209717085, transform.getZoom(), 1e-9);
        runTo(transform, 1.0);
        EXPECT_NEAR(60.0, transform.getLatLng().longitude(), 1e-8);
        EXPECT_NEAR(60.0, transform.getLatLng().latitude(), 1e-8);
        EXPECT_NEAR(2.0, transform.getZoom(), 1e-9);
    }
    {
        Transform transform;
        setUpGlobeCamera(transform, {0.0, 0.0}, 3.0);
        transform.flyTo(CameraOptions().withCenter(LatLng{60.0, 60.0}).withZoom(6.0), linearSecond());
        runTo(transform, 0.5);
        EXPECT_NEAR(54.663652625664554, transform.getLatLng().longitude(), 1e-9);
        EXPECT_NEAR(56.470588235293114, transform.getLatLng().latitude(), 1e-9);
        EXPECT_NEAR(2.4045752372957860, transform.getZoom(), 1e-9);
        runTo(transform, 1.0);
        EXPECT_NEAR(60.0, transform.getLatLng().longitude(), 1e-8);
        EXPECT_NEAR(60.0, transform.getLatLng().latitude(), 1e-8);
        EXPECT_NEAR(6.0, transform.getZoom(), 1e-9);
    }
}

TEST(GlobeTransform, FlyStaysAboveTheMinimumZoom) {
    Transform transform;
    setUpGlobeCamera(transform, {0.0, 0.0}, 20.0);
    AnimationOptions animation = linearSecond();
    animation.minZoom = 10.0;
    transform.flyTo(CameraOptions().withCenter(LatLng{0.0, 1.0}).withZoom(20.0), animation);
    double lowest = 20.0;
    for (int frame = 1; frame <= 10; ++frame) {
        runTo(transform, frame / 10.0);
        lowest = std::min(lowest, transform.getZoom());
    }
    // GL JS bottoms out at 9.9999993 on the same frames.
    EXPECT_GE(lowest, 10.0 - 1e-5);
    EXPECT_LE(lowest, 10.01);
    EXPECT_NEAR(20.0, transform.getZoom(), 1e-9);
}

TEST(GlobeTransform, CameraForBoundsFitsTheGlobe) {
    Transform transform;
    setUpGlobeCamera(transform, {0.0, 0.0}, 0.0);
    const LatLngBounds bounds = LatLngBounds::hull({16.0, -133.0}, {50.0, -68.0});
    const std::vector<LatLng> corners = {
        bounds.northwest(), bounds.southwest(), bounds.southeast(), bounds.northeast()};

    // The flat fit the globe starts from is 1.4e-8 zoom from GL JS's 2.4694852833012204 on Mercator already, and the
    // globe fit inherits that; the zooms below are GL JS's to within 1.6e-8.
    {
        Transform flat;
        flat.resize({512, 512});
        flat.jumpTo(CameraOptions().withCenter(LatLng{0.0, 0.0}).withZoom(0.0));
        const CameraOptions flatFit = cameraForLatLngs(corners, flat, {});
        ASSERT_TRUE(flatFit.zoom);
        EXPECT_NEAR(2.4694852833012204, *flatFit.zoom, 1e-7);
    }

    CameraOptions fit = cameraForLatLngs(corners, transform, {});
    ASSERT_TRUE(fit.center && fit.zoom);
    EXPECT_NEAR(-100.5, fit.center->longitude(), 1e-9);
    EXPECT_NEAR(34.717077774070773, fit.center->latitude(), 1e-9);
    EXPECT_NEAR(2.4964711163134550, *fit.zoom, 1e-7);

    fit = cameraForLatLngs(corners, transform, {15.0, 15.0, 15.0, 15.0});
    ASSERT_TRUE(fit.center && fit.zoom);
    EXPECT_NEAR(-100.5, fit.center->longitude(), 1e-9);
    EXPECT_NEAR(34.717077774070773, fit.center->latitude(), 1e-9);
    EXPECT_NEAR(2.3985253272853888, *fit.zoom, 1e-7);

    transform.jumpTo(CameraOptions().withBearing(45.0));
    fit = cameraForLatLngs(corners, transform, {});
    ASSERT_TRUE(fit.center && fit.zoom);
    EXPECT_NEAR(-100.5, fit.center->longitude(), 1e-9);
    EXPECT_NEAR(34.717077774070773, fit.center->latitude(), 1e-9);
    EXPECT_NEAR(2.4499189488903967, *fit.zoom, 1e-7);
}

TEST(TileProjector, MercatorMatchesTheTileMatrix) {
    Transform transform;
    transform.resize({512, 512});
    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 0.0}).withZoom(1.0).withBearing(30.0).withPitch(20.0));
    const TransformState& state = transform.getState();
    ASSERT_FALSE(state.isGlobeRendering());

    // The bottom-right corner of tile 1/0/0 is lat 0, lng 0: the map center.
    const UnwrappedTileID tileID(1, 0, 0);
    const TileProjector projector(state, tileID);
    const Point<double> corner{util::EXTENT, util::EXTENT};
    vec4 pos = {{corner.x, corner.y, 0, 1}};
    matrix::transformMat4(pos, pos, state.getProjectionData(tileID).mainMatrix);

    const auto projected = projector.project(corner);
    EXPECT_NEAR(pos[0] / pos[3], projected.point.x, 1e-9);
    EXPECT_NEAR(pos[1] / pos[3], projected.point.y, 1e-9);
    EXPECT_NEAR(0.0, projected.point.x, 1e-6);
    EXPECT_NEAR(0.0, projected.point.y, 1e-6);
    EXPECT_DOUBLE_EQ(pos[3], projected.signedDistanceFromCamera);
    EXPECT_FALSE(projected.occluded);
    EXPECT_FALSE(projector.project({0.0, 0.0}).occluded);
    EXPECT_DOUBLE_EQ(1.0, projector.circleRadiusCorrection());
    EXPECT_DOUBLE_EQ(1.0, projector.pitchedTextCorrection({100.0, 100.0}));
    EXPECT_DOUBLE_EQ(1.0, state.getProjection().pixelScale(state));
}

TEST(TileProjector, GlobeOccludesTheFarSideAndCorrectsForLatitude) {
    Transform transform;
    setUpGlobe(transform, {40.0, 0.0}, 1.0);
    const TransformState& state = transform.getState();
    const UnwrappedTileID tileID(1, 0, 0);
    const TileProjector projector(state, tileID);

    // The bottom-right corner (lat 0, lng 0) faces the camera; the bottom-left one (lng -180, lat 0) is behind it.
    EXPECT_FALSE(projector.project({util::EXTENT, util::EXTENT}).occluded);
    EXPECT_TRUE(projector.project({0.0, util::EXTENT}).occluded);

    const double cos40 = std::cos(util::deg2rad(40.0));
    EXPECT_NEAR(cos40, projector.circleRadiusCorrection(), 1e-12);
    EXPECT_NEAR(1.0 / cos40, state.getProjection().pixelScale(state), 1e-12);
    // Tile y of latitude 60 in tile 1/0/0.
    const double tileY = Projection::project(LatLng{60.0, 0.0}, 2.0).y / util::tileSize_D * util::EXTENT;
    EXPECT_NEAR(cos40 / std::cos(util::deg2rad(60.0)), projector.pitchedTextCorrection({100.0, tileY}), 1e-6);
    EXPECT_NEAR(cos40, projector.pitchedTextCorrection({100.0, util::EXTENT}), 1e-6);
}

TEST(Transform, GlobeLocationOcclusion) {
    Transform transform;
    setUpGlobe(transform, {0, 0}, 1);
    EXPECT_FALSE(transform.isLocationOccluded({0, 0}));
    EXPECT_FALSE(transform.isLocationOccluded({0, 60}));
    EXPECT_FALSE(transform.isLocationOccluded({-60, 0}));
    EXPECT_TRUE(transform.isLocationOccluded({0, 100}));
    EXPECT_TRUE(transform.isLocationOccluded({0, 180}));
    EXPECT_TRUE(transform.isLocationOccluded({-30, -120}));

    setUpGlobe(transform, {0, 180}, 1);
    EXPECT_TRUE(transform.isLocationOccluded({0, 0}));
    EXPECT_FALSE(transform.isLocationOccluded({0, 180}));

    transform.setProjectionDefinition(ProjectionDefinition("mercator"));
    EXPECT_FALSE(transform.isLocationOccluded({0, 0}));

    Transform unsized;
    unsized.setProjectionDefinition(ProjectionDefinition("vertical-perspective"));
    EXPECT_FALSE(unsized.isLocationOccluded({0, 180}));
}

TEST(Transform, GlobeCenterLongitudeStaysContinuous) {
    Transform transform;
    setUpGlobe(transform, {0, 179}, 1);
    transform.jumpTo(CameraOptions().withCenter(LatLng{0, -179}));
    EXPECT_NEAR(181, transform.getLatLng(LatLng::Unwrapped).longitude(), 1e-9);
    EXPECT_NEAR(-179, transform.getLatLng(LatLng::Wrapped).longitude(), 1e-9);
    transform.moveBy({-40, 0});
    EXPECT_GT(transform.getLatLng(LatLng::Unwrapped).longitude(), 181);
    transform.jumpTo(CameraOptions().withCenter(LatLng{0, 179}));
    EXPECT_NEAR(179, transform.getLatLng(LatLng::Unwrapped).longitude(), 1e-9);

    // Mercator keeps its wrapped center.
    transform.setProjectionDefinition(ProjectionDefinition("mercator"));
    transform.jumpTo(CameraOptions().withCenter(LatLng{0, -179}));
    EXPECT_NEAR(-179, transform.getLatLng(LatLng::Unwrapped).longitude(), 1e-9);
}
