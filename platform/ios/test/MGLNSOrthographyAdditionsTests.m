#import <XCTest/XCTest.h>

#import "NSOrthography+MGLAdditions.h"
#import "MGLVectorTileSource_Private.h"

@interface MGLNSOrthographyAdditionsTests : XCTestCase

@end

@implementation MGLNSOrthographyAdditionsTests

- (void)testStreetsLanguages {
    for (NSString *language in [MGLVectorTileSource mapboxStreetsLanguages]) {
        NSString *dominantScript = [NSOrthography mgl_dominantScriptForMapboxStreetsLanguage:language];
        XCTAssertNotEqualObjects(dominantScript, @"Zyyy", @"Mapbox Streets languages should have dominant script");
    }
}

@end
