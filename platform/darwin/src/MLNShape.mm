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

static id MLNGeoJSONObjectByReplacingEmptyCoordinates(id object, BOOL *changed) {
  if ([object isKindOfClass:[NSArray class]]) {
    NSArray *array = object;
    NSMutableArray *updatedArray = nil;
    for (NSUInteger index = 0; index < array.count; index++) {
      id updatedObject = MLNGeoJSONObjectByReplacingEmptyCoordinates(array[index], changed);
      if (updatedObject != array[index]) {
        if (!updatedArray) {
          updatedArray = [array mutableCopy];
        }
        updatedArray[index] = updatedObject;
      }
    }
    return updatedArray ?: object;
  }

  if (![object isKindOfClass:[NSDictionary class]]) {
    return object;
  }

  NSDictionary *dictionary = object;
  NSString *type = dictionary[@"type"];
  if (![type isKindOfClass:[NSString class]]) {
    return object;
  }
  id coordinates = dictionary[@"coordinates"];
  if (MLNIsGeoJSONGeometryType(type) && [coordinates isKindOfClass:[NSArray class]] &&
      [coordinates count] == 0) {
    *changed = YES;
    return [NSNull null];
  }

  NSString *childKey = nil;
  if ([type isEqualToString:@"Feature"]) {
    childKey = @"geometry";
  } else if ([type isEqualToString:@"FeatureCollection"]) {
    childKey = @"features";
  } else if ([type isEqualToString:@"GeometryCollection"]) {
    childKey = @"geometries";
  }

  if (!childKey) {
    return object;
  }

  id child = dictionary[childKey];
  if (!child) {
    return object;
  }

  id updatedChild = MLNGeoJSONObjectByReplacingEmptyCoordinates(child, changed);
  if (updatedChild == child) {
    return object;
  }

  NSMutableDictionary *updatedDictionary = [dictionary mutableCopy];
  updatedDictionary[childKey] = updatedChild;
  return updatedDictionary;
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

  try {
    const auto geojson = mapbox::geojson::parse(string.UTF8String);
    return MLNShapeFromGeoJSON(geojson);
  } catch (std::runtime_error &err) {
    NSString *failureReason = @(err.what());
    NSData *utf8Data = [string dataUsingEncoding:NSUTF8StringEncoding];
    id jsonObject = [NSJSONSerialization JSONObjectWithData:utf8Data options:0 error:nil];
    BOOL changed = NO;
    id updatedJSONObject = MLNGeoJSONObjectByReplacingEmptyCoordinates(jsonObject, &changed);
    if (changed) {
      if ([updatedJSONObject isKindOfClass:[NSNull class]]) {
        updatedJSONObject = @{@"type" : @"GeometryCollection", @"geometries" : @[]};
      }
      NSData *updatedData = [NSJSONSerialization dataWithJSONObject:updatedJSONObject
                                                            options:NSJSONWritingFragmentsAllowed
                                                              error:nil];
      NSString *updatedString = [[NSString alloc] initWithData:updatedData
                                                      encoding:NSUTF8StringEncoding];
      try {
        const auto geojson = mapbox::geojson::parse(updatedString.UTF8String);
        return MLNShapeFromGeoJSON(geojson);
      } catch (std::runtime_error &updatedError) {
        failureReason = @(updatedError.what());
      }
    }
    if (outError) {
      *outError = [NSError errorWithDomain:MLNErrorDomain
                                      code:MLNErrorCodeUnknown
                                  userInfo:@{
                                    NSLocalizedFailureReasonErrorKey : failureReason,
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
