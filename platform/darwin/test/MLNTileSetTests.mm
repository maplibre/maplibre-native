#import <XCTest/XCTest.h>

#import <Mapbox.h>
#import "MLNVectorTileSource.h"
#import "MLNTileSource_Private.h"
#import "MLNGeometry_Private.h"

#include <mbgl/util/tileset.hpp>

@interface MLNTileSetTests : XCTestCase

@end

@implementation MLNTileSetTests

namespace {
    // a tile set that provides an mbgl tile set
    NSArray *tileURLTemplates = @[@"tile.1", @"tile.2", @"tile.3"];

}

- (void)testTileSetFromTileURLTemplates {
    mbgl::Tileset tileSet = MLNTileSetFromTileURLTemplates(tileURLTemplates, nil);

    // has the correct URL templates
    XCTAssertEqual(tileSet.tiles.size(), 3UL);
    XCTAssertEqual(tileSet.tiles[0], "tile.1");
    XCTAssertEqual(tileSet.tiles[1], "tile.2");
    XCTAssertEqual(tileSet.tiles[2], "tile.3");

    // has the default scheme
    XCTAssertEqual(tileSet.scheme, mbgl::Tileset::Scheme::XYZ);

    // when the tile set has no min or max zoom level set
    // the mbgl object has default values for min and max zoom level
    XCTAssertEqual(tileSet.zoomRange.min, 0);
    XCTAssertEqual(tileSet.zoomRange.max, 22);

    // when the tile set has min and/or max zoom level set
    tileSet = MLNTileSetFromTileURLTemplates(@[@"tile.1"], @{
        MLNTileSourceOptionMinimumZoomLevel: @1,
        MLNTileSourceOptionMaximumZoomLevel: @2,
    });

    // the mbgl object reflects the set values for min and max zoom level
    XCTAssertEqual(tileSet.zoomRange.min, 1);
    XCTAssertEqual(tileSet.zoomRange.max, 2);

    // when the tile set has a bounds set
    MLNCoordinateBounds bounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(12, 34), CLLocationCoordinate2DMake(56, 78));
    tileSet = MLNTileSetFromTileURLTemplates(@[@"tile.1"], @{
        MLNTileSourceOptionCoordinateBounds: @(bounds),
    });

    // the mbgl object reflects the set values for the bounds
    XCTAssert(!!tileSet.bounds, @"The bounds are set after setting the bounds");
    if (tileSet.bounds) {
        MLNCoordinateBounds actual = MLNCoordinateBoundsFromLatLngBounds(*tileSet.bounds);
        XCTAssert(MLNCoordinateBoundsEqualToCoordinateBounds(bounds, actual), @"The bounds round-trip");
    }

    // when the tile set has an attribution
    NSString *attribution = @"my tileset © ©️🎈";
    tileSet = MLNTileSetFromTileURLTemplates(tileURLTemplates, @{
        MLNTileSourceOptionAttributionHTMLString: attribution,
    });

    // the attribution is reflected by the mbgl tileset
    XCTAssertEqual(tileSet.attribution, attribution.UTF8String);

    // when the tile set has attribution infos
    MLNAttributionInfo *mapboxInfo = [[MLNAttributionInfo alloc] initWithTitle:[[NSAttributedString alloc] initWithString:@"Mapbox"]
                                                                           URL:[NSURL URLWithString:@"https://www.mapbox.com/"]];
#if TARGET_OS_IPHONE
    UIColor *redColor = [UIColor redColor];
#else
    // CSS uses the sRGB color space.
    // AppKit incorrectly uses calibrated RGB when exporting HTML, so input
    // calibrated RGB to ensure round-tripping.
    // <rdar://problem/46115233> <http://www.openradar.me/46115233>
    NSColor *redColor = [NSColor colorWithCalibratedRed:1 green:0 blue:0 alpha:1];
#endif
    NSAttributedString *gl = [[NSAttributedString alloc] initWithString:@"GL" attributes:@{
        NSBackgroundColorAttributeName: redColor,
    }];
    MLNAttributionInfo *glInfo = [[MLNAttributionInfo alloc] initWithTitle:gl URL:nil];
    tileSet = MLNTileSetFromTileURLTemplates(tileURLTemplates, @{
        MLNTileSourceOptionAttributionInfos: @[mapboxInfo, glInfo],
    });

    // the attribution is reflected by the mbgl tileset
#if TARGET_OS_IPHONE
    NSString *html;
    if (@available(iOS 13.0, *)) {
        // TODO: investigate visual impact
        // iOS 13 evidently changes font size from points to pixels
        html = (@"<font style=\"font-family: 'Helvetica'; font-weight: normal; font-style: normal; font-size: 12.00px\">"
                @"<a href=\"https://www.mapbox.com/\">Mapbox</a> </font>"
                @"<font style=\"font-family: 'Helvetica'; font-weight: normal; font-style: normal; font-size: 12.00px; background-color: #ff0000\">GL</font>\n");
    } else {
        html = (@"<font style=\"font-family: 'Helvetica'; font-weight: normal; font-style: normal; font-size: 12.00pt\">"
                @"<a href=\"https://www.mapbox.com/\">Mapbox</a> </font>"
                @"<font style=\"font-family: 'Helvetica'; font-weight: normal; font-style: normal; font-size: 12.00pt; background-color: #ff0000\">GL</font>\n");
    }
