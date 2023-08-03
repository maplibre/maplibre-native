#import <Foundation/Foundation.h>

#import <mbgl/util/feature.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface NSDictionary (MLNAdditions)

- (mbgl::PropertyMap)mgl_propertyMap;

@end

NS_ASSUME_NONNULL_END
