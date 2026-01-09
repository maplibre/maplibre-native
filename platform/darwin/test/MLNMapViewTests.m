#import <Mapbox.h>
#import <XCTest/XCTest.h>
#import <TargetConditionals.h>

#if TARGET_OS_IPHONE
    #define MLNEdgeInsetsZero UIEdgeInsetsZero
#else
    #define MLNEdgeInsetsZero NSEdgeInsetsZero
#endif

static MLNMapView *mapView;
@interface MLNMapViewTests : XCTestCase <MLNMapViewDelegate>
@end

@implementation MLNMapViewTests {
    XCTestExpectation *_styleLoadingExpectation;
    XCTestExpectation *_styleLoadErrorExpectation;
}

- (void)setUp {
    [super setUp];

    [MLNSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    mapView = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 64, 64) styleURL:styleURL];
    mapView.delegate = self;
    if (!mapView.style) {
        _styleLoadingExpectation = [self expectationWithDescription:@"Map view should finish loading style."];
        [self waitForExpectationsWithTimeout:10 handler:nil];
    }
}

- (void)tearDown {
    _styleLoadingExpectation = nil;
    mapView = nil;
    [MLNSettings setApiKey:nil];
    [super tearDown];
}

- (void)mapView:(MLNMapView *)mapView didFinishLoadingStyle:(MLNStyle *)style {
    XCTAssertNotNil(mapView.style);
    XCTAssertEqual(mapView.style, style);

    [_styleLoadingExpectation fulfill];
}

- (void)mapViewDidFailLoadingMap:(MLNMapView *)mapView withError:(NSError *)error {
    XCTAssertNotNil(error);
    XCTAssertEqual(error.domain, MLNErrorDomain);
    if (error.code == MLNErrorCodeLoadStyleFailed || error.code == MLNErrorCodeParseStyleFailed) {
        [_styleLoadErrorExpectation fulfill];
    }
}

- (void)testCoordinateBoundsConversion {
    [mapView setCenterCoordinate:CLLocationCoordinate2DMake(33, 179)];

    MLNCoordinateBounds leftAntimeridianBounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(-75, 175), CLLocationCoordinate2DMake(75, 180));
    CGRect leftAntimeridianBoundsRect = [mapView convertCoordinateBounds:leftAntimeridianBounds toRectToView:mapView];

    MLNCoordinateBounds rightAntimeridianBounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(-75, -180), CLLocationCoordinate2DMake(75, -175));
    CGRect rightAntimeridianBoundsRect = [mapView convertCoordinateBounds:rightAntimeridianBounds toRectToView:mapView];

    MLNCoordinateBounds spanningBounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(24, 140), CLLocationCoordinate2DMake(44, 240));
    CGRect spanningBoundsRect = [mapView convertCoordinateBounds:spanningBounds toRectToView:mapView];

    // If the resulting CGRect from -convertCoordinateBounds:toRectToView:
    // intersects the set of bounds to the left and right of the
    // antimeridian, then we know that the CGRect spans across the antimeridian
    XCTAssertTrue(CGRectIntersectsRect(spanningBoundsRect, leftAntimeridianBoundsRect), @"Resulting");
    XCTAssertTrue(CGRectIntersectsRect(spanningBoundsRect, rightAntimeridianBoundsRect), @"Something");
}

