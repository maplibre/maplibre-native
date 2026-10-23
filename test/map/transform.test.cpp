#include <mln/test/util.hpp>

#include <gmock/gmock.h>
#include <array>
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
#include <tuple>

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

TEST(Transform, PaddingPreservesMinimumPitch) {
    for (const uint32_t size : {1u, 64u}) {
        for (const double minPitch : {0.0, 15.0}) {
            SCOPED_TRACE(::testing::Message() << "size=" << size << ", minPitch=" << minPitch);
            Transform transform;
            transform.resize({size, size});
            transform.setMinPitch(minPitch);
            transform.setMaxPitch(60);
            transform.jumpTo(CameraOptions().withCenter(LatLng{40.7128, -74.006}).withZoom(9.5).withPitch(minPitch));

            transform.jumpTo(CameraOptions().withPadding(EdgeInsets{size * 28.0, 392, 0, 0}));
            EXPECT_DOUBLE_EQ(util::deg2rad(minPitch), transform.getPitch());

            transform.jumpTo(CameraOptions().withPadding(EdgeInsets{size * 24.0, 392, 0, 0}));
            EXPECT_DOUBLE_EQ(util::deg2rad(minPitch), transform.getPitch());
            EXPECT_DOUBLE_EQ(size * 24.0, transform.getState().getEdgeInsets().top());

            transform.resize({800, 600});
            EXPECT_DOUBLE_EQ(util::deg2rad(minPitch), transform.getPitch());
            transform.jumpTo(CameraOptions().withPadding(EdgeInsets{}).withPitch(45));
            EXPECT_DOUBLE_EQ(util::deg2rad(45), transform.getPitch());
        }
    }
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

TEST(Transform, CameraFollowsJumpsBelowAFractionalMinimumZoom) {
    const double mercatorY = 0.5 - std::log(std::tan(pi / 4 + util::deg2rad(10.0) / 2)) / (2 * pi);
    for (const double minZoom : {1.525373023184723, 1.55, 3.1, 6.2}) {
        Transform transform;
        transform.resize({512, 512});
        transform.setMinZoom(minZoom);
        transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 0.0}).withZoom(minZoom + 2.0));
        transform.jumpTo(CameraOptions().withCenter(LatLng{10.0, 20.0}).withZoom(0.0));
        EXPECT_TRUE(transform.getState().valid()) << minZoom;
        const vec3 position = transform.getFreeCameraOptions().position.value();
        EXPECT_NEAR(200.0 / 360.0, position[0], 1e-12) << minZoom;
        EXPECT_NEAR(mercatorY, position[1], 1e-12) << minZoom;
    }
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
                        const double scale = state.getScale();

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

