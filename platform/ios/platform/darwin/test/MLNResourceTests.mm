#import <Mapbox.h>
#import <XCTest/XCTest.h>
#import <mbgl/storage/resource.hpp>

namespace mbgl {
    extern NSURL *resourceURL(const Resource& resource);
    extern BOOL isValidMapboxEndpoint(NSURL *url);
}
    
@interface MLNResourceTests : XCTestCase
@end

@implementation MLNResourceTests

- (void)testValidEndpoints {
    using namespace mbgl;

    XCTAssertTrue(isValidMapboxEndpoint([NSURL URLWithString:@"https://mapbox.com"]));
    XCTAssertTrue(isValidMapboxEndpoint([NSURL URLWithString:@"https://mapbox.cn"]));
    XCTAssertTrue(isValidMapboxEndpoint([NSURL URLWithString:@"https://example.mapbox.com"]));
    XCTAssertTrue(isValidMapboxEndpoint([NSURL URLWithString:@"https://example.mapbox.cn"]));

    XCTAssertFalse(isValidMapboxEndpoint([NSURL URLWithString:@"https://example.com"]));
    XCTAssertFalse(isValidMapboxEndpoint([NSURL URLWithString:@"https://example.cn"]));
    XCTAssertFalse(isValidMapboxEndpoint([NSURL URLWithString:@"https://examplemapbox.com"]));
    XCTAssertFalse(isValidMapboxEndpoint([NSURL URLWithString:@"https://examplemapbox.cn"]));
}

- (void)internalTestOfflineQueryParameterIsAddedForOfflineResource:(std::string)testURL {
    
    using namespace mbgl;

    // Is our test URL "correct" for subsequent checks?
    {
        NSURL *url = [NSURL URLWithString:@(testURL.c_str())];
        NSURLComponents *components = [NSURLComponents componentsWithURL:url resolvingAgainstBaseURL:NO];
        NSArray<NSURLQueryItem *> *items = components.queryItems;
        XCTAssert(items.count == 2);
    }

    Resource resource(Resource::Kind::Unknown, testURL);

    // Now check offline
    resource.setUsage(Resource::Usage::Offline);
    
    {
        NSURL *url = resourceURL(resource);
        NSURLComponents *components = [NSURLComponents componentsWithURL:url resolvingAgainstBaseURL:NO];
        
        // For offline, we expect a single offline query item
        NSInteger foundCount = 0;
        
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
        for (NSURLQueryItem *item in components.queryItems) {
            if (([item.name isEqualToString:@"offline"] && [item.value isEqualToString:@"true"]) ||
                ([item.name isEqualToString:@"a"] && [item.value isEqualToString:@"one"]) ||
                ([item.name isEqualToString:@"b"] && [item.value isEqualToString:@"two"])) {
                foundCount++;
            }
            XCTAssertFalse([item.name isEqualToString:@"sku"]);
        }

        XCTAssert(foundCount == 3);
#else
        // NOTE: Currently the macOS SDK does not supply the sku or offline query parameters
        for (NSURLQueryItem *item in components.queryItems) {
            if (([item.name isEqualToString:@"a"] && [item.value isEqualToString:@"one"]) ||
                ([item.name isEqualToString:@"b"] && [item.value isEqualToString:@"two"])) {
                foundCount++;
            }
            XCTAssertFalse([item.name isEqualToString:@"sku"]);
        }
        
        XCTAssert(foundCount == 2);
#endif
    }
}

- (void)testOfflineQueryParameterIsAddedForOfflineResource {
    std::string testURL = "test://mapbox.com/testing_offline_query?a=one&b=two";
    [self internalTestOfflineQueryParameterIsAddedForOfflineResource:testURL];
}

- (void)testOfflineQueryParameterIsAddedForOfflineResourceForChina {
    std::string testURL = "test://mapbox.cn/testing_offline_query?a=one&b=two";
    [self internalTestOfflineQueryParameterIsAddedForOfflineResource:testURL];
}

@end
