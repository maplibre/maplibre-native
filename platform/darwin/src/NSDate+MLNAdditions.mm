#import <ratio>
#import "NSDate+MLNAdditions.h"

mln::Duration MLNDurationFromTimeInterval(NSTimeInterval duration) {
  return std::chrono::duration_cast<mln::Duration>(std::chrono::duration<NSTimeInterval>(duration));
}

NSTimeInterval MLNTimeIntervalFromDuration(mln::Duration duration) {
  return std::chrono::duration<NSTimeInterval, std::ratio<1>>(duration).count();
}