TEST(Transform, GlobeProjectionDefinition) {
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

// A transition past either end, from a literal or an overshooting curve, holds at that end's projection.
TEST(Transform, GlobeProjectionTransitionPastEitherEnd) {
    Transform transform;
    transform.resize({512, 512});
    transform.setProjectionDefinition(ProjectionDefinition("mercator", "vertical-perspective", 1.5));
    EXPECT_DOUBLE_EQ(1.0, transform.getState().getProjectionTransition());
    transform.jumpTo(CameraOptions().withZoom(1));
    for (int step = 1; step <= 3; ++step) {
        transform.jumpTo(CameraOptions().withCenter(LatLng{0, 40.0 + 120.0 * step}));
    }
    ASSERT_NEAR(400, transform.getState().getLatLng(LatLng::Unwrapped).longitude(), 1e-9);
    // Past Mercator the globe leaves as it does at 0, so the center wraps back into Mercator's world.
    transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective", "mercator", 1.5));
    EXPECT_FALSE(transform.getState().isGlobeRendering());
    EXPECT_DOUBLE_EQ(0.0, transform.getState().getProjectionTransition());
    EXPECT_NEAR(40, transform.getState().getLatLng(LatLng::Unwrapped).longitude(), 1e-9);
}

// An animation that crosses into the globe ends where a jump to its camera would, at a zoom only the globe allows.
TEST(Transform, AnimationIntoTheGlobeTakesTheGlobesMinimumZoom) {
    const auto projectionForZoom = [](double zoom) {
        return ProjectionDefinition(zoom < 3 ? "vertical-perspective" : "mercator");
    };
    // A viewport taller than the world raises Mercator's minimum zoom, which the globe does not have.
    for (const Size size : {Size{512, 512}, Size{512, 1024}}) {
        for (const auto& [center, zoom] : {std::pair{LatLng{60, 0}, -0.5}, std::pair{LatLng{0, 0}, 0.0}}) {
            for (const bool fly : {false, true}) {
                SCOPED_TRACE(std::string(fly ? "flyTo" : "easeTo") + " to zoom " + std::to_string(zoom) + " at " +
                             std::to_string(center.latitude()) + " on " + std::to_string(size.height) + " px");
                Transform transform;
                transform.resize(size);
                transform.setProjectionForZoom(projectionForZoom);
                transform.setProjectionDefinition(projectionForZoom(5));
                transform.jumpTo(CameraOptions().withCenter(center).withZoom(5));
                const auto camera = CameraOptions().withCenter(center).withZoom(zoom);
                const AnimationOptions animation(Milliseconds(1000));
                fly ? transform.flyTo(camera, animation) : transform.easeTo(camera, animation);
                // As the map does each frame, the projection follows the zoom.
                for (int frame = 1; frame <= 20; ++frame) {
                    transform.updateTransitions(transform.getTransitionStart() + Milliseconds(50 * frame));
                    transform.setProjectionDefinition(projectionForZoom(transform.getZoom()));
                }
                EXPECT_TRUE(transform.getState().isGlobeRendering());
                EXPECT_NEAR(zoom, transform.getZoom(), 1e-9);
            }
        }
    }
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

// Near the limb the zoom around a point leans on GL JS's damped heuristic, which adds up step by step: an animated
// zoom moves the center about as far as the same zoom in one jump.
TEST(GlobeTransform, AnimatedZoomAroundAPointNearTheLimbMovesLikeAJump) {
    const ScreenCoordinate anchor{256 + 159, 256};
    Transform jumped;
    setUpGlobeCamera(jumped, {0.0, 0.0}, 1.0);
    jumped.jumpTo(CameraOptions().withZoom(2.0).withAnchor(anchor));
    const double jumpMove = jumped.getLatLng().longitude();
    ASSERT_GT(jumpMove, 1.0);

    Transform animated;
    setUpGlobeCamera(animated, {0.0, 0.0}, 1.0);
    animated.easeTo(CameraOptions().withZoom(2.0).withAnchor(anchor), linearSecond());
    for (int frame = 1; frame <= 10; ++frame) {
        runTo(animated, frame / 10.0);
    }
    EXPECT_NEAR(2.0, animated.getZoom(), 1e-9);
    const double animatedMove = animated.getLatLng().longitude();
    EXPECT_GT(animatedMove, 0.75 * jumpMove);
    EXPECT_LT(animatedMove, 1.25 * jumpMove);
}

// An animation around an anchor ends where the same change in one jump does, zoom included: the planet keeps its
// size as the anchor carries the center to another latitude.
TEST(GlobeTransform, AnimationsAroundAnAnchorEndLikeAJump) {
    const ScreenCoordinate anchor{256, 256 - 150};
    for (const bool rotate : {false, true}) {
        SCOPED_TRACE(rotate ? "rotate" : "zoom");
        const CameraOptions change = rotate ? CameraOptions().withBearing(60.0).withAnchor(anchor)
                                            : CameraOptions().withZoom(4.0).withAnchor(anchor);
        Transform jumped;
        setUpGlobeCamera(jumped, {45.0, 0.0}, 3.0);
        jumped.jumpTo(change);
        ASSERT_GT(std::abs(jumped.getLatLng().latitude() - 45.0), 0.5);

        Transform animated;
        setUpGlobeCamera(animated, {45.0, 0.0}, 3.0);
        animated.easeTo(change, linearSecond());
        for (int frame = 1; frame <= 10; ++frame) {
            runTo(animated, frame / 10.0);
        }
        EXPECT_NEAR(jumped.getLatLng().latitude(), animated.getLatLng().latitude(), 1e-6);
        EXPECT_NEAR(jumped.getLatLng().longitude(), animated.getLatLng().longitude(), 1e-6);
        EXPECT_NEAR(jumped.getZoom(), animated.getZoom(), 1e-6);
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

    // Past the Mercator range the center goes as far toward the pole as the zoom it lands on allows, and the zoom
    // follows the edge of the range: the planet keeps its size. A drag over the pole stops where the zoom lets the
    // center go. GL JS: 89 at zoom 1.9907089805355933, then 89.50632407943635.
    const double radiusBefore = transform.getState().getGlobeRadiusPixels();
    transform.jumpTo(CameraOptions().withCenter(LatLng{89.0, 0.0}));
    EXPECT_NEAR(89.0, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(1.9907089805355933, transform.getZoom(), 1e-6);
    EXPECT_NEAR(radiusBefore, transform.getState().getGlobeRadiusPixels(), 1e-6);
    transform.moveBy({0.0, 300.0});
    EXPECT_NEAR(89.50632407943635, transform.getLatLng().latitude(), 1e-6);
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

TEST(GlobeTransform, CameraFollowsTheGlobeBelowTheMinimumScale) {
    Transform transform;
    setUpGlobe(transform, {80.0, 0.0}, 8.0);
    transform.setMinZoom(6.0);
    transform.jumpTo(CameraOptions().withCenter(LatLng{80.0, 90.0}).withZoom(0.0));
    EXPECT_NEAR(transform.getState().getMinZoomAtLatitude(80.0), transform.getZoom(), 1e-9);
    EXPECT_LT(transform.getZoom(), 6.0);
    EXPECT_TRUE(transform.getState().valid());
    const vec3 position = transform.getFreeCameraOptions().position.value();
    EXPECT_NEAR(0.75, position[0], 1e-12);
    EXPECT_NEAR(0.5 - std::log(std::tan(pi / 4 + util::deg2rad(80.0) / 2)) / (2 * pi), position[1], 1e-12);
}

TEST(GlobeTransform, ZoomAdjustment) {
    EXPECT_DOUBLE_EQ(0.0, VerticalPerspectiveProjection::zoomAdjustment(20.0, 20.0));
    EXPECT_NEAR(-1.0, VerticalPerspectiveProjection::zoomAdjustment(0.0, 60.0), 1e-12);
    EXPECT_NEAR(1.0, VerticalPerspectiveProjection::zoomAdjustment(60.0, 0.0), 1e-12);
    // Past the Mercator range the globe keeps its size at the edge of the range.
    EXPECT_EQ(0.0, VerticalPerspectiveProjection::zoomAdjustment(util::LATITUDE_MAX, 90.0));
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

// Bounds fitted with uneven padding fill the padded part of the view the camera shows: every corner and edge
// midpoint lands inside it and the nearest touches its edge.
TEST(GlobeTransform, CameraForBoundsFitsThePaddedView) {
    Transform transform;
    setUpGlobeCamera(transform, {0.0, 0.0}, 0.0);
    const LatLngBounds bounds = LatLngBounds::hull({16.0, -133.0}, {50.0, -68.0});
    const std::vector<LatLng> corners = {
        bounds.northwest(), bounds.southwest(), bounds.southeast(), bounds.northeast()};
    const double midLatitude = (bounds.north() + bounds.south()) / 2.0;
    const double midLongitude = (bounds.west() + bounds.east()) / 2.0;
    const std::vector<LatLng> samples = {bounds.northwest(),
                                         bounds.southwest(),
                                         bounds.southeast(),
                                         bounds.northeast(),
                                         {midLatitude, bounds.west()},
                                         {midLatitude, bounds.east()},
                                         {bounds.north(), midLongitude},
                                         {bounds.south(), midLongitude}};

    for (const EdgeInsets& padding : {EdgeInsets{0, 0, 200, 0}, EdgeInsets{150, 60, 0, 0}}) {
        SCOPED_TRACE(::testing::Message()
                     << padding.top() << " " << padding.left() << " " << padding.bottom() << " " << padding.right());
        const CameraOptions fit = cameraForLatLngs(corners, transform, padding);
        ASSERT_TRUE(fit.center && fit.zoom);
        Transform shown(transform.getState());
        shown.jumpTo(fit);
        const TransformState& state = shown.getState();
        const double left = padding.left() / 512.0 * 2.0 - 1.0;
        const double right = (512.0 - padding.right()) / 512.0 * 2.0 - 1.0;
        const double top = padding.top() / 512.0 * -2.0 + 1.0;
        const double bottom = (512.0 - padding.bottom()) / 512.0 * -2.0 + 1.0;
        double nearest = std::numeric_limits<double>::infinity();
        for (const LatLng& sample : samples) {
            const vec3 surface = VerticalPerspectiveProjection::surfaceVector(sample);
            vec4 clip = {{surface[0], surface[1], surface[2], 1.0}};
            matrix::transformMat4(clip, clip, state.getGlobeViewProjectionMatrix());
            const double x = clip[0] / clip[3];
            const double y = clip[1] / clip[3];
            nearest = std::min({nearest, x - left, right - x, top - y, y - bottom});
        }
        EXPECT_NEAR(0.0, nearest, 1e-3);
    }
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

// A query on the globe is padded with GL JS's Mercator pitch factor, taken at the screen corner's unclamped Mercator
// position: the values are GL JS's `GlobeTransform.maxPitchScaleFactor()` for the same views.
// Map-aligned circles, pitched text and line widths blend their globe size with Mercator's through the projection
// transition: the values are GL JS's `GlobeTransform` corrections for the same center and transition.
TEST(Transform, GlobeSizeCorrectionsBlendThroughTheTransition) {
    struct Case {
        double latitude;
        double transition;
        double circle;
        double pixel;
        double pitched;
    };
    for (const auto& c :
         {Case{.latitude = 52.499167,
               .transition = 0.5,
               .circle = 0.8043864815927202,
               .pixel = 1.321324254256826,
               .pitched = 1.2637600088069},
          Case{.latitude = 60, .transition = 0.25, .circle = 0.875, .pixel = 1.25, .pitched = 1.0636473098322572},
          Case{.latitude = 60, .transition = 1, .circle = 0.5, .pixel = 2, .pitched = 1.2545892393290283},
          Case{.latitude = 0, .transition = 0.5, .circle = 1, .pixel = 1, .pitched = 1.7545892393290279}}) {
        Transform transform;
        transform.resize({256, 256});
        transform.setProjectionDefinition(
            ProjectionDefinition(ProjectionType::Mercator, ProjectionType::VerticalPerspective, c.transition));
        transform.jumpTo(CameraOptions().withCenter(LatLng{c.latitude, 13.418056}).withZoom(11.5));
        const auto& state = transform.getState();
        const auto& projection = state.getProjection();
        EXPECT_NEAR(c.circle, projection.circleRadiusCorrection(state), 1e-9);
        EXPECT_NEAR(c.pixel, projection.pixelScale(state), 1e-9);
        // An anchor a quarter of the way down the zoom 0 tile, at 66.5 degrees north.
        EXPECT_NEAR(c.pitched, projection.pitchedTextCorrection(state, {4096, 2048}, UnwrappedTileID(0, 0, 0)), 1e-9);
    }
}

TEST(Transform, GlobeMaxPitchScaleFactor) {
    struct View {
        LatLng center;
        double zoom;
        double pitch;
        double factor;
    };
    for (const auto& view : {View{.center = {20, 30}, .zoom = 1, .pitch = 50, .factor = 1.659065925812732},
                             View{.center = {0, 0}, .zoom = 1, .pitch = 60, .factor = 2.3660254037842425},
                             View{.center = {20, 30}, .zoom = 2, .pitch = 50, .factor = 1.6590659258122578}}) {
        Transform transform;
        transform.resize({512, 512});
        transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective"));
        transform.jumpTo(CameraOptions().withCenter(view.center).withZoom(view.zoom).withPitch(view.pitch));
        EXPECT_NEAR(view.factor, transform.getState().maxPitchScaleFactor(), 1e-4);
    }
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

// Leaving the globe applies Mercator's limits again: its lowest zoom for the view's height and a wrapped center.
TEST(Transform, LeavingTheGlobeConstrainsTheCamera) {
    Transform transform;
    transform.resize({400, 800});
    transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective"));
    transform.jumpTo(CameraOptions().withCenter(LatLng{0, 0}).withZoom(0.3));
    transform.jumpTo(CameraOptions().withCenter(LatLng{0, 120}));
    transform.jumpTo(CameraOptions().withCenter(LatLng{0, -150}));
    ASSERT_NEAR(210, transform.getLatLng(LatLng::Unwrapped).longitude(), 1e-9);
    ASSERT_NEAR(0.3, transform.getZoom(), 1e-9);

    transform.setProjectionDefinition(ProjectionDefinition("mercator"));
    EXPECT_NEAR(std::log2(800.0 / 512.0), transform.getZoom(), 1e-9);
    EXPECT_NEAR(-150, transform.getLatLng(LatLng::Unwrapped).longitude(), 1e-9);
}

// Bounds across the antimeridian keep a globe center past 180 degrees, which the globe hands over wrapped.
TEST(Transform, GlobeBoundsAcrossTheAntimeridian) {
    Transform transform;
    setUpGlobe(transform, {0, 150}, 3);
    transform.setLatLngBounds(LatLngBounds::hull({-10.0, 120.0}, {10.0, 240.0}));
    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 175.0}));
    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, -175.0}));
    EXPECT_NEAR(185.0, transform.getLatLng(LatLng::Unwrapped).longitude(), 1e-9);

    // A drag east across the antimeridian.
    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 179.0}));
    transform.moveBy({-100, 0});
    EXPECT_GT(transform.getLatLng(LatLng::Unwrapped).longitude(), 180.0);
    EXPECT_LT(transform.getLatLng(LatLng::Unwrapped).longitude(), 240.0);
}

