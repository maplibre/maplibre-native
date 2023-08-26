#import "MLNImageSource.h"

#import "MLNGeometry_Private.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNSource_Private.h"
#import "MLNTileSource_Private.h"
#import "NSURL+MLNAdditions.h"
#if TARGET_OS_IPHONE
    #import "UIImage+MLNAdditions.h"
#else
    #import "NSImage+MLNAdditions.h"
#endif

#include <mbgl/style/sources/image_source.hpp>
#include <mbgl/util/premultiply.hpp>

@interface MLNImageSource ()
- (instancetype)initWithIdentifier:(NSString *)identifier coordinateQuad:(MLNCoordinateQuad)coordinateQuad NS_DESIGNATED_INITIALIZER;

@property (nonatomic, readonly) mbgl::style::ImageSource *rawSource;

@end

@implementation MLNImageSource

- (instancetype)initWithIdentifier:(NSString *)identifier coordinateQuad:(MLNCoordinateQuad)coordinateQuad {

    const auto coordsArray = MLNLatLngArrayFromCoordinateQuad(coordinateQuad);
    auto source = std::make_unique<mbgl::style::ImageSource>(identifier.UTF8String, coordsArray);
    return self = [super initWithPendingSource:std::move(source)];
}


- (instancetype)initWithIdentifier:(NSString *)identifier coordinateQuad:(MLNCoordinateQuad)coordinateQuad URL:(NSURL *)url {
    self =  [self initWithIdentifier:identifier coordinateQuad: coordinateQuad];
    self.URL = url;
    return self;
}


- (instancetype)initWithIdentifier:(NSString *)identifier coordinateQuad:(MLNCoordinateQuad)coordinateQuad image:(MLNImage *)image {
    self =  [self initWithIdentifier:identifier coordinateQuad: coordinateQuad];
    self.image = image;

    return self;
}

- (NSURL *)URL {
    MLNAssertStyleSourceIsValid();
    auto url = self.rawSource->getURL();
    return url ? [NSURL URLWithString:@(url->c_str())] : nil;
}

- (void)setURL:(NSURL *)url {
    MLNAssertStyleSourceIsValid();
    if (url) {
        self.rawSource->setURL(url.mgl_URLByStandardizingScheme.absoluteString.UTF8String);
        _image = nil;
    } else {
        self.image = nullptr;
    }
}

- (void)setImage:(MLNImage *)image {
    MLNAssertStyleSourceIsValid();
    if (image != nullptr) {
        self.rawSource->setImage(image.mgl_premultipliedImage);
    } else {
        self.rawSource->setImage(mbgl::PremultipliedImage({0,0}));
    }
    _image = image;
}

- (MLNCoordinateQuad)coordinates {
    MLNAssertStyleSourceIsValid();
    return MLNCoordinateQuadFromLatLngArray(self.rawSource->getCoordinates());
}

- (void)setCoordinates: (MLNCoordinateQuad)coordinateQuad {
    MLNAssertStyleSourceIsValid();
    self.rawSource->setCoordinates(MLNLatLngArrayFromCoordinateQuad(coordinateQuad));
}

- (NSString *)description {
    if (self.rawSource) {
        return [NSString stringWithFormat:@"<%@: %p; identifier = %@; coordinates = %@; URL = %@; image = %@>",
                NSStringFromClass([self class]), (void *)self, self.identifier,
                MLNStringFromCoordinateQuad(self.coordinates),
                self.URL,
                self.image];
    }
    else {
        return [NSString stringWithFormat:@"<%@: %p; identifier = %@; coordinates = <unknown>; URL = <unknown>; image = %@>",
                NSStringFromClass([self class]), (void *)self, self.identifier, self.image];
    }
}

- (mbgl::style::ImageSource *)rawSource {
    return (mbgl::style::ImageSource *)super.rawSource;
}

- (NSString *)attributionHTMLString {
    if (!self.rawSource) {
        MLNAssert(0, @"Source with identifier `%@` was invalidated after a style change", self.identifier);
        return nil;
    }

    auto attribution = self.rawSource->getAttribution();
    return attribution ? @(attribution->c_str()) : nil;
}

@end
