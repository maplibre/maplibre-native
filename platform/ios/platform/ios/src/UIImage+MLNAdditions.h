#import <UIKit/UIKit.h>

#import "MLNTypes.h"

#include <mbgl/style/image.hpp>

NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXTERN MLN_EXPORT MLNExceptionName const MLNResourceNotFoundException;

@interface UIImage (MLNAdditions)

- (nullable instancetype)initWithMLNStyleImage:(const mbgl::style::Image &)styleImage;

- (nullable instancetype)initWithMLNPremultipliedImage:(const mbgl::PremultipliedImage&&)mbglImage scale:(CGFloat)scale;

- (std::unique_ptr<mbgl::style::Image>)mgl_styleImageWithIdentifier:(NSString *)identifier;

- (mbgl::PremultipliedImage)mgl_premultipliedImage;

+ (UIImage *)mgl_resourceImageNamed:(NSString *)imageName;

- (BOOL)isDataEqualTo:(UIImage*)otherImage;

@end

NS_ASSUME_NONNULL_END
