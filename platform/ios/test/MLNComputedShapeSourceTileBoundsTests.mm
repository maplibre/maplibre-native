#import <MapLibre.h>
#import <XCTest/XCTest.h>

#import "MLNGeometry_Private.h"
#import "MLNSource_Private.h"

#include <mln/style/sources/custom_geometry_source.hpp>
#include <mln/test/crs.hpp>
#include <mln/tile/tile_id.hpp>

#include <array>
#include <cmath>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace {

NSString *const MLNTestPolarProjectionName = @"test:ios-computed-polar";

// The tile the tests fetch, 3/4/5.
constexpr uint8_t MLNTestTileZ = 3;
constexpr uint32_t MLNTestTileX = 4;
constexpr uint32_t MLNTestTileY = 5;

// Its box in the test polar stereographic projection, from GL JS's `tileIdToLngLatBounds`.
constexpr double MLNTestPolarTileWest = -45;
constexpr double MLNTestPolarTileSouth = 68.53700782445765;
constexpr double MLNTestPolarTileEast = 0;
constexpr double MLNTestPolarTileNorth = 80.31077447189598;

MLNCoordinateReferenceSystem *MLNTestPolarStereographic() {
  auto crs = std::make_shared<const mln::CRS>(
      mln::test::polarStereographic(MLNTestPolarProjectionName.UTF8String));
  MLNTileMatrixSet tileMatrixSet = {{crs->tileMatrixSet.origin[0], crs->tileMatrixSet.origin[1]},
                                    crs->tileMatrixSet.extentAtZoom0};
  return [[MLNCoordinateReferenceSystem alloc] initWithName:MLNTestPolarProjectionName
      tileMatrixSet:tileMatrixSet
      forward:^MLNCRSPosition(CLLocationCoordinate2D coordinate) {
        std::array<double, 2> lngLat{coordinate.longitude, coordinate.latitude};
        std::array<double, 2> position{};
        crs->forward({&lngLat, 1}, {&position, 1});
        return {position[0], position[1]};
      }
      inverse:^CLLocationCoordinate2D(MLNCRSPosition crsPosition) {
        std::array<double, 2> position{crsPosition.x, crsPosition.y};
        std::array<double, 2> lngLat{};
        crs->inverse({&position, 1}, {&lngLat, 1});
        return CLLocationCoordinate2DMake(lngLat[1], lngLat[0]);
      }];
}

NSString *MLNTestStringFromBounds(MLNCoordinateBounds bounds) {
  return [NSString stringWithFormat:@"west %.17g, south %.17g, east %.17g, north %.17g",
                                    bounds.sw.longitude, bounds.sw.latitude, bounds.ne.longitude,
                                    bounds.ne.latitude];
}

}  // namespace

// Records the bounds each tile is fetched with, on whichever thread the source asks.
@interface MLNTestTileBoundsRecorder : NSObject <MLNComputedShapeSourceDataSource>

- (std::vector<MLNCoordinateBounds>)boundsAtZoomLevel:(NSUInteger)zoomLevel;

@end

@implementation MLNTestTileBoundsRecorder {
  std::mutex _mutex;
  std::vector<std::pair<MLNCoordinateBounds, NSUInteger>> _requests;
}

- (NSArray<MLNShape<MLNFeature> *> *)featuresInCoordinateBounds:(MLNCoordinateBounds)bounds
                                                      zoomLevel:(NSUInteger)zoomLevel {
  std::scoped_lock lock(_mutex);
  _requests.emplace_back(bounds, zoomLevel);
  return @[];
}

- (std::vector<MLNCoordinateBounds>)boundsAtZoomLevel:(NSUInteger)zoomLevel {
  std::scoped_lock lock(_mutex);
  std::vector<MLNCoordinateBounds> bounds;
  for (const auto &[requestBounds, requestZoomLevel] : _requests) {
    if (requestZoomLevel == zoomLevel) {
      bounds.push_back(requestBounds);
    }
  }
  return bounds;
}

@end

@interface MLNComputedShapeSourceTileBoundsTests : XCTestCase <MLNMapSnapshotterDelegate>

@property (nonatomic) MLNTestTileBoundsRecorder *recorder;
// Core's bounds for the test tile, read while the source is in the snapshot's style.
@property (nonatomic) MLNCoordinateBounds coreTileBounds;

@end

@implementation MLNComputedShapeSourceTileBoundsTests

- (void)setUp {
  [super setUp];
  NSError *error;
  XCTAssertTrue([MLNStyle addProjection:MLNTestPolarStereographic() error:&error], @"%@", error);
  self.recorder = [[MLNTestTileBoundsRecorder alloc] init];
}

- (void)tearDown {
  XCTAssertTrue([MLNStyle removeProjectionNamed:MLNTestPolarProjectionName error:nil]);
  [super tearDown];
}

- (void)mapSnapshotter:(MLNMapSnapshotter *)snapshotter didFinishLoadingStyle:(MLNStyle *)style {
  MLNComputedShapeSource *source = [[MLNComputedShapeSource alloc] initWithIdentifier:@"computed"
                                                                           dataSource:self.recorder
                                                                              options:nil];
  [style addSource:source];
  [style addLayer:[[MLNCircleStyleLayer alloc] initWithIdentifier:@"computed" source:source]];
  auto *rawSource = static_cast<mln::style::CustomGeometrySource *>(source.rawSource);
  self.coreTileBounds = MLNCoordinateBoundsFromLatLngBounds(
      rawSource->getTileBounds(mln::CanonicalTileID(MLNTestTileZ, MLNTestTileX, MLNTestTileY)));
}

