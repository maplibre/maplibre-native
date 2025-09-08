#import <Foundation/Foundation.h>

#import "MLNFoundation.h"

#ifndef MLN_LOGGING_DISABLED
#ifndef MLN_LOGGING_ENABLE_DEBUG
#ifndef NDEBUG
#define MLN_LOGGING_ENABLE_DEBUG 1
#endif
#endif

NS_ASSUME_NONNULL_BEGIN

/**
 Constants indicating the message's logging level.
 */
typedef NS_ENUM(NSInteger, MLNLoggingLevel) {
  /**
   None-level won't print any messages.
   */
  MLNLoggingLevelNone = 0,
  /**
   Fault-level messages contain system-level error information.
   */
  MLNLoggingLevelFault,
  /**
   Error-level messages contain information that is intended to aid in process-level
   errors.
  */
  MLNLoggingLevelError,
  /**
   Warning-level messages contain warning information for potential risks.
   */
  MLNLoggingLevelWarning,
  /**
   Info-level messages contain information that may be helpful for flow tracing
   but is not essential.
   */
  MLNLoggingLevelInfo,
/**
 Debug-level messages contain information that may be helpful for troubleshooting
 specific problems.
 */
#if MLN_LOGGING_ENABLE_DEBUG
  MLNLoggingLevelDebug,
#endif
  /**
   Verbose-level will print all messages.
   */
  MLNLoggingLevelVerbose,
};

/**
 A block to be called once `loggingLevel` is set to a higher value than
 ``MLNLoggingLevel/MLNLoggingLevelNone``.

 @param loggingLevel The message logging level.
 @param filePath The description of the file and method for the calling message.
 @param line The line where the message is logged.
 @param message The logging message.
 */
typedef void (^MLNLoggingBlockHandler)(MLNLoggingLevel loggingLevel, NSString *filePath,
                                       NSUInteger line, NSString *message);

/**
 The ``MLNLoggingConfiguration`` object provides a global way to set this SDK logging levels
 and logging handler.
 */
MLN_EXPORT
@interface MLNLoggingConfiguration : NSObject

/**
 The handler this SDK uses to log messages.

 If this property is set to nil or if no custom handler is provided this property
 is set to the default handler.

 The default handler uses `os_log` and `NSLog` for iOS 10+ and iOS < 10 respectively.
 */
@property (nonatomic, copy, null_resettable) MLNLoggingBlockHandler handler;

/**
 The logging level.

 The default value is ``MLNLoggingLevel/MLNLoggingLevelNone``.

 Setting this property includes logging levels less than or equal to the setted value.
 */
@property (assign, nonatomic) MLNLoggingLevel loggingLevel;

/**
 Returns the shared logging object.
 */
@property (class, nonatomic, readonly) MLNLoggingConfiguration *sharedConfiguration;

- (MLNLoggingBlockHandler)handler UNAVAILABLE_ATTRIBUTE;

@end

NS_ASSUME_NONNULL_END
#endif
