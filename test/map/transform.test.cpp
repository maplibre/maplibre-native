#include <mbgl/test/util.hpp>

#include <gmock/gmock.h>
#include <cmath>
#include <mbgl/map/transform.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/util/quaternion.hpp>
#include <mbgl/map/transform_active.hpp>

#include <numbers>

using namespace std::numbers;
using namespace mbgl;

class TransfromParametrized : public testing::TestWithParam<std::shared_ptr<Transform>> {};

TEST_P(TransfromParametrized, InvalidZoom) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1, 1});

    ASSERT_DOUBLE_EQ(0, transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform->getLatLng().longitude());
    ASSERT_DOUBLE_EQ(0, transform->getZoom());

    transform->jumpTo(CameraOptions().withZoom(1.0));

    ASSERT_DOUBLE_EQ(1, transform->getZoom());

    const double invalid = NAN;

    transform->jumpTo(CameraOptions().withZoom(invalid));

    ASSERT_DOUBLE_EQ(0, transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform->getLatLng().longitude());
    ASSERT_DOUBLE_EQ(1, transform->getZoom());

    transform->jumpTo(CameraOptions().withCenter(LatLng()).withZoom(invalid));

    ASSERT_DOUBLE_EQ(0, transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform->getLatLng().longitude());
    ASSERT_DOUBLE_EQ(1, transform->getZoom());

    transform->jumpTo(CameraOptions().withZoom(transform->getState().getMaxZoom() + 0.1));
    ASSERT_DOUBLE_EQ(transform->getZoom(), transform->getState().getMaxZoom());

    // Executing flyTo with an empty size causes frameZoom to be NaN.
    transform->flyTo(CameraOptions()
                         .withCenter(LatLng{util::LATITUDE_MAX, util::LONGITUDE_MAX})
                         .withZoom(transform->getState().getMaxZoom()));
    transform->updateTransitions(transform->getTransitionStart() + transform->getTransitionDuration());
    ASSERT_DOUBLE_EQ(transform->getZoom(), transform->getState().getMaxZoom());

    // Executing flyTo with maximum zoom level to the same zoom level causes
    // frameZoom to be bigger than maximum zoom.
    transform->resize(Size{100, 100});
    transform->flyTo(CameraOptions()
                         .withCenter(LatLng{util::LATITUDE_MAX, util::LONGITUDE_MAX})
                         .withZoom(transform->getState().getMaxZoom()));
    transform->updateTransitions(transform->getTransitionStart() + transform->getTransitionDuration());

    ASSERT_TRUE(transform->getState().valid());
    ASSERT_DOUBLE_EQ(transform->getState().getMaxZoom(), transform->getZoom());
}

TEST_P(TransfromParametrized, InvalidBearing) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1, 1});

    ASSERT_DOUBLE_EQ(0, transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform->getLatLng().longitude());
    ASSERT_DOUBLE_EQ(0, transform->getZoom());

    transform->jumpTo(CameraOptions().withZoom(1.0).withBearing(2.0));
    ASSERT_DOUBLE_EQ(0, transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform->getLatLng().longitude());
    ASSERT_DOUBLE_EQ(1, transform->getZoom());
    ASSERT_DOUBLE_EQ(util::deg2rad(-2.0), transform->getBearing());

    const double invalid = NAN;

    transform->jumpTo(CameraOptions().withBearing(invalid));
    ASSERT_DOUBLE_EQ(0, transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform->getLatLng().longitude());
    ASSERT_DOUBLE_EQ(1, transform->getZoom());
    ASSERT_DOUBLE_EQ(util::deg2rad(-2.0), transform->getBearing());
}

TEST_P(TransfromParametrized, IntegerZoom) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1, 1});

    auto checkIntegerZoom = [&transform](uint8_t zoomInt, double zoom) {
        transform->jumpTo(CameraOptions().withZoom(zoom));
        ASSERT_NEAR(transform->getZoom(), zoom, 1e-8);
        ASSERT_EQ(transform->getState().getIntegerZoom(), zoomInt);
        ASSERT_NEAR(transform->getState().getZoomFraction(), zoom - zoomInt, 1e-8);
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

TEST_P(TransfromParametrized, PerspectiveProjection) {
    LatLng loc;

    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1000, 1000});

    // 0.9 rad ~ 51.56620156 deg
    transform->jumpTo(CameraOptions().withCenter(LatLng{38.0, -77.0}).withZoom(10.0).withPitch(51.56620156));

    // expected values are from maplibre-gl-js

    loc = transform->getLatLng();
    ASSERT_DOUBLE_EQ(-77, loc.longitude());
    ASSERT_DOUBLE_EQ(38, loc.latitude());

    loc = transform->getState().screenCoordinateToLatLng({0, 1000});
    ASSERT_NEAR(-77.59198961199148, loc.longitude(), 1e-6);
    ASSERT_NEAR(38.74661326302018, loc.latitude(), 1e-6);

    loc = transform->getState().screenCoordinateToLatLng({1000, 0});
    ASSERT_NEAR(-76.75823239205641, loc.longitude(), 1e-6);
    ASSERT_NEAR(37.692872969426375, loc.latitude(), 1e-6);

    ScreenCoordinate point = transform->getState().latLngToScreenCoordinate({38.74661326302018, -77.59198961199148});
    ASSERT_NEAR(point.x, 0.0, 1e-5);
    ASSERT_NEAR(point.y, 1000.0, 1e-4);

    point = transform->getState().latLngToScreenCoordinate({37.692872969426375, -76.75823239205641});
    ASSERT_NEAR(point.x, 1000.0, 1e-5);
    ASSERT_NEAR(point.y, 0.0, 1e-4);

    mbgl::vec4 p;
    point = transform->getState().latLngToScreenCoordinate({37.692872969426375, -76.75823239205641}, p);
    ASSERT_NEAR(point.x, 1000.0, 1e-5);
    ASSERT_NEAR(point.y, 0.0, 1e-4);
    ASSERT_GT(p[3], 0.0);

    transform->jumpTo(CameraOptions().withCenter(LatLng{38.0, -77.0}).withZoom(18.0).withPitch(51.56620156));
    point = transform->getState().latLngToScreenCoordinate({7.692872969426375, -76.75823239205641}, p);
    ASSERT_LT(p[3], 0.0);
}

TEST_P(TransfromParametrized, UnwrappedLatLng) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1000, 1000});

    // 0.9 rad ~ 51.56620156 deg
    transform->jumpTo(CameraOptions().withCenter(LatLng{38.0, -77.0}).withZoom(10.0).withPitch(51.56620156));

    const TransformState& state = transform->getState();

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

TEST_P(TransfromParametrized, ConstrainHeightOnly) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->setConstrainMode(ConstrainMode::HeightOnly);
    transform->resize({2, 2});

    transform->jumpTo(CameraOptions().withCenter(LatLngBounds::world().southwest()).withZoom(util::MAX_ZOOM));
    ASSERT_NEAR(-util::LATITUDE_MAX, transform->getLatLng().latitude(), 1e-7);
    ASSERT_NEAR(-util::LONGITUDE_MAX, transform->getLatLng().longitude(), 1e-7);

    transform->jumpTo(CameraOptions().withCenter(LatLngBounds::world().northeast()));
    ASSERT_NEAR(util::LATITUDE_MAX, transform->getLatLng().latitude(), 1e-7);
    ASSERT_NEAR(-util::LONGITUDE_MAX, transform->getLatLng().longitude(), 1e-7);
}

