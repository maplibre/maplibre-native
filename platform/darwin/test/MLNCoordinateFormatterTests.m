#import <Mapbox.h>
#import <XCTest/XCTest.h>

@interface MLNCoordinateFormatterTests : XCTestCase

@end

@implementation MLNCoordinateFormatterTests

- (void)testStrings {
    MLNCoordinateFormatter *shortFormatter = [[MLNCoordinateFormatter alloc] init];
    shortFormatter.unitStyle = NSFormattingUnitStyleShort;
    XCTAssertTrue(shortFormatter.allowsSeconds, @"Arcseconds should be allowed by default.");
    XCTAssertTrue(shortFormatter.allowsMinutes, @"Arcminutes should be allowed by default.");

    MLNCoordinateFormatter *mediumFormatter = [[MLNCoordinateFormatter alloc] init];
    XCTAssertEqual(mediumFormatter.unitStyle, NSFormattingUnitStyleMedium, @"Unit style should be medium by default.");

    MLNCoordinateFormatter *longFormatter = [[MLNCoordinateFormatter alloc] init];
    longFormatter.unitStyle = NSFormattingUnitStyleLong;

    CLLocationCoordinate2D coordinate;

    coordinate = CLLocationCoordinate2DMake(38.9131982, -77.0325453144239);
    XCTAssertEqualObjects([shortFormatter stringFromCoordinate:coordinate], @"38°54′48″N, 77°1′57″W");
    XCTAssertEqualObjects([mediumFormatter stringFromCoordinate:coordinate], @"38°54′48″ north, 77°1′57″ west");
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    XCTAssertEqualObjects([longFormatter stringFromCoordinate:coordinate], @"38 degree(s), 54 minute(s), and 48 second(s) north by 77 degree(s), 1 minute(s), and 57 second(s) west");
    #else
    XCTAssertEqualObjects([longFormatter stringFromCoordinate:coordinate], @"38 degrees, 54 minutes, and 48 seconds north by 77 degrees, 1 minute, and 57 seconds west");
    #endif

    shortFormatter.allowsSeconds = NO;
    mediumFormatter.allowsSeconds = NO;
    longFormatter.allowsSeconds = NO;

    coordinate = CLLocationCoordinate2DMake(38.9131982, -77.0325453144239);
    XCTAssertEqualObjects([shortFormatter stringFromCoordinate:coordinate], @"38°55′N, 77°2′W");
    XCTAssertEqualObjects([mediumFormatter stringFromCoordinate:coordinate], @"38°55′ north, 77°2′ west");
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    XCTAssertEqualObjects([longFormatter stringFromCoordinate:coordinate], @"38 degree(s) and 55 minute(s) north by 77 degree(s) and 2 minute(s) west");
    #else
    XCTAssertEqualObjects([longFormatter stringFromCoordinate:coordinate], @"38 degrees and 55 minutes north by 77 degrees and 2 minutes west");
    #endif
    

    shortFormatter.allowsMinutes = NO;
    mediumFormatter.allowsMinutes = NO;
    longFormatter.allowsMinutes = NO;

    coordinate = CLLocationCoordinate2DMake(38.9131982, -77.0325453144239);
    XCTAssertEqualObjects([shortFormatter stringFromCoordinate:coordinate], @"39°N, 77°W");
    XCTAssertEqualObjects([mediumFormatter stringFromCoordinate:coordinate], @"39° north, 77° west");
    
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    XCTAssertEqualObjects([longFormatter stringFromCoordinate:coordinate], @"39 degree(s) north by 77 degree(s) west");
    #else
    XCTAssertEqualObjects([longFormatter stringFromCoordinate:coordinate], @"39 degrees north by 77 degrees west");
    #endif

}

@end