#else
    NSString *html = (@"<font face=\"Helvetica\" size=\"3\" style=\"font: 12.0px Helvetica\">"
                      @"<a href=\"https://www.mapbox.com/\">Mapbox</a> </font>"
                      @"<font face=\"Helvetica\" size=\"3\" style=\"font: 12.0px Helvetica; background-color: #ff0000\">GL</font>\n");
#endif
    XCTAssertEqualObjects(@(tileSet.attribution.c_str()), html);

    // when the tile coordinate system is changed using an NSNumber
    tileSet = MLNTileSetFromTileURLTemplates(tileURLTemplates, @{
        MLNTileSourceOptionTileCoordinateSystem: @(MLNTileCoordinateSystemTMS),
    });

    // the scheme is reflected by the mbgl tileset
    XCTAssertEqual(tileSet.scheme, mbgl::Tileset::Scheme::TMS);

    // when the tile coordinate system is changed using an NSValue
    MLNTileCoordinateSystem tms = MLNTileCoordinateSystemTMS;
    tileSet = MLNTileSetFromTileURLTemplates(tileURLTemplates, @{
        MLNTileSourceOptionTileCoordinateSystem: [NSValue value:&tms withObjCType:@encode(MLNTileCoordinateSystem)],
    });

    // the scheme is reflected by the mbgl tileset
    XCTAssertEqual(tileSet.scheme, mbgl::Tileset::Scheme::TMS);
}

- (void)testTileSetFromTileURLTemplatesRasterEncodings {
    mbgl::Tileset tileSet;

    // when the DEM encoding is changed using an NSNumber
    tileSet = MLNTileSetFromTileURLTemplates(tileURLTemplates, @{
        MLNTileSourceOptionDEMEncoding: @(MLNDEMEncodingTerrarium),
    });

    // the encoding is reflected by the mbgl tileset
    XCTAssertEqual(tileSet.rasterEncoding, mbgl::Tileset::RasterEncoding::Terrarium);
    XCTAssertFalse(tileSet.vectorEncoding);

    // when the raster encoding is changed using an NSValue
    MLNDEMEncoding terrarium = MLNDEMEncodingTerrarium;
    tileSet = MLNTileSetFromTileURLTemplates(tileURLTemplates, @{
        MLNTileSourceOptionDEMEncoding: [NSValue value:&terrarium withObjCType:@encode(MLNDEMEncoding)],
    });

    // the encoding is reflected by the mbgl tileset
    XCTAssertEqual(tileSet.rasterEncoding, mbgl::Tileset::RasterEncoding::Terrarium);
    XCTAssertFalse(tileSet.vectorEncoding);
}

- (void)testTileSetFromTileURLTemplatesVectorEncodings {
    mbgl::Tileset tileSet;

    // when the raster encoding is changed using an NSNumber
    tileSet = MLNTileSetFromTileURLTemplates(tileURLTemplates, @{
        MLNVectorTileSourceOptionEncoding: @(MLNVectorTileSourceEncodingMapbox),
    });

    // the encoding is reflected by the mbgl tileset
    XCTAssertEqual(tileSet.vectorEncoding, mbgl::Tileset::VectorEncoding::Mapbox);
    XCTAssertFalse(tileSet.rasterEncoding);

    // when the raster encoding is changed using an NSNumber
    tileSet = MLNTileSetFromTileURLTemplates(tileURLTemplates, @{
        MLNVectorTileSourceOptionEncoding: @(MLNVectorTileSourceEncodingMLT),
    });

    // the encoding is reflected by the mbgl tileset
    XCTAssertEqual(tileSet.vectorEncoding, mbgl::Tileset::VectorEncoding::MLT);
    XCTAssertFalse(tileSet.rasterEncoding);

    // when the raster encoding is changed using an NSValue
    MLNVectorTileSourceEncoding mlt = MLNVectorTileSourceEncodingMLT;
    tileSet = MLNTileSetFromTileURLTemplates(tileURLTemplates, @{
        MLNVectorTileSourceOptionEncoding: [NSValue value:&mlt withObjCType:@encode(MLNVectorTileSourceEncoding)],
    });

    // the encoding is reflected by the mbgl tileset
    XCTAssertEqual(tileSet.vectorEncoding, mbgl::Tileset::VectorEncoding::MLT);
    XCTAssertFalse(tileSet.rasterEncoding);
}

- (void)testInvalidTileSet {
    // a tile set that provides an mbgl tile set and invalid (crossed) minimum and maximum zoom levels throws an exception
    XCTAssertThrowsSpecificNamed(MLNTileSetFromTileURLTemplates(@[@"tile.1"], @{
        MLNTileSourceOptionMinimumZoomLevel: @10,
        MLNTileSourceOptionMaximumZoomLevel: @9,
    }), NSException, NSInvalidArgumentException);
}

@end
