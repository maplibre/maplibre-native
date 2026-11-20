#import <XCTest/XCTest.h>

// Runs inside BenchmarkApp, which starts the benchmark when it launches. The results are logged with NSLog.
@interface BenchmarkTests : XCTestCase
@end

@implementation BenchmarkTests

- (void)testBenchmark {
    [self expectationForNotification:@"MBXBenchmarkDidFinish" object:nil handler:nil];
    [self waitForExpectationsWithTimeout:1000 handler:nil];
}

@end
