#import <MapLibre.h>

#import <XCTest/XCTest.h>

static NSString *const MLNTerrainTestStyleWithTerrain =
    @"{\"version\":8,"
    @"\"sources\":{\"dem\":{\"type\":\"raster-dem\",\"encoding\":\"terrarium\","
    @"\"tiles\":[\"https://example.invalid/{z}/{x}/{y}.png\"]}},"
    @"\"terrain\":{\"source\":\"dem\",\"exaggeration\":2.5},"
    @"\"layers\":[]}";

static NSString *const MLNTerrainTestStyleWithoutTerrain =
    @"{\"version\":8,"
    @"\"sources\":{\"dem\":{\"type\":\"raster-dem\",\"encoding\":\"terrarium\","
    @"\"tiles\":[\"https://example.invalid/{z}/{x}/{y}.png\"]}},"
    @"\"layers\":[]}";

@interface MLNTerrainTests : XCTestCase <MLNMapViewDelegate>

@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) XCTestExpectation *styleLoadingExpectation;

@end

@implementation MLNTerrainTests

- (void)setUp {
  [super setUp];

  NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner"
                                                             withExtension:@"json"];
  self.mapView = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 100, 100) styleURL:styleURL];
  self.mapView.delegate = self;
  if (!self.mapView.style) {
    [self waitForStyleToLoad];
  }
}

- (void)tearDown {
  self.mapView = nil;
  self.styleLoadingExpectation = nil;
  [super tearDown];
}

- (void)mapView:(MLNMapView *)mapView didFinishLoadingStyle:(MLNStyle *)style {
  [self.styleLoadingExpectation fulfill];
}

- (void)waitForStyleToLoad {
  self.styleLoadingExpectation =
      [self expectationWithDescription:@"Map view should finish loading style."];
  [self waitForExpectationsWithTimeout:10 handler:nil];
}

- (void)loadStyleJSON:(NSString *)json {
  self.styleLoadingExpectation =
      [self expectationWithDescription:@"Map view should finish loading style."];
  self.mapView.styleJSON = json;
  [self waitForExpectationsWithTimeout:10 handler:nil];
}

- (void)testStyleHasNoTerrainByDefault {
  XCTAssertNil(self.mapView.style.terrain);
}

- (void)testSettingTerrainReadsBack {
  self.mapView.style.terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem"
                                                               exaggeration:1.75];

  MLNTerrain *terrain = self.mapView.style.terrain;
  XCTAssertNotNil(terrain);
  XCTAssertEqualObjects(terrain.sourceIdentifier, @"dem");
  XCTAssertEqual(terrain.exaggeration, 1.75);
}

- (void)testTerrainReadsBackEqualToTheOneThatWasSet {
  // 1.4 is not exactly representable. The renderer keeps a float, so the object must too.
  MLNTerrain *terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem" exaggeration:1.4];
  self.mapView.style.terrain = terrain;

  XCTAssertEqualObjects(self.mapView.style.terrain, terrain);
  XCTAssertEqual(self.mapView.style.terrain.hash, terrain.hash);
}

- (void)testDefaultExaggerationIsOne {
  MLNTerrain *terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem"];
  XCTAssertEqual(terrain.exaggeration, 1);

  self.mapView.style.terrain = terrain;
  XCTAssertEqual(self.mapView.style.terrain.exaggeration, 1);
}

- (void)testZeroExaggerationIsKept {
  self.mapView.style.terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem" exaggeration:0];

  XCTAssertNotNil(self.mapView.style.terrain);
  XCTAssertEqual(self.mapView.style.terrain.exaggeration, 0);
}

- (void)testSettingTerrainToNilClearsIt {
  self.mapView.style.terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem"];
  XCTAssertNotNil(self.mapView.style.terrain);

  self.mapView.style.terrain = nil;
  XCTAssertNil(self.mapView.style.terrain);

  self.mapView.style.terrain = nil;
  XCTAssertNil(self.mapView.style.terrain, @"Clearing terrain twice is harmless.");
}

- (void)testSettingTerrainReplacesThePreviousOne {
  self.mapView.style.terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem" exaggeration:1];
  self.mapView.style.terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"other-dem"
                                                               exaggeration:3];

  XCTAssertEqualObjects(self.mapView.style.terrain.sourceIdentifier, @"other-dem");
  XCTAssertEqual(self.mapView.style.terrain.exaggeration, 3);
}

- (void)testTerrainIsImmutableAndCopyReturnsTheSameObject {
  MLNTerrain *terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem" exaggeration:2];
  XCTAssertEqual([terrain copy], terrain);
}

- (void)testTerrainEquality {
  MLNTerrain *terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem" exaggeration:1.5];
  MLNTerrain *same = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem" exaggeration:1.5];
  XCTAssertEqualObjects(terrain, same);
  XCTAssertEqual(terrain.hash, same.hash);

  XCTAssertNotEqualObjects(terrain, [[MLNTerrain alloc] initWithSourceIdentifier:@"other-dem"
                                                                    exaggeration:1.5]);
  XCTAssertNotEqualObjects(terrain, [[MLNTerrain alloc] initWithSourceIdentifier:@"dem"
                                                                    exaggeration:2]);
  XCTAssertNotEqualObjects(terrain, @"dem");
}

