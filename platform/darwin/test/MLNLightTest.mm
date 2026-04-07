// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.
#import <XCTest/XCTest.h>
#import <Mapbox.h>

#import "MLNLight_Private.h"

#import "../../darwin/src/NSDate+MLNAdditions.h"

#import <mbgl/style/light.hpp>
#import <mbgl/style/types.hpp>
#include <mbgl/style/transition_options.hpp>

@interface MLNLightTest : XCTestCase

@end

@implementation MLNLightTest

- (void)testProperties {

    MLNTransition defaultTransition = MLNTransitionMake(0, 0);
    MLNTransition transition = MLNTransitionMake(6, 3);
    mbgl::style::TransitionOptions transitionOptions { { MLNDurationFromTimeInterval(6) }, { MLNDurationFromTimeInterval(3) } };

    // anchor
    {
        mbgl::style::Light light;
        MLNLight *mglLight = [[MLNLight alloc] initWithMBGLLight:&light];
        auto lightFromMLNLight = mglLight.mbglLight;

        XCTAssertEqual(light.getDefaultAnchor(), lightFromMLNLight.getAnchor());
        XCTAssertEqual(mglLight.anchor.expressionType, NSConstantValueExpressionType, @"mglLight.anchor isn’t a constant value expression.");
        XCTAssertEqualObjects(mglLight.anchor.constantValue, @"viewport");

        mbgl::style::PropertyValue<mbgl::style::LightAnchorType> propertyValue = { mbgl::style::LightAnchorType::Viewport };
        light.setAnchor(propertyValue);
        mglLight = [[MLNLight alloc] initWithMBGLLight:&light];
        lightFromMLNLight = mglLight.mbglLight;

        XCTAssertEqual(light.getAnchor(), lightFromMLNLight.getAnchor());
    }

    // position
    {
        mbgl::style::Light light;
        MLNLight *mglLight = [[MLNLight alloc] initWithMBGLLight:&light];
        auto lightFromMLNLight = mglLight.mbglLight;

        XCTAssertEqual(light.getDefaultPosition(), lightFromMLNLight.getPosition());
        auto positionTransition = lightFromMLNLight.getPositionTransition();
        XCTAssert(positionTransition.delay && MLNTimeIntervalFromDuration(*positionTransition.delay) == defaultTransition.delay);
        XCTAssert(positionTransition.duration && MLNTimeIntervalFromDuration(*positionTransition.duration) == defaultTransition.duration);

        std::array<float, 3> positionArray = { { 6, 180, 90 } };
        mbgl::style::Position position = { positionArray };
        mbgl::style::PropertyValue<mbgl::style::Position> propertyValue = { position };
        light.setPosition(propertyValue);
        light.setPositionTransition(transitionOptions);

        mglLight = [[MLNLight alloc] initWithMBGLLight:&light];
        lightFromMLNLight = mglLight.mbglLight;

        XCTAssertEqual(light.getPosition(), lightFromMLNLight.getPosition());
        positionTransition = lightFromMLNLight.getPositionTransition();
        XCTAssert(positionTransition.delay && MLNTimeIntervalFromDuration(*positionTransition.delay) == transition.delay);
        XCTAssert(positionTransition.duration && MLNTimeIntervalFromDuration(*positionTransition.duration) == transition.duration);

    }

    // color
    {
        mbgl::style::Light light;
        MLNLight *mglLight = [[MLNLight alloc] initWithMBGLLight:&light];
        auto lightFromMLNLight = mglLight.mbglLight;

        XCTAssertEqual(light.getDefaultColor(), lightFromMLNLight.getColor());
        auto colorTransition = lightFromMLNLight.getColorTransition();
        XCTAssert(colorTransition.delay && MLNTimeIntervalFromDuration(*colorTransition.delay) == defaultTransition.delay);
        XCTAssert(colorTransition.duration && MLNTimeIntervalFromDuration(*colorTransition.duration) == defaultTransition.duration);

        mbgl::style::PropertyValue<mbgl::Color> propertyValue = { { 1, 0, 0, 1 } };
        light.setColor(propertyValue);
        light.setColorTransition(transitionOptions);

        mglLight = [[MLNLight alloc] initWithMBGLLight:&light];
        lightFromMLNLight = mglLight.mbglLight;

        XCTAssertEqual(light.getColor(), lightFromMLNLight.getColor());
        colorTransition = lightFromMLNLight.getColorTransition();
        XCTAssert(colorTransition.delay && MLNTimeIntervalFromDuration(*colorTransition.delay) == transition.delay);
        XCTAssert(colorTransition.duration && MLNTimeIntervalFromDuration(*colorTransition.duration) == transition.duration);

    }

    // intensity
    {
        mbgl::style::Light light;
        MLNLight *mglLight = [[MLNLight alloc] initWithMBGLLight:&light];
        auto lightFromMLNLight = mglLight.mbglLight;

        XCTAssertEqual(light.getDefaultIntensity(), lightFromMLNLight.getIntensity());
        auto intensityTransition = lightFromMLNLight.getIntensityTransition();
        XCTAssert(intensityTransition.delay && MLNTimeIntervalFromDuration(*intensityTransition.delay) == defaultTransition.delay);
        XCTAssert(intensityTransition.duration && MLNTimeIntervalFromDuration(*intensityTransition.duration) == defaultTransition.duration);

        mbgl::style::PropertyValue<float> propertyValue = { 1.0 };
        light.setIntensity(propertyValue);
        light.setIntensityTransition(transitionOptions);

        mglLight = [[MLNLight alloc] initWithMBGLLight:&light];
        lightFromMLNLight = mglLight.mbglLight;

        XCTAssertEqual(light.getIntensity(), lightFromMLNLight.getIntensity());
        intensityTransition = lightFromMLNLight.getIntensityTransition();
        XCTAssert(intensityTransition.delay && MLNTimeIntervalFromDuration(*intensityTransition.delay) == transition.delay);
        XCTAssert(intensityTransition.duration && MLNTimeIntervalFromDuration(*intensityTransition.duration) == transition.duration);

    }

}

- (void)testValueAdditions {
    MLNSphericalPosition position = MLNSphericalPositionMake(1.15, 210, 30);

    XCTAssertEqual([NSValue valueWithMLNSphericalPosition:position].MLNSphericalPositionValue.radial, position.radial);
    XCTAssertEqual([NSValue valueWithMLNSphericalPosition:position].MLNSphericalPositionValue.azimuthal, position.azimuthal);
    XCTAssertEqual([NSValue valueWithMLNSphericalPosition:position].MLNSphericalPositionValue.polar, position.polar);
    XCTAssertEqual([NSValue valueWithMLNLightAnchor:MLNLightAnchorMap].MLNLightAnchorValue, MLNLightAnchorMap);
    XCTAssertEqual([NSValue valueWithMLNLightAnchor:MLNLightAnchorViewport].MLNLightAnchorValue, MLNLightAnchorViewport);
}

@end
