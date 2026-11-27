#import <MapLibre.h>
#import <XCTest/XCTest.h>

#import "MLNMapView_Private.h"

#include <mln/map/map.hpp>
#include <mln/map/map_options.hpp>

@interface MLNMapViewKeepUprightMarginTests : XCTestCase
@end

@implementation MLNMapViewKeepUprightMarginTests

- (void)setUp {
  [super setUp];
  [MLNSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
}

- (void)tearDown {
  [MLNSettings setApiKey:nil];
  [super tearDown];
}

- (MLNMapView *)mapViewWithOptions:(MLNMapOptions *)options {
  options.styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner"
                                                              withExtension:@"json"];
  return [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 64, 64) options:options];
}

- (void)testKeepUprightMarginIsOffByDefault {
  MLNMapOptions *options = [[MLNMapOptions alloc] init];
  XCTAssertEqual(options.keepUprightMargin, 0.0f);
  MLNMapView *mapView = [self mapViewWithOptions:options];
  XCTAssertEqual(mapView.mbglMap.getMapOptions().keepUprightMargin(), 0.0f);
}

- (void)testKeepUprightMarginIsPassedToTheMap {
  MLNMapOptions *options = [[MLNMapOptions alloc] init];
  options.keepUprightMargin = 20;
  MLNMapView *mapView = [self mapViewWithOptions:options];
  XCTAssertEqual(mapView.mbglMap.getMapOptions().keepUprightMargin(), 20.0f);
}

- (void)testKeepUprightMarginIsClamped {
  MLNMapOptions *options = [[MLNMapOptions alloc] init];
  options.keepUprightMargin = 90;
  MLNMapView *mapView = [self mapViewWithOptions:options];
  XCTAssertEqual(mapView.mbglMap.getMapOptions().keepUprightMargin(), 45.0f);
}

@end