// The camera the platforms read carries the wrapped center, whichever copy of it the globe keeps.
// Bounds across the antimeridian keep the center in their own range, past 180 degrees: the camera reports it there,
// so jumping back to the reported camera does not move the map, on either projection.
TEST(Transform, CameraRoundTripsInsideBoundsAcrossTheAntimeridian) {
    for (const bool globe : {false, true}) {
        SCOPED_TRACE(globe ? "globe" : "mercator");
        Transform transform;
        transform.resize({800, 600});
        if (globe) {
            transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective"));
        }
        transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 180.0}).withZoom(5.0));
        transform.setLatLngBounds(LatLngBounds::hull({-10.0, 170.0}, {10.0, 190.0}));
        transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 185.0}));
        ASSERT_NEAR(185.0, transform.getLatLng(LatLng::Unwrapped).longitude(), 1e-9);
        transform.jumpTo(transform.getCameraOptions(std::nullopt));
        EXPECT_NEAR(185.0, transform.getLatLng(LatLng::Unwrapped).longitude(), 1e-9);
        if (globe) {
            transform.setProjectionDefinition(ProjectionDefinition("mercator"));
            EXPECT_NEAR(185.0, transform.getLatLng(LatLng::Unwrapped).longitude(), 1e-9);
        }
    }
}

TEST(Transform, GlobeCameraCenterIsWrapped) {
    Transform transform;
    setUpGlobe(transform, {0, 179}, 1);
    transform.jumpTo(CameraOptions().withCenter(LatLng{0, -179}));
    ASSERT_NEAR(181, transform.getLatLng(LatLng::Unwrapped).longitude(), 1e-9);
    EXPECT_NEAR(-179, transform.getCameraOptions(std::nullopt).center->longitude(), 1e-9);
}

// The expected centers are GL JS's inertia (handlePanInertia) for the same transforms: a fling turns the planet by
// degrees per pixel at the center's latitude, and the zoom follows the latitude to keep the planet's size.
TEST(Transform, GlobeFlingEndsAtGLJSInertiaCenter) {
    struct Pan {
        LatLng center;
        double zoom;
        double bearing;
        ScreenCoordinate offset;
        LatLng expected;
    };
    for (const auto& pan : {Pan{{30, -20}, 4, 30, {120, -80}, {29.64674667013475, -26.3247433402695}},
                            Pan{{-45, 170}, 8, -100, {-50, 25}, {-44.91280009391755, 169.90853166980656}},
                            Pan{{70, 0}, 5, 0, {200, 0}, {70, -4.23757815424085}}}) {
        SCOPED_TRACE("zoom " + std::to_string(pan.zoom));
        Transform transform;
        setUpGlobe(transform, pan.center, pan.zoom, pan.bearing);
        transform.moveBy(pan.offset, linearSecond());
        transform.updateTransitions(transform.getTransitionStart() + Milliseconds(1000));
        EXPECT_NEAR(pan.expected.latitude(), transform.getLatLng().latitude(), 1e-9);
        EXPECT_NEAR(pan.expected.longitude(), transform.getLatLng().longitude(), 1e-9);
        EXPECT_NEAR(
            pan.zoom + VerticalPerspectiveProjection::zoomAdjustment(pan.center.latitude(), pan.expected.latitude()),
            transform.getZoom(),
            1e-9);
    }
}

