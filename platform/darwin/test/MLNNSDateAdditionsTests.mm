#import <XCTest/XCTest.h>

#include <mln/util/chrono.hpp>
#import "../../darwin/src/NSDate+MLNAdditions.h"

using namespace std::chrono_literals;

@interface MLNNSDateAdditionsTests : XCTestCase
@end

@implementation MLNNSDateAdditionsTests

- (void)testDurationToNSTimeInterval {
  NSTimeInterval timeInterval = 5;
  mln::Duration duration = MLNDurationFromTimeInterval(timeInterval);
  NSTimeInterval durationTimeInterval = MLNTimeIntervalFromDuration(duration);

  mln::Duration expectedDuration = 5s;
  mln::Duration expectedDurationMiliSeconds = 5000ms;
  mln::Duration expectedDurationMicroSeconds = 5000000us;
  mln::Duration expectedDurationNanoSeconds = 5000000000ns;

  XCTAssertEqual(timeInterval, durationTimeInterval);
  XCTAssertEqual(timeInterval, MLNTimeIntervalFromDuration(expectedDuration));
  XCTAssertEqual(timeInterval, MLNTimeIntervalFromDuration(expectedDurationMiliSeconds));
  XCTAssertEqual(timeInterval, MLNTimeIntervalFromDuration(expectedDurationMicroSeconds));
  XCTAssertEqual(timeInterval, MLNTimeIntervalFromDuration(expectedDurationNanoSeconds));

  mln::Duration durationMiliSeconds = 2500ms;
  mln::Duration durationMicroSeconds = 2500000us;
  mln::Duration durationNanoSeconds = 2500000000ns;

  XCTAssertEqual(NSTimeInterval(2.5), MLNTimeIntervalFromDuration(durationMiliSeconds));
  XCTAssertEqual(NSTimeInterval(2.5), MLNTimeIntervalFromDuration(durationMicroSeconds));
  XCTAssertEqual(NSTimeInterval(2.5), MLNTimeIntervalFromDuration(durationNanoSeconds));
}

@end
