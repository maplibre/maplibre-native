#import <Mapbox/Mapbox.h>

#import <XCTest/XCTest.h>

@interface MGLVersionTests : XCTestCase

@end

@implementation MGLVersionTests

- (void)testVersionNumber {
    XCTAssertGreaterThanOrEqual(MapboxVersionNumber, 0);
}

@end