// A fling toward a pole eases along its own meridian, as GL JS's inertia does, as far toward the pole as the zoom it
// starts from lets the center go: onto the pole from zoom 1.5 on this 800 by 600 view, short of it from zoom 3.
// Moving the location under the end of the fling to the center took it over the pole, and the planet spun as it
// bounced back.
TEST(Transform, GlobeFlingTowardAPoleStaysOnItsMeridian) {
    struct Fling {
        double zoom;
        std::array<std::pair<double, double>, 2> halves; // GL JS's latitude and zoom halfway and at the end
    };
    for (const auto& fling :
         {Fling{1.5, {{{75, 0.5500156865235035}, {90, -1.0350518437681169}}}},
          Fling{3, {{{73.2171601091423, 2.2078682014999638}, {86.43432021828463, 0.4649481562318831}}}}}) {
        SCOPED_TRACE("zoom " + std::to_string(fling.zoom));
        Transform transform;
        setUpGlobe(transform, {60, 10}, fling.zoom);
        transform.moveBy({0, 3000}, linearSecond());
        double latitude = 60;
        for (int frame = 1; frame <= 20; ++frame) {
            transform.updateTransitions(transform.getTransitionStart() + Milliseconds(50 * frame));
            EXPECT_NEAR(10, transform.getLatLng().longitude(), 1e-9);
            EXPECT_GE(transform.getLatLng().latitude(), latitude - 1e-9);
            latitude = transform.getLatLng().latitude();
            if (frame % 10 == 0) {
                const auto& [expectedLatitude, expectedZoom] = fling.halves[(frame / 10) - 1];
                EXPECT_NEAR(expectedLatitude, latitude, 1e-6);
                EXPECT_NEAR(expectedZoom, transform.getZoom(), 1e-6);
            }
        }
    }
}

// A fling worth more than half a turn stops at 179.5° the way it was going, like GL JS's inertia, rather than easing
// the shorter way round, against the fling.
TEST(Transform, GlobeFlingStopsShortOfHalfATurn) {
    Transform transform;
    setUpGlobe(transform, {0, 0}, 0.5);
    transform.moveBy({-5000, 0}, linearSecond());
    double longitude = 0;
    for (int frame = 1; frame <= 20; ++frame) {
        transform.updateTransitions(transform.getTransitionStart() + Milliseconds(50 * frame));
        EXPECT_GE(transform.getLatLng(LatLng::Unwrapped).longitude(), longitude - 1e-9);
        longitude = transform.getLatLng(LatLng::Unwrapped).longitude();
    }
    EXPECT_NEAR(179.5, longitude, 1e-9);
    EXPECT_NEAR(0, transform.getLatLng().latitude(), 1e-9);
}

// GL JS's drag (`versorSetLocationAtPoint` in globe_utils.ts at a6976b2d2) on a 640 by 480 vertical perspective
// globe, one step at a time from the state GL JS had before it. GL JS builds the step's rotation with
// `quat.fromValues`, a Float32Array, which near a pole moves its center by up to 2e-3 degrees a step; the expected
// centers are GL JS's drag run in doubles. The location under a pixel differs from GL JS's by a few times 1e-8
// degrees, which a step carries.
TEST(GlobeTransform, DragCenterMatchesGLJS) {
    struct Step {
        const char* name;
        LatLng center;
        double zoom;
        double bearing;
        double pitch;
        ScreenCoordinate point;
        ScreenCoordinate panDelta;
        LatLng expected;
    };
    for (const auto& step : {
             Step{"center-drag-east 0", {20, 10}, 1, 0, 0, {330, 240}, {10, 0}, {19.96528220871278, 6.48152888184427}},
             Step{"center-drag-east 2",
                  {19.93062984381388, 2.96383097806563},
                  1.0006345588283656,
                  0,
                  0,
                  {330, 240},
                  {10, 0},
                  {19.89604275335306, -0.5530967909885476}},
             Step{"center-drag-east 4",
                  {19.86152078591744, -4.06925749079096},
                  1.0012643498868492,
                  0,
                  0,
                  {330, 240},
                  {10, 0},
                  {19.8270637906032, -7.584654172731689}},
             Step{"off-center-drag 2",
                  {16.074405186537916, 4.212860800653175},
                  1.0322017912167816,
                  0,
                  0,
                  {400, 180},
                  {6, -4},
                  {14.115311037321156, 1.3609496277557582}},
             Step{"off-center-drag 4",
                  {12.158623543717447, -1.4683004008273883},
                  1.057008507859097,
                  0,
                  0,
                  {400, 180},
                  {6, -4},
                  {10.204304619531165, -4.278521558340458}},
             Step{"drag-north-into-pole 0",
                  {70, -100},
                  1,
                  0,
                  0,
                  {320, 300},
                  {0, 6},
                  {70.73728720083854, -100.00000000000205}},
             Step{"drag-north-into-pole 20",
                  {84.74574401678998, -100.00000000000068},
                  -0.9010463352914224,
                  0,
                  0,
                  {320, 300},
                  {0, 6},
                  {85.48303121762915, -100.00000000000068}},
             Step{"drag-north-into-pole 21",
                  {85.48303121762915, -100.00000000000068},
                  -0.9872050442266238,
                  0,
                  0,
                  {320, 300},
                  {0, 6},
                  {86.22031841846909, -100.00000000000068}},
             Step{"drag-north-into-pole 27",
                  {89.90675442266692, -100.00000000000068},
                  -0.9872050442266238,
                  0,
                  0,
                  {320, 300},
                  {0, 6},
                  {90, -100.00000000000068}},
             Step{"drag-north-into-pole 28",
                  {90, -100.00000000000068},
                  -0.9872050442266238,
                  0,
                  0,
                  {320, 300},
                  {0, 6},
                  {90, -100.00000000000068}},
             Step{"side-drag-at-pole 0",
                  {85.0511287798066, -100},
                  1,
                  0,
                  0,
                  {380, 240},
                  {6, 0},
                  {85.0477656113963, -101.87137817590371}},
             Step{"side-drag-at-pole 30",
                  {84.95121303919578, -155.737980622539},
                  1.0287640984336475,
                  0,
                  0,
                  {380, 240},
                  {6, 0},
                  {84.94791671962916, -157.58177081818087}},
             Step{"side-drag-at-pole 59",
                  {84.85648183647973, 151.15114974424728},
                  1.0555120380074723,
                  0,
                  0,
                  {380, 240},
                  {6, 0},
                  {84.85324650692623, 149.33280710271083}},
             Step{"side-drag-centred-pole 0",
                  {89, 0},
                  0.5,
                  0,
                  0,
                  {340, 200},
                  {0, 8},
                  {89.34438829057913, 13.508337366700857}},
             Step{"side-drag-centred-pole 2",
                  {89.68877639444831, 22.57677926378733},
                  0.5,
                  0,
                  0,
                  {340, 200},
                  {0, 8},
                  {90, 28.803865724291313}},
             Step{"side-drag-centred-pole 15",
                  {90, 83.80781405685116},
                  0.5,
                  0,
                  0,
                  {340, 200},
                  {0, 8},
                  {90, 88.39147641789782}},
             Step{"side-drag-centred-pole 29",
                  {90, 147.97908711150444},
                  0.5,
                  0,
                  0,
                  {340, 200},
                  {0, 8},
                  {90, 152.5627494725511}},
             Step{"drag-off-globe 0", {10, 5}, 1, 0, 0, {620, 240}, {20, 0}, {9.982236219514714, 1.5494610302675937}},
             Step{"drag-off-globe 2",
                  {9.964504640751409, -1.9008899375490955},
                  1.0001573094988878,
                  0,
                  0,
                  {620, 240},
                  {20, 0},
                  {9.946805201839899, -5.3510535945838456}},
             Step{"drag-off-globe 4",
                  {9.929137841049005, -8.801030629313573},
                  1.0003134815524495,
                  0,
                  0,
                  {620, 240},
                  {20, 0},
                  {9.911502496783827, -12.250821727578682}},
             Step{"drag-bearing-pitch 0",
                  {40, 30},
                  2.5,
                  30,
                  40,
                  {300, 330},
                  {-7, 5},
                  {40.100686468998106, 30.988488451527928}},
             Step{"drag-bearing-pitch 4",
                  {40.40265566873954, 33.96278116325789},
                  2.4914316029001595,
                  30,
                  40,
                  {300, 330},
                  {-7, 5},
                  {40.50328185738783, 34.95717865523261}},
             Step{"drag-bearing-pitch 7",
                  {40.70448865902955, 36.95046737255302},
                  2.4849281889924244,
                  30,
                  40,
                  {300, 330},
                  {-7, 5},
                  {40.80506918481425, 37.94937335237137}},
             Step{"drag-across-antimeridian 0", {0, 175}, 1.5, 0, 0, {300, 240}, {-12, 0}, {0, 177.9946248399193}},
             Step{"drag-across-antimeridian 5",
                  {0, -170.02687580040782},
                  1.5,
                  0,
                  0,
                  {300, 240},
                  {-12, 0},
                  {0, -167.0322509604884}},
             Step{"drag-across-antimeridian 9",
                  {0, -158.0483764407433},
                  1.5,
                  0,
                  0,
                  {300, 240},
                  {-12, 0},
                  {0, -155.0537516008231}},
             Step{"drag-south-pole 0",
                  {-80, 40},
                  0.8,
                  0,
                  0,
                  {320, 200},
                  {0, -7},
                  {-80.49305711248556, 40.00000000000148}},
             Step{"drag-south-pole 10",
                  {-84.93057112485239, 40.00000000000193},
                  -0.17465574390064653,
                  0,
                  0,
                  {320, 200},
                  {0, -7},
                  {-85.42362823733717, 40.00000000000193}},
             Step{"drag-south-pole 19",
                  {-89.36808513722191, 40.00000000000193},
                  -0.20929101946440645,
                  0,
                  0,
                  {320, 200},
                  {0, -7},
                  {-89.86114224970903, 40.00000000000193}},
             Step{"drag-south-pole 20",
                  {-89.86114224970903, 40.00000000000193},
                  -0.20929101946440645,
                  0,
                  0,
                  {320, 200},
                  {0, -7},
                  {-90, 40.00000000000193}},
         }) {
        SCOPED_TRACE(step.name);
        Transform transform;
        transform.resize({640, 480});
        transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective"));
        transform.jumpTo(CameraOptions()
                             .withCenter(step.center)
                             .withZoom(step.zoom)
                             .withBearing(step.bearing)
                             .withPitch(step.pitch));
        const LatLng center = VerticalPerspectiveProjection::dragCenter(
            transform.getState(), {step.point.x, 480 - step.point.y}, {step.panDelta.x, -step.panDelta.y});
        EXPECT_NEAR(step.expected.latitude(), center.latitude(), 1e-7);
        EXPECT_NEAR(
            0.0,
            VerticalPerspectiveProjection::differenceOfAnglesDegrees(step.expected.longitude(), center.longitude()),
            1e-4);
    }
}

