#import <MapLibre.h>
#import <XCTest/XCTest.h>

@interface MLNMapViewPixelRatioTests : XCTestCase

@property (nonatomic) UIWindow *window;

@end

@implementation MLNMapViewPixelRatioTests

- (void)setUp {
  [super setUp];
  self.window = [[UIWindow alloc] initWithFrame:CGRectMake(0, 0, 320, 480)];
  self.window.rootViewController = [[UIViewController alloc] init];
  [self.window makeKeyAndVisible];
}

- (void)tearDown {
  self.window.hidden = YES;
  self.window = nil;
  [super tearDown];
}

- (MLNMapView *)mapViewWithPixelRatio:(CGFloat)pixelRatio {
  MLNMapOptions *options = [[MLNMapOptions alloc] init];
  options.styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner"
                                                              withExtension:@"json"];
  options.pixelRatio = pixelRatio;
  MLNMapView *mapView = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 320, 480)
                                                  options:options];
  [self.window.rootViewController.view addSubview:mapView];
  [mapView setNeedsLayout];
  [mapView layoutIfNeeded];
  return mapView;
}

- (void)testPixelRatioDefaultsToZero {
  XCTAssertEqual([[MLNMapOptions alloc] init].pixelRatio, 0);
}

#if MLN_RENDER_BACKEND_METAL

- (void)testDefaultPixelRatioFollowsScreen {
  MLNMapView *mapView = [self mapViewWithPixelRatio:0];
  CGFloat scale = self.window.screen.nativeScale;
  CGSize drawableSize = mapView.backendResource.mtkView.drawableSize;
  XCTAssertEqualWithAccuracy(drawableSize.width, 320 * scale, 1);
  XCTAssertEqualWithAccuracy(drawableSize.height, 480 * scale, 1);
}

- (void)testPixelRatioOverridesScreenScale {
  CGFloat pixelRatio = self.window.screen.nativeScale + 2;
  MLNMapView *mapView = [self mapViewWithPixelRatio:pixelRatio];
  MTKView *mtkView = mapView.backendResource.mtkView;
  XCTAssertEqualWithAccuracy(mtkView.contentScaleFactor, pixelRatio, 0.001);
  XCTAssertEqualWithAccuracy(mtkView.drawableSize.width, 320 * pixelRatio, 1);
  XCTAssertEqualWithAccuracy(mtkView.drawableSize.height, 480 * pixelRatio, 1);
}

#endif

@end
