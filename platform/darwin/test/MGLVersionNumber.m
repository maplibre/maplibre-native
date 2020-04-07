#import <Mapbox/Mapbox.h>

#import <XCTest/XCTest.h>

@interface MGLVersionTests : XCTestCase

@end

@implementation MGLVersionTests

- (void)testVersionNumber {
#if TARGET_OS_IPHONE
    XCTAssertEqual(1, MapboxVersionNumber);
#else
    XCTAssertGreaterThan(MapboxVersionNumber, 1);
    XCTAssertGreaterThan(MapboxVersionString[0] + MapboxVersionString[1] + MapboxVersionString[2], 0);
#endif
}

@end