// Dragging sideways at the pole turns the globe around it with the latitude held, as GL JS's drag does. The center
// used to come from the location beside it, so it slid away from the pole and the planet spun.
TEST(GlobeTransform, DragAroundThePoleHoldsItsLatitude) {
    Transform transform;
    transform.resize({640, 480});
    transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective"));
    transform.jumpTo(CameraOptions().withCenter(LatLng{util::LATITUDE_MAX, -100.0}).withZoom(1.0));
    for (int step = 0; step < 60; ++step) {
        transform.moveBy({6, 0}, ScreenCoordinate{380, 240});
    }
    // GL JS after the same 60 steps, run in doubles as in DragCenterMatchesGLJS.
    EXPECT_NEAR(84.85324650692623, transform.getLatLng().latitude(), 1e-6);
    EXPECT_NEAR(
        0.0,
        VerticalPerspectiveProjection::differenceOfAnglesDegrees(149.33280710271083, transform.getLatLng().longitude()),
        1e-6);
    EXPECT_NEAR(1.0564167839713543, transform.getZoom(), 1e-6);
}

// Dragging north over the pole stops on it, as GL JS's drag does, the zoom holding the planet's size from the edge of
// the Mercator range on. GL JS after the same 40 steps, run in doubles: (90, -100) at zoom -0.9872050442266238.
TEST(GlobeTransform, DragOverThePoleStopsOnIt) {
    Transform transform;
    transform.resize({640, 480});
    transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective"));
    transform.jumpTo(CameraOptions().withCenter(LatLng{70.0, -100.0}).withZoom(1.0));
    for (int step = 0; step < 40; ++step) {
        transform.moveBy({0, 6}, ScreenCoordinate{320, 300});
    }
    EXPECT_EQ(90.0, transform.getLatLng().latitude());
    EXPECT_NEAR(-100.0, transform.getLatLng().longitude(), 1e-6);
    EXPECT_NEAR(-0.9872050442266238, transform.getZoom(), 1e-6);
}