TEST_P(TransfromParametrized, ConstrainWidthAndHeight) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->setConstrainMode(ConstrainMode::WidthAndHeight);
    transform->resize({2, 2});

    transform->jumpTo(CameraOptions().withCenter(LatLngBounds::world().southwest()).withZoom(util::MAX_ZOOM));
    ASSERT_NEAR(-util::LATITUDE_MAX, transform->getLatLng().latitude(), 1e-7);
    ASSERT_NEAR(-util::LONGITUDE_MAX, transform->getLatLng().longitude(), 1e-6);

    transform->jumpTo(CameraOptions().withCenter(LatLngBounds::world().northeast()));
    ASSERT_NEAR(util::LATITUDE_MAX, transform->getLatLng().latitude(), 1e-7);
    ASSERT_NEAR(-util::LONGITUDE_MAX, transform->getLatLng().longitude(), 1e-6);
}

TEST_P(TransfromParametrized, Anchor) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1000, 1000});

    const LatLng latLng{10, -100};
    const ScreenCoordinate anchorPoint = {150, 150};

    transform->jumpTo(CameraOptions().withCenter(latLng).withZoom(10.0));
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform->getLatLng().longitude());
    ASSERT_DOUBLE_EQ(10, transform->getZoom());
    ASSERT_DOUBLE_EQ(0, transform->getBearing());

    const LatLng anchorLatLng = transform->getState().screenCoordinateToLatLng(anchorPoint);
    ASSERT_NE(latLng.latitude(), anchorLatLng.latitude());
    ASSERT_NE(latLng.longitude(), anchorLatLng.longitude());

    transform->jumpTo(CameraOptions().withCenter(latLng).withZoom(3.0));
    ASSERT_DOUBLE_EQ(3, transform->getZoom());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withZoom(3.5));
    ASSERT_DOUBLE_EQ(3.5, transform->getZoom());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withZoom(5.5).withAnchor(anchorPoint));
    ASSERT_DOUBLE_EQ(5.5, transform->getZoom());
    ASSERT_NE(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_NE(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withCenter(latLng).withZoom(3.0));
    ASSERT_DOUBLE_EQ(3, transform->getZoom());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withZoom(5.0));
    ASSERT_DOUBLE_EQ(5, transform->getZoom());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withZoom(7.0).withAnchor(anchorPoint));
    ASSERT_DOUBLE_EQ(7, transform->getZoom());
    ASSERT_NE(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_NE(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withCenter(latLng).withZoom(2.0));
    ASSERT_DOUBLE_EQ(2, transform->getZoom());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withZoom(4.0));
    ASSERT_DOUBLE_EQ(4, transform->getZoom());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withZoom(8.0).withAnchor(anchorPoint));
    ASSERT_DOUBLE_EQ(8, transform->getZoom());
    ASSERT_NE(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_NE(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withCenter(latLng).withZoom(10.0).withBearing(-45.0));
    ASSERT_DOUBLE_EQ(pi / 4, transform->getBearing());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withBearing(0.0));
    ASSERT_DOUBLE_EQ(0, transform->getBearing());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withBearing(45.0).withAnchor(anchorPoint));
    ASSERT_DOUBLE_EQ(util::deg2rad(-45.0), transform->getBearing());

    // Anchor coordinates are imprecise because we are converting from an integer pixel.
    ASSERT_NEAR(anchorLatLng.latitude(), transform->getLatLng().latitude(), 0.5);
    ASSERT_NEAR(anchorLatLng.longitude(), transform->getLatLng().longitude(), 0.5);

    transform->jumpTo(CameraOptions().withCenter(latLng).withZoom(10.0).withPitch(10.0));
    ASSERT_DOUBLE_EQ(util::deg2rad(10.0), transform->getPitch());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withPitch(15.0));
    ASSERT_DOUBLE_EQ(util::deg2rad(15.0), transform->getPitch());
    ASSERT_DOUBLE_EQ(latLng.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng.longitude(), transform->getLatLng().longitude());

    transform->jumpTo(CameraOptions().withPitch(20.0).withAnchor(anchorPoint));
    ASSERT_DOUBLE_EQ(util::deg2rad(20.0), transform->getPitch());

    // Anchor coordinates are imprecise because we are converting from an integer pixel.
    ASSERT_NEAR(anchorLatLng.latitude(), transform->getLatLng().latitude(), 0.5);
    ASSERT_NEAR(anchorLatLng.longitude(), transform->getLatLng().longitude(), 0.5);
}

TEST_P(TransfromParametrized, Padding) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1000, 1000});

    ASSERT_DOUBLE_EQ(0, transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform->getLatLng().longitude());
    CameraOptions nonPaddedCameraOptions = CameraOptions().withCenter(LatLng{10, -100}).withZoom(10.0);
    transform->jumpTo(nonPaddedCameraOptions);

    const LatLng trueCenter = transform->getLatLng();
    ASSERT_DOUBLE_EQ(10, trueCenter.latitude());
    ASSERT_DOUBLE_EQ(-100, trueCenter.longitude());
    ASSERT_DOUBLE_EQ(10, transform->getZoom());

    const LatLng screenCenter = transform->screenCoordinateToLatLng({
        1000.0 / 2.0,
        1000.0 / 2.0,
    });
    const LatLng upperHalfCenter = transform->screenCoordinateToLatLng({
        1000.0 / 2.0,
        1000.0 * 0.25,
    });

    EdgeInsets padding(1000.0 / 2.0, 0, 0, 0);
    // CameraOption center and zoom don't change when padding changes: center of
    // viewport remains the same as padding defines viwport center offset in rendering.
    CameraOptions paddedOptions = CameraOptions().withPadding(padding);
    transform->jumpTo(paddedOptions);
    const LatLng theSameCenter = transform->getLatLng();
    ASSERT_DOUBLE_EQ(trueCenter.latitude(), theSameCenter.latitude());
    ASSERT_DOUBLE_EQ(trueCenter.longitude(), theSameCenter.longitude());

    // However, LatLng is now at the center of lower half - verify conversion
    // from screen coordinate to LatLng.
    const LatLng paddedLowerHalfScreenCenter = transform->screenCoordinateToLatLng({
        1000.0 / 2.0,
        1000.0 * 0.75,
    });
    ASSERT_NEAR(screenCenter.latitude(), paddedLowerHalfScreenCenter.latitude(), 1e-10);
    ASSERT_NEAR(screenCenter.longitude(), paddedLowerHalfScreenCenter.longitude(), 1e-10);

    // LatLng previously in upper half center, should now be under screen center.
    const LatLng paddedScreenCenter = transform->screenCoordinateToLatLng({
        1000.0 / 2.0,
        1000.0 / 2.0,
    });
    ASSERT_NEAR(upperHalfCenter.latitude(), paddedScreenCenter.latitude(), 1e-10);
    ASSERT_NEAR(upperHalfCenter.longitude(), paddedScreenCenter.longitude(), 1e-10);
}

