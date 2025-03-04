#import <Foundation/Foundation.h>

#import "MLNFoundation.h"
#import "MLNStyleLayer.h"

NS_ASSUME_NONNULL_BEGIN

@class MLNSource;

/**
 ``MLNForegroundStyleLayer`` is an abstract superclass for style layers whose
 content is defined by an ``MLNSource`` object.

 Create instances of ``MLNRasterStyleLayer``, ``MLNRasterStyleLayer``, and the
 concrete subclasses of ``MLNVectorStyleLayer`` in order to use
 ``MLNForegroundStyleLayer``'s methods. Do not create instances of
 ``MLNForegroundStyleLayer`` directly, and do not create your own subclasses of
 this class.
 */
MLN_EXPORT
@interface MLNForegroundStyleLayer : MLNStyleLayer

// MARK: Initializing a Style Layer

- (instancetype)init
    __attribute__((unavailable("Use -init methods of concrete subclasses instead.")));

// MARK: Specifying a Style Layer’s Content

/**
 Identifier of the source from which the receiver obtains the data to style.
 */
@property (nonatomic, readonly, nullable) NSString *sourceIdentifier;

@end

NS_ASSUME_NONNULL_END
