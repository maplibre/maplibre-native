#import <Mapbox.h>
#import <XCTest/XCTest.h>
#import <TargetConditionals.h>

#if TARGET_OS_IPHONE
    #define MLNEdgeInsetsZero UIEdgeInsetsZero
#else
    #define MLNEdgeInsetsZero NSEdgeInsetsZero
#endif

@interface MLNMapViewTests : XCTestCase <MLNMapViewDelegate>

@property (nonatomic) MLNMapView *mapView;

@end

@implementation MLNMapViewTests {
    XCTestExpectation *_styleLoadingExpectation;
}

- (void)setUp {
    [super setUp];

    [MLNSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    self.mapView = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 64, 64) styleURL:styleURL];
    self.mapView.delegate = self;
    if (!self.mapView.style) {
        _styleLoadingExpectation = [self expectationWithDescription:@"Map view should finish loading style."];
        [self waitForExpectationsWithTimeout:10 handler:nil];
    }
}

- (void)tearDown {
    _styleLoadingExpectation = nil;
    self.mapView = nil;
    [MLNSettings setApiKey:nil];
    [super tearDown];
}

- (void)mapView:(MLNMapView *)mapView didFinishLoadingStyle:(MLNStyle *)style {
    XCTAssertNotNil(mapView.style);
    XCTAssertEqual(mapView.style, style);

    [_styleLoadingExpectation fulfill];
}

- (void)testCoordinateBoundsConversion {
    [self.mapView setCenterCoordinate:CLLocationCoordinate2DMake(33, 179)];

    MLNCoordinateBounds leftAntimeridianBounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(-75, 175), CLLocationCoordinate2DMake(75, 180));
    CGRect leftAntimeridianBoundsRect = [self.mapView convertCoordinateBounds:leftAntimeridianBounds toRectToView:self.mapView];

    MLNCoordinateBounds rightAntimeridianBounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(-75, -180), CLLocationCoordinate2DMake(75, -175));
    CGRect rightAntimeridianBoundsRect = [self.mapView convertCoordinateBounds:rightAntimeridianBounds toRectToView:self.mapView];

    MLNCoordinateBounds spanningBounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(24, 140), CLLocationCoordinate2DMake(44, 240));
    CGRect spanningBoundsRect = [self.mapView convertCoordinateBounds:spanningBounds toRectToView:self.mapView];

    // If the resulting CGRect from -convertCoordinateBounds:toRectToView:
    // intersects the set of bounds to the left and right of the
    // antimeridian, then we know that the CGRect spans across the antimeridian
    XCTAssertTrue(CGRectIntersectsRect(spanningBoundsRect, leftAntimeridianBoundsRect), @"Resulting");
    XCTAssertTrue(CGRectIntersectsRect(spanningBoundsRect, rightAntimeridianBoundsRect), @"Something");
}