// GL JS's drag without the fixed bearing (`versorSetLocationAtPoint(tr, location, anchor, panDelta, false)` in
// globe_utils.ts at 638b3f9ec) on a 640 by 480 globe with a 45 degree field of view, run in doubles: one step at a time
// from the state GL JS had before it, the anchor the cursor while it is on the planet and the center otherwise. Steps
// with the cursor off the planet go through the falloff, where native and GL JS differ by a few times 1e-7 degrees.
TEST(GlobeTransform, FreeBearingDragMatchesGLJS) {
    struct Step {
        const char* name;
        LatLng center;
        double zoom;
        double bearing;
        double pitch;
        ScreenCoordinate anchor;
        ScreenCoordinate panDelta;
        LatLng expected;
        double expectedBearing;
    };
    for (const auto& step : {
             Step{"mid-globe-east 0",
                  {20.0, 10.0},
                  1.0,
                  0.0,
                  0.0,
                  {340.0, 240.0},
                  {10.0, 0.0},
                  {19.964831035703163, 6.458738472277901},
                  -1.2105018410808273},
             Step{"mid-globe-east 7",
                  {18.019206354761504, -16.65450732839554},
                  1.0171849312490204,
                  -8.825955295711765,
                  0.0,
                  {410.0, 240.0},
                  {10.0, 0.0},
                  {17.29289010262656, -21.199373456415287},
                  -10.205107464700406},
             Step{"over-north-pole 13",
                  {79.41551935272437, -99.99999999999989},
                  0.10315332639360152,
                  3.410605131648481e-13,
                  0.0,
                  {320.0, 284.0},
                  {0.0, 6.0},
                  {80.14639071459133, -99.99999999999807},
                  2.0463630789890885e-12},
             Step{"over-north-pole 25",
                  {88.49295346569899, -99.99999999998397},
                  -0.9872050442266241,
                  1.671196514507756e-11,
                  0.0,
                  {320.0, 356.0},
                  {0.0, 6.0},
                  {89.29419866472097, -99.99999999996294},
                  3.8198777474462986e-11},
             Step{"over-north-pole 26",
                  {89.29419866472097, -99.99999999996294},
                  -0.9872050442266241,
                  3.8198777474462986e-11,
                  0.0,
                  {320.0, 362.0},
                  {0.0, 6.0},
                  {89.89453036004743, 79.99999999974375},
                  179.99999999974605},
             Step{"over-north-pole 27",
                  {89.89453036004743, 79.99999999974375},
                  -0.9872050442266241,
                  179.99999999974605,
                  0.0,
                  {320.0, 368.0},
                  {0.0, 6.0},
                  {89.07238060450973, 79.99999999996999},
                  179.99999999997203},
             Step{"over-north-pole 39",
                  {78.96870340062405, 79.99999999999284},
                  0.16209611242931793,
                  179.999999999995,
                  0.0,
                  {320.0, 440.0},
                  {0.0, 6.0},
                  {77.9082916607387, 79.99999999999159},
                  179.99999999999363},
             Step{"across-pole-on-globe 19",
                  {89.340072693097, -100.00000000001774},
                  -0.009291019464406594,
                  -1.6825651982799172e-11,
                  0.0,
                  {320.0, 320.0},
                  {0.0, 8.0},
                  {89.83806502107838, -100.00000000007714},
                  -7.651124178664759e-11},
             Step{"across-pole-on-globe 20",
                  {89.83806502107838, -100.00000000007714},
                  -0.009291019464406594,
                  -7.651124178664759e-11,
                  0.0,
                  {320.0, 328.0},
                  {0.0, 8.0},
                  {89.6617557958742, 80.00000000003092},
                  -179.99999999996658},
             Step{"across-pole-on-globe 21",
                  {89.6617557958742, 80.00000000003092},
                  -0.009291019464406594,
                  -179.99999999996658,
                  0.0,
                  {320.0, 336.0},
                  {0.0, 8.0},
                  {89.15914508699329, 80.00000000001307},
                  -179.99999999998465},
             Step{"beside-north-pole 0",
                  {85.05, -100.0},
                  1.0,
                  0.0,
                  0.0,
                  {386.0, 240.0},
                  {6.0, 0.0},
                  {85.04662278483669, -102.12115815315576},
                  -2.113243435839649},
             Step{"beside-north-pole 29",
                  {82.61847217815794, -148.0453022994218},
                  1.5742929546363558,
                  -47.80814125537427,
                  0.0,
                  {560.0, 240.0},
                  {6.0, 0.0},
                  {82.46976369277144, -149.0658211264905},
                  -48.82003297275798},
             Step{"off-globe-and-back 5",
                  {59.63182334571161, 0.0},
                  0.015938893735801596,
                  0.0,
                  0.0,
                  {320.0, 240.0},
                  {0.0, 25.0},
                  {68.48553614142399, 0.0},
                  0.0},
             Step{"off-globe-and-back 9",
                  {84.95332547144974, 180.0},
                  -2.506889874048772,
                  180.0,
                  0.0,
                  {320.0, 240.0},
                  {0.0, 25.0},
                  {76.09961267574481, 180.0},
                  180.0},
             Step{"off-globe-and-back 15",
                  {84.95332547144743, 180.0},
                  -2.5068898740481123,
                  180.0,
                  0.0,
                  {320.0, 240.0},
                  {0.0, -25.0},
                  {86.19296173284701, 0.0},
                  0.0},
             Step{"off-globe-and-back 20",
                  {33.13938452686452, 0.0},
                  0.7438908494163972,
                  0.0,
                  0.0,
                  {320.0, 315.0},
                  {0.0, -25.0},
                  {20.561601743914935, 0.0},
                  0.0},
             Step{"bearing-pitch 0",
                  {40.0, 30.0},
                  1.5,
                  30.0,
                  20.0,
                  {308.0, 255.0},
                  {8.0, -5.0},
                  {39.90522931489954, 27.677367870127796},
                  28.54888958514232},
             Step{"bearing-pitch 9",
                  {36.75658194038969, 8.665623636743362},
                  1.5647074495325435,
                  17.020204416245633,
                  20.0,
                  {380.0, 210.0},
                  {8.0, -5.0},
                  {36.00738235824049, 6.187079071036919},
                  15.610527473047796},
         }) {
        SCOPED_TRACE(step.name);
        Transform transform;
        transform.resize({640, 480});
        transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective"));
        transform.jumpTo(CameraOptions()
                             .withCenter(step.center)
                             .withZoom(step.zoom)
                             .withBearing(step.bearing)
                             .withPitch(step.pitch)
                             .withFov(45.0));
        const auto drag = VerticalPerspectiveProjection::dragCamera(
            transform.getState(), {step.anchor.x, 480 - step.anchor.y}, {step.panDelta.x, -step.panDelta.y});
        EXPECT_NEAR(step.expected.latitude(), drag.center.latitude(), 1e-6);
        EXPECT_NEAR(0.0,
                    VerticalPerspectiveProjection::differenceOfAnglesDegrees(step.expected.longitude(),
                                                                             drag.center.longitude()),
                    1e-6);
        EXPECT_NEAR(
            0.0, VerticalPerspectiveProjection::differenceOfAnglesDegrees(step.expectedBearing, drag.bearing), 1e-6);
    }
}

// With a free bearing the location under the cursor stays under it, over the pole too, where the bearing turns round;
// with the bearing held the drag stops on the pole and the location stays behind.
TEST(GlobeTransform, FreeBearingDragKeepsTheLocationUnderTheCursor) {
    for (const bool freeBearing : {true, false}) {
        SCOPED_TRACE(freeBearing ? "free bearing" : "bearing held");
        Transform transform;
        setUpGlobe(transform, {80, -100}, 1);
        transform.setGlobeFreeBearing(freeBearing);
        ScreenCoordinate cursor{400, 220};
        double maxSlip = 0;
        for (int step = 0; step < 40; ++step) {
            const LatLng grabbed = transform.screenCoordinateToLatLng(cursor);
            cursor.y += 8;
            transform.moveBy({0, 8}, cursor);
            const ScreenCoordinate point = transform.latLngToScreenCoordinate(grabbed);
            maxSlip = std::max(maxSlip, std::hypot(point.x - cursor.x, point.y - cursor.y));
        }
        if (freeBearing) {
            EXPECT_LT(maxSlip, 1e-6);
            EXPECT_NEAR(80.0, transform.getLatLng().longitude(), 1e-6);
            EXPECT_NEAR(180.0, std::abs(util::rad2deg(transform.getBearing())), 1e-6);
        } else {
            EXPECT_GT(maxSlip, 4.0);
            EXPECT_EQ(90.0, transform.getLatLng().latitude());
        }
    }
}

// A zoom around a point beside the pole: with the bearing held the center's longitude swings towards the point's and
// the planet turns about the pole, as GL JS's does; with a free bearing the planet only grows around the point.
TEST(GlobeTransform, FreeBearingZoomAroundThePoleKeepsTheViewsOrientation) {
    for (const bool freeBearing : {true, false}) {
        SCOPED_TRACE(freeBearing ? "free bearing" : "bearing held");
        Transform transform;
        setUpGlobe(transform, {89, 0}, 1);
        transform.setGlobeFreeBearing(freeBearing);
        const ScreenCoordinate anchor{520, 220};
        const LatLng location = transform.screenCoordinateToLatLng(anchor);
        const LatLng beside = transform.screenCoordinateToLatLng({anchor.x + 20, anchor.y});
        transform.jumpTo(CameraOptions().withZoom(1.6).withAnchor(anchor));
        const ScreenCoordinate point = transform.latLngToScreenCoordinate(location);
        const ScreenCoordinate besidePoint = transform.latLngToScreenCoordinate(beside);
        const double turn = util::rad2deg(std::atan2(besidePoint.y - point.y, besidePoint.x - point.x));
        if (freeBearing) {
            EXPECT_NEAR(anchor.x, point.x, 1e-6);
            EXPECT_NEAR(anchor.y, point.y, 1e-6);
            EXPECT_NEAR(0.0, turn, 0.5);
        } else {
            EXPECT_GT(std::abs(turn), 10.0);
        }
    }
}