- (void)testTerrainFromStyleJSON {
  [self loadStyleJSON:MLNTerrainTestStyleWithTerrain];

  MLNTerrain *terrain = self.mapView.style.terrain;
  XCTAssertNotNil(terrain);
  XCTAssertEqualObjects(terrain.sourceIdentifier, @"dem");
  XCTAssertEqual(terrain.exaggeration, 2.5);
  XCTAssertEqualObjects(terrain, [[MLNTerrain alloc] initWithSourceIdentifier:@"dem"
                                                                 exaggeration:2.5]);
}

- (void)testTerrainFromStyleJSONCanBeCleared {
  [self loadStyleJSON:MLNTerrainTestStyleWithTerrain];
  XCTAssertNotNil(self.mapView.style.terrain);

  self.mapView.style.terrain = nil;
  XCTAssertNil(self.mapView.style.terrain);
}

- (void)testTerrainFromStyleJSONCanBeReplaced {
  [self loadStyleJSON:MLNTerrainTestStyleWithTerrain];

  self.mapView.style.terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem"
                                                               exaggeration:0.5];
  XCTAssertEqual(self.mapView.style.terrain.exaggeration, 0.5);
}

- (void)testLoadingAStyleReplacesTheTerrainSetFromCode {
  [self loadStyleJSON:MLNTerrainTestStyleWithoutTerrain];
  self.mapView.style.terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem"];
  XCTAssertNotNil(self.mapView.style.terrain);

  [self loadStyleJSON:MLNTerrainTestStyleWithoutTerrain];
  XCTAssertNil(self.mapView.style.terrain);

  [self loadStyleJSON:MLNTerrainTestStyleWithTerrain];
  XCTAssertEqual(self.mapView.style.terrain.exaggeration, 2.5);

  [self loadStyleJSON:MLNTerrainTestStyleWithoutTerrain];
  XCTAssertNil(self.mapView.style.terrain);
}

// Returns the "before the style has finished loading" warning logged while the block runs.
- (nullable NSString *)loadingWarningLoggedDuring:(void (^)(void))block {
  __block NSString *warning;
  MLNLoggingConfiguration.sharedConfiguration.loggingLevel = MLNLoggingLevelWarning;
  MLNLoggingConfiguration.sharedConfiguration.handler =
      ^(MLNLoggingLevel level, NSString *filePath, NSUInteger line, NSString *message) {
        if (level == MLNLoggingLevelWarning &&
            [message containsString:@"before the style has finished loading"]) {
          warning = message;
        }
      };
  block();
  MLNLoggingConfiguration.sharedConfiguration.handler = nil;
  MLNLoggingConfiguration.sharedConfiguration.loggingLevel = MLNLoggingLevelNone;
  return warning;
}

- (void)testSettingTerrainBeforeTheStyleHasLoadedLogsAWarning {
  MLNStyle *style = self.mapView.style;
  self.mapView.styleURL = [NSURL URLWithString:@"http://127.0.0.1:1/style.json"];

  NSString *warning = [self loadingWarningLoggedDuring:^{
    style.terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem"];
  }];

  XCTAssertNotNil(warning);
}

- (void)testSettingTerrainAfterTheStyleHasLoadedDoesNotLog {
  NSString *warning = [self loadingWarningLoggedDuring:^{
    self.mapView.style.terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"dem"];
    self.mapView.style.terrain = nil;
  }];

  XCTAssertNil(warning);
}

- (void)testTerrainLoadModeDefaultsToQualityAndRoundTrips {
  XCTAssertEqual(self.mapView.terrainLoadMode, MLNTerrainLoadModeQuality);

  self.mapView.terrainLoadMode = MLNTerrainLoadModeBalanced;
  XCTAssertEqual(self.mapView.terrainLoadMode, MLNTerrainLoadModeBalanced);

  self.mapView.terrainLoadMode = MLNTerrainLoadModePerformance;
  XCTAssertEqual(self.mapView.terrainLoadMode, MLNTerrainLoadModePerformance);

  self.mapView.terrainLoadMode = MLNTerrainLoadModeQuality;
  XCTAssertEqual(self.mapView.terrainLoadMode, MLNTerrainLoadModeQuality);
}

- (void)testTerrainSkirtLengthDefaultsToAutoAndRoundTrips {
  XCTAssertEqual(self.mapView.terrainSkirtLength, MLNTerrainSkirtLengthAuto);

  self.mapView.terrainSkirtLength = MLNTerrainSkirtLengthNone;
  XCTAssertEqual(self.mapView.terrainSkirtLength, MLNTerrainSkirtLengthNone);

  self.mapView.terrainSkirtLength = MLNTerrainSkirtLengthAuto;
  XCTAssertEqual(self.mapView.terrainSkirtLength, MLNTerrainSkirtLengthAuto);
}

- (void)testTerrainOptionsSurviveLoadingAStyle {
  self.mapView.terrainLoadMode = MLNTerrainLoadModePerformance;
  self.mapView.terrainSkirtLength = MLNTerrainSkirtLengthNone;

  [self loadStyleJSON:MLNTerrainTestStyleWithTerrain];

  XCTAssertEqual(self.mapView.terrainLoadMode, MLNTerrainLoadModePerformance);
  XCTAssertEqual(self.mapView.terrainSkirtLength, MLNTerrainSkirtLengthNone);
}

@end
