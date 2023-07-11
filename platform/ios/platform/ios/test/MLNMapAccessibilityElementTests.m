#import <Mapbox.h>
#import <XCTest/XCTest.h>

#import "../../ios/src/MLNMapAccessibilityElement.h"

@interface MLNMapAccessibilityElementTests : XCTestCase
@end

@implementation MLNMapAccessibilityElementTests

- (void)testFeatureLabels {
    MLNPointFeature *feature = [[MLNPointFeature alloc] init];
    feature.attributes = @{
        @"name": @"Local",
        @"name_en": @"English",
        @"name_es": @"Spanish",
        @"name_fr": @"French",
        @"name_tlh": @"Klingon",
    };
    MLNFeatureAccessibilityElement *element = [[MLNFeatureAccessibilityElement alloc] initWithAccessibilityContainer:self feature:feature];
    XCTAssertEqualObjects(element.accessibilityLabel, @"English", @"Accessibility label should be localized.");

    feature.attributes = @{
        @"name": @"Цинциннати",
        @"name_en": @"Цинциннати",
    };
    element = [[MLNFeatureAccessibilityElement alloc] initWithAccessibilityContainer:self feature:feature];
    XCTAssertEqualObjects(element.accessibilityLabel, @"Cincinnati", @"Accessibility label should be romanized.");
}

- (void)testPlaceFeatureValues {
    MLNPointFeature *feature = [[MLNPointFeature alloc] init];
    feature.attributes = @{
        @"type": @"village_green",
    };
    MLNPlaceFeatureAccessibilityElement *element = [[MLNPlaceFeatureAccessibilityElement alloc] initWithAccessibilityContainer:self feature:feature];
    XCTAssertEqualObjects(element.accessibilityValue, @"village green");
    
    feature = [[MLNPointFeature alloc] init];
    feature.attributes = @{
        @"maki": @"cat",
    };
    element = [[MLNPlaceFeatureAccessibilityElement alloc] initWithAccessibilityContainer:self feature:feature];
    XCTAssertEqualObjects(element.accessibilityValue, @"cat");
    
    feature = [[MLNPointFeature alloc] init];
    feature.attributes = @{
        @"elevation_ft": @31337,
        @"elevation_m": @1337,
    };
    element = [[MLNPlaceFeatureAccessibilityElement alloc] initWithAccessibilityContainer:self feature:feature];
    XCTAssertEqualObjects(element.accessibilityValue, @"31,337 feet");
}

- (void)testRoadFeatureValues {
    CLLocationCoordinate2D coordinates[] = {
        CLLocationCoordinate2DMake(0, 0),
        CLLocationCoordinate2DMake(0, 1),
        CLLocationCoordinate2DMake(1, 2),
        CLLocationCoordinate2DMake(2, 2),
    };
    MLNPolylineFeature *roadFeature = [MLNPolylineFeature polylineWithCoordinates:coordinates count:sizeof(coordinates) / sizeof(coordinates[0])];
    roadFeature.attributes = @{
        @"ref": @"42",
        @"oneway": @"true",
    };
    MLNRoadFeatureAccessibilityElement *element = [[MLNRoadFeatureAccessibilityElement alloc] initWithAccessibilityContainer:self feature:roadFeature];
    XCTAssertEqualObjects(element.accessibilityValue, @"Route 42, One way, southwest to northeast");
    
    CLLocationCoordinate2D opposingCoordinates[] = {
        CLLocationCoordinate2DMake(2, 1),
        CLLocationCoordinate2DMake(1, 0),
    };
    MLNPolylineFeature *opposingRoadFeature = [MLNPolylineFeature polylineWithCoordinates:opposingCoordinates count:sizeof(opposingCoordinates) / sizeof(opposingCoordinates[0])];
    opposingRoadFeature.attributes = @{
        @"ref": @"42",
        @"oneway": @"true",
    };
    MLNMultiPolylineFeature *dividedRoadFeature = [MLNMultiPolylineFeature multiPolylineWithPolylines:@[roadFeature, opposingRoadFeature]];
    dividedRoadFeature.attributes = @{
        @"ref": @"42",
    };
    element = [[MLNRoadFeatureAccessibilityElement alloc] initWithAccessibilityContainer:self feature:dividedRoadFeature];
    XCTAssertEqualObjects(element.accessibilityValue, @"Route 42, Divided road, southwest to northeast");
}

@end
