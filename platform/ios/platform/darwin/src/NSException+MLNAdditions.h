#import <Foundation/Foundation.h>
#import "MLNLoggingConfiguration_Private.h"

#define MLNAssertIsMainThread() MLNAssert([[NSThread currentThread] isMainThread], @"%s must be accessed on the main thread, not %@", __PRETTY_FUNCTION__, [NSThread currentThread])
