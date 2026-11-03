#import <MapLibre.h>
#import <XCTest/XCTest.h>

@interface MLNMapViewCenterClampTests : XCTestCase <MLNMapViewDelegate>

@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) XCTestExpectation *styleLoadingExpectation;

@end

@implementation MLNMapViewCenterClampTests

- (void)setUp {
    [super setUp];

    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    self.mapView = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 400, 400) styleURL:styleURL];
    self.mapView.delegate = self;

    if (!self.mapView.style) {
        _styleLoadingExpectation = [self expectationWithDescription:@"Map view should finish loading style."];
        [self waitForExpectationsWithTimeout:10 handler:nil];
    }
}

- (void)mapView:(MLNMapView *)mapView didFinishLoadingStyle:(MLNStyle *)style {
    XCTAssertNotNil(mapView.style);
    XCTAssertEqual(mapView.style, style);

    [_styleLoadingExpectation fulfill];
}

- (void)tearDown {
    self.mapView = nil;
    [super tearDown];
}

- (void)testCenterClampedToGroundDefaultsToOff {
    XCTAssertFalse(self.mapView.centerClampedToGround);
}

- (void)testCenterClampedToGroundGetterSetter {
    self.mapView.centerClampedToGround = YES;
    XCTAssertTrue(self.mapView.centerClampedToGround);

    self.mapView.centerClampedToGround = NO;
    XCTAssertFalse(self.mapView.centerClampedToGround);
}

@end