#if TARGET_OS_IPHONE
- (void)testUserTrackingModeCompletion {
    __block BOOL completed = NO;
    [self.mapView setUserTrackingMode:MLNUserTrackingModeNone animated:NO completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when the mode is unchanged.");

    completed = NO;
    [self.mapView setUserTrackingMode:MLNUserTrackingModeNone animated:YES completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when the mode is unchanged.");

    completed = NO;
    [self.mapView setUserTrackingMode:MLNUserTrackingModeFollow animated:NO completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when there’s no location.");

    completed = NO;
    [self.mapView setUserTrackingMode:MLNUserTrackingModeFollowWithHeading animated:YES completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when there’s no location.");
}

- (void)testTargetCoordinateCompletion {
    __block BOOL completed = NO;
    [self.mapView setTargetCoordinate:kCLLocationCoordinate2DInvalid animated:NO completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when the target coordinate is unchanged.");

    completed = NO;
    [self.mapView setTargetCoordinate:kCLLocationCoordinate2DInvalid animated:YES completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when the target coordinate is unchanged.");

    completed = NO;
    [self.mapView setUserTrackingMode:MLNUserTrackingModeFollow animated:NO completionHandler:nil];
    [self.mapView setTargetCoordinate:CLLocationCoordinate2DMake(39.128106, -84.516293) animated:YES completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when not tracking user course.");

    completed = NO;
    [self.mapView setUserTrackingMode:MLNUserTrackingModeFollowWithCourse animated:NO completionHandler:nil];
    [self.mapView setTargetCoordinate:CLLocationCoordinate2DMake(39.224407, -84.394957) animated:YES completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when there’s no location.");
}
#endif

- (void)testVisibleCoordinatesCompletion {
    XCTestExpectation *expectation = [self expectationWithDescription:@"Completion block should get called when not animated"];
    MLNCoordinateBounds unitBounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(0, 0), CLLocationCoordinate2DMake(1, 1));
    [self.mapView setVisibleCoordinateBounds:unitBounds edgePadding:MLNEdgeInsetsZero animated:NO completionHandler:^{
        [expectation fulfill];
    }];
    [self waitForExpectations:@[expectation] timeout:1];

#if TARGET_OS_IPHONE
    expectation = [self expectationWithDescription:@"Completion block should get called when animated"];
    CLLocationCoordinate2D antiunitCoordinates[] = {
        CLLocationCoordinate2DMake(0, 0),
        CLLocationCoordinate2DMake(-1, -1),
    };
    [self.mapView setVisibleCoordinates:antiunitCoordinates
                             count:sizeof(antiunitCoordinates) / sizeof(antiunitCoordinates[0])
                       edgePadding:UIEdgeInsetsZero
                         direction:0
                          duration:0
           animationTimingFunction:nil
                 completionHandler:^{
        [expectation fulfill];
    }];
    [self waitForExpectations:@[expectation] timeout:1];
#endif
}

- (void)testShowAnnotationsCompletion {
    __block BOOL completed = NO;
    [self.mapView showAnnotations:@[] edgePadding:MLNEdgeInsetsZero animated:NO completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when there are no annotations to show.");

    XCTestExpectation *expectation = [self expectationWithDescription:@"Completion block should get called when not animated"];
    MLNPointAnnotation *annotation = [[MLNPointAnnotation alloc] init];
    [self.mapView showAnnotations:@[annotation] edgePadding:MLNEdgeInsetsZero animated:NO completionHandler:^{
        [expectation fulfill];
    }];
    [self waitForExpectations:@[expectation] timeout:1];

    expectation = [self expectationWithDescription:@"Completion block should get called when animated."];
    [self.mapView showAnnotations:@[annotation] edgePadding:MLNEdgeInsetsZero animated:YES completionHandler:^{
        [expectation fulfill];
    }];
    [self waitForExpectations:@[expectation] timeout:1];
}

- (void)testTileCache {
    self.mapView.tileCacheEnabled = NO;
    XCTAssertEqual(self.mapView.tileCacheEnabled, NO);

    self.mapView.tileCacheEnabled = YES;
    XCTAssertEqual(self.mapView.tileCacheEnabled, YES);
}

- (void)testStyleJSON {
    // Test getting style JSON
    NSString *styleJSON = self.mapView.styleJSON;
    XCTAssertNotNil(styleJSON, @"Style JSON should not be nil");

    // Verify the JSON is valid
    NSError *error = nil;
    id jsonObject = [NSJSONSerialization JSONObjectWithData:[styleJSON dataUsingEncoding:NSUTF8StringEncoding]
                                                   options:0
                                                     error:&error];
    XCTAssertNil(error, @"Style JSON should be valid JSON");
    XCTAssertNotNil(jsonObject, @"Style JSON should parse to a valid object");
    XCTAssertTrue([jsonObject isKindOfClass:[NSDictionary class]], @"Style JSON should represent a dictionary");

    // Reset style loading expectation before setting new style
    _styleLoadingExpectation = nil;

    // Test setting style JSON
    NSString *newStyleJSON = @"{\"version\": 8, \"sources\": {}, \"layers\": []}";
    self.mapView = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 64, 64) styleJSON:newStyleJSON];

    // Verify the style was updated
    NSString *updatedStyleJSON = self.mapView.styleJSON;
    XCTAssertEqualObjects([self normalizeJSON:updatedStyleJSON],
                         [self normalizeJSON:newStyleJSON],
                         @"Style JSON should match what was set");

    // Reset to style URL for other tests
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    self.mapView.styleURL = styleURL;
}

// Helper method to normalize JSON strings for comparison
- (NSString *)normalizeJSON:(NSString *)jsonString {
    NSError *error = nil;
    id jsonObject = [NSJSONSerialization JSONObjectWithData:[jsonString dataUsingEncoding:NSUTF8StringEncoding]
                                                   options:0
                                                     error:&error];
    if (error) {
        return jsonString;
    }

    NSData *normalizedData = [NSJSONSerialization dataWithJSONObject:jsonObject
                                                           options:0
                                                             error:&error];
    if (error) {
        return jsonString;
    }

    NSString *normalizedString = [[NSString alloc] initWithData:normalizedData encoding:NSUTF8StringEncoding];
    return normalizedString ?: jsonString;
}

@end