TEST_P(TransfromParametrized, MoveBy) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1000, 1000});

    transform->jumpTo(CameraOptions().withCenter(LatLng()).withZoom(10.0));

    LatLng trueCenter = transform->getLatLng();
    ASSERT_DOUBLE_EQ(0, trueCenter.latitude());
    ASSERT_DOUBLE_EQ(0, trueCenter.longitude());
    ASSERT_DOUBLE_EQ(10, transform->getZoom());

    for (uint8_t x = 0; x < 20; ++x) {
        bool odd = x % 2;
        bool forward = x % 10;

        LatLng coordinate = transform->screenCoordinateToLatLng({odd ? 400. : 600., forward ? 400. : 600});
        transform->moveBy({odd ? 100. : -100., forward ? 100. : -100});

        trueCenter = transform->getLatLng();
        ASSERT_NEAR(coordinate.latitude(), trueCenter.latitude(), 1e-8);
        ASSERT_NEAR(coordinate.longitude(), trueCenter.longitude(), 1e-8);
    }

    // We have ~1.1 precision loss for each coordinate for 20 rounds of moveBy.
    ASSERT_NEAR(0.0, trueCenter.latitude(), 1.1);
    ASSERT_NEAR(0.0, trueCenter.longitude(), 1.1);
}

TEST_P(TransfromParametrized, Antimeridian) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1000, 1000});

    transform->jumpTo(CameraOptions().withCenter(LatLng()).withZoom(1.0));

    // San Francisco
    const LatLng coordinateSanFrancisco{37.7833, -122.4167};
    ScreenCoordinate pixelSF = transform->latLngToScreenCoordinate(coordinateSanFrancisco);
    ASSERT_DOUBLE_EQ(151.79249437176432, pixelSF.x);
    ASSERT_DOUBLE_EQ(383.76720782527661, pixelSF.y);

    transform->jumpTo(CameraOptions().withCenter(LatLng{0.0, -181.0}));

    ScreenCoordinate pixelSFLongest = transform->latLngToScreenCoordinate(coordinateSanFrancisco);
    ASSERT_DOUBLE_EQ(-357.36306616412816, pixelSFLongest.x);
    ASSERT_DOUBLE_EQ(pixelSF.y, pixelSFLongest.y);
    LatLng unwrappedSF = coordinateSanFrancisco.wrapped();
    unwrappedSF.unwrapForShortestPath(transform->getLatLng());

    ScreenCoordinate pixelSFShortest = transform->latLngToScreenCoordinate(unwrappedSF);
    ASSERT_DOUBLE_EQ(666.63694385219173, pixelSFShortest.x);
    ASSERT_DOUBLE_EQ(pixelSF.y, pixelSFShortest.y);

    transform->jumpTo(CameraOptions().withCenter(LatLng{0.0, 179.0}));
    pixelSFShortest = transform->latLngToScreenCoordinate(coordinateSanFrancisco);
    ASSERT_DOUBLE_EQ(pixelSFLongest.x, pixelSFShortest.x);
    ASSERT_DOUBLE_EQ(pixelSFLongest.y, pixelSFShortest.y);

    // Waikiri
    const LatLng coordinateWaikiri{-16.9310, 179.9787};
    transform->jumpTo(CameraOptions().withCenter(coordinateWaikiri).withZoom(10.0));
    ScreenCoordinate pixelWaikiri = transform->latLngToScreenCoordinate(coordinateWaikiri);
    ASSERT_DOUBLE_EQ(500, pixelWaikiri.x);
    ASSERT_DOUBLE_EQ(500, pixelWaikiri.y);

    transform->jumpTo(CameraOptions().withCenter(LatLng{coordinateWaikiri.latitude(), 180.0213}));
    ScreenCoordinate pixelWaikiriLongest = transform->latLngToScreenCoordinate(coordinateWaikiri);
    ASSERT_DOUBLE_EQ(524725.96438108233, pixelWaikiriLongest.x);
    ASSERT_DOUBLE_EQ(pixelWaikiri.y, pixelWaikiriLongest.y);

    LatLng unwrappedWaikiri = coordinateWaikiri.wrapped();
    unwrappedWaikiri.unwrapForShortestPath(transform->getLatLng());
    ScreenCoordinate pixelWaikiriShortest = transform->latLngToScreenCoordinate(unwrappedWaikiri);
    ASSERT_DOUBLE_EQ(437.95925272648344, pixelWaikiriShortest.x);
    ASSERT_DOUBLE_EQ(pixelWaikiri.y, pixelWaikiriShortest.y);

    LatLng coordinateFromPixel = transform->screenCoordinateToLatLng(pixelWaikiriLongest);
    ASSERT_NEAR(coordinateWaikiri.latitude(), coordinateFromPixel.latitude(), 1e-4);
    ASSERT_NEAR(coordinateWaikiri.longitude(), coordinateFromPixel.longitude(), 1e-4);

    transform->jumpTo(CameraOptions().withCenter(LatLng{coordinateWaikiri.latitude(), 180.0213}));
    pixelWaikiriShortest = transform->latLngToScreenCoordinate(coordinateWaikiri);
    ASSERT_DOUBLE_EQ(pixelWaikiriLongest.x, pixelWaikiriShortest.x);
    ASSERT_DOUBLE_EQ(pixelWaikiriLongest.y, pixelWaikiriShortest.y);

    coordinateFromPixel = transform->screenCoordinateToLatLng(pixelWaikiriShortest);
    ASSERT_NEAR(coordinateWaikiri.latitude(), coordinateFromPixel.latitude(), 1e-4);
    ASSERT_NEAR(coordinateWaikiri.longitude(), coordinateFromPixel.longitude(), 1e-4);
}

TEST_P(TransfromParametrized, Camera) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1000, 1000});

    const LatLng latLng1{45, 135};
    CameraOptions cameraOptions1 = CameraOptions().withCenter(latLng1).withZoom(20.0);
    transform->jumpTo(cameraOptions1);
    ASSERT_DOUBLE_EQ(latLng1.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng1.longitude(), transform->getLatLng().longitude());
    ASSERT_DOUBLE_EQ(20, transform->getZoom());

    const LatLng latLng2{-45, -135};
    CameraOptions cameraOptions2 = CameraOptions().withCenter(latLng2).withZoom(10.0);
    transform->jumpTo(cameraOptions2);
    ASSERT_DOUBLE_EQ(latLng2.latitude(), transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(latLng2.longitude(), transform->getLatLng().longitude());
    ASSERT_DOUBLE_EQ(10, transform->getZoom());

    AnimationOptions easeOptions(Seconds(1));
    easeOptions.transitionFrameFn = [&](double t) {
        ASSERT_TRUE(t >= 0 && t <= 1);
        ASSERT_GE(latLng1.latitude(), transform->getLatLng().latitude());
        ASSERT_LE(latLng1.longitude(), transform->getLatLng().longitude());
    };
    easeOptions.transitionFinishFn = [&]() {
        ASSERT_DOUBLE_EQ(latLng1.latitude(), transform->getLatLng().latitude());
        ASSERT_DOUBLE_EQ(latLng1.longitude(), transform->getLatLng().longitude());
        ASSERT_DOUBLE_EQ(20, transform->getZoom());
    };

    transform->easeTo(cameraOptions1, easeOptions);
    ASSERT_TRUE(transform->inTransition());
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(250));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(500));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(750));
    transform->updateTransitions(transform->getTransitionStart() + transform->getTransitionDuration());
    ASSERT_FALSE(transform->inTransition());

    AnimationOptions flyOptions(Seconds(1));
    flyOptions.transitionFrameFn = [&](double t) {
        ASSERT_TRUE(t >= 0 && t <= 1);
        ASSERT_LE(latLng2.latitude(), transform->getLatLng().latitude());
        ASSERT_GE(latLng2.longitude(), transform->getLatLng(LatLng::Unwrapped).longitude());
    };
    flyOptions.transitionFinishFn = [&]() {
        // XXX Fix precision loss in flyTo:
        // https://github.com/mapbox/mapbox-gl-native/issues/4298
        ASSERT_DOUBLE_EQ(latLng2.latitude(), transform->getLatLng().latitude());
        ASSERT_DOUBLE_EQ(latLng2.longitude(), transform->getLatLng().longitude());
        ASSERT_NEAR(10.0, transform->getZoom(), 1e-5);
    };

    transform->flyTo(cameraOptions2, flyOptions);
    ASSERT_TRUE(transform->inTransition());
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(250));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(500));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(750));
    transform->updateTransitions(transform->getTransitionStart() + transform->getTransitionDuration());
    ASSERT_FALSE(transform->inTransition());

    // Anchor and center points are mutually exclusive.
    CameraOptions camera;
    camera.center = LatLng{0, 0};
    camera.anchor = ScreenCoordinate{0, 0}; // top-left
    camera.zoom = transform->getState().getMaxZoom();
    transform->easeTo(camera, AnimationOptions(Seconds(1)));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(250));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(500));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(750));
    transform->updateTransitions(transform->getTransitionStart() + transform->getTransitionDuration());
    ASSERT_DOUBLE_EQ(transform->getLatLng().latitude(), 0);
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), 0);
}

