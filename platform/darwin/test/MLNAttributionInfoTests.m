#import <Mapbox.h>
#import <XCTest/XCTest.h>

#import "MLNAttributionInfo_Private.h"

@interface MLNAttributionInfoTests : XCTestCase

@end

@implementation MLNAttributionInfoTests

- (void)setUp {
    [MLNSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
}

- (void)tearDown {
    [MLNSettings setApiKey:nil];
}

- (void)testParsing {
    static NSString * const htmlStrings[] = {
        @"<a href=\"https://www.mapbox.com/about/maps/\" target=\"_blank\">&copy; Mapbox</a> "
        @"<a href=\"http://www.openstreetmap.org/about/\" target=\"_blank\">©️ OpenStreetMap</a> "
        @"CC&nbsp;BY-SA "
        @"<a class=\"mapbox-improve-map\" href=\"https://apps.mapbox.com/feedback/\" target=\"_blank\">Improve this map</a>",
    };

    NSMutableArray<MLNAttributionInfo *> *infos = [NSMutableArray array];
    for (NSUInteger i = 0; i < sizeof(htmlStrings) / sizeof(htmlStrings[0]); i++) {
        NSArray *subinfos = [MLNAttributionInfo attributionInfosFromHTMLString:htmlStrings[i]
                                                                      fontSize:0
                                                                     linkColor:nil];
        [infos growArrayByAddingAttributionInfosFromArray:subinfos];
    }

    XCTAssertEqual(infos.count, 4);

    XCTAssertEqualObjects(infos[0].title.string, @"© Mapbox");
    XCTAssertEqualObjects(infos[0].URL, [NSURL URLWithString:@"https://www.mapbox.com/about/maps/"]);
    XCTAssertFalse(infos[0].feedbackLink);

    XCTAssertEqualObjects(infos[1].title.string, @"©️ OpenStreetMap");
    XCTAssertEqualObjects(infos[1].URL, [NSURL URLWithString:@"http://www.openstreetmap.org/about/"]);
    XCTAssertFalse(infos[1].feedbackLink);

    XCTAssertEqualObjects(infos[2].title.string, @"CC\u00a0BY-SA");
    XCTAssertNil(infos[2].URL);
    XCTAssertFalse(infos[2].feedbackLink);
}

- (void)testStyle {
    static NSString * const htmlStrings[] = {
        @"<a href=\"https://www.mapbox.com/\">Mapbox</a>",
    };

    CGFloat fontSize = 72;
    MLNColor *color = [MLNColor redColor];
    NSMutableArray<MLNAttributionInfo *> *infos = [NSMutableArray array];
    for (NSUInteger i = 0; i < sizeof(htmlStrings) / sizeof(htmlStrings[0]); i++) {
        NSArray *subinfos = [MLNAttributionInfo attributionInfosFromHTMLString:htmlStrings[i]
                                                                      fontSize:72
                                                                     linkColor:color];
        [infos growArrayByAddingAttributionInfosFromArray:subinfos];
    }

    XCTAssertEqual(infos.count, 1);

    XCTAssertEqualObjects(infos[0].title.string, @"Mapbox");
    XCTAssertNil([infos[0].title attribute:NSLinkAttributeName atIndex:0 effectiveRange:nil]);
    XCTAssertEqualObjects([infos[0].title attribute:NSUnderlineStyleAttributeName atIndex:0 effectiveRange:nil], @(NSUnderlineStyleSingle));

#if TARGET_OS_IPHONE
    UIFont *font;
#else
    NSFont *font;
#endif
    font = [infos[0].title attribute:NSFontAttributeName atIndex:0 effectiveRange:nil];
    XCTAssertEqual(font.pointSize, fontSize);

    CGFloat r, g, b, a;
    [color getRed:&r green:&g blue:&b alpha:&a];
    MLNColor *linkColor = [infos[0].title attribute:NSForegroundColorAttributeName atIndex:0 effectiveRange:nil];
    CGFloat linkR, linkG, linkB, linkA;
    [linkColor getRed:&linkR green:&linkG blue:&linkB alpha:&linkA];
    XCTAssertEqual(r, linkR);
    XCTAssertEqual(g, linkG);
    XCTAssertEqual(b, linkB);
    XCTAssertEqual(a, linkA);
}

- (void)testDedupe {
    static NSString * const htmlStrings[] = {
        @"World",
        @"Hello World",
        @"Another Source",
        @"Hello",
        @"Hello World",
    };

    NSMutableArray<MLNAttributionInfo *> *infos = [NSMutableArray array];
    for (NSUInteger i = 0; i < sizeof(htmlStrings) / sizeof(htmlStrings[0]); i++) {
        NSArray *subinfos = [MLNAttributionInfo attributionInfosFromHTMLString:htmlStrings[i]
                                                                      fontSize:0
                                                                     linkColor:nil];
        [infos growArrayByAddingAttributionInfosFromArray:subinfos];
    }

    XCTAssertEqual(infos.count, 2);
    XCTAssertEqualObjects(infos[0].title.string, @"Hello World");
    XCTAssertEqualObjects(infos[1].title.string, @"Another Source");
}

@end
