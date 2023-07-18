#import "MLNShape_Private.h"

#import "MLNFeature_Private.h"

#import "NSString+MLNAdditions.h"
#import "MLNTypes.h"

#import <mbgl/util/geo.hpp>

bool operator==(const CLLocationCoordinate2D lhs, const CLLocationCoordinate2D rhs) {
    return lhs.latitude == rhs.latitude && lhs.longitude == rhs.longitude;
}

@implementation MLNShape

+ (nullable MLNShape *)shapeWithData:(NSData *)data encoding:(NSStringEncoding)encoding error:(NSError * _Nullable *)outError {
    NSString *string = [[NSString alloc] initWithData:data encoding:encoding];
    if (!string) {
        if (outError) {
            *outError = [NSError errorWithDomain:MLNErrorDomain code:MLNErrorCodeUnknown userInfo:nil];
        }
        return nil;
    }

    try {
        const auto geojson = mapbox::geojson::parse(string.UTF8String);
        return MLNShapeFromGeoJSON(geojson);
    } catch (std::runtime_error &err) {
        if (outError) {
            *outError = [NSError errorWithDomain:MLNErrorDomain code:MLNErrorCodeUnknown userInfo:@{
                NSLocalizedFailureReasonErrorKey: @(err.what()),
            }];
        }
        return nil;
    }
}

- (mbgl::GeoJSON)geoJSONObject {
    return self.geometryObject;
}

- (mbgl::Geometry<double>)geometryObject {
    [NSException raise:MLNAbstractClassException
                format:@"MLNShape is an abstract class"];
    return mbgl::Point<double>();
}

- (NSData *)geoJSONDataUsingEncoding:(NSStringEncoding)encoding {
    auto geometry = self.geoJSONObject;
    NSString *string = @(mapbox::geojson::stringify(geometry).c_str());
    return [string dataUsingEncoding:NSUTF8StringEncoding];
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if (self = [super init]) {
        _title = [coder decodeObjectOfClass:[NSString class] forKey:@"title"];
        _subtitle = [coder decodeObjectOfClass:[NSString class] forKey:@"subtitle"];
#if !TARGET_OS_IPHONE
        _toolTip = [coder decodeObjectOfClass:[NSString class] forKey:@"toolTip"];
#endif
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_title forKey:@"title"];
    [coder encodeObject:_subtitle forKey:@"subtitle"];
#if !TARGET_OS_IPHONE
    [coder encodeObject:_toolTip forKey:@"toolTip"];
#endif
}

- (BOOL)isEqual:(id)other
{
    if (other == self) { return YES; }
    id <MLNAnnotation> annotation = other;

#if TARGET_OS_IPHONE
    return ((!_title && ![annotation title]) || [_title isEqualToString:[annotation title]])
            && ((!_subtitle && ![annotation subtitle]) || [_subtitle isEqualToString:[annotation subtitle]]);
#else
    return ((!_title && ![annotation title]) || [_title isEqualToString:[annotation title]])
            && ((!_subtitle && ![annotation subtitle]) || [_subtitle isEqualToString:[annotation subtitle]])
            && ((!_toolTip && ![annotation toolTip]) || [_toolTip isEqualToString:[annotation toolTip]]);
#endif
}

- (NSUInteger)hash
{
    NSUInteger hash = _title.hash + _subtitle.hash;
#if !TARGET_OS_IPHONE
    hash += _toolTip.hash;
#endif
    return hash;
}

- (CLLocationCoordinate2D)coordinate
{
    [NSException raise:MLNAbstractClassException
                format:@"MLNShape is an abstract class"];
    return kCLLocationCoordinate2DInvalid;
}

@end
