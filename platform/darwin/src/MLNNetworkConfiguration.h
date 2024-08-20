#import <Foundation/Foundation.h>

#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

@class MLNNetworkConfiguration;

@protocol MLNNetworkConfigurationDelegate <NSObject>
@optional

/**
 :nodoc:
 Provides an `NSURLSession` object for the specified ``MLNNetworkConfiguration``.
 This API should be considered experimental, likely to be removed or changed in
 future releases.

 This method is called from background threads, i.e. it is not called on the main
 thread.

 > Note: Background sessions (i.e. created with
 `-[NSURLSessionConfiguration backgroundSessionConfigurationWithIdentifier:]`)
 and sessions created with a delegate that conforms to `NSURLSessionDataDelegate`
 are not supported at this time.
 */
- (NSURLSession *)sessionForNetworkConfiguration:(MLNNetworkConfiguration *)configuration;
@end

/**
 The ``MLNNetworkConfiguration`` object provides a global way to set a base
 `NSURLSessionConfiguration`, and other resources.
 */
MLN_EXPORT
@interface MLNNetworkConfiguration : NSObject

/**
 :nodoc:
 Delegate for the ``MLNNetworkConfiguration`` class.
 */
@property (nonatomic, weak) id<MLNNetworkConfigurationDelegate> delegate;

/**
 Returns the shared instance of the ``MLNNetworkConfiguration`` class.
 */
@property (class, nonatomic, readonly) MLNNetworkConfiguration *sharedManager;

/**
 The session configuration object that is used by the `NSURLSession` objects
 in this SDK.

 If this property is set to nil or if no session configuration is provided this property
 is set to the default session configuration.

 Assign this object before instantiating any ``MLNMapView`` object, or using
 ``MLNOfflineStorage``

 > Note: `NSURLSession` objects store a copy of this configuration. Any further changes
 to mutable properties on this configuration object passed to a session’s initializer
 will not affect the behavior of that session.

 > Note: Background sessions are not currently supported.
 */
@property (atomic, strong, null_resettable) NSURLSessionConfiguration *sessionConfiguration;

@end

NS_ASSUME_NONNULL_END