// A fling over the pole with the bearing free turns the planet on about one axis: the center moves away from where it
// started at every frame, over the pole and down the far side, and the bearing turns round. With the bearing held,
// GL JS's fling, it stops on the pole.
TEST(GlobeTransform, FreeBearingFlingCarriesOnOverThePole) {
    for (const bool freeBearing : {true, false}) {
        SCOPED_TRACE(freeBearing ? "free bearing" : "bearing held");
        Transform transform;
        setUpGlobe(transform, {85, -100}, 1);
        transform.setGlobeFreeBearing(freeBearing);
        const vec3 start = VerticalPerspectiveProjection::surfaceVector(transform.getLatLng());
        transform.moveBy({0, 400}, linearSecond());
        double previous = 0;
        for (int frame = 1; frame <= 20; ++frame) {
            transform.updateTransitions(transform.getTransitionStart() + Milliseconds(50 * frame));
            const vec3 center = VerticalPerspectiveProjection::surfaceVector(transform.getLatLng());
            const double distance = std::acos(
                std::clamp(start[0] * center[0] + start[1] * center[1] + start[2] * center[2], -1.0, 1.0));
            EXPECT_GE(distance, previous - 1e-9);
            previous = distance;
        }
        if (freeBearing) {
            EXPECT_LT(transform.getLatLng().latitude(), 85.0);
            EXPECT_NEAR(80.0, transform.getLatLng().longitude(), 1e-6);
            EXPECT_NEAR(180.0, std::abs(util::rad2deg(transform.getBearing())), 1e-6);
        } else {
            EXPECT_EQ(90.0, transform.getLatLng().latitude());
            EXPECT_NEAR(0.0, util::rad2deg(transform.getBearing()), 1e-9);
        }
    }
}

// GL JS #8629, its camera at 512 by 512: the center reaches the pole while the area past the Mercator range fits in
// the view, and comes back to the edge of the range as a zoom in grows that area past it.
TEST(GlobeTransform, CenterReachesThePoleAtLowZoom) {
    Transform transform;
    setUpGlobeCamera(transform, {60.2, 0.0}, 8.0);
    transform.jumpTo(CameraOptions().withCenter(LatLng{90.0, 0.0}).withZoom(0.0));
    EXPECT_EQ(90.0, transform.getLatLng().latitude());
    EXPECT_NEAR(0.0, transform.getZoom(), 1e-9);

    setUpGlobeCamera(transform, {0.0, 0.0}, 0.0);
    transform.jumpTo(CameraOptions().withCenter(LatLng{90.0, 0.0}).withZoom(2.0));
    EXPECT_NEAR(88.57750940365021, transform.getLatLng().latitude(), 1e-6);

    for (const auto& [zoom, latitude] : std::array<std::pair<double, double>, 6>{{{1.0, 90.0},
                                                                                  {1.5, 90.0},
                                                                                  {2.0, 88.57750940365021},
                                                                                  {2.5, 87.06149291090357},
                                                                                  {3.0, 85.98950736849177},
                                                                                  {4.0, util::LATITUDE_MAX}}}) {
        SCOPED_TRACE("zoom " + std::to_string(zoom));
        setUpGlobeCamera(transform, {90.0, 0.0}, 0.5);
        transform.jumpTo(CameraOptions().withZoom(zoom));
        EXPECT_NEAR(latitude, transform.getLatLng().latitude(), 1e-6);
        EXPECT_NEAR(zoom, transform.getZoom(), 1e-9);
    }
}

