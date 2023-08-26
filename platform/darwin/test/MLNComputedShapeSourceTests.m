#import <XCTest/XCTest.h>

#import <Mapbox.h>


@interface MLNComputedShapeSourceTests : XCTestCase
@end

@implementation MLNComputedShapeSourceTests

- (void)testInitializer {
    MLNComputedShapeSource *source = [[MLNComputedShapeSource alloc] initWithIdentifier:@"id" options:@{}];
    XCTAssertNotNil(source);
    XCTAssertNotNil(source.requestQueue);
    XCTAssertNil(source.dataSource);
}

- (void)testNilOptions {
    MLNComputedShapeSource *source = [[MLNComputedShapeSource alloc] initWithIdentifier:@"id" options:nil];
    XCTAssertNotNil(source);
}


@end
