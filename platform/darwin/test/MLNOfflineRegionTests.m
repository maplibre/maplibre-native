#import <Mapbox.h>

#import <XCTest/XCTest.h>

@interface MLNOfflineRegionTests : XCTestCase

@end

@implementation MLNOfflineRegionTests

- (void)setUp {
    [super setUp];
    [MLNSettings useWellKnownTileServer:MLNMapTiler];
}

- (void)testStyleURLs {
    MLNCoordinateBounds bounds = MLNCoordinateBoundsMake(kCLLocationCoordinate2DInvalid, kCLLocationCoordinate2DInvalid);
    MLNTilePyramidOfflineRegion *region = [[MLNTilePyramidOfflineRegion alloc] initWithStyleURL:nil bounds:bounds fromZoomLevel:0 toZoomLevel:DBL_MAX];
    XCTAssertEqualObjects(region.styleURL, [MLNStyle defaultStyleURL], @"Default style expected.");
    
    NSURL *localURL = [NSURL URLWithString:@"beautiful.style"];
    XCTAssertThrowsSpecificNamed([[MLNTilePyramidOfflineRegion alloc] initWithStyleURL:localURL bounds:bounds fromZoomLevel:0 toZoomLevel:DBL_MAX], NSException, MLNInvalidStyleURLException, @"No exception raised when initializing region with a local file URL as the style URL.");
}

- (void)testTilePyramidRegionEquality {
    [MLNSettings useWellKnownTileServer:MLNMapTiler];
    MLNCoordinateBounds bounds = MLNCoordinateBoundsMake(kCLLocationCoordinate2DInvalid, kCLLocationCoordinate2DInvalid);
    MLNTilePyramidOfflineRegion *original = [[MLNTilePyramidOfflineRegion alloc] initWithStyleURL:[[MLNStyle predefinedStyle:@"Bright"] url] bounds:bounds fromZoomLevel:5 toZoomLevel:10];
    MLNTilePyramidOfflineRegion *copy = [original copy];
    XCTAssertEqualObjects(original, copy, @"Tile pyramid region should be equal to its copy.");
    
    XCTAssertEqualObjects(original.styleURL, copy.styleURL, @"Style URL has changed.");
    XCTAssert(MLNCoordinateBoundsEqualToCoordinateBounds(original.bounds, copy.bounds), @"Bounds have changed.");
    XCTAssertEqual(original.minimumZoomLevel, copy.minimumZoomLevel, @"Minimum zoom level has changed.");
    XCTAssertEqual(original.maximumZoomLevel, copy.maximumZoomLevel, @"Maximum zoom level has changed.");
    XCTAssertEqual(original.includesIdeographicGlyphs, copy.includesIdeographicGlyphs, @"Include ideographs has changed.");
}

- (void)testGeometryRegionEquality {
    NSString *geojson = @"{\"type\": \"Point\", \"coordinates\": [-3.8671874999999996, 52.482780222078226] }";
    NSError *error;
    MLNShape *shape = [MLNShape shapeWithData: [geojson dataUsingEncoding:NSUTF8StringEncoding] encoding: NSUTF8StringEncoding error:&error];
    XCTAssertNil(error);
    
    MLNShapeOfflineRegion *original = [[MLNShapeOfflineRegion alloc] initWithStyleURL:[[MLNStyle predefinedStyle:@"Bright"] url] shape:shape fromZoomLevel:5 toZoomLevel:10];
    original.includesIdeographicGlyphs = NO;
    MLNShapeOfflineRegion *copy = [original copy];
    XCTAssertEqualObjects(original, copy, @"Shape region should be equal to its copy.");
    
    XCTAssertEqualObjects(original.styleURL, copy.styleURL, @"Style URL has changed.");
    XCTAssertEqualObjects(original.shape, copy.shape, @"Geometry has changed.");
    XCTAssertEqual(original.minimumZoomLevel, copy.minimumZoomLevel, @"Minimum zoom level has changed.");
    XCTAssertEqual(original.maximumZoomLevel, copy.maximumZoomLevel, @"Maximum zoom level has changed.");
    XCTAssertEqual(original.includesIdeographicGlyphs, copy.includesIdeographicGlyphs, @"Include ideographs has changed.");
}

- (void)testIncludesIdeographicGlyphsByDefault {
    
    // Tile pyramid offline region
    {
        MLNCoordinateBounds bounds = MLNCoordinateBoundsMake(kCLLocationCoordinate2DInvalid, kCLLocationCoordinate2DInvalid);
        MLNTilePyramidOfflineRegion *tilePyramidOfflineRegion = [[MLNTilePyramidOfflineRegion alloc] initWithStyleURL:[[MLNStyle predefinedStyle:@"Bright"] url] bounds:bounds fromZoomLevel:5 toZoomLevel:10];
        XCTAssertFalse(tilePyramidOfflineRegion.includesIdeographicGlyphs, @"tile pyramid offline region should not include ideographic glyphs");
    }
    
    // Shape offline region
    {
        NSString *geojson = @"{\"type\": \"Point\", \"coordinates\": [-3.8671874999999996, 52.482780222078226] }";
        NSError *error;
        MLNShape *shape = [MLNShape shapeWithData: [geojson dataUsingEncoding:NSUTF8StringEncoding] encoding: NSUTF8StringEncoding error:&error];
        XCTAssertNil(error);
        MLNShapeOfflineRegion *shapeOfflineRegion = [[MLNShapeOfflineRegion alloc] initWithStyleURL:[[MLNStyle predefinedStyle:@"Bright"] url] shape:shape fromZoomLevel:5 toZoomLevel:10];
        XCTAssertFalse(shapeOfflineRegion.includesIdeographicGlyphs, @"tile pyramid offline region should not include ideographic glyphs");
    }
}

@end
