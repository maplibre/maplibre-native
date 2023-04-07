#import <Cocoa/Cocoa.h>

#include <mbgl/style/image.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface NSImage (MLNAdditions)

- (nullable instancetype)initWithMLNPremultipliedImage:(mbgl::PremultipliedImage&&)image;

- (nullable instancetype)initWithMLNStyleImage:(const mbgl::style::Image &)image;

- (std::unique_ptr<mbgl::style::Image>)mgl_styleImageWithIdentifier:(NSString *)identifier;

- (mbgl::PremultipliedImage) mgl_premultipliedImage;

@end

NS_ASSUME_NONNULL_END