#if TARGET_OS_IPHONE
- (void)testUserTrackingModeCompletion {
    __block BOOL completed = NO;
    [mapView setUserTrackingMode:MLNUserTrackingModeNone animated:NO completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when the mode is unchanged.");

    completed = NO;
    [mapView setUserTrackingMode:MLNUserTrackingModeNone animated:YES completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when the mode is unchanged.");

    completed = NO;
    [mapView setUserTrackingMode:MLNUserTrackingModeFollow animated:NO completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when there’s no location.");

    completed = NO;
    [mapView setUserTrackingMode:MLNUserTrackingModeFollowWithHeading animated:YES completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when there’s no location.");
}

- (void)testTargetCoordinateCompletion {
    __block BOOL completed = NO;
    [mapView setTargetCoordinate:kCLLocationCoordinate2DInvalid animated:NO completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when the target coordinate is unchanged.");

    completed = NO;
    [mapView setTargetCoordinate:kCLLocationCoordinate2DInvalid animated:YES completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when the target coordinate is unchanged.");

    completed = NO;
    [mapView setUserTrackingMode:MLNUserTrackingModeFollow animated:NO completionHandler:nil];
    [mapView setTargetCoordinate:CLLocationCoordinate2DMake(39.128106, -84.516293) animated:YES completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when not tracking user course.");

    completed = NO;
    [mapView setUserTrackingMode:MLNUserTrackingModeFollowWithCourse animated:NO completionHandler:nil];
    [mapView setTargetCoordinate:CLLocationCoordinate2DMake(39.224407, -84.394957) animated:YES completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when there’s no location.");
}
#endif

- (void)testVisibleCoordinatesCompletion {
    XCTestExpectation *expectation = [self expectationWithDescription:@"Completion block should get called when not animated"];
    MLNCoordinateBounds unitBounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(0, 0), CLLocationCoordinate2DMake(1, 1));
    [mapView setVisibleCoordinateBounds:unitBounds edgePadding:MLNEdgeInsetsZero animated:NO completionHandler:^{
        [expectation fulfill];
    }];
    [self waitForExpectations:@[expectation] timeout:1];

#if TARGET_OS_IPHONE
    expectation = [self expectationWithDescription:@"Completion block should get called when animated"];
    CLLocationCoordinate2D antiunitCoordinates[] = {
        CLLocationCoordinate2DMake(0, 0),
        CLLocationCoordinate2DMake(-1, -1),
    };
    [mapView setVisibleCoordinates:antiunitCoordinates
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
    [mapView showAnnotations:@[] edgePadding:MLNEdgeInsetsZero animated:NO completionHandler:^{
        completed = YES;
    }];
    XCTAssertTrue(completed, @"Completion block should get called synchronously when there are no annotations to show.");

    XCTestExpectation *expectation = [self expectationWithDescription:@"Completion block should get called when not animated"];
    MLNPointAnnotation *annotation = [[MLNPointAnnotation alloc] init];
    [mapView showAnnotations:@[annotation] edgePadding:MLNEdgeInsetsZero animated:NO completionHandler:^{
        [expectation fulfill];
    }];
    [self waitForExpectations:@[expectation] timeout:1];

    expectation = [self expectationWithDescription:@"Completion block should get called when animated."];
    [mapView showAnnotations:@[annotation] edgePadding:MLNEdgeInsetsZero animated:YES completionHandler:^{
        [expectation fulfill];
    }];
    [self waitForExpectations:@[expectation] timeout:1];
}

- (void)testTileCache {
    mapView.tileCacheEnabled = NO;
    XCTAssertEqual(mapView.tileCacheEnabled, NO);

    mapView.tileCacheEnabled = YES;
    XCTAssertEqual(mapView.tileCacheEnabled, YES);
}

- (void)testStyleJSONWhenMapViewInitWithStyleURL {
    // Test getting style JSON
    NSString *styleJSON = mapView.styleJSON;
    XCTAssertNotNil(styleJSON, @"Style JSON should not be nil");
    NSString * expectedJSON = @"{\"version\":8,\"sources\":{},\"layers\":[]}";
    XCTAssertEqualObjects([self normalizeJSON:expectedJSON],
                          [self normalizeJSON:styleJSON],
                         @"Style JSON should be expected");

    // Verify the JSON is valid
    NSError *error = nil;
    id jsonObject = [NSJSONSerialization JSONObjectWithData:[styleJSON dataUsingEncoding:NSUTF8StringEncoding]
                                                   options:0
                                                     error:&error];
    XCTAssertNil(error, @"Style JSON should be valid JSON");
    XCTAssertNotNil(jsonObject, @"Style JSON should parse to a valid object");
    XCTAssertTrue([jsonObject isKindOfClass:[NSDictionary class]], @"Style JSON should represent a dictionary");
}

- (void)testMapViewInitWithStyleJSON {
    // Test setting style JSON
    NSString *styleJSON = @"{\"version\": 8, \"sources\": { \"mapbox\": {\"type\": \"vector\", \"tiles\": [ \"local://tiles/{z}-{x}-{y}.mvt\" ] }}, \"layers\": [], \"metadata\": { \"test\": 1, \"type\": \"template\"}}";
    MLNMapView* mapViewWithJSON = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 64, 64) styleJSON:styleJSON];

    // Verify the style was updated
    NSString *loadedStyleJSON = mapViewWithJSON.styleJSON;
    XCTAssertEqualObjects([self normalizeJSON:loadedStyleJSON],
                          [self normalizeJSON:styleJSON],
                         @"Style JSON should match what was set");

    XCTAssertNotNil(mapViewWithJSON.style);
    XCTAssertNotNil(mapViewWithJSON.style.sources);
}

- (void)testUpdateStyleJSON {
    _styleLoadingExpectation = nil;
    _styleLoadErrorExpectation = [self expectationWithDescription:@"Style should load error"];

    // Test setting style JSON
    NSString *newStyleJSON = @"{\"version\": 8, \"sources\": { \"mapbox\": {\"type\": \"vector\", \"tiles\": [ \"local://tiles/{z}-{x}-{y}.mvt\" ] }}, \"layers\": [], \"metadata\": { \"test\": 1, \"type\": \"template\"}}";
    mapView.styleJSON = newStyleJSON;

    // Verify the style was updated
    NSString *updatedStyleJSON = mapView.styleJSON;
    XCTAssertEqualObjects([self normalizeJSON:updatedStyleJSON],
                         [self normalizeJSON:newStyleJSON],
                         @"Style JSON should match what was set");

    XCTAssertNotNil(mapView.style);
    XCTAssertNotNil(mapView.style.sources);
    // source "org.maplibre.annotations" is added by default
    XCTAssertEqual(mapView.style.sources.count, 2UL);

    // Test invalid JSON syntax
    NSString *invalidJSON = @"{invalid json";
    mapView.styleJSON = invalidJSON;
    [self waitForExpectations:@[_styleLoadErrorExpectation] timeout:10];
}

- (void)testStyleJSONAfterAddLayer {
    // Test getting style JSON
    NSString *styleJSON = mapView.styleJSON;
    XCTAssertNotNil(styleJSON, @"Style JSON should not be nil");

    // Add a raster tile source
    MLNVectorTileSource *vectorTileSource = [[MLNVectorTileSource alloc] initWithIdentifier:@"some-identifier" tileURLTemplates:@[] options:nil];
    [mapView.style addSource:vectorTileSource];

    // Add a layer using it
    MLNFillStyleLayer *fillLayer = [[MLNFillStyleLayer alloc] initWithIdentifier:@"fillLayer" source:vectorTileSource];
    [mapView.style addLayer:fillLayer];

    // Style JSON should not be updated
    NSString *updatedStyleJSON = mapView.styleJSON;

    XCTAssertEqualObjects([self normalizeJSON:updatedStyleJSON],
                          [self normalizeJSON:styleJSON],
                         @"Style JSON should be updated");
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
