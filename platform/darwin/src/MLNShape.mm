#import "MLNShape_Private.h"

#import "MLNFeature_Private.h"

#import "MLNTypes.h"
#import "NSString+MLNAdditions.h"

#import <mln/util/geo.hpp>

bool operator==(const CLLocationCoordinate2D lhs, const CLLocationCoordinate2D rhs) {
  return lhs.latitude == rhs.latitude && lhs.longitude == rhs.longitude;
}

static BOOL MLNIsGeoJSONGeometryType(NSString *type) {
  return [type isEqualToString:@"Point"] || [type isEqualToString:@"MultiPoint"] ||
         [type isEqualToString:@"LineString"] || [type isEqualToString:@"MultiLineString"] ||
         [type isEqualToString:@"Polygon"] || [type isEqualToString:@"MultiPolygon"];
}

static NSRegularExpression *MLNEmptyArrayRegularExpression(void) {
  static NSRegularExpression *regularExpression;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    regularExpression = [NSRegularExpression regularExpressionWithPattern:@"\\[\\s*\\]"
                                                                  options:0
                                                                    error:nil];
  });
  return regularExpression;
}

static BOOL MLNReplaceEmptyFeatureCoordinates(id object) {
  if (![object isKindOfClass:[NSMutableDictionary class]]) {
    return NO;
  }

  NSMutableDictionary *dictionary = object;
  NSString *type = dictionary[@"type"];
  if (![type isKindOfClass:[NSString class]]) {
    return NO;
  }
  if ([type isEqualToString:@"Feature"]) {
    id geometry = dictionary[@"geometry"];
    if (![geometry isKindOfClass:[NSDictionary class]]) {
      return NO;
    }

    NSString *geometryType = geometry[@"type"];
    id coordinates = geometry[@"coordinates"];
    if (![geometryType isKindOfClass:[NSString class]] || !MLNIsGeoJSONGeometryType(geometryType) ||
        ![coordinates isKindOfClass:[NSArray class]] || [coordinates count] != 0) {
      return NO;
    }

    dictionary[@"geometry"] = [NSNull null];
    return YES;
  }

  if (![type isEqualToString:@"FeatureCollection"]) {
    return NO;
  }

  NSArray *features = dictionary[@"features"];
  if (![features isKindOfClass:[NSArray class]]) {
    return NO;
  }

  BOOL changed = NO;
  for (id feature in features) {
    changed |= MLNReplaceEmptyFeatureCoordinates(feature);
  }
  return changed;
}

static NSString *MLNGeoJSONStringByReplacingEmptyFeatureCoordinates(NSString *string) {
  NSRange range = NSMakeRange(0, string.length);
  if (![MLNEmptyArrayRegularExpression() firstMatchInString:string options:0 range:range]) {
    return string;
  }

  NSData *data = [string dataUsingEncoding:NSUTF8StringEncoding];
  id jsonObject = [NSJSONSerialization JSONObjectWithData:data
                                                  options:NSJSONReadingMutableContainers
                                                    error:nil];
  if (!MLNReplaceEmptyFeatureCoordinates(jsonObject)) {
    return string;
  }

  NSData *updatedData = [NSJSONSerialization dataWithJSONObject:jsonObject options:0 error:nil];
  NSString *updatedString = [[NSString alloc] initWithData:updatedData
                                                  encoding:NSUTF8StringEncoding];
  return updatedString ?: string;
}

@implementation MLNShape

+ (nullable MLNShape *)shapeWithData:(NSData *)data
                            encoding:(NSStringEncoding)encoding
                               error:(NSError *_Nullable *)outError {
  NSString *string = [[NSString alloc] initWithData:data encoding:encoding];
  if (!string) {
    if (outError) {
      *outError = [NSError errorWithDomain:MLNErrorDomain code:MLNErrorCodeUnknown userInfo:nil];
    }
    return nil;
  }

  NSString *normalizedString = MLNGeoJSONStringByReplacingEmptyFeatureCoordinates(string);
  try {
    const auto geojson = mapbox::geojson::parse(normalizedString.UTF8String);
    return MLNShapeFromGeoJSON(geojson);
  } catch (std::runtime_error &err) {
    if (outError) {
      *outError = [NSError errorWithDomain:MLNErrorDomain
                                      code:MLNErrorCodeUnknown
                                  userInfo:@{
                                    NSLocalizedFailureReasonErrorKey : @(err.what()),
                                  }];
    }
    return nil;
  }
}

- (mln::GeoJSON)geoJSONObject {
  return self.geometryObject;
}

- (mln::Geometry<double>)geometryObject {
  [NSException raise:MLNAbstractClassException format:@"MLNShape is an abstract class"];
  return mln::Point<double>();
}

- (NSData *)geoJSONDataUsingEncoding:(NSStringEncoding)encoding {
  auto geometry = self.geoJSONObject;
  NSString *string = @(mapbox::geojson::stringify(geometry).c_str());
  return [string dataUsingEncoding:NSUTF8StringEncoding];
}

+ (BOOL)supportsSecureCoding {
  return YES;
}

- (instancetype)initWithCoder:(NSCoder *)coder {
  if (self = [super init]) {
    _title = [coder decodeObjectOfClass:[NSString class] forKey:@"title"];
    _subtitle = [coder decodeObjectOfClass:[NSString class] forKey:@"subtitle"];
#if !TARGET_OS_IPHONE
    _toolTip = [coder decodeObjectOfClass:[NSString class] forKey:@"toolTip"];
#endif
  }
  return self;
}

- (void)encodeWithCoder:(NSCoder *)coder {
  [coder encodeObject:_title forKey:@"title"];
  [coder encodeObject:_subtitle forKey:@"subtitle"];
#if !TARGET_OS_IPHONE
  [coder encodeObject:_toolTip forKey:@"toolTip"];
#endif
}

- (BOOL)isEqual:(id)other {
  if (other == self) {
    return YES;
  }
  id<MLNAnnotation> annotation = other;

#if TARGET_OS_IPHONE
  return ((!_title && ![annotation title]) || [_title isEqualToString:[annotation title]]) &&
         ((!_subtitle && ![annotation subtitle]) ||
          [_subtitle isEqualToString:[annotation subtitle]]);
#else
  return ((!_title && ![annotation title]) || [_title isEqualToString:[annotation title]]) &&
         ((!_subtitle && ![annotation subtitle]) ||
          [_subtitle isEqualToString:[annotation subtitle]]) &&
         ((!_toolTip && ![annotation toolTip]) || [_toolTip isEqualToString:[annotation toolTip]]);
#endif
}

- (NSUInteger)hash {
  NSUInteger hash = _title.hash + _subtitle.hash;
#if !TARGET_OS_IPHONE
  hash += _toolTip.hash;
#endif
  return hash;
}

- (CLLocationCoordinate2D)coordinate {
  [NSException raise:MLNAbstractClassException format:@"MLNShape is an abstract class"];
  return kCLLocationCoordinate2DInvalid;
}

@end
