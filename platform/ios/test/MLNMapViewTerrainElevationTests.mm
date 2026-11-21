#import <MapLibre.h>
#import <XCTest/XCTest.h>

#import "MLNMapView_Private.h"

#include <cmath>

#include <mln/map/map.hpp>
#include <mln/style/style.hpp>
#include <mln/style/terrain.hpp>
#include <mln/util/image.hpp>

namespace {

constexpr uint32_t demTileSize = 64;
constexpr double plateauMeters = 1000.0;

// A flat DEM tile in Terrain-RGB ("mapbox") encoding: -10000 + (R * 65536 + G * 256 + B) * 0.1
NSData *plateauTile(double meters) {
  const auto encoded = static_cast<uint32_t>(std::lround((meters + 10000.0) * 10.0));
  mln::PremultipliedImage image({demTileSize, demTileSize});
  for (uint32_t i = 0; i < demTileSize * demTileSize; ++i) {
    uint8_t *pixel = image.data.get() + i * 4;
    pixel[0] = static_cast<uint8_t>((encoded >> 16) & 0xFF);
    pixel[1] = static_cast<uint8_t>((encoded >> 8) & 0xFF);
    pixel[2] = static_cast<uint8_t>(encoded & 0xFF);
    pixel[3] = 255;
  }
  const std::string png = mln::encodePNG(image);
  return [NSData dataWithBytes:png.data() length:png.size()];
}

}  // namespace

@interface MLNMapViewTerrainElevationTests : XCTestCase <MLNMapViewDelegate>

@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) UIWindow *window;
@property (nonatomic) XCTestExpectation *styleLoadingExpectation;
@property (nonatomic) NSURL *directory;

@end

@implementation MLNMapViewTerrainElevationTests

// Innsbruck, inside the one DEM tile the test styles serve.
static const CLLocationCoordinate2D innsbruck = {47.2692, 11.4041};

- (void)setUp {
  [super setUp];

  self.directory =
      [NSURL fileURLWithPath:[NSTemporaryDirectory()
                                 stringByAppendingPathComponent:[[NSUUID UUID] UUIDString]]
                 isDirectory:YES];
  [[NSFileManager defaultManager] createDirectoryAtURL:self.directory
                           withIntermediateDirectories:YES
                                            attributes:nil
                                                 error:nil];
}

- (void)tearDown {
  self.mapView.delegate = nil;
  self.mapView = nil;
  self.window = nil;
  [[NSFileManager defaultManager] removeItemAtURL:self.directory error:nil];

  [super tearDown];
}

- (void)mapView:(MLNMapView *)mapView didFinishLoadingStyle:(MLNStyle *)style {
  [_styleLoadingExpectation fulfill];
}

// Loads a style with a raster-dem source over local tiles and terrain on it. With `servesTiles`
// NO the tile URLs point at files that do not exist, so every DEM tile fails to load.
- (void)loadTerrainStyleWithExaggeration:(double)exaggeration servesTiles:(BOOL)servesTiles {
  if (servesTiles) {
    NSData *tile = plateauTile(plateauMeters);
    // The tiles are served at every zoom up to maxzoom, so write the ones that can cover the view.
    for (int z = 0; z <= 2; z++) {
      for (int x = 0; x < (1 << z); x++) {
        for (int y = 0; y < (1 << z); y++) {
          NSString *name = [NSString stringWithFormat:@"%d-%d-%d.png", z, x, y];
          XCTAssertTrue([tile writeToURL:[self.directory URLByAppendingPathComponent:name]
                              atomically:YES]);
        }
      }
    }
  }
  NSString *tiles =
      [[self.directory URLByAppendingPathComponent:@"{z}-{x}-{y}.png"] absoluteString];
  tiles = [tiles stringByReplacingOccurrencesOfString:@"%7B" withString:@"{"];
  tiles = [tiles stringByReplacingOccurrencesOfString:@"%7D" withString:@"}"];
  NSDictionary *style = @{
    @"version" : @8,
    @"sources" : @{
      @"dem" : @{
        @"type" : @"raster-dem",
        @"tiles" : @[ tiles ],
        @"encoding" : @"mapbox",
        @"maxzoom" : @2,
        @"tileSize" : @(demTileSize)
      }
    },
    @"terrain" : @{@"source" : @"dem", @"exaggeration" : @(exaggeration)},
    @"layers" : @[]
  };
  NSURL *styleURL = [self.directory URLByAppendingPathComponent:@"style.json"];
  NSData *json = [NSJSONSerialization dataWithJSONObject:style options:0 error:nil];
  XCTAssertTrue([json writeToURL:styleURL atomically:YES]);
  [self loadMapViewWithStyleURL:styleURL];
}

