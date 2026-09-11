#import <Mapbox.h>
#import <XCTest/XCTest.h>

@interface MLNTerrainTests : XCTestCase <MLNMapViewDelegate>

@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) XCTestExpectation *styleLoadingExpectation;

@end

@implementation MLNTerrainTests

- (void)setUp {
    [super setUp];

    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    self.mapView = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 400, 400) styleURL:styleURL];
    self.mapView.delegate = self;

    if (!self.mapView.style) {
        _styleLoadingExpectation = [self expectationWithDescription:@"Map view should finish loading style."];
        [self waitForExpectationsWithTimeout:10 handler:nil];
    }
}

- (void)mapView:(MLNMapView *)mapView didFinishLoadingStyle:(MLNStyle *)style {
    [_styleLoadingExpectation fulfill];
}

- (void)tearDown {
    self.mapView = nil;
    [super tearDown];
}

- (void)testTerrainRoundTripsSourceIdentifierAndExaggeration {
    MLNTerrain *terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"terrain-dem" exaggeration:1.5];
    XCTAssertEqualObjects(terrain.sourceIdentifier, @"terrain-dem");
    XCTAssertEqualWithAccuracy(terrain.exaggeration, 1.5, 0.0001);
}

- (void)testTerrainConvenienceInitializerDefaultsExaggerationToOne {
    MLNTerrain *terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"terrain-dem"];
    XCTAssertEqualObjects(terrain.sourceIdentifier, @"terrain-dem");
    XCTAssertEqualWithAccuracy(terrain.exaggeration, 1.0, 0.0001);
}

- (void)testTerrainEquality {
    MLNTerrain *terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"terrain-dem" exaggeration:1.5];
    MLNTerrain *sameTerrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"terrain-dem" exaggeration:1.5];
    XCTAssertEqualObjects(terrain, sameTerrain);
    XCTAssertEqual(terrain.hash, sameTerrain.hash);
}

- (void)testTerrainInequality {
    MLNTerrain *terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"terrain-dem" exaggeration:1.5];

    MLNTerrain *differentSource = [[MLNTerrain alloc] initWithSourceIdentifier:@"other-dem" exaggeration:1.5];
    XCTAssertNotEqualObjects(terrain, differentSource);

    MLNTerrain *differentExaggeration = [[MLNTerrain alloc] initWithSourceIdentifier:@"terrain-dem" exaggeration:2.0];
    XCTAssertNotEqualObjects(terrain, differentExaggeration);

    XCTAssertNotEqualObjects(terrain, @"not a terrain object");
}

- (void)testSettingStyleTerrainRoundTrips {
    XCTAssertNil(self.mapView.style.terrain, @"Style should have no terrain by default.");

    MLNTerrain *terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"terrain-dem" exaggeration:1.75];
    self.mapView.style.terrain = terrain;

    MLNTerrain *readBack = self.mapView.style.terrain;
    XCTAssertNotNil(readBack);
    XCTAssertEqualObjects(readBack.sourceIdentifier, @"terrain-dem");
    XCTAssertEqualWithAccuracy(readBack.exaggeration, 1.75, 0.0001);
}

- (void)testSettingStyleTerrainToNilClearsIt {
    self.mapView.style.terrain = [[MLNTerrain alloc] initWithSourceIdentifier:@"terrain-dem"];
    XCTAssertNotNil(self.mapView.style.terrain);

    self.mapView.style.terrain = nil;
    XCTAssertNil(self.mapView.style.terrain);
}

- (void)testSetTerrainWithSourceIdentifierConvenienceMethod {
    [self.mapView setTerrainWithSourceIdentifier:@"terrain-dem" exaggeration:2.25];

    MLNTerrain *readBack = self.mapView.style.terrain;
    XCTAssertNotNil(readBack);
    XCTAssertEqualObjects(readBack.sourceIdentifier, @"terrain-dem");
    XCTAssertEqualWithAccuracy(readBack.exaggeration, 2.25, 0.0001);

    [self.mapView setTerrainWithSourceIdentifier:nil exaggeration:1.0];
    XCTAssertNil(self.mapView.style.terrain);
}

- (void)testTerrainLoadModeRoundTrips {
    self.mapView.terrainLoadMode = MLNTerrainLoadModeBalanced;
    XCTAssertEqual(self.mapView.terrainLoadMode, MLNTerrainLoadModeBalanced);

    self.mapView.terrainLoadMode = MLNTerrainLoadModePerformance;
    XCTAssertEqual(self.mapView.terrainLoadMode, MLNTerrainLoadModePerformance);

    self.mapView.terrainLoadMode = MLNTerrainLoadModeQuality;
    XCTAssertEqual(self.mapView.terrainLoadMode, MLNTerrainLoadModeQuality);
}

- (void)testTerrainSkirtLengthRoundTrips {
    self.mapView.terrainSkirtLength = MLNTerrainSkirtLengthNone;
    XCTAssertEqual(self.mapView.terrainSkirtLength, MLNTerrainSkirtLengthNone);

    self.mapView.terrainSkirtLength = MLNTerrainSkirtLengthAuto;
    XCTAssertEqual(self.mapView.terrainSkirtLength, MLNTerrainSkirtLengthAuto);
}

// The core `mln::TerrainLoadMode` and `mln::TerrainSkirtLength` enums (include/mln/map/mode.hpp)
// are pinned at these ordinals; a `static_assert` in MLNMapView.mm guards them at the ObjC++
// boundary. This test guards the ObjC public enums themselves against accidental reordering.
- (void)testEnumOrdinalsMatchCoreValues {
    XCTAssertEqual(MLNTerrainLoadModeQuality, 0u);
    XCTAssertEqual(MLNTerrainLoadModeBalanced, 1u);
    XCTAssertEqual(MLNTerrainLoadModePerformance, 2u);

    XCTAssertEqual(MLNTerrainSkirtLengthAuto, 0u);
    XCTAssertEqual(MLNTerrainSkirtLengthNone, 1u);
}

@end
