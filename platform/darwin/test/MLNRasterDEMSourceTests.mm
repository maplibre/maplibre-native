#import <XCTest/XCTest.h>

#import <MapLibre.h>

@interface MLNRasterDEMSourceTests : XCTestCase
@end

@implementation MLNRasterDEMSourceTests

// A `pmtiles://` or TileJSON URL cannot carry the encoding, so before the initializer took
// options a Terrarium tileset behind a URL always decoded as Mapbox (maplibre-native#4357).
- (void)testConfigurationURLSourceTakesTheEncodingOption {
  NSURL *url = [NSURL URLWithString:@"pmtiles://https://example.com/terrain.pmtiles"];

  MLNRasterDEMSource *terrarium = [[MLNRasterDEMSource alloc]
      initWithIdentifier:@"terrarium"
        configurationURL:url
                tileSize:512
                 options:@{MLNTileSourceOptionDEMEncoding : @(MLNDEMEncodingTerrarium)}];
  XCTAssertEqual(terrarium.encoding, MLNDEMEncodingTerrarium);
  XCTAssertEqualObjects(terrarium.configurationURL, url);

  MLNRasterDEMSource *mapbox = [[MLNRasterDEMSource alloc]
      initWithIdentifier:@"mapbox"
        configurationURL:url
                tileSize:512
                 options:@{MLNTileSourceOptionDEMEncoding : @(MLNDEMEncodingMapbox)}];
  XCTAssertEqual(mapbox.encoding, MLNDEMEncodingMapbox);
}

- (void)testEncodingDefaultsToMapbox {
  NSURL *url = [NSURL URLWithString:@"https://example.com/terrain.json"];

  XCTAssertEqual([[MLNRasterDEMSource alloc] initWithIdentifier:@"a" configurationURL:url].encoding,
                 MLNDEMEncodingMapbox);
  XCTAssertEqual([[MLNRasterDEMSource alloc] initWithIdentifier:@"b"
                                               configurationURL:url
                                                       tileSize:512
                                                        options:nil]
                     .encoding,
                 MLNDEMEncodingMapbox);
}

- (void)testTileURLTemplatesSourceReportsTheEncodingOption {
  MLNRasterDEMSource *source = [[MLNRasterDEMSource alloc]
      initWithIdentifier:@"templates"
        tileURLTemplates:@[ @"https://example.com/{z}/{x}/{y}.png" ]
                 options:@{MLNTileSourceOptionDEMEncoding : @(MLNDEMEncodingTerrarium)}];
  XCTAssertEqual(source.encoding, MLNDEMEncodingTerrarium);
}

- (void)testEncodingOptionOfTheWrongTypeRaises {
  NSURL *url = [NSURL URLWithString:@"https://example.com/terrain.json"];
  XCTAssertThrowsSpecificNamed(
      [[MLNRasterDEMSource alloc]
          initWithIdentifier:@"bad"
            configurationURL:url
                    tileSize:512
                     options:@{MLNTileSourceOptionDEMEncoding : @"terrarium"}],
      NSException, NSInvalidArgumentException);
}

@end
