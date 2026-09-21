#import <UIKit/UIKit.h>

#import "MLNTypes.h"

#include <mln/style/image.hpp>

NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXTERN MLN_EXPORT MLNExceptionName const MLNResourceNotFoundException;

@interface UIImage (MLNAdditions)

- (nullable instancetype)initWithMLNStyleImage:(const mln::style::Image &)styleImage;

- (nullable instancetype)initWithMLNPremultipliedImage:(const mln::PremultipliedImage &&)mbglImage
                                                 scale:(CGFloat)scale;

- (std::unique_ptr<mln::style::Image>)mgl_styleImageWithIdentifier:(NSString *)identifier;

- (mln::PremultipliedImage)mgl_premultipliedImage;

+ (UIImage *)mgl_resourceImageNamed:(NSString *)imageName;

- (BOOL)isDataEqualTo:(UIImage *)otherImage;

@end

NS_ASSUME_NONNULL_END
