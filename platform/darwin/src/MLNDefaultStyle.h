#import <Foundation/Foundation.h>

#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

/**
 The ``MLNDefaultStyle`` defines the predefined vendor style
 */
MLN_EXPORT
@interface MLNDefaultStyle : NSObject

/**
The style URL
 */
@property (nonatomic, retain) NSURL* url;

/**
The style name
 */
@property (nonatomic, retain) NSString* name;

/**
The style version
 */
@property (nonatomic) int version;

@end

NS_ASSUME_NONNULL_END