TEST_P(TransfromParametrized, ProjectionMode) {
    std::shared_ptr<Transform> transform = GetParam();

    transform->setProjectionMode(ProjectionMode().withAxonometric(true).withXSkew(1.0).withYSkew(0.0));
    auto options = transform->getProjectionMode();

    EXPECT_TRUE(*options.axonometric);
    EXPECT_EQ(*options.xSkew, 1.0);
    EXPECT_EQ(*options.ySkew, 0.0);
}

TEST_P(TransfromParametrized, IsPanning) {
    std::shared_ptr<Transform> transform = GetParam();

    AnimationOptions easeOptions(Seconds(1));
    easeOptions.transitionFrameFn = [&transform](double) {
        ASSERT_TRUE(transform->getState().isPanning());
    };

    transform->resize({1000, 1000});
    transform->easeTo(CameraOptions().withCenter(LatLng(0, 360.0)), easeOptions);
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(250));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(500));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(750));
    transform->updateTransitions(transform->getTransitionStart() + transform->getTransitionDuration());
}

void testDefaultTransform(Transform& transform,
                          const uint32_t& cameraWillChangeCount,
                          const uint32_t& cameraDidChangeCount) {
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

TEST(Transfrom, DefaultTransform) {
    struct TransformObserver : public mbgl::TransformObserver {
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
    testDefaultTransform(transform, cameraWillChangeCount, cameraDidChangeCount);

    cameraWillChangeCount = 0;
    cameraDidChangeCount = 0;
    TransformActive transformActive(observer);
    testDefaultTransform(transformActive, cameraWillChangeCount, cameraDidChangeCount);
}

TEST_P(TransfromParametrized, LatLngBounds) {
    const LatLng nullIsland{};
    const LatLng sanFrancisco{37.7749, -122.4194};

    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1000, 1000});

    transform->jumpTo(CameraOptions().withCenter(LatLng()).withZoom(transform->getState().getMaxZoom()));

    // Default bounds.
    ASSERT_EQ(transform->getState().getLatLngBounds(), LatLngBounds());
    ASSERT_EQ(transform->getLatLng(), nullIsland);

    // Invalid bounds.
    try {
        transform->setLatLngBounds(LatLngBounds::empty());
        ASSERT_TRUE(false) << "Should throw";
    } catch (...) {
        ASSERT_EQ(transform->getState().getLatLngBounds(), LatLngBounds());
    }

    transform->jumpTo(CameraOptions().withCenter(sanFrancisco));
    ASSERT_NEAR(transform->getLatLng().latitude(), sanFrancisco.latitude(), 1e-8);
    ASSERT_NEAR(transform->getLatLng().longitude(), sanFrancisco.longitude(), 1e-8);

    // Single location.
    transform->setLatLngBounds(LatLngBounds::singleton(sanFrancisco));
    ASSERT_EQ(transform->getLatLng(), sanFrancisco);

    //    -1   |   0   |  +1
    // ┌───┬───┰───┬───┰───┬───┐
    // │   │   ┃•  │   ┃   │   │
    // ├───┼───╂───┼───╂───┼───┤
    // │   │   ┃▓▓▓│▓▓▓┃   │   │
    // └───┴───┸───┴───┸───┴───┘
    transform->setLatLngBounds(LatLngBounds::hull({-90.0, -180.0}, {0.0, 180.0}));
    transform->jumpTo(CameraOptions().withCenter(sanFrancisco));
    ASSERT_EQ(transform->getLatLng().latitude(), 0.0);
    ASSERT_EQ(transform->getLatLng().longitude(), sanFrancisco.longitude());

    // Try crossing the antimeridian from the left.
    transform->jumpTo(CameraOptions().withCenter(LatLng{0.0, -200.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), -180.0);

    // Try crossing the antimeridian from the right.
    transform->jumpTo(CameraOptions().withCenter(LatLng{0.0, 200.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng(LatLng::Unwrapped).longitude(), 180.0);
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), -180.0);

    //    -1   |   0   |  +1
    // ┌───┬───┰───┬───┰───┬───┐
    // │   │   ┃•  │▓▓▓┃   │   │
    // ├───┼───╂───┼───╂───┼───┤
    // │   │   ┃   │▓▓▓┃   │   │
    // └───┴───┸───┴───┸───┴───┘
    transform->setLatLngBounds(LatLngBounds::hull({-90.0, 0.0}, {90.0, 180.0}));
    transform->jumpTo(CameraOptions().withCenter(sanFrancisco));
    ASSERT_NEAR(transform->getLatLng().latitude(), sanFrancisco.latitude(), 1e-8);
    ASSERT_EQ(transform->getLatLng().longitude(), 0.0);

    //    -1   |   0   |  +1
    // ┌───┬───┰───┬───┰───┬───┐
    // │   │   ┃•  │   ┃   │   │
    // ├───┼───╂───┼───╂───┼───┤
    // │   │   ┃   │▓▓▓┃   │   │
    // └───┴───┸───┴───┸───┴───┘
    transform->setLatLngBounds(LatLngBounds::hull({-90.0, 0.0}, {0.0, 180.0}));
    transform->jumpTo(CameraOptions().withCenter(sanFrancisco));
    ASSERT_EQ(transform->getLatLng().latitude(), 0.0);
    ASSERT_EQ(transform->getLatLng().longitude(), 0.0);

    //    -1   |   0   |  +1
    // ┌───┬───┰───┬───┰───┬───┐
    // │   │   ┃   │  ▓┃▓  │   │
    // ├───┼───╂───┼───╂───┼───┤
    // │   │   ┃   │   ┃   │   │
    // └───┴───┸───┴───┸───┴───┘
    LatLng inside{45.0, 150.0};
    transform->setLatLngBounds(LatLngBounds::hull({0.0, 120.0}, {90.0, 240.0}));
    transform->jumpTo(CameraOptions().withCenter(inside));
    ASSERT_EQ(transform->getLatLng().latitude(), inside.latitude());
    ASSERT_EQ(transform->getLatLng().longitude(), inside.longitude());

    transform->jumpTo(CameraOptions().withCenter(LatLng{0.0, 140.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), 140.0);

    transform->jumpTo(CameraOptions().withCenter(LatLng{0.0, 160.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), 160.0);

    // Constrain latitude only.
    transform->jumpTo(CameraOptions().withCenter(LatLng{-45.0, inside.longitude()}));
    ASSERT_EQ(transform->getLatLng().latitude(), 0.0);
    ASSERT_EQ(transform->getLatLng().longitude(), inside.longitude());

    // Crossing the antimeridian, within bounds.
    transform->jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), 181.0}));
    ASSERT_EQ(transform->getLatLng().longitude(), -179.0);

    // Crossing the antimeridian, outside bounds.
    transform->jumpTo(CameraOptions().withCenter(inside));
    transform->jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), 250.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), -120.0);

    // Constrain to the left edge.
    transform->jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), 119.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), 120.0);

    // Simulate swipe to the left.
    mbgl::AnimationOptions easeOptions(mbgl::Seconds(1));
    easeOptions.transitionFrameFn = [&](double /* t */) {
        ASSERT_NEAR(transform->getLatLng().longitude(), 120.0, 1e-4);
    };
    easeOptions.transitionFinishFn = [&]() {
        ASSERT_NEAR(transform->getLatLng().longitude(), 120.0, 1e-4);
    };
    transform->moveBy(ScreenCoordinate{-500, -500}, easeOptions);

    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(0));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(250));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(500));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(750));
    transform->updateTransitions(transform->getTransitionStart() + transform->getTransitionDuration());

    // Constrain to the right edge.
    transform->jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), 241.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), -120.0);

    // Simulate swipe to the right.
    easeOptions.transitionFrameFn = [&](double /* t */) {
        ASSERT_NEAR(transform->getLatLng().longitude(), -120.0, 1e-4);
    };
    easeOptions.transitionFinishFn = [&]() {
        ASSERT_NEAR(transform->getLatLng().longitude(), -120.0, 1e-4);
    };
    transform->moveBy(ScreenCoordinate{500, 500}, easeOptions);

    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(0));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(250));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(500));
    transform->updateTransitions(transform->getTransitionStart() + Milliseconds(750));
    transform->updateTransitions(transform->getTransitionStart() + transform->getTransitionDuration());

    //    -1   |   0   |  +1
    // ┌───┬───┰───┬───┰───┬───┐
    // │   │   ┃   │   ┃   │   │
    // ├───┼───╂───┼───╂───┼───┤
    // │   │  ▓┃▓  │   ┃   │   │
    // └───┴───┸───┴───┸───┴───┘
    inside = LatLng{-45.0, -150.0};
    transform->setLatLngBounds(LatLngBounds::hull({-90.0, -240.0}, {0.0, -120.0}));
    transform->jumpTo(CameraOptions().withCenter(inside));
    ASSERT_DOUBLE_EQ(transform->getLatLng().latitude(), inside.latitude());
    ASSERT_EQ(transform->getLatLng().longitude(), inside.longitude());

    transform->jumpTo(CameraOptions().withCenter(LatLng{0.0, -140.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), -140.0);

    transform->jumpTo(CameraOptions().withCenter(LatLng{0.0, -160.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), -160.0);

    // Constrain latitude only.
    transform->jumpTo(CameraOptions().withCenter(LatLng{45.0, inside.longitude()}));
    ASSERT_EQ(transform->getLatLng().latitude(), 0.0);
    ASSERT_EQ(transform->getLatLng().longitude(), inside.longitude());

    // Crossing the antimeridian, within bounds.
    transform->jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), -181.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng().latitude(), inside.latitude());
    ASSERT_EQ(transform->getLatLng().longitude(), 179.0);

    // Crossing the antimeridian, outside bounds.
    transform->jumpTo(CameraOptions().withCenter(inside));
    transform->jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), -250.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), 120.0);

    // Constrain to the left edge.
    transform->jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), -119.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), -120.0);

    transform->moveBy(ScreenCoordinate{-500, 0});
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), -120.0);

    // Constrain to the right edge.
    transform->jumpTo(CameraOptions().withCenter(LatLng{inside.latitude(), -241.0}));
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), 120.0);

    transform->moveBy(ScreenCoordinate{500, 0});
    ASSERT_DOUBLE_EQ(transform->getLatLng().longitude(), 120.0);
}

