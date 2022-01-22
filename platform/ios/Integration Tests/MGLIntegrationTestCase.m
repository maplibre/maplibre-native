#import "MGLIntegrationTestCase.h"

@implementation MGLIntegrationTestCase

+ (XCTestSuite*)defaultTestSuite {

    XCTestSuite *defaultTestSuite = [super defaultTestSuite];

    NSArray *tests = defaultTestSuite.tests;

    XCTestSuite *newTestSuite = [XCTestSuite testSuiteWithName:defaultTestSuite.name];

    BOOL runPendingTests = [[[NSProcessInfo processInfo] environment][@"MAPBOX_RUN_PENDING_TESTS"] boolValue];
    NSString *apiKey = [[NSProcessInfo processInfo] environment][@"MGL_API_KEY"];

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
            if (!apiKey) {
                printf("warning: MGL_API_KEY env var is required for test '%s' - skipping.\n", test.name.UTF8String);
                continue;
            }
        }

        [newTestSuite addTest:test];
    }

    return newTestSuite;
}

- (void)setUp {
    [super setUp];

    NSString *apiKey;

    if ([self.name containsString:@"🔒"]) {
        apiKey = [[NSProcessInfo processInfo] environment][@"MGL_API_KEY"];

        if (!apiKey) {
            printf("warning: MGL_API_KEY env var is required for test '%s' - trying anyway.\n", self.name.UTF8String);
        }
    }

    [MGLSettings setApiKey:apiKey ?: @"pk.feedcafedeadbeefbadebede"];
}
@end
