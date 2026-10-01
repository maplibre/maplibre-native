#import <Foundation/Foundation.h>

#import <mln/util/feature.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface NSDictionary (MLNAdditions)

- (mln::PropertyMap)mgl_propertyMap;

@end

NS_ASSUME_NONNULL_END