TEST_P(TransfromParametrized, ConstrainScreenToBounds) {
    std::shared_ptr<Transform> transform = GetParam();

    transform->resize({500, 500});
    transform->setLatLngBounds(LatLngBounds::hull({40.0, -10.0}, {70.0, 40.0}));
    transform->setConstrainMode(ConstrainMode::Screen);

    // Request impossible zoom
    transform->easeTo(CameraOptions().withCenter(LatLng{56, 11}).withZoom(1));
    ASSERT_NEAR(transform->getZoom(), 2.81378, 1e-4);

    // Request impossible center left
    transform->easeTo(CameraOptions().withCenter(LatLng{56, -65}).withZoom(4));
    ASSERT_NEAR(transform->getLatLng().longitude(), 0.98632, 1e-4);
    ASSERT_NEAR(transform->getLatLng().latitude(), 56.0, 1e-4);

    // Request impossible center top
    transform->easeTo(CameraOptions().withCenter(LatLng{80, 11}).withZoom(4));
    ASSERT_NEAR(transform->getLatLng().longitude(), 11.0, 1e-4);
    ASSERT_NEAR(transform->getLatLng().latitude(), 65.88603, 1e-4);

    // Request impossible center right
    transform->easeTo(CameraOptions().withCenter(LatLng{56, 50}).withZoom(4));
    ASSERT_NEAR(transform->getLatLng().longitude(), 29.01367, 1e-4);
    ASSERT_NEAR(transform->getLatLng().latitude(), 56.0, 1e-4);

    // Request impossible center bottom
    transform->easeTo(CameraOptions().withCenter(LatLng{30, 11}).withZoom(4));
    ASSERT_NEAR(transform->getLatLng().longitude(), 11.0, 1e-4);
    ASSERT_NEAR(transform->getLatLng().latitude(), 47.89217, 1e-4);

    // Request impossible center with anchor
    transform->easeTo(CameraOptions().withAnchor(ScreenCoordinate{250, 250}).withCenter(LatLng{56, -65}).withZoom(4));
    ASSERT_NEAR(transform->getLatLng().longitude(), 0.98632, 1e-4);
    ASSERT_NEAR(transform->getLatLng().latitude(), 56.0, 1e-4);

    // Request impossible center (anchor)
    transform->easeTo(CameraOptions().withAnchor(ScreenCoordinate{250, 250}).withZoom(4));
    ASSERT_NEAR(transform->getLatLng().longitude(), 0.98632, 1e-4);
    ASSERT_NEAR(transform->getLatLng().latitude(), 56.0, 1e-4);

    // Fly to impossible center
    transform->flyTo(CameraOptions().withCenter(LatLng{56, -65}).withZoom(4));
    ASSERT_NEAR(transform->getZoom(), 4.0, 1e-4);
    ASSERT_NEAR(transform->getLatLng().longitude(), 0.98632, 1e-4);
    ASSERT_NEAR(transform->getLatLng().latitude(), 56.0, 1e-4);

    // Fly to impossible center and zoom
    transform->flyTo(CameraOptions().withCenter(LatLng{56, -65}).withZoom(2));
    ASSERT_NEAR(transform->getZoom(), 4.0, 1e-4);
    ASSERT_NEAR(transform->getLatLng().longitude(), 0.98632, 1e-4);
    ASSERT_NEAR(transform->getLatLng().latitude(), 56.0, 1e-4);
}

