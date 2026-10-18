#import <Mapbox.h>
#import <XCTest/XCTest.h>

#import "MLNMapView_Private.h"

#include <mln/map/map.hpp>
#include <mln/style/style.hpp>
#include <mln/style/terrain.hpp>

// Records the drawing context of the first frame drawn after `drawExpectation` is set
// whose terrain exaggeration matches `expectsTerrain`.
@interface MLNTerrainRecordingStyleLayer : MLNCustomStyleLayer
@property (nonatomic) XCTestExpectation *drawExpectation;
@property (nonatomic) BOOL expectsTerrain;
@property (nonatomic) MLNStyleLayerDrawingContext drawnContext;
#if MLN_RENDER_BACKEND_METAL
@property (nonatomic) id<MTLTexture> drawnDepthTexture;
#endif
@end

@implementation MLNTerrainRecordingStyleLayer

- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context {
  if (!self.drawExpectation || (context.terrainExaggeration != 0) != self.expectsTerrain) {
    return;
  }
  self.drawnContext = context;
#if MLN_RENDER_BACKEND_METAL
  self.drawnDepthTexture = self.terrainDepthTexture;
#endif
  [self.drawExpectation fulfill];
  self.drawExpectation = nil;
}

@end

@interface MLNCustomStyleLayerTerrainTests : XCTestCase <MLNMapViewDelegate>

@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) UIWindow *window;
@property (nonatomic) XCTestExpectation *styleLoadingExpectation;
@property (nonatomic) MLNTerrainRecordingStyleLayer *layer;

@end

@implementation MLNCustomStyleLayerTerrainTests

- (void)setUp {
  [super setUp];

  [MLNSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
  NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner"
                                                             withExtension:@"json"];
  CGRect bounds = CGRectMake(0, 0, 200, 200);
  self.mapView = [[MLNMapView alloc] initWithFrame:bounds styleURL:styleURL];
  self.mapView.delegate = self;

  // The custom layer is only drawn while the map view is on screen
  self.window = [[UIWindow alloc] initWithFrame:bounds];
  [self.window addSubview:self.mapView];
  [self.window makeKeyAndVisible];

  if (!self.mapView.style) {
    _styleLoadingExpectation =
        [self expectationWithDescription:@"Map view should finish loading style."];
    [self waitForExpectationsWithTimeout:10 handler:nil];
  }

  self.layer = [[MLNTerrainRecordingStyleLayer alloc] initWithIdentifier:@"terrain-recorder"];
  [self.mapView.style addLayer:self.layer];
}

- (void)tearDown {
  self.layer = nil;
  self.mapView = nil;
  self.window = nil;
  [MLNSettings setApiKey:nil];

  [super tearDown];
}

- (void)mapView:(MLNMapView *)mapView didFinishLoadingStyle:(MLNStyle *)style {
  [_styleLoadingExpectation fulfill];
}

- (void)waitForDrawWithTerrain:(BOOL)expectsTerrain {
  self.layer.expectsTerrain = expectsTerrain;
  self.layer.drawExpectation =
      [self expectationWithDescription:@"Custom layer should draw a frame."];
  [self.layer setNeedsDisplay];
  [self waitForExpectationsWithTimeout:10 handler:nil];
}

- (void)testTerrainOff {
  [self waitForDrawWithTerrain:NO];

  XCTAssertEqual(self.layer.drawnContext.terrainExaggeration, 0);
#if MLN_RENDER_BACKEND_METAL
  XCTAssertNil(self.layer.drawnDepthTexture);
#endif
}

- (void)testTerrainOn {
  // No DEM source backs the terrain, so the terrain is on but has no mesh or depth to
  // draw; this checks what the custom layer is handed, not the depth texture's contents.
  self.mapView.mbglMap.getStyle().setTerrain(std::make_unique<mln::style::Terrain>("dem", 1.5f));
  [self waitForDrawWithTerrain:YES];

  XCTAssertEqualWithAccuracy(self.layer.drawnContext.terrainExaggeration, 1.5, 1e-6);

  self.mapView.mbglMap.getStyle().setTerrain(nullptr);
  [self waitForDrawWithTerrain:NO];

  XCTAssertEqual(self.layer.drawnContext.terrainExaggeration, 0);
#if MLN_RENDER_BACKEND_METAL
  XCTAssertNil(self.layer.drawnDepthTexture);
#endif
}

@end
