#import "NSImage+MLNAdditions.h"

#include <mbgl/util/image+MLNAdditions.hpp>

BOOL MLNEdgeInsetsIsZero(NSEdgeInsets edgeInsets) {
    return edgeInsets.left == 0 && edgeInsets.top == 0 && edgeInsets.right == 0 && edgeInsets.bottom == 0;
}

@implementation NSImage (MLNAdditions)

- (nullable instancetype)initWithMLNPremultipliedImage:(mbgl::PremultipliedImage&&)src {
    CGImageRef image = CGImageCreateWithMLNPremultipliedImage(std::move(src));
    if (!image) {
        return nil;
    }

    self = [self initWithCGImage:image size:NSZeroSize];
    CGImageRelease(image);
    return self;
}

- (nullable instancetype)initWithMLNStyleImage:(const mbgl::style::Image &)styleImage {
    CGImageRef image = CGImageCreateWithMLNPremultipliedImage(styleImage.getImage().clone());
    if (!image) {
        return nil;
    }

    NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithCGImage:image];
    CGImageRelease(image);
    CGFloat scale = styleImage.getPixelRatio();
    NSSize size = NSMakeSize(styleImage.getImage().size.width / scale,
                             styleImage.getImage().size.height / scale);
    if (self = [self initWithSize:size]) {
        [self addRepresentation:rep];
        [self setTemplate:styleImage.isSdf()];
        if (!styleImage.getStretchX().empty() || !styleImage.getStretchY().empty()) {
            self.resizingMode = NSImageResizingModeStretch;
        } else {
            self.resizingMode = NSImageResizingModeTile;
        }
        if (auto content = styleImage.getContent()) {
            self.capInsets = NSEdgeInsetsMake(content->top / scale,
                                              content->left / scale,
                                              size.height - content->bottom / scale,
                                              size.width - content->right / scale);
        }
    }
    return self;
}

- (std::unique_ptr<mbgl::style::Image>)mgl_styleImageWithIdentifier:(NSString *)identifier {
    mbgl::PremultipliedImage cPremultipliedImage = self.mgl_premultipliedImage;
    auto imageWidth = cPremultipliedImage.size.width;
    
    float scale = static_cast<float>(imageWidth) / self.size.width;
    mbgl::style::ImageStretches stretchX, stretchY;
    if (self.resizingMode == NSImageResizingModeStretch) {
        stretchX.push_back({
            self.capInsets.left * scale, (self.size.width - self.capInsets.right) * scale,
        });
        stretchY.push_back({
            self.capInsets.top * scale, (self.size.height - self.capInsets.bottom) * scale,
        });
    }
    
    std::optional<mbgl::style::ImageContent> imageContent;
    if (!MLNEdgeInsetsIsZero(self.capInsets)) {
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
    return MLNPremultipliedImageFromCGImage(ref);
}

@end