TEST_P(TransfromParametrized, InvalidPitch) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1, 1});

    ASSERT_DOUBLE_EQ(0, transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform->getLatLng().longitude());
    ASSERT_DOUBLE_EQ(0, transform->getZoom());
    ASSERT_DOUBLE_EQ(0, transform->getPitch());

    transform->jumpTo(CameraOptions().withZoom(1.0).withPitch(45));
    ASSERT_DOUBLE_EQ(1, transform->getZoom());
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform->getPitch());

    const double invalid = NAN;

    transform->jumpTo(CameraOptions().withPitch(invalid));
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform->getPitch());

    transform->jumpTo(CameraOptions().withPitch(60));
    ASSERT_DOUBLE_EQ(util::deg2rad(60), transform->getPitch());
}

TEST_P(TransfromParametrized, MinMaxPitch) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({1, 1});

    ASSERT_DOUBLE_EQ(0, transform->getLatLng().latitude());
    ASSERT_DOUBLE_EQ(0, transform->getLatLng().longitude());
    ASSERT_DOUBLE_EQ(0, transform->getZoom());
    ASSERT_DOUBLE_EQ(0, transform->getPitch());

    transform->jumpTo(CameraOptions().withZoom(1.0).withPitch(60));
    ASSERT_DOUBLE_EQ(1, transform->getZoom());
    ASSERT_DOUBLE_EQ(transform->getState().getMaxPitch(), transform->getPitch());
    ASSERT_DOUBLE_EQ(util::deg2rad(60), transform->getPitch());

    transform->setMaxPitch(70);
    transform->jumpTo(CameraOptions().withPitch(70));
    ASSERT_DOUBLE_EQ(transform->getState().getMaxPitch(), transform->getPitch());
    ASSERT_DOUBLE_EQ(util::deg2rad(60), transform->getPitch());

    transform->setMaxPitch(45);
    transform->jumpTo(CameraOptions().withPitch(60));
    ASSERT_DOUBLE_EQ(transform->getState().getMaxPitch(), transform->getPitch());
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform->getPitch());

    transform->jumpTo(CameraOptions().withPitch(0));
    ASSERT_DOUBLE_EQ(transform->getState().getMinPitch(), transform->getPitch());
    ASSERT_DOUBLE_EQ(0, transform->getPitch());

    transform->setMinPitch(-10);
    transform->jumpTo(CameraOptions().withPitch(-10));
    ASSERT_DOUBLE_EQ(transform->getState().getMinPitch(), transform->getPitch());
    ASSERT_DOUBLE_EQ(0, transform->getPitch());

    transform->setMinPitch(15);
    transform->jumpTo(CameraOptions().withPitch(0));
    ASSERT_DOUBLE_EQ(transform->getState().getMinPitch(), transform->getPitch());
    ASSERT_DOUBLE_EQ(util::deg2rad(15), transform->getPitch());

    transform->setMinPitch(45);
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform->getState().getMinPitch());
    transform->setMaxPitch(45);
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform->getState().getMaxPitch());

    transform->setMaxPitch(10);
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform->getState().getMaxPitch());

    transform->setMinPitch(60);
    ASSERT_DOUBLE_EQ(util::deg2rad(45), transform->getState().getMinPitch());
}

static const double abs_double_error = 1e-5;

MATCHER_P(Vec3NearEquals1E5, vec, "") {
    return std::fabs(vec[0] - arg[0]) <= abs_double_error && std::fabs(vec[1] - arg[1]) <= abs_double_error &&
           std::fabs(vec[2] - arg[2]) <= abs_double_error;
}

TEST_P(TransfromParametrized, FreeCameraOptionsInvalidSize) {
    std::shared_ptr<Transform> transform = GetParam();
    FreeCameraOptions options;

    options.orientation = vec4{{1.0, 1.0, 1.0, 1.0}};
    options.position = vec3{{0.1, 0.2, 0.3}};
    transform->setFreeCameraOptions(options);

    const auto updatedOrientation = transform->getFreeCameraOptions().orientation.value();
    const auto updatedPosition = transform->getFreeCameraOptions().position.value();

    EXPECT_DOUBLE_EQ(0.0, updatedOrientation[0]);
    EXPECT_DOUBLE_EQ(0.0, updatedOrientation[1]);
    EXPECT_DOUBLE_EQ(0.0, updatedOrientation[2]);
    EXPECT_DOUBLE_EQ(1.0, updatedOrientation[3]);

    EXPECT_THAT(updatedPosition, Vec3NearEquals1E5(vec3{{0.0, 0.0, 0.0}}));
}

TEST_P(TransfromParametrized, FreeCameraOptionsNanInput) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({100, 100});
    FreeCameraOptions options;

    options.position = vec3{{0.5, 0.5, 0.25}};
    transform->setFreeCameraOptions(options);

    options.position = vec3{{0.0, 0.0, NAN}};
    transform->setFreeCameraOptions(options);
    EXPECT_EQ((vec3{{0.5, 0.5, 0.25}}), transform->getFreeCameraOptions().position.value());

    // Only the invalid parameter should be discarded
    options.position = vec3{{0.3, 0.1, 0.2}};
    options.orientation = vec4{{NAN, 0.0, NAN, 0.0}};
    transform->setFreeCameraOptions(options);
    EXPECT_THAT(transform->getFreeCameraOptions().position.value(), Vec3NearEquals1E5(vec3{{0.3, 0.1, 0.2}}));
    EXPECT_EQ(Quaternion::identity.m, transform->getFreeCameraOptions().orientation.value());
}

TEST_P(TransfromParametrized, FreeCameraOptionsInvalidZ) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({100, 100});
    FreeCameraOptions options;

    // Invalid z-value (<= 0.0 || > 1) should be clamped to respect both min&max zoom values
    options.position = vec3{{0.1, 0.1, 0.0}};
    transform->setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(transform->getState().getMaxZoom(), transform->getState().getZoom());
    EXPECT_GT(transform->getFreeCameraOptions().position.value()[2], 0.0);

    options.position = vec3{{0.5, 0.2, 123.456}};
    transform->setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(transform->getState().getMinZoom(), transform->getState().getZoom());
    EXPECT_LE(transform->getFreeCameraOptions().position.value()[2], 1.0);
}

TEST_P(TransfromParametrized, FreeCameraOptionsInvalidOrientation) {
    // Invalid orientations that cannot be clamped into a valid range
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({100, 100});

    FreeCameraOptions options;
    options.orientation = vec4{{0.0, 0.0, 0.0, 0.0}};
    transform->setFreeCameraOptions(options);
    EXPECT_EQ(Quaternion::identity.m, transform->getFreeCameraOptions().orientation);

    // Gimbal lock. Both forward and up vectors are on xy-plane
    options.orientation = Quaternion::fromAxisAngle(vec3{{0.0, 1.0, 0.0}}, pi / 2).m;
    transform->setFreeCameraOptions(options);
    EXPECT_EQ(Quaternion::identity.m, transform->getFreeCameraOptions().orientation);

    // Camera is upside down
    options.orientation = Quaternion::fromAxisAngle(vec3{{1.0, 0.0, 0.0}}, pi / 2 + pi / 4).m;
    transform->setFreeCameraOptions(options);
    EXPECT_EQ(Quaternion::identity.m, transform->getFreeCameraOptions().orientation);
}

