#import <Mapbox/Mapbox.h>
#import <XCTest/XCTest.h>

#import "MLNAttributionButton.h"
#import "MLNAttributionInfo.h"

@interface MLNAttributionButtonTests : XCTestCase

@end

@implementation MLNAttributionButtonTests

- (void)testPlainSymbol {
    NSAttributedString *title = [[NSAttributedString alloc] initWithString:@"® & ™ Mapbox" attributes:@{
        NSUnderlineStyleAttributeName: @(NSUnderlineStyleSingle),
    }];
    MLNAttributionInfo *info = [[MLNAttributionInfo alloc] initWithTitle:title URL:nil];
    MLNAttributionButton *button = [[MLNAttributionButton alloc] initWithAttributionInfo:info];

    NSRange symbolUnderlineRange;
    NSNumber *symbolUnderline = [button.attributedTitle attribute:NSUnderlineStyleAttributeName atIndex:0 effectiveRange:&symbolUnderlineRange];
    XCTAssertNil(symbolUnderline);
    XCTAssertEqual(symbolUnderlineRange.length, 6);

    NSRange wordUnderlineRange;
    NSNumber *wordUnderline = [button.attributedTitle attribute:NSUnderlineStyleAttributeName atIndex:6 effectiveRange:&wordUnderlineRange];
    XCTAssertEqualObjects(wordUnderline, @(NSUnderlineStyleSingle));
    XCTAssertEqual(wordUnderlineRange.length, 6);
}

@end
