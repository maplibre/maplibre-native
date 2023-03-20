#import "MLNLoggingConfiguration.h"

NS_INLINE NSString *MLNStringFromBOOL(BOOL value) {
    return value ? @"YES" : @"NO";
}

#if TARGET_OS_OSX
NS_INLINE NSString *MLNStringFromNSEdgeInsets(NSEdgeInsets insets) {
    return [NSString stringWithFormat:@"{ top: %f, left: %f, bottom: %f, right: %f", insets.top, insets.left, insets.bottom, insets.right];
}
#endif

#ifdef MLN_LOGGING_DISABLED

#define MLNLogInfo(...)
#define MLNLogDebug(...)
#define MLNLogWarning(...)
#define MLNLogError(...)
#define MLNLogFault(...)

#else

#if MLN_LOGGING_ENABLE_DEBUG
    #define MLNLogDebug(message, ...) MLNLogWithType(MLNLoggingLevelDebug, __PRETTY_FUNCTION__, __LINE__, message, ##__VA_ARGS__)
#else
    #define MLNLogDebug(...)
#endif

#define MLNLogInfo(message, ...)     MLNLogWithType(MLNLoggingLevelInfo, __PRETTY_FUNCTION__, __LINE__, message, ##__VA_ARGS__)
#define MLNLogWarning(message, ...)  MLNLogWithType(MLNLoggingLevelWarning, __PRETTY_FUNCTION__, __LINE__, message, ##__VA_ARGS__)
#define MLNLogError(message, ...)    MLNLogWithType(MLNLoggingLevelError, __PRETTY_FUNCTION__, __LINE__, message, ##__VA_ARGS__)
#define MLNLogFault(message, ...)    MLNLogWithType(MLNLoggingLevelFault, __PRETTY_FUNCTION__, __LINE__, message, ##__VA_ARGS__)

#endif

#define MLNAssert(expression, message, ...) \
    __extension__({ \
        if (__builtin_expect(!(expression), 0)) { \
            MLNLogFault(message, ##__VA_ARGS__); \
        } \
        NSAssert(expression, message, ##__VA_ARGS__); \
    })
#define MLNCAssert(expression, message, ...) \
    __extension__({ \
        if (__builtin_expect(!(expression), 0)) { \
            MLNLogFault(message, ##__VA_ARGS__); \
        } \
        NSCAssert(expression, message, ##__VA_ARGS__); \
    })


#ifndef MLN_LOGGING_DISABLED

#define MLNLogWithType(type, function, line, message, ...) \
{ \
    if ([MLNLoggingConfiguration sharedConfiguration].loggingLevel != MLNLoggingLevelNone && type <= [MLNLoggingConfiguration sharedConfiguration].loggingLevel) \
    { \
        [[MLNLoggingConfiguration sharedConfiguration] logCallingFunction:function functionLine:line messageType:type format:(message), ##__VA_ARGS__]; \
    } \
}

@interface MLNLoggingConfiguration (Private)

- (void)logCallingFunction:(const char *)callingFunction functionLine:(NSUInteger)functionLine messageType:(MLNLoggingLevel)type format:(id)messageFormat, ...;

@end
#endif