TEST_P(TransfromParametrized, FreeCameraOptionsSetOrientation) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({100, 100});
    FreeCameraOptions options;

    options.orientation = Quaternion::identity.m;
    transform->setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getBearing());
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getPitch());
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getX());
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getY());

    options.orientation = Quaternion::fromAxisAngle(vec3{{1.0, 0.0, 0.0}}, util::deg2rad(-60.0)).m;
    transform->setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getBearing());
    EXPECT_DOUBLE_EQ(util::deg2rad(60.0), transform->getState().getPitch());
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getX());
    EXPECT_DOUBLE_EQ(206.0, transform->getState().getY());

    options.orientation = Quaternion::fromAxisAngle(vec3{{0.0, 0.0, 1.0}}, util::deg2rad(56.0)).m;
    transform->setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(util::deg2rad(-56.0), transform->getState().getBearing());
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getPitch());
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getX());
    EXPECT_NEAR(152.192378, transform->getState().getY(), 1e-6);

    options.orientation = Quaternion::fromEulerAngles(0.0, 0.0, util::deg2rad(-179.0))
                              .multiply(Quaternion::fromEulerAngles(util::deg2rad(-30.0), 0.0, 0.0))
                              .m;
    transform->setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(util::deg2rad(179.0), transform->getState().getBearing());
    EXPECT_DOUBLE_EQ(util::deg2rad(30.0), transform->getState().getPitch());
    EXPECT_NEAR(1.308930, transform->getState().getX(), 1e-6);
    EXPECT_NEAR(56.813889, transform->getState().getY(), 1e-6);
}

static std::tuple<vec3, vec3, vec3> rotatedFrame(const std::array<double, 4>& quaternion) {
    Quaternion q(quaternion);
    return std::make_tuple(
        q.transform({{1.0, 0.0, 0.0}}), q.transform({{0.0, -1.0, 0.0}}), q.transform({{0.0, 0.0, -1.0}}));
}

TEST_P(TransfromParametrized, FreeCameraOptionsClampPitch) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({100, 100});
    FreeCameraOptions options;
    vec3 right, up, forward;

    options.orientation = Quaternion::fromAxisAngle(vec3{{1.0, 0.0, 0.0}}, util::deg2rad(-85.0)).m;
    transform->setFreeCameraOptions(options);
    EXPECT_DOUBLE_EQ(util::PITCH_MAX, transform->getState().getPitch());
    std::tie(right, up, forward) = rotatedFrame(transform->getFreeCameraOptions().orientation.value());
    EXPECT_THAT(right, Vec3NearEquals1E5(vec3{{1.0, 0.0, 0.0}}));
    EXPECT_THAT(up, Vec3NearEquals1E5(vec3{{0, -0.5, 0.866025}}));
    EXPECT_THAT(forward, Vec3NearEquals1E5(vec3{{0, -0.866025, -0.5}}));
}

TEST_P(TransfromParametrized, FreeCameraOptionsClampToBounds) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({100, 100});
    transform->setConstrainMode(ConstrainMode::WidthAndHeight);
    transform->jumpTo(CameraOptions().withZoom(8.56));
    FreeCameraOptions options;

    // Place camera to an arbitrary position looking away from the map
    options.position = vec3{{-100.0, -10000.0, 1000.0}};
    options.orientation = Quaternion::fromEulerAngles(util::deg2rad(-45.0), 0.0, 0.0).m;
    transform->setFreeCameraOptions(options);

    // Map center should be clamped to width/2 pixels away from map borders
    EXPECT_DOUBLE_EQ(206.0, transform->getState().getX());
    EXPECT_DOUBLE_EQ(206.0, transform->getState().getY());
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getBearing());
    EXPECT_DOUBLE_EQ(util::deg2rad(45.0), transform->getState().getPitch());

    vec3 right, up, forward;
    std::tie(right, up, forward) = rotatedFrame(transform->getFreeCameraOptions().orientation.value());
    EXPECT_THAT(transform->getFreeCameraOptions().position.value(),
                Vec3NearEquals1E5(vec3{{0.0976562, 0.304816, 0.20716}}));
    EXPECT_THAT(right, Vec3NearEquals1E5(vec3{{1.0, 0.0, 0.0}}));
    EXPECT_THAT(up, Vec3NearEquals1E5(vec3{{0, -0.707107, 0.707107}}));
    EXPECT_THAT(forward, Vec3NearEquals1E5(vec3{{0, -0.707107, -0.707107}}));
}

TEST_P(TransfromParametrized, FreeCameraOptionsInvalidState) {
    std::shared_ptr<Transform> transform = GetParam();

    // Invalid size
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getX());
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getY());
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getBearing());
    EXPECT_DOUBLE_EQ(0.0, transform->getState().getPitch());

    const auto options = transform->getFreeCameraOptions();
    EXPECT_THAT(options.position.value(), Vec3NearEquals1E5(vec3{{0.0, 0.0, 0.0}}));
}

TEST_P(TransfromParametrized, FreeCameraOptionsOrientationRoll) {
    std::shared_ptr<Transform> transform = GetParam();
    FreeCameraOptions options;
    transform->resize({100, 100});

    const auto orientationWithoutRoll = Quaternion::fromEulerAngles(-pi / 4, 0.0, 0.0);
    const auto orientationWithRoll = orientationWithoutRoll.multiply(Quaternion::fromEulerAngles(0.0, 0.0, pi / 4));

    options.orientation = orientationWithRoll.m;
    transform->setFreeCameraOptions(options);
    options = transform->getFreeCameraOptions();

    EXPECT_NEAR(options.orientation.value()[0], orientationWithoutRoll.x, 1e-9);
    EXPECT_NEAR(options.orientation.value()[1], orientationWithoutRoll.y, 1e-9);
    EXPECT_NEAR(options.orientation.value()[2], orientationWithoutRoll.z, 1e-9);
    EXPECT_NEAR(options.orientation.value()[3], orientationWithoutRoll.w, 1e-9);

    EXPECT_NEAR(util::deg2rad(45.0), transform->getState().getPitch(), 1e-9);
    EXPECT_NEAR(0.0, transform->getState().getBearing(), 1e-9);
    EXPECT_NEAR(0.0, transform->getState().getX(), 1e-9);
    EXPECT_NEAR(150.0, transform->getState().getY(), 1e-9);
}

TEST_P(TransfromParametrized, FreeCameraOptionsStateSynchronization) {
    std::shared_ptr<Transform> transform = GetParam();
    transform->resize({100, 100});
    vec3 right, up, forward;

    transform->jumpTo(CameraOptions().withPitch(0.0).withBearing(0.0));
    std::tie(right, up, forward) = rotatedFrame(transform->getFreeCameraOptions().orientation.value());
    EXPECT_THAT(transform->getFreeCameraOptions().position.value(), Vec3NearEquals1E5(vec3{{0.5, 0.5, 0.29296875}}));
    EXPECT_THAT(right, Vec3NearEquals1E5(vec3{{1.0, 0.0, 0.0}}));
    EXPECT_THAT(up, Vec3NearEquals1E5(vec3{{0.0, -1.0, 0.0}}));
    EXPECT_THAT(forward, Vec3NearEquals1E5(vec3{{0.0, 0.0, -1.0}}));

    transform->jumpTo(CameraOptions().withCenter(LatLng{60.1699, 24.9384}));
    EXPECT_THAT(transform->getFreeCameraOptions().position.value(),
                Vec3NearEquals1E5(vec3{{0.569273, 0.289453, 0.292969}}));

    transform->jumpTo(CameraOptions().withPitch(20.0).withBearing(77.0).withCenter(LatLng{-20.0, 20.0}));
    EXPECT_THAT(transform->getFreeCameraOptions().position.value(),
                Vec3NearEquals1E5(vec3{{0.457922, 0.57926, 0.275301}}));

    // Invalid pitch
    transform->jumpTo(CameraOptions().withPitch(-10.0).withBearing(0.0));
    std::tie(right, up, forward) = rotatedFrame(transform->getFreeCameraOptions().orientation.value());
    EXPECT_THAT(transform->getFreeCameraOptions().position.value(),
                Vec3NearEquals1E5(vec3{{0.555556, 0.556719, 0.292969}}));
    EXPECT_THAT(right, Vec3NearEquals1E5(vec3{{1.0, 0.0, 0.0}}));
    EXPECT_THAT(up, Vec3NearEquals1E5(vec3{{0.0, -1.0, 0.0}}));
    EXPECT_THAT(forward, Vec3NearEquals1E5(vec3{{0.0, 0.0, -1.0}}));

    transform->jumpTo(CameraOptions().withPitch(85.0).withBearing(0.0).withCenter(LatLng{-80.0, 0.0}));
    std::tie(right, up, forward) = rotatedFrame(transform->getFreeCameraOptions().orientation.value());
    EXPECT_THAT(transform->getFreeCameraOptions().position.value(), Vec3NearEquals1E5(vec3{{0.5, 1.14146, 0.146484}}));
    EXPECT_THAT(right, Vec3NearEquals1E5(vec3{{1.0, 0.0, 0.0}}));
    EXPECT_THAT(up, Vec3NearEquals1E5(vec3{{0, -0.5, 0.866025}}));
    EXPECT_THAT(forward, Vec3NearEquals1E5(vec3{{0, -0.866025, -0.5}}));
}

