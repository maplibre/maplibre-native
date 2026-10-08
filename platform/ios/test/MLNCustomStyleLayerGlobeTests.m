#import <Mapbox.h>
#import <XCTest/XCTest.h>

@interface MLNRecordingStyleLayer : MLNCustomStyleLayer
@property (nonatomic) MLNStyleLayerDrawingContext lastContext;
@property (nonatomic) NSInteger drawCount;
@end

@implementation MLNRecordingStyleLayer

- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context {
    self.lastContext = context;
    self.drawCount++;
}

@end

@interface MLNCustomStyleLayerGlobeTests : XCTestCase <MLNMapViewDelegate>
@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) UIWindow *window;
@property (nonatomic) XCTestExpectation *styleLoadingExpectation;
@property (nonatomic) XCTestExpectation *renderFrameExpectation;
@end

@implementation MLNCustomStyleLayerGlobeTests

- (MLNRecordingStyleLayer *)drawStyleNamed:(NSString *)name
{
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:name withExtension:@"json"];
    _styleLoadingExpectation = [self expectationWithDescription:@"style"];
    _mapView = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 256, 256) styleURL:styleURL];
    _mapView.delegate = self;
    _window = [[UIWindow alloc] initWithFrame:_mapView.bounds];
    [_window addSubview:_mapView];
    [_window makeKeyAndVisible];
    [self waitForExpectationsWithTimeout:10 handler:nil];

    MLNRecordingStyleLayer *layer = [[MLNRecordingStyleLayer alloc] initWithIdentifier:@"recording"];
    [_mapView.style addLayer:layer];
    for (int frame = 0; frame < 2; frame++) {
        _renderFrameExpectation = [self expectationWithDescription:@"frame"];
        [layer setNeedsDisplay];
        [self waitForExpectationsWithTimeout:10 handler:nil];
    }
    return layer;
}

- (void)testDrawingContextCarriesTheGlobe
{
    MLNRecordingStyleLayer *layer = [self drawStyleNamed:@"globe"];
    XCTAssertGreaterThan(layer.drawCount, 0);
    MLNStyleLayerDrawingContext context = layer.lastContext;
    XCTAssertTrue(context.globe);
    XCTAssertEqual(context.projectionTransition, 1.0);
    XCTAssertNotEqual(context.globeProjectionMatrix.m00, 0.0);
    XCTAssertNotEqual(context.globeClippingPlane[3], 0.0, @"the plane's offset from the sphere's center");
}

- (void)testDrawingContextOnMercator
{
    MLNRecordingStyleLayer *layer = [self drawStyleNamed:@"one-liner"];
    XCTAssertGreaterThan(layer.drawCount, 0);
    MLNStyleLayerDrawingContext context = layer.lastContext;
    XCTAssertFalse(context.globe);
    XCTAssertEqual(context.projectionTransition, 0.0);
}

- (void)mapView:(MLNMapView *)mapView didFinishLoadingStyle:(MLNStyle *)style
{
    [_styleLoadingExpectation fulfill];
    _styleLoadingExpectation = nil;
}

- (void)mapViewDidFinishRenderingFrame:(MLNMapView *)mapView fullyRendered:(BOOL)fullyRendered
{
    [_renderFrameExpectation fulfill];
    _renderFrameExpectation = nil;
}

@end
