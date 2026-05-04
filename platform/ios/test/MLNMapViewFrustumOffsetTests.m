#import <Mapbox.h>
#import <XCTest/XCTest.h>

@interface MLNMapViewFrustumOffsetTests : XCTestCase <MLNMapViewDelegate>

@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) XCTestExpectation *styleLoadingExpectation;

@end

@implementation MLNMapViewFrustumOffsetTests

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

- (void)testFrustumOffsetGetterSetter {
    // Test default value (should be UIEdgeInsetsZero)
    UIEdgeInsets defaultOffset = self.mapView.frustumOffset;
    XCTAssertEqual(defaultOffset.top, 0.0);
    XCTAssertEqual(defaultOffset.left, 0.0);
    XCTAssertEqual(defaultOffset.bottom, 0.0);
    XCTAssertEqual(defaultOffset.right, 0.0);

    // Test setting a non-zero offset
    UIEdgeInsets testOffset = UIEdgeInsetsMake(50.0, 40.0, 30.0, 20.0);
    self.mapView.frustumOffset = testOffset;

    UIEdgeInsets retrievedOffset = self.mapView.frustumOffset;
    XCTAssertEqualWithAccuracy(retrievedOffset.top, testOffset.top, 0.01);
    XCTAssertEqualWithAccuracy(retrievedOffset.left, testOffset.left, 0.01);
    XCTAssertEqualWithAccuracy(retrievedOffset.bottom, testOffset.bottom, 0.01);
    XCTAssertEqualWithAccuracy(retrievedOffset.right, testOffset.right, 0.01);
}

- (void)testFrustumOffsetZeroInsets {
    // Test setting zero insets
    UIEdgeInsets zeroOffset = UIEdgeInsetsZero;
    self.mapView.frustumOffset = zeroOffset;

    UIEdgeInsets retrievedOffset = self.mapView.frustumOffset;
    XCTAssertEqual(retrievedOffset.top, 0.0);
    XCTAssertEqual(retrievedOffset.left, 0.0);
    XCTAssertEqual(retrievedOffset.bottom, 0.0);
    XCTAssertEqual(retrievedOffset.right, 0.0);
}

- (void)testFrustumOffsetMultipleChanges {
    // Test changing the offset multiple times
    UIEdgeInsets firstOffset = UIEdgeInsetsMake(10.0, 20.0, 30.0, 40.0);
    self.mapView.frustumOffset = firstOffset;
    UIEdgeInsets retrievedOffset = self.mapView.frustumOffset;
    XCTAssertEqualWithAccuracy(retrievedOffset.top, 10.0, 0.01);
    XCTAssertEqualWithAccuracy(retrievedOffset.left, 20.0, 0.01);
    XCTAssertEqualWithAccuracy(retrievedOffset.bottom, 30.0, 0.01);
    XCTAssertEqualWithAccuracy(retrievedOffset.right, 40.0, 0.01);

    UIEdgeInsets secondOffset = UIEdgeInsetsMake(100.0, 200.0, 300.0, 400.0);
    self.mapView.frustumOffset = secondOffset;
    retrievedOffset = self.mapView.frustumOffset;
    XCTAssertEqualWithAccuracy(retrievedOffset.top, 100.0, 0.01);
    XCTAssertEqualWithAccuracy(retrievedOffset.left, 200.0, 0.01);
    XCTAssertEqualWithAccuracy(retrievedOffset.bottom, 300.0, 0.01);
    XCTAssertEqualWithAccuracy(retrievedOffset.right, 400.0, 0.01);

    // Test setting back to zero
    UIEdgeInsets zeroOffset = UIEdgeInsetsZero;
    self.mapView.frustumOffset = zeroOffset;
    retrievedOffset = self.mapView.frustumOffset;
    XCTAssertEqual(retrievedOffset.top, 0.0);
    XCTAssertEqual(retrievedOffset.left, 0.0);
    XCTAssertEqual(retrievedOffset.bottom, 0.0);
    XCTAssertEqual(retrievedOffset.right, 0.0);
}

@end