// iOS's quick zoom is anchored on the middle of the view, so with the center on the pole it zooms around the pole, a
// location with no longitude of its own. The center keeps its meridian and comes down it once the zoom in brings the
// pole's area past the view: GL JS's latitudes with its camera at 512 by 512, the center at 30°E. GL JS turns the
// planet about the pole towards the prime meridian when the center is more than 45° from it; here it keeps the meridian
// there too. A zoom around the pole from below it is GL JS's: the center stays put and the pole moves off the anchor.
TEST(GlobeTransform, ZoomAroundThePoleKeepsTheCentersMeridian) {
    const std::array<double, 19> gljsLatitudes{90.0,
                                               90.0,
                                               90.0,
                                               90.0,
                                               90.0,
                                               90.0,
                                               89.7739138279545,
                                               89.34717269436429,
                                               88.9490091378788,
                                               88.57750940365021,
                                               88.23088789526778,
                                               87.90747859237072,
                                               87.60572704299784,
                                               87.3241828921856,
                                               87.06149291090357,
                                               86.81639449182124,
                                               86.58770958064392,
                                               86.37433901384881,
                                               86.17525723560607};
    for (const double longitude : {30.0, 120.0}) {
        SCOPED_TRACE("longitude " + std::to_string(longitude));
        Transform transform;
        setUpGlobeCamera(transform, {90.0, longitude}, 1.0);
        double zoom = 1.0;
        for (const double latitude : gljsLatitudes) {
            zoom += 0.1;
            SCOPED_TRACE("zoom " + std::to_string(zoom));
            transform.jumpTo(CameraOptions().withZoom(zoom).withAnchor(ScreenCoordinate{256, 256}));
            EXPECT_NEAR(longitude, transform.getLatLng().longitude(), 1e-9);
            EXPECT_NEAR(latitude, transform.getLatLng().latitude(), 1e-6);
        }
    }

    Transform transform;
    setUpGlobeCamera(transform, {88.0, 30.0}, 1.5);
    const ScreenCoordinate pole = transform.latLngToScreenCoordinate(LatLng{90.0, 0.0});
    transform.jumpTo(CameraOptions().withZoom(1.7).withAnchor(pole));
    EXPECT_NEAR(88.0, transform.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(30.0, transform.getLatLng().longitude(), 1e-9);
    EXPECT_NEAR(149.15333659991416, transform.latLngToScreenCoordinate(LatLng{90.0, 0.0}).y, 1e-4);
}

// Without a zoom, GL JS's jump bounds a center past the Mercator range by the zoom it lands on, and its ease and
// flight bound it by the zoom they start from (GL JS's camera at 512 by 512).
TEST(GlobeTransform, JumpBoundsThePoleByTheZoomItLandsOn) {
    const double landingZoom = -0.5129658685303338;
    Transform jump;
    setUpGlobeCamera(jump, {10.0, 0.0}, 3.0);
    jump.jumpTo(CameraOptions().withCenter(LatLng{89.0, 30.0}));
    EXPECT_NEAR(89.0, jump.getLatLng().latitude(), 1e-9);
    EXPECT_NEAR(30.0, jump.getLatLng().longitude(), 1e-9);
    EXPECT_NEAR(landingZoom, jump.getZoom(), 1e-6);

    Transform ease;
    setUpGlobeCamera(ease, {10.0, 0.0}, 3.0);
    ease.easeTo(CameraOptions().withCenter(LatLng{89.0, 30.0}), linearSecond());
    runTo(ease, 0.5);
    EXPECT_NEAR(47.99475368424589, ease.getLatLng().latitude(), 1e-6);
    EXPECT_NEAR(7.785022180215378, ease.getLatLng().longitude(), 1e-6);
    EXPECT_NEAR(2.442592415258865, ease.getZoom(), 1e-6);
    runTo(ease, 1.0);
    EXPECT_NEAR(85.98950736849177, ease.getLatLng().latitude(), 1e-6);
    EXPECT_NEAR(30.0, ease.getLatLng().longitude(), 1e-9);
    EXPECT_NEAR(landingZoom, ease.getZoom(), 1e-6);

    Transform fly;
    setUpGlobeCamera(fly, {10.0, 0.0}, 3.0);
    fly.flyTo(CameraOptions().withCenter(LatLng{89.0, 30.0}), linearSecond());
    runTo(fly, 1.0);
    EXPECT_NEAR(85.98950736849132, fly.getLatLng().latitude(), 1e-6);
    EXPECT_NEAR(landingZoom, fly.getZoom(), 1e-6);
}

// Eases onto, off, and at the pole follow GL JS's camera at 512 by 512, halfway and at the end: a zoom in at the pole
// eases the center back to the edge of the Mercator range.
TEST(GlobeTransform, EasesOntoAndOffThePole) {
    struct Frame {
        double latitude;
        double longitude;
        double zoom;
    };
    struct Ease {
        const char* name;
        LatLng start;
        double startZoom;
        CameraOptions camera;
        std::array<Frame, 2> halves;
    };
    for (const auto& ease :
         {Ease{"onto the pole",
               {60.0, 20.0},
               2.0,
               CameraOptions().withCenter(LatLng{90.0, 20.0}).withZoom(0.0),
               {{{78.05612639366316, 20, 0.9949038280649232}, {90, 20, 0}}}},
          Ease{"onto the pole without a zoom",
               {60.0, 20.0},
               1.0,
               CameraOptions().withCenter(LatLng{89.5, 50.0}),
               {{{74.75, 29.139640453666175, 0.07330592239273392}, {89.5, 50, -1.5350518437681169}}}},
          Ease{"off the pole",
               {90.0, 0.0},
               0.0,
               CameraOptions().withCenter(LatLng{30.0, 40.0}).withZoom(1.0),
               {{{68.78679656440357, 24.852188049491588, 0.9047067439922991}, {30, 40, 1}}}},
          Ease{"zooming in at the pole",
               {90.0, 0.0},
               0.5,
               CameraOptions().withZoom(4.0),
               {{{86.50061975668255, 0, 2.25}, {util::LATITUDE_MAX, 0, 4}}}}}) {
        SCOPED_TRACE(ease.name);
        Transform transform;
        setUpGlobeCamera(transform, ease.start, ease.startZoom);
        transform.easeTo(ease.camera, linearSecond());
        for (int half = 1; half <= 2; ++half) {
            runTo(transform, half / 2.0);
            const Frame& expected = ease.halves[half - 1];
            EXPECT_NEAR(expected.latitude, transform.getLatLng().latitude(), 1e-6);
            EXPECT_NEAR(expected.longitude, transform.getLatLng().longitude(), 1e-6);
            EXPECT_NEAR(expected.zoom, transform.getZoom(), 1e-6);
        }
    }
}

// Flights land on the pole as GL JS's do (its camera at 512 by 512). From zoom 2, GL JS's flight to the pole itself
// stops at that zoom's limit, as its `setLocationAtPoint` has no solution for the pole at the center of the view; a
// flight to any other latitude lands, as this one to 89.9 does, and GL JS's own test flies onto the pole from zoom 8.
TEST(GlobeTransform, FliesOntoThePole) {
    Transform transform;
    setUpGlobeCamera(transform, {60.2, 0.0}, 8.0);
    transform.flyTo(CameraOptions().withCenter(LatLng{90.0, 0.0}).withZoom(0.0), linearSecond());
    runTo(transform, 1.0);
    EXPECT_EQ(90.0, transform.getLatLng().latitude());
    EXPECT_NEAR(0.0, transform.getZoom(), 1e-9);

    setUpGlobeCamera(transform, {60.0, 20.0}, 2.0);
    transform.flyTo(CameraOptions().withCenter(LatLng{89.9, 20.0}).withZoom(0.0), linearSecond());
    for (const auto& [k, latitude, zoom] : std::array<std::tuple<double, double, double>, 2>{
             {{0.5, 77.69090627898427, 0.6859841910974791}, {1.0, 89.9, 0.0}}}) {
        SCOPED_TRACE("k " + std::to_string(k));
        runTo(transform, k);
        EXPECT_NEAR(latitude, transform.getLatLng().latitude(), 1e-6);
        EXPECT_NEAR(20.0, transform.getLatLng().longitude(), 1e-9);
        EXPECT_NEAR(zoom, transform.getZoom(), 1e-6);
    }
}

// GL JS's zoom around a point near the pole on an 800 by 600 globe: the center goes past the Mercator range as far as
// the new zoom lets it.
TEST(GlobeTransform, ZoomAroundNearThePoleMatchesGLJS) {
    struct Zoom {
        LatLng center;
        double zoom;
        double zoomDelta;
        ScreenCoordinate anchor;
        LatLng expected;
    };
    for (const auto& zoom : {Zoom{{89.0, 0.0}, 1.0, 1.0, {450, 200}, {88.2247980153879, 39.898924941760924}},
                             Zoom{{90.0, 0.0}, 0.5, 2.0, {400, 300}, {87.69055327579845, 0.0}}}) {
        SCOPED_TRACE("zoom " + std::to_string(zoom.zoom));
        Transform transform;
        setUpGlobe(transform, zoom.center, zoom.zoom);
        transform.jumpTo(CameraOptions().withZoom(zoom.zoom + zoom.zoomDelta).withAnchor(zoom.anchor));
        EXPECT_NEAR(zoom.expected.latitude(), transform.getLatLng().latitude(), 1e-6);
        EXPECT_NEAR(zoom.expected.longitude(), transform.getLatLng().longitude(), 1e-6);
        EXPECT_NEAR(zoom.zoom + zoom.zoomDelta, transform.getZoom(), 1e-9);
    }
}

// On Mercator a zoom around a point across the antimeridian leaves the center on the next world copy, as it does on
// main: the camera reports that copy, which converts back to the middle of the view.
TEST(Transform, MercatorCameraCenterIsOnTheTransformsWorldCopy) {
    Transform transform;
    transform.resize({800, 600});
    transform.jumpTo(CameraOptions().withCenter(LatLng{0, 179.9}).withZoom(5));
    transform.jumpTo(CameraOptions().withZoom(6).withAnchor(ScreenCoordinate{700, 300}));
    ASSERT_GT(transform.getLatLng(LatLng::Unwrapped).longitude(), 180.0);
    const LatLng center = *transform.getCameraOptions(std::nullopt).center;
    EXPECT_NEAR(transform.getLatLng(LatLng::Unwrapped).longitude(), center.longitude(), 1e-9);
    const ScreenCoordinate point = transform.latLngToScreenCoordinate(center);
    EXPECT_NEAR(400.0, point.x, 1e-6);
    EXPECT_NEAR(300.0, point.y, 1e-6);
}