// Snapshots a 256-point square at the test tile's zoom level around `center`, which is inside
// the test tile, and returns the bounds the data source was asked for at that zoom level. The
// snapshot waits for every tile in view.
- (std::vector<MLNCoordinateBounds>)tileBoundsInProjection:(nullable NSString *)projection
                                                    center:(CLLocationCoordinate2D)center {
  NSString *projectionJSON =
      projection ? [NSString stringWithFormat:@"\"projection\": {\"type\": \"%@\"}, ", projection]
                 : @"";
  NSString *styleJSON = [NSString
      stringWithFormat:@"{\"version\": 8, %@\"sources\": {}, \"layers\": []}", projectionJSON];
  NSURL *styleURL = [NSURL
      fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:
                                                  @"computed-shape-source-tile-bounds.json"]];
  NSError *error;
  XCTAssertTrue([styleJSON writeToURL:styleURL
                           atomically:YES
                             encoding:NSUTF8StringEncoding
                                error:&error],
                @"%@", error);

  MLNMapCamera *camera = [MLNMapCamera camera];
  camera.centerCoordinate = center;
  MLNMapSnapshotOptions *options =
      [[MLNMapSnapshotOptions alloc] initWithStyleURL:styleURL
                                               camera:camera
                                                 size:CGSizeMake(256, 256)];
  options.zoomLevel = MLNTestTileZ;
  MLNMapSnapshotter *snapshotter = [[MLNMapSnapshotter alloc] initWithOptions:options];
  snapshotter.delegate = self;
  XCTestExpectation *expectation = [self expectationWithDescription:@"The snapshot is taken"];
  [snapshotter startWithCompletionHandler:^(MLNMapSnapshot *snapshot, NSError *snapshotError) {
    XCTAssertNil(snapshotError);
    [expectation fulfill];
  }];
  [self waitForExpectations:@[ expectation ] timeout:30];

  std::vector<MLNCoordinateBounds> bounds = [self.recorder boundsAtZoomLevel:MLNTestTileZ];
  for (const MLNCoordinateBounds &tileBounds : bounds) {
    NSLog(@"A zoom %u tile was fetched with %@", MLNTestTileZ, MLNTestStringFromBounds(tileBounds));
  }
  return bounds;
}

- (void)assertBounds:(const std::vector<MLNCoordinateBounds> &)bounds
             contain:(MLNCoordinateBounds)expected {
  NSMutableArray<NSString *> *fetched = [NSMutableArray array];
  BOOL found = NO;
  for (const MLNCoordinateBounds &tileBounds : bounds) {
    [fetched addObject:MLNTestStringFromBounds(tileBounds)];
    found = found || MLNCoordinateBoundsEqualToCoordinateBounds(tileBounds, expected);
  }
  XCTAssertTrue(found, @"No tile was fetched with %@; tiles were fetched with: %@",
                MLNTestStringFromBounds(expected), fetched);
}

- (void)testPolarTileIsFetchedWithItsBoundsInTheProjection {
  MLNCoordinateReferenceSystem *projection = MLNTestPolarStereographic();
  MLNTileMatrixSet matrix = projection.tileMatrixSet;
  const double tileExtent = matrix.extentAtZoom0 / (1 << MLNTestTileZ);
  CLLocationCoordinate2D center =
      projection.inverse({matrix.origin.x + (MLNTestTileX + 0.5) * tileExtent,
                          matrix.origin.y - (MLNTestTileY + 0.5) * tileExtent});

  std::vector<MLNCoordinateBounds> bounds = [self tileBoundsInProjection:MLNTestPolarProjectionName
                                                                  center:center];

  [self assertBounds:bounds contain:self.coreTileBounds];
  XCTAssertEqualWithAccuracy(self.coreTileBounds.sw.longitude, MLNTestPolarTileWest, 1e-12);
  XCTAssertEqualWithAccuracy(self.coreTileBounds.sw.latitude, MLNTestPolarTileSouth, 1e-12);
  XCTAssertEqualWithAccuracy(self.coreTileBounds.ne.longitude, MLNTestPolarTileEast, 1e-12);
  XCTAssertEqualWithAccuracy(self.coreTileBounds.ne.latitude, MLNTestPolarTileNorth, 1e-12);
}

- (void)testMercatorTileIsFetchedWithTheTilesBounds {
  const double n = 1 << MLNTestTileZ;
  CLLocationCoordinate2D center = CLLocationCoordinate2DMake(
      std::atan(std::sinh(M_PI * (1 - 2 * (MLNTestTileY + 0.5) / n))) * 180 / M_PI,
      (MLNTestTileX + 0.5) / n * 360 - 180);

  std::vector<MLNCoordinateBounds> bounds = [self tileBoundsInProjection:nil center:center];

  MLNCoordinateBounds tileBounds = MLNCoordinateBoundsFromLatLngBounds(
      mln::LatLngBounds(mln::CanonicalTileID(MLNTestTileZ, MLNTestTileX, MLNTestTileY)));
  [self assertBounds:bounds contain:tileBounds];
  XCTAssertTrue(MLNCoordinateBoundsEqualToCoordinateBounds(self.coreTileBounds, tileBounds));
}

@end
