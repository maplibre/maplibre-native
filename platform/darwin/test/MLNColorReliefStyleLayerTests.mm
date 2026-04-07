// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNStyleLayerTests.h"
#import "../../darwin/src/NSDate+MLNAdditions.h"

#import "MLNStyleLayer_Private.h"

#include <mbgl/style/layers/color_relief_layer.hpp>
#include <mbgl/style/transition_options.hpp>
#include <mbgl/style/expression/dsl.hpp>

@interface MLNColorReliefLayerTests : MLNStyleLayerTests
@end

@implementation MLNColorReliefLayerTests

+ (NSString *)layerType {
    return @"color-relief";
}

- (void)testProperties {
    MLNPointFeature *feature = [[MLNPointFeature alloc] init];
    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"sourceID" shape:feature options:nil];

    MLNColorReliefStyleLayer *layer = [[MLNColorReliefStyleLayer alloc] initWithIdentifier:@"layerID" source:source];
    XCTAssertNotEqual(layer.rawLayer, nullptr);
    XCTAssertEqualObjects(@(layer.rawLayer->getTypeInfo()->type), @"color-relief");
    auto rawLayer = static_cast<mbgl::style::ColorReliefLayer*>(layer.rawLayer);

    MLNTransition transitionTest = MLNTransitionMake(5, 4);


    // color-relief-opacity
    {
        XCTAssertTrue(rawLayer->getColorReliefOpacity().isUndefined(),
                      @"color-relief-opacity should be unset initially.");
        NSExpression *defaultExpression = layer.colorReliefOpacity;

        NSExpression *constantExpression = [NSExpression expressionWithFormat:@"1"];
        layer.colorReliefOpacity = constantExpression;
        mbgl::style::PropertyValue<float> propertyValue = { 1.0 };
        XCTAssertEqual(rawLayer->getColorReliefOpacity(), propertyValue,
                       @"Setting colorReliefOpacity to a constant value expression should update color-relief-opacity.");
        XCTAssertEqualObjects(layer.colorReliefOpacity, constantExpression,
                              @"colorReliefOpacity should round-trip constant value expressions.");

        constantExpression = [NSExpression expressionWithFormat:@"1"];
        XCTExpectFailure(@"Awaiting unit test refactoring https://github.com/maplibre/maplibre-native/issues/421");
        NSExpression *functionExpression = [NSExpression expressionWithFormat:@"mgl_step:from:stops:($zoomLevel, %@, %@)", constantExpression, @{@18: constantExpression}];
        layer.colorReliefOpacity = functionExpression;

        {
            using namespace mbgl::style::expression::dsl;
            propertyValue = mbgl::style::PropertyExpression<float>(
                step(zoom(), literal(1.0), 18.0, literal(1.0))
            );
        }

        XCTAssertEqual(rawLayer->getColorReliefOpacity(), propertyValue,
                       @"Setting colorReliefOpacity to a camera expression should update color-relief-opacity.");
        XCTAssertEqualObjects(layer.colorReliefOpacity, functionExpression,
                              @"colorReliefOpacity should round-trip camera expressions.");


        layer.colorReliefOpacity = nil;
        XCTAssertTrue(rawLayer->getColorReliefOpacity().isUndefined(),
                      @"Unsetting colorReliefOpacity should return color-relief-opacity to the default value.");
        XCTAssertEqualObjects(layer.colorReliefOpacity, defaultExpression,
                              @"colorReliefOpacity should return the default value after being unset.");

        functionExpression = [NSExpression expressionForKeyPath:@"bogus"];
        XCTAssertThrowsSpecificNamed(layer.colorReliefOpacity = functionExpression, NSException, NSInvalidArgumentException, @"MLNColorReliefLayer should raise an exception if a camera-data expression is applied to a property that does not support key paths to feature attributes.");
        functionExpression = [NSExpression expressionWithFormat:@"mgl_step:from:stops:(bogus, %@, %@)", constantExpression, @{@18: constantExpression}];
        functionExpression = [NSExpression expressionWithFormat:@"mgl_interpolate:withCurveType:parameters:stops:($zoomLevel, 'linear', nil, %@)", @{@10: functionExpression}];
        XCTAssertThrowsSpecificNamed(layer.colorReliefOpacity = functionExpression, NSException, NSInvalidArgumentException, @"MLNColorReliefLayer should raise an exception if a camera-data expression is applied to a property that does not support key paths to feature attributes.");
        // Transition property test
        layer.colorReliefOpacityTransition = transitionTest;
        auto toptions = rawLayer->getColorReliefOpacityTransition();
        XCTAssert(toptions.delay && MLNTimeIntervalFromDuration(*toptions.delay) == transitionTest.delay);
        XCTAssert(toptions.duration && MLNTimeIntervalFromDuration(*toptions.duration) == transitionTest.duration);

        MLNTransition colorReliefOpacityTransition = layer.colorReliefOpacityTransition;
        XCTAssertEqual(colorReliefOpacityTransition.delay, transitionTest.delay);
        XCTAssertEqual(colorReliefOpacityTransition.duration, transitionTest.duration);
    }
}

- (void)testPropertyNames {
    [self testPropertyName:@"color-relief-opacity" isBoolean:NO];
}

@end
