#import "MLNShapeOfflineRegion.h"

#if !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
    #import <Cocoa/Cocoa.h>
#else
    #import <UIKit/UIKit.h>
#endif

#import "MLNOfflineRegion_Private.h"
#import "MLNShapeOfflineRegion_Private.h"
#import "MLNFeature_Private.h"
#import "MLNShape_Private.h"
#import "MLNStyle.h"
#import "MLNLoggingConfiguration_Private.h"

@interface MLNShapeOfflineRegion () <MLNOfflineRegion_Private, MLNShapeOfflineRegion_Private>

@end

@implementation MLNShapeOfflineRegion {
    NSURL *_styleURL;
}

@synthesize styleURL = _styleURL;
@synthesize includesIdeographicGlyphs = _includesIdeographicGlyphs;

-(NSDictionary *)offlineStartEventAttributes {
    return @{};
}

+ (BOOL)supportsSecureCoding {
    return YES;
}

- (instancetype)init {
    MLNLogInfo(@"Calling this initializer is not allowed.");
    [NSException raise:@"Method unavailable"
                format:
     @"-[MLNShapeOfflineRegion init] is unavailable. "
     @"Use -initWithStyleURL:shape:fromZoomLevel:toZoomLevel: instead."];
    return nil;
}

- (instancetype)initWithStyleURL:(NSURL *)styleURL shape:(MLNShape *)shape fromZoomLevel:(double)minimumZoomLevel toZoomLevel:(double)maximumZoomLevel {
    MLNLogDebug(@"Initializing styleURL: %@ shape: %@ fromZoomLevel: %f toZoomLevel: %f", styleURL, shape, minimumZoomLevel, maximumZoomLevel);
    if (self = [super init]) {
        if (!styleURL) {
            styleURL = [MLNStyle defaultStyleURL];
        }

        if (!styleURL.scheme) {
            [NSException raise:@"Invalid style URL" format:
             @"%@ does not support setting a relative file URL as the style URL. "
             @"To download the online resources required by this style, "
             @"specify a URL to an online copy of this style. "
             @"For Mapbox-hosted styles, use the mapbox: scheme.",
             NSStringFromClass([self class])];
        }

        _styleURL = styleURL;
        _shape = shape;
        _minimumZoomLevel = minimumZoomLevel;
        _maximumZoomLevel = maximumZoomLevel;
        _includesIdeographicGlyphs = NO;
    }
    return self;
}

- (instancetype)initWithOfflineRegionDefinition:(const mbgl::OfflineGeometryRegionDefinition &)definition {
    NSURL *styleURL = [NSURL URLWithString:@(definition.styleURL.c_str())];
    MLNShape *shape = MLNShapeFromGeoJSON(definition.geometry);
    MLNShapeOfflineRegion* result = [self initWithStyleURL:styleURL shape:shape fromZoomLevel:definition.minZoom toZoomLevel:definition.maxZoom];
    result.includesIdeographicGlyphs = definition.includeIdeographs;
    return result;
}

- (const mbgl::OfflineRegionDefinition)offlineRegionDefinition {
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
    const float scaleFactor = [UIScreen instancesRespondToSelector:@selector(nativeScale)] ? [[UIScreen mainScreen] nativeScale] : [[UIScreen mainScreen] scale];
#elif TARGET_OS_MAC
    const float scaleFactor = [NSScreen mainScreen].backingScaleFactor;
#endif
    return mbgl::OfflineGeometryRegionDefinition(_styleURL.absoluteString.UTF8String,
                                                 _shape.geometryObject,
                                                 _minimumZoomLevel, _maximumZoomLevel,
                                                 scaleFactor, _includesIdeographicGlyphs);
}

- (nullable instancetype)initWithCoder:(NSCoder *)coder {
    MLNLogInfo(@"Initializing with coder.");
    NSURL *styleURL = [coder decodeObjectForKey:@"styleURL"];
    MLNShape * shape = [coder decodeObjectForKey:@"shape"];
    double minimumZoomLevel = [coder decodeDoubleForKey:@"minimumZoomLevel"];
    double maximumZoomLevel = [coder decodeDoubleForKey:@"maximumZoomLevel"];

    MLNShapeOfflineRegion* result = [self initWithStyleURL:styleURL shape:shape fromZoomLevel:minimumZoomLevel toZoomLevel:maximumZoomLevel];
    result.includesIdeographicGlyphs = [coder decodeBoolForKey:@"includesIdeographicGlyphs"];
    return result;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_styleURL forKey:@"styleURL"];
    [coder encodeObject:_shape forKey:@"shape"];
    [coder encodeDouble:_maximumZoomLevel forKey:@"maximumZoomLevel"];
    [coder encodeDouble:_minimumZoomLevel forKey:@"minimumZoomLevel"];
    [coder encodeBool:_includesIdeographicGlyphs forKey:@"includesIdeographicGlyphs"];
}

- (id)copyWithZone:(nullable NSZone *)zone {
    MLNShapeOfflineRegion* result = [[[self class] allocWithZone:zone] initWithStyleURL:_styleURL shape:_shape fromZoomLevel:_minimumZoomLevel toZoomLevel:_maximumZoomLevel];
    result.includesIdeographicGlyphs = _includesIdeographicGlyphs;
    return result;
}

- (BOOL)isEqual:(id)other {
    if (other == self) {
        return YES;
    }
    if (![other isKindOfClass:[self class]]) {
        return NO;
    }

    MLNShapeOfflineRegion *otherRegion = other;
    return (_minimumZoomLevel == otherRegion->_minimumZoomLevel
            && _maximumZoomLevel == otherRegion->_maximumZoomLevel
            && _shape.geometryObject == otherRegion->_shape.geometryObject
            && [_styleURL isEqual:otherRegion->_styleURL]
            && _includesIdeographicGlyphs == otherRegion->_includesIdeographicGlyphs);
}

- (NSUInteger)hash {
    return (_styleURL.hash
            + _shape.hash
            + @(_minimumZoomLevel).hash + @(_maximumZoomLevel).hash
            + @(_includesIdeographicGlyphs).hash);
}

@end
