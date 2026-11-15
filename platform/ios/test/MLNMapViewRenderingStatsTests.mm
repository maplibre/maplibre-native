#import <MapLibre.h>
#import <XCTest/XCTest.h>

#import "MLNMapView_Private.h"

#include <mln/gfx/rendering_stats.hpp>

// Keeps what the delegate was handed, since the map view reuses one MLNRenderingStats object.
@interface MLNRenderingStatsRecorder : NSObject <MLNMapViewDelegate>
@property (nonatomic) NSInteger callCount;
@property (nonatomic) int numDrapeTargetsDeferred;
@property (nonatomic) int numNewTileBuildsDeferred;
@property (nonatomic) int numTerrainCoverPendingFrames;
@end

@implementation MLNRenderingStatsRecorder

- (void)mapViewDidFinishRenderingFrame:(MLNMapView *)mapView
                         fullyRendered:(BOOL)fullyRendered
                        renderingStats:(MLNRenderingStats *)renderingStats {
  self.callCount++;
  self.numDrapeTargetsDeferred = renderingStats.numDrapeTargetsDeferred;
  self.numNewTileBuildsDeferred = renderingStats.numNewTileBuildsDeferred;
  self.numTerrainCoverPendingFrames = renderingStats.numTerrainCoverPendingFrames;
}

@end

@interface MLNMapViewRenderingStatsTests : XCTestCase
@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) MLNRenderingStatsRecorder *recorder;
@end

@implementation MLNMapViewRenderingStatsTests

- (void)setUp {
  [super setUp];

  [MLNSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
  NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner"
                                                             withExtension:@"json"];
  self.mapView = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 200, 200) styleURL:styleURL];
  self.recorder = [[MLNRenderingStatsRecorder alloc] init];
  self.mapView.delegate = self.recorder;
}

- (void)tearDown {
  self.mapView.delegate = nil;
  self.mapView = nil;
  self.recorder = nil;
  [MLNSettings setApiKey:nil];

  [super tearDown];
}

// The renderer reports a frame through this method; the delegate gets the stats it was given.
- (void)testDelegateReceivesTheFollowUpFrameCounters {
  mln::gfx::RenderingStats coreStats;
  coreStats.numDrapeTargetsDeferred = 3;
  coreStats.numNewTileBuildsDeferred = 5;
  coreStats.numTerrainCoverPendingFrames = 2;

  [self.mapView mapViewDidFinishRenderingFrameFullyRendered:YES renderingStats:coreStats];

  XCTAssertEqual(self.recorder.callCount, 1);
  XCTAssertEqual(self.recorder.numDrapeTargetsDeferred, 3);
  XCTAssertEqual(self.recorder.numNewTileBuildsDeferred, 5);
  XCTAssertEqual(self.recorder.numTerrainCoverPendingFrames, 2);

  // A later frame replaces the values rather than adding to them.
  coreStats.numDrapeTargetsDeferred = 0;
  coreStats.numNewTileBuildsDeferred = 9;
  coreStats.numTerrainCoverPendingFrames = 0;
  [self.mapView mapViewDidFinishRenderingFrameFullyRendered:YES renderingStats:coreStats];

  XCTAssertEqual(self.recorder.callCount, 2);
  XCTAssertEqual(self.recorder.numDrapeTargetsDeferred, 0);
  XCTAssertEqual(self.recorder.numNewTileBuildsDeferred, 9);
  XCTAssertEqual(self.recorder.numTerrainCoverPendingFrames, 0);
}

@end
