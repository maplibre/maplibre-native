#import "UIImage+MLNAdditions.h"
#import "NSBundle+MLNAdditions.h"

#include <mbgl/util/image+MLNAdditions.hpp>

const MLNExceptionName MLNResourceNotFoundException = @"MLNResourceNotFoundException";

BOOL MLNEdgeInsetsIsZero(UIEdgeInsets edgeInsets) {
    return edgeInsets.left == 0 && edgeInsets.top == 0 && edgeInsets.right == 0 && edgeInsets.bottom == 0;
}

@implementation UIImage (MLNAdditions)

- (nullable instancetype)initWithMLNStyleImage:(const mbgl::style::Image &)styleImage
{
    CGImageRef image = CGImageCreateWithMLNPremultipliedImage(styleImage.getImage().clone());
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
            UIImageResizingMode resizingMode = UIImageResizingModeTile;
            if (!styleImage.getStretchX().empty() || !styleImage.getStretchY().empty())
            {
                resizingMode = UIImageResizingModeStretch;
            }
            self = [self resizableImageWithCapInsets:capInsets resizingMode:resizingMode];
        }
        if (!styleImage.getStretchX().empty() || !styleImage.getStretchY().empty())
        {
            self = [self resizableImageWithCapInsets:self.capInsets resizingMode:UIImageResizingModeStretch];
        }
    }
    CGImageRelease(image);
    return self;
}

- (nullable instancetype)initWithMLNPremultipliedImage:(const mbgl::PremultipliedImage&&)mbglImage scale:(CGFloat)scale
{
    CGImageRef image = CGImageCreateWithMLNPremultipliedImage(mbglImage.clone());
    if (!image) {
        return nil;
    }

    self = [self initWithCGImage:image scale:scale orientation:UIImageOrientationUp];
    
    CGImageRelease(image);
    return self;
}

- (std::unique_ptr<mbgl::style::Image>)mgl_styleImageWithIdentifier:(NSString *)identifier {
    mbgl::style::ImageStretches stretchX, stretchY;
    if (self.resizingMode == UIImageResizingModeStretch)
    {
        stretchX.push_back({
            self.capInsets.left * self.scale, (self.size.width - self.capInsets.right) * self.scale,
        });
        stretchY.push_back({
            self.capInsets.top * self.scale, (self.size.height - self.capInsets.bottom) * self.scale,
        });
    }
    
    std::optional<mbgl::style::ImageContent> imageContent;
    if (!MLNEdgeInsetsIsZero(self.capInsets))
    {
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
    return MLNPremultipliedImageFromCGImage(self.CGImage);
}

+ (UIImage *)mgl_resourceImageNamed:(NSString *)imageName {
    UIImage *image = [UIImage imageNamed:imageName
                                inBundle:[NSBundle mgl_frameworkBundle]
           compatibleWithTraitCollection:nil];

    if (!image) {
        [NSException raise:MLNResourceNotFoundException format:@"The resource named “%@” could not be found in the Mapbox framework bundle.", imageName];
    }

    return image;
}

+(UIImage *)NormalizedImage:(UIImage *)sourceImage {
    CGSize scaledSize = CGSizeMake(sourceImage.size.width * sourceImage.scale, sourceImage.size.height * sourceImage.scale);
    UIGraphicsBeginImageContextWithOptions(scaledSize, NO, UIScreen.mainScreen.scale);
    [sourceImage drawInRect:(CGRect){ .origin = CGPointZero, .size = scaledSize }];
    UIImage *normalizedImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return normalizedImage;
}


- (BOOL)isDataEqualTo:(UIImage*)otherImage {
    NSData *leftData = UIImagePNGRepresentation([UIImage NormalizedImage:self]);
    NSData *rightData = UIImagePNGRepresentation([UIImage NormalizedImage:otherImage]);
    return [leftData isEqualToData:rightData];
}

@end
