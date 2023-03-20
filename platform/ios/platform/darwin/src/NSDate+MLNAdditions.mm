#import "NSDate+MLNAdditions.h"
#import <ratio>

mbgl::Duration MLNDurationFromTimeInterval(NSTimeInterval duration)
{
    return std::chrono::duration_cast<mbgl::Duration>(std::chrono::duration<NSTimeInterval>(duration));
}

NSTimeInterval MLNTimeIntervalFromDuration(mbgl::Duration duration)
{
    return std::chrono::duration<NSTimeInterval, std::ratio<1>>(duration).count();
}
