#import "NSImage+MGLAdditions.h"

#include <mbgl/util/image+MGLAdditions.hpp>

BOOL MGLEdgeInsetsIsZero(NSEdgeInsets edgeInsets) {
    return edgeInsets.left == 0 && edgeInsets.top == 0 && edgeInsets.right == 0 && edgeInsets.bottom == 0;
}

@implementation NSImage (MGLAdditions)

- (nullable instancetype)initWithMGLPremultipliedImage:(mbgl::PremultipliedImage&&)src {
    CGImageRef image = CGImageCreateWithMGLPremultipliedImage(std::move(src));
    if (!image) {
        return nil;
    }

    self = [self initWithCGImage:image size:NSZeroSize];
    CGImageRelease(image);
    return self;
}

- (nullable instancetype)initWithMGLStyleImage:(const mbgl::style::Image &)styleImage {
    CGImageRef image = CGImageCreateWithMGLPremultipliedImage(styleImage.getImage().clone());
    if (!image) {
        return nil;
    }

    NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithCGImage:image];
    CGImageRelease(image);
    CGFloat w = styleImage.getImage().size.width / styleImage.getPixelRatio();
    CGFloat h = styleImage.getImage().size.height / styleImage.getPixelRatio();
    if (self = [self initWithSize:NSMakeSize(w, h)]) {
        [self addRepresentation:rep];
        [self setTemplate:styleImage.isSdf()];
    }
    return self;
}

- (std::unique_ptr<mbgl::style::Image>)mgl_styleImageWithIdentifier:(NSString *)identifier {
    mbgl::PremultipliedImage cPremultipliedImage = self.mgl_premultipliedImage;
    auto imageWidth = cPremultipliedImage.size.width;
    
    float scale = static_cast<float>(imageWidth) / self.size.width;
    mbgl::style::ImageStretches stretchX = {{
        self.capInsets.left * scale, (self.size.width - self.capInsets.right) * scale,
    }};
    mbgl::style::ImageStretches stretchY = {{
        self.capInsets.top * scale, (self.size.height - self.capInsets.bottom) * scale,
    }};
    
    mbgl::optional<mbgl::style::ImageContent> imageContent;
    if (!MGLEdgeInsetsIsZero(self.capInsets)) {
        imageContent = (mbgl::style::ImageContent){
            .left = static_cast<float>(self.capInsets.left * scale),
            .top = static_cast<float>(self.capInsets.top * scale),
            .right = static_cast<float>((self.size.width - self.capInsets.right) * scale),
            .bottom = static_cast<float>((self.size.height - self.capInsets.bottom) * scale),
        };
    }
    
    return std::make_unique<mbgl::style::Image>([identifier UTF8String],
                                                std::move(cPremultipliedImage),
                                                scale,
                                                [self isTemplate],
                                                stretchX, stretchY,
                                                imageContent);
}

- (mbgl::PremultipliedImage)mgl_premultipliedImage {
    CGImageRef ref = [self CGImageForProposedRect:nullptr context:nullptr hints:nullptr];
    return MGLPremultipliedImageFromCGImage(ref);
}

@end
