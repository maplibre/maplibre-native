#import <MapLibre.h>
#import <XCTest/XCTest.h>

#import "MLNRenderingStats_Private.h"

#include <mln/gfx/rendering_stats.hpp>

@interface MLNRenderingStatsTests : XCTestCase
@end

@implementation MLNRenderingStatsTests

- (void)testFollowUpFrameCountersAreZeroByDefault {
  MLNRenderingStats *stats = [[MLNRenderingStats alloc] init];

  XCTAssertEqual(stats.numDrapeTargetsDeferred, 0);
  XCTAssertEqual(stats.numNewTileBuildsDeferred, 0);
  XCTAssertEqual(stats.numTerrainCoverPendingFrames, 0);
}

- (void)testFollowUpFrameCountersCarryTheCoreValues {
  // Distinct values, so a counter copied from the wrong core field is caught.
  mln::gfx::RenderingStats coreStats;
  coreStats.numDrapeTargetsDeferred = 7;
  coreStats.numNewTileBuildsDeferred = 11;
  coreStats.numTerrainCoverPendingFrames = 13;

  MLNRenderingStats *stats = [[MLNRenderingStats alloc] init];
  [stats setCoreData:coreStats];

  XCTAssertEqual(stats.numDrapeTargetsDeferred, 7);
  XCTAssertEqual(stats.numNewTileBuildsDeferred, 11);
  XCTAssertEqual(stats.numTerrainCoverPendingFrames, 13);
}

@end