- (void)loadMapViewWithStyleURL:(NSURL *)styleURL {
  CGRect bounds = CGRectMake(0, 0, 200, 200);
  self.mapView = [[MLNMapView alloc] initWithFrame:bounds styleURL:styleURL];
  self.mapView.delegate = self;
  self.mapView.centerCoordinate = innsbruck;
  self.mapView.zoomLevel = 2;

  // The renderer only draws, and so only loads terrain, while the map view is on screen.
  self.window = [[UIWindow alloc] initWithFrame:bounds];
  [self.window addSubview:self.mapView];
  [self.window makeKeyAndVisible];

  if (!self.mapView.style) {
    _styleLoadingExpectation =
        [self expectationWithDescription:@"Map view should finish loading style."];
    [self waitForExpectationsWithTimeout:10 handler:nil];
  }
}

// Spins the run loop until the map view reports a height at the coordinate, or the timeout.
- (NSNumber *)waitForElevationAtCoordinate:(CLLocationCoordinate2D)coordinate {
  NSDate *deadline = [NSDate dateWithTimeIntervalSinceNow:15];
  NSNumber *elevation = [self.mapView terrainElevationForCoordinate:coordinate];
  while (!elevation && [deadline timeIntervalSinceNow] > 0) {
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
    elevation = [self.mapView terrainElevationForCoordinate:coordinate];
  }
  return elevation;
}

// Spins the run loop for a while, so frames are drawn and tile requests finish.
- (void)runForSeconds:(NSTimeInterval)seconds {
  [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:seconds]];
}

- (void)testNilWithoutTerrain {
  NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner"
                                                             withExtension:@"json"];
  [self loadMapViewWithStyleURL:styleURL];
  [self runForSeconds:1];

  XCTAssertNil([self.mapView terrainElevationForCoordinate:innsbruck]);
  XCTAssertNil([self.mapView terrainElevationForCoordinate:CLLocationCoordinate2DMake(0, 0)]);
}

- (void)testNilForInvalidCoordinate {
  [self loadTerrainStyleWithExaggeration:1 servesTiles:YES];
  // Terrain is up and answering, so a nil below comes from the coordinate and not from the map.
  XCTAssertNotNil([self waitForElevationAtCoordinate:innsbruck]);

  XCTAssertNil([self.mapView terrainElevationForCoordinate:CLLocationCoordinate2DMake(NAN, NAN)]);
  XCTAssertNil([self.mapView terrainElevationForCoordinate:CLLocationCoordinate2DMake(91, 11)]);
  XCTAssertNil([self.mapView terrainElevationForCoordinate:CLLocationCoordinate2DMake(47, 181)]);
  XCTAssertNil([self.mapView terrainElevationForCoordinate:kCLLocationCoordinate2DInvalid]);
  XCTAssertNotNil([self.mapView terrainElevationForCoordinate:innsbruck]);
}

- (void)testElevationMatchesTheDEMTile {
  [self loadTerrainStyleWithExaggeration:1 servesTiles:YES];

  NSNumber *elevation = [self waitForElevationAtCoordinate:innsbruck];
  XCTAssertNotNil(elevation, @"The DEM tile should load and answer.");
  XCTAssertEqualWithAccuracy(elevation.doubleValue, plateauMeters, 1.0);
}

- (void)testElevationIncludesTheExaggeration {
  [self loadTerrainStyleWithExaggeration:2 servesTiles:YES];

  NSNumber *elevation = [self waitForElevationAtCoordinate:innsbruck];
  XCTAssertNotNil(elevation, @"The DEM tile should load and answer.");
  XCTAssertEqualWithAccuracy(elevation.doubleValue, plateauMeters * 2, 2.0);
}

- (void)testNilWhenTheDEMTileHasNotLoaded {
  [self loadTerrainStyleWithExaggeration:1 servesTiles:NO];
  [self runForSeconds:3];

  XCTAssertNil([self.mapView terrainElevationForCoordinate:innsbruck]);
}

- (void)testNilAfterTerrainIsRemoved {
  [self loadTerrainStyleWithExaggeration:1 servesTiles:YES];
  XCTAssertNotNil([self waitForElevationAtCoordinate:innsbruck]);

  self.mapView.mbglMap.getStyle().setTerrain(nullptr);

  NSDate *deadline = [NSDate dateWithTimeIntervalSinceNow:15];
  while ([self.mapView terrainElevationForCoordinate:innsbruck] &&
         [deadline timeIntervalSinceNow] > 0) {
    [self runForSeconds:0.05];
  }
  XCTAssertNil([self.mapView terrainElevationForCoordinate:innsbruck]);
}

@end
