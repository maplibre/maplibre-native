#import <XCTest/XCTest.h>

#import "NSOrthography+MLNAdditions.h"
#import "MLNVectorTileSource_Private.h"

@interface MLNNSOrthographyAdditionsTests : XCTestCase

@end

@implementation MLNNSOrthographyAdditionsTests

- (void)testStreetsLanguages {
    for (NSString *language in [MLNVectorTileSource mapboxStreetsLanguages]) {
        NSString *dominantScript = [NSOrthography mgl_dominantScriptForMapboxStreetsLanguage:language];
        XCTAssertNotEqualObjects(dominantScript, @"Zyyy", @"Mapbox Streets languages should have dominant script");
    }
}

@end
