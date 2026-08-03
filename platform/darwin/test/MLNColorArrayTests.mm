#import <Mapbox.h>
#import <XCTest/XCTest.h>

#import "MLNStyleLayer_Private.h"

#include <mbgl/style/layers/hillshade_layer.hpp>

/// Regression tests for https://github.com/maplibre/maplibre-native/issues/4453.
///
/// `colorArray` and `numberArray` properties must accept a scalar, the same way the style spec
/// (which keeps scalar defaults for them), `mbgl::style::conversion` and MapLibre GL JS do.
@interface MLNColorArrayTests : XCTestCase
@end

@implementation MLNColorArrayTests

- (MLNHillshadeStyleLayer *)hillshadeLayer {
  MLNPointFeature *feature = [[MLNPointFeature alloc] init];
  MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"sourceID"
                                                                shape:feature
                                                              options:nil];
  return [[MLNHillshadeStyleLayer alloc] initWithIdentifier:@"layerID" source:source];
}

- (void)testColorArrayPropertyAcceptsSingleColor {
  MLNHillshadeStyleLayer *layer = [self hillshadeLayer];
  auto rawLayer = static_cast<mbgl::style::HillshadeLayer *>(layer.rawLayer);

  layer.hillshadeShadowColor = [NSExpression expressionForConstantValue:[MLNColor redColor]];

  mbgl::style::PropertyValue<std::vector<mbgl::Color>> propertyValue =
      std::vector<mbgl::Color>{mbgl::Color::red()};
  XCTAssertEqual(rawLayer->getHillshadeShadowColor(), propertyValue,
                 @"Setting hillshadeShadowColor to a single color should update "
                 @"hillshade-shadow-color with a one-element color array.");
}

- (void)testColorArrayPropertyAcceptsColorArray {
  MLNHillshadeStyleLayer *layer = [self hillshadeLayer];
  auto rawLayer = static_cast<mbgl::style::HillshadeLayer *>(layer.rawLayer);

  layer.hillshadeHighlightColor = [NSExpression
      expressionForConstantValue:@[ [MLNColor redColor], [MLNColor blueColor] ]];

  mbgl::style::PropertyValue<std::vector<mbgl::Color>> propertyValue =
      std::vector<mbgl::Color>{mbgl::Color::red(), mbgl::Color::blue()};
  XCTAssertEqual(rawLayer->getHillshadeHighlightColor(), propertyValue,
                 @"Setting hillshadeHighlightColor to an array of colors should update "
                 @"hillshade-highlight-color.");
}

- (void)testNumberArrayPropertyAcceptsSingleNumber {
  MLNHillshadeStyleLayer *layer = [self hillshadeLayer];
  auto rawLayer = static_cast<mbgl::style::HillshadeLayer *>(layer.rawLayer);

  layer.hillshadeIlluminationDirection = [NSExpression expressionForConstantValue:@335];

  mbgl::style::PropertyValue<std::vector<float>> propertyValue = std::vector<float>{335};
  XCTAssertEqual(rawLayer->getHillshadeIlluminationDirection(), propertyValue,
                 @"Setting hillshadeIlluminationDirection to a single number should update "
                 @"hillshade-illumination-direction with a one-element number array.");
}

@end
