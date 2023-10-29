#import <Foundation/Foundation.h>

#import "MLNFoundation.h"
#include <mbgl/util/chrono.hpp>

NS_ASSUME_NONNULL_BEGIN


/// Converts from a duration in seconds to a duration object usable in mbgl.
MLN_EXPORT
mbgl::Duration MLNDurationFromTimeInterval(NSTimeInterval duration);

/// Converts from an mbgl duration object to a duration in seconds.
MLN_EXPORT
NSTimeInterval MLNTimeIntervalFromDuration(mbgl::Duration duration);

NS_ASSUME_NONNULL_END
