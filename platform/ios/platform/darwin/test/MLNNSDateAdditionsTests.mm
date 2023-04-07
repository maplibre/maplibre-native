#import <XCTest/XCTest.h>

#include <mbgl/util/chrono.hpp>
#import "../../darwin/src/NSDate+MLNAdditions.h"

using namespace std::chrono_literals;

@interface MLNNSDateAdditionsTests : XCTestCase
@end

@implementation MLNNSDateAdditionsTests

- (void)testDurationToNSTimeInterval {
    
    NSTimeInterval timeInterval = 5;
    mbgl::Duration duration = MLNDurationFromTimeInterval(timeInterval);
    NSTimeInterval durationTimeInterval = MLNTimeIntervalFromDuration(duration);
    
    mbgl::Duration expectedDuration = 5s;
    mbgl::Duration expectedDurationMiliSeconds = 5000ms;
    mbgl::Duration expectedDurationMicroSeconds = 5000000us;
    mbgl::Duration expectedDurationNanoSeconds = 5000000000ns;
    
    XCTAssertEqual(timeInterval, durationTimeInterval);
    XCTAssertEqual(timeInterval, MLNTimeIntervalFromDuration(expectedDuration));
    XCTAssertEqual(timeInterval, MLNTimeIntervalFromDuration(expectedDurationMiliSeconds));
    XCTAssertEqual(timeInterval, MLNTimeIntervalFromDuration(expectedDurationMicroSeconds));
    XCTAssertEqual(timeInterval, MLNTimeIntervalFromDuration(expectedDurationNanoSeconds));
    
    mbgl::Duration durationMiliSeconds = 2500ms;
    mbgl::Duration durationMicroSeconds = 2500000us;
    mbgl::Duration durationNanoSeconds = 2500000000ns;
    
    XCTAssertEqual(NSTimeInterval(2.5), MLNTimeIntervalFromDuration(durationMiliSeconds));
    XCTAssertEqual(NSTimeInterval(2.5), MLNTimeIntervalFromDuration(durationMicroSeconds));
    XCTAssertEqual(NSTimeInterval(2.5), MLNTimeIntervalFromDuration(durationNanoSeconds));
    
}

@end
