#import "MGLIntegrationTestCase.h"

@implementation MGLIntegrationTestCase

+ (XCTestSuite*)defaultTestSuite {

    XCTestSuite *defaultTestSuite = [super defaultTestSuite];

    NSArray *tests = defaultTestSuite.tests;

    XCTestSuite *newTestSuite = [XCTestSuite testSuiteWithName:defaultTestSuite.name];

    BOOL runPendingTests = [[[NSProcessInfo processInfo] environment][@"MAPBOX_RUN_PENDING_TESTS"] boolValue];
    NSString *accessToken = [[NSProcessInfo processInfo] environment][@"MAPBOX_ACCESS_TOKEN"];

    for (XCTest *test in tests) {

        // Check for pending tests
        if ([test.name containsString:@"PENDING"] ||
            [test.name containsString:@"🙁"]) {
            if (!runPendingTests) {
                printf("warning: '%s' is a pending test - skipping\n", test.name.UTF8String);
                continue;
            }
        }

        // Check for tests that require a valid access token
        if ([test.name containsString:@"🔒"]) {
            if (!accessToken) {
                printf("warning: MAPBOX_ACCESS_TOKEN env var is required for test '%s' - skipping.\n", test.name.UTF8String);
                continue;
            }
        }

        [newTestSuite addTest:test];
    }

    return newTestSuite;
}

- (void)setUp {
    [super setUp];

    NSString *accessToken;

    if ([self.name containsString:@"🔒"]) {
        accessToken = [[NSProcessInfo processInfo] environment][@"MAPBOX_ACCESS_TOKEN"];

        if (!accessToken) {
            printf("warning: MAPBOX_ACCESS_TOKEN env var is required for test '%s' - trying anyway.\n", self.name.UTF8String);
        }
    }

    [MGLAccountManager setAccessToken:accessToken ?: @"pk.feedcafedeadbeefbadebede"];
}
@end