TEST(Transform, ConcurrentAnimation) {
    TransformActive transform;
    transform.resize({1, 1});

    const LatLng defaultLatLng{0, 0};
    CameraOptions defaultCameraOptions =
        CameraOptions().withCenter(defaultLatLng).withZoom(0).withPitch(0).withBearing(0);
    transform.jumpTo(defaultCameraOptions);
    ASSERT_DOUBLE_EQ(transform.getLatLng().latitude(), 0);
    ASSERT_DOUBLE_EQ(transform.getLatLng().longitude(), 0);
    ASSERT_DOUBLE_EQ(0, transform.getZoom());
    ASSERT_DOUBLE_EQ(0, transform.getPitch());
    ASSERT_DOUBLE_EQ(0, transform.getBearing());

    const LatLng latLng{45, 135};
    const double zoom = 10;
    CameraOptions zoomLatLngCameraOptions = CameraOptions().withCenter(latLng).withZoom(zoom);
    AnimationOptions zoomLatLngOptions(Seconds(1));
    int zoomLatLngFrameCallbackCount = 0;
    zoomLatLngOptions.transitionFrameFn = [&](double t) {
        zoomLatLngFrameCallbackCount++;

        ASSERT_TRUE(t >= 0 && t <= 1);
        ASSERT_GE(latLng.latitude(), transform.getLatLng().latitude());
        ASSERT_GE(latLng.longitude(), transform.getLatLng().longitude());
        ASSERT_GE(zoom, transform.getZoom());
    };
    int zoomLatLngFinishCallbackCount = 0;
    zoomLatLngOptions.transitionFinishFn = [&]() {
        zoomLatLngFinishCallbackCount++;

        ASSERT_DOUBLE_EQ(latLng.latitude(), transform.getLatLng().latitude());
        ASSERT_DOUBLE_EQ(latLng.longitude(), transform.getLatLng().longitude());
        ASSERT_DOUBLE_EQ(zoom, transform.getZoom());
    };
    transform.easeTo(zoomLatLngCameraOptions, zoomLatLngOptions);

    ASSERT_TRUE(transform.inTransition());

    TimePoint transitionStart = transform.getTransitionStart();

    const double pitch = 60;
    const double bearing = 45;
    CameraOptions pitchBearingCameraOptions = CameraOptions().withPitch(pitch).withBearing(bearing);
    AnimationOptions pitchBearingOptions(Seconds(2));
    int pitchBearingFrameCallbackCount = 0;
    pitchBearingOptions.transitionFrameFn = [&](double t) {
        pitchBearingFrameCallbackCount++;

        ASSERT_TRUE(t >= 0 && t <= 1);
        ASSERT_GE(util::deg2rad(pitch), transform.getPitch());
        ASSERT_LE(-util::deg2rad(bearing), transform.getBearing());
    };
    int pitchBearingFinishCallbackCount = 0;
    pitchBearingOptions.transitionFinishFn = [&]() {
        pitchBearingFinishCallbackCount++;

        ASSERT_DOUBLE_EQ(util::deg2rad(pitch), transform.getPitch());
        ASSERT_DOUBLE_EQ(-util::deg2rad(bearing), transform.getBearing());
    };
    transform.easeTo(pitchBearingCameraOptions, pitchBearingOptions);

    ASSERT_TRUE(transform.inTransition());
    transform.updateTransitions(transitionStart + Milliseconds(500));
    transform.updateTransitions(transitionStart + Milliseconds(900));
    ASSERT_TRUE(transform.inTransition()); // Second Transition is still running
    transform.updateTransitions(transform.getTransitionStart() + Seconds(2));
    ASSERT_FALSE(transform.inTransition());

    ASSERT_EQ(zoomLatLngFrameCallbackCount, 2);
    ASSERT_EQ(zoomLatLngFinishCallbackCount, 1);
    ASSERT_EQ(pitchBearingFrameCallbackCount, 2);
    ASSERT_EQ(pitchBearingFinishCallbackCount, 1);

    // Test cancelTransitions with concurrent animations
    const LatLng latLng2{0, 0};
    const double zoom2 = 0;
    CameraOptions zoomLatLngCameraOptions2 = CameraOptions().withCenter(latLng2).withZoom(zoom2);
    AnimationOptions zoomLatLngOptions2(Seconds(1));
    transform.easeTo(zoomLatLngCameraOptions2, zoomLatLngOptions2);

    const double pitch2 = 0;
    const double bearing2 = 0;
    CameraOptions pitchBearingCameraOptions2 = CameraOptions().withPitch(pitch2).withBearing(bearing2);
    AnimationOptions pitchBearingOptions2(Seconds(2));
    transform.easeTo(pitchBearingCameraOptions2, pitchBearingOptions2);

    ASSERT_TRUE(transform.inTransition());
    transform.cancelTransitions();
    ASSERT_FALSE(transform.inTransition());

    // Reset State
    transform.jumpTo(defaultCameraOptions);

    zoomLatLngFrameCallbackCount = 0;
    zoomLatLngFinishCallbackCount = 0;
    transform.easeTo(zoomLatLngCameraOptions, zoomLatLngOptions);

    transitionStart = transform.getTransitionStart();

    pitchBearingFrameCallbackCount = 0;
    pitchBearingFinishCallbackCount = 0;
    pitchBearingOptions.duration = Seconds(0);
    transform.easeTo(pitchBearingCameraOptions, pitchBearingOptions);

    ASSERT_TRUE(transform.inTransition());
    transform.updateTransitions(transitionStart + Milliseconds(500));
    transform.updateTransitions(transitionStart + Seconds(1));
    ASSERT_FALSE(transform.inTransition());

    ASSERT_EQ(zoomLatLngFrameCallbackCount, 2);
    ASSERT_EQ(zoomLatLngFinishCallbackCount, 1);
    ASSERT_EQ(pitchBearingFrameCallbackCount, 0);
    ASSERT_EQ(pitchBearingFinishCallbackCount, 1);
}

INSTANTIATE_TEST_SUITE_P(Transform,
                         TransfromParametrized,
                         ::testing::Values(std::make_shared<Transform>(TransformObserver::nullObserver()),
                                           std::make_shared<TransformActive>()));
