#import "UIImage+MGLAdditions.h"
#import "NSBundle+MGLAdditions.h"

#include <mbgl/util/image+MGLAdditions.hpp>

const MGLExceptionName MGLResourceNotFoundException = @"MGLResourceNotFoundException";

BOOL MGLEdgeInsetsIsZero(UIEdgeInsets edgeInsets) {
    return edgeInsets.left == 0 && edgeInsets.top == 0 && edgeInsets.right == 0 && edgeInsets.bottom == 0;
}

@implementation UIImage (MGLAdditions)

- (nullable instancetype)initWithMGLStyleImage:(const mbgl::style::Image &)styleImage
{
    CGImageRef image = CGImageCreateWithMGLPremultipliedImage(styleImage.getImage().clone());
    if (!image) {
        return nil;
    }

    CGFloat scale = styleImage.getPixelRatio();
    if (self = [self initWithCGImage:image scale:scale orientation:UIImageOrientationUp])
    {
        if (styleImage.isSdf())
        {
            self = [self imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
        }
        
        if (auto content = styleImage.getContent())
        {
            UIEdgeInsets capInsets = UIEdgeInsetsMake(content->top / scale,
                                                      content->left / scale,
                                                      self.size.height - content->bottom / scale,
                                                      self.size.width - content->right / scale);
            self = [self resizableImageWithCapInsets:capInsets resizingMode:UIImageResizingModeStretch];
        }
    }
    CGImageRelease(image);
    return self;
}

- (nullable instancetype)initWithMGLPremultipliedImage:(const mbgl::PremultipliedImage&&)mbglImage scale:(CGFloat)scale
{
    CGImageRef image = CGImageCreateWithMGLPremultipliedImage(mbglImage.clone());
    if (!image) {
        return nil;
    }

    self = [self initWithCGImage:image scale:scale orientation:UIImageOrientationUp];
    
    CGImageRelease(image);
    return self;
}

- (std::unique_ptr<mbgl::style::Image>)mgl_styleImageWithIdentifier:(NSString *)identifier {
    mbgl::style::ImageStretches stretchX = {{
        self.capInsets.left / self.scale, (self.size.width - self.capInsets.right) / self.scale,
    }};
    mbgl::style::ImageStretches stretchY = {{
        self.capInsets.top / self.scale, (self.size.height - self.capInsets.bottom) / self.scale,
    }};
    
    mbgl::optional<mbgl::style::ImageContent> imageContent;
    if (!MGLEdgeInsetsIsZero(self.capInsets)) {
        imageContent = (mbgl::style::ImageContent){
            .left = static_cast<float>(self.capInsets.left * self.scale),
            .top = static_cast<float>(self.capInsets.top * self.scale),
            .right = static_cast<float>((self.size.width - self.capInsets.right) * self.scale),
            .bottom = static_cast<float>((self.size.height - self.capInsets.bottom) * self.scale),
        };
    }
    
    BOOL isTemplate = self.renderingMode == UIImageRenderingModeAlwaysTemplate;
    return std::make_unique<mbgl::style::Image>([identifier UTF8String],
                                                self.mgl_premultipliedImage,
                                                static_cast<float>(self.scale),
                                                isTemplate,
                                                stretchX, stretchY,
                                                imageContent);
}

- (mbgl::PremultipliedImage)mgl_premultipliedImage {
    return MGLPremultipliedImageFromCGImage(self.CGImage);
}

+ (UIImage *)mgl_resourceImageNamed:(NSString *)imageName {
    UIImage *image = [UIImage imageNamed:imageName
                                inBundle:[NSBundle mgl_frameworkBundle]
           compatibleWithTraitCollection:nil];

    if (!image) {
        [NSException raise:MGLResourceNotFoundException format:@"The resource named “%@” could not be found in the Mapbox framework bundle.", imageName];
    }

    return image;
}

@end
