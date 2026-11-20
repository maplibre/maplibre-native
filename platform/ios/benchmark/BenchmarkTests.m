#import <XCTest/XCTest.h>

// Runs inside BenchmarkApp, which starts the benchmark when it launches.
@interface BenchmarkTests : XCTestCase
@end

@implementation BenchmarkTests

- (void)testBenchmark {
    [self expectationForNotification:@"MBXBenchmarkDidFinish"
                              object:nil
                             handler:^BOOL(NSNotification *notification) {
                                 // scripts/aws-device-farm/collect-benchmark-outputs.mjs picks this line up from the test output.
                                 NSString *results = [[NSString alloc] initWithData:notification.userInfo[@"results"]
                                                                           encoding:NSUTF8StringEncoding];
                                 printf("MLN_BENCHMARK_RESULTS %s\n", results.UTF8String);
                                 fflush(stdout);
                                 return YES;
                             }];
    [self waitForExpectationsWithTimeout:1000 handler:nil];
}

@end
