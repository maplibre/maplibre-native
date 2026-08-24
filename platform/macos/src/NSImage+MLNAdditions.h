#import <Cocoa/Cocoa.h>

#include <mln/style/image.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface NSImage (MLNAdditions)

- (nullable instancetype)initWithMLNPremultipliedImage:(mln::PremultipliedImage &&)image;

- (nullable instancetype)initWithMLNStyleImage:(const mln::style::Image &)image;

- (std::unique_ptr<mln::style::Image>)mgl_styleImageWithIdentifier:(NSString *)identifier;

- (mln::PremultipliedImage)mgl_premultipliedImage;

@end

NS_ASSUME_NONNULL_END
