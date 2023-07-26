#import "MLNShape.h"

#import <mbgl/util/geojson.hpp>
#import <mbgl/util/geometry.hpp>
#import <mbgl/util/geo.hpp>

bool operator==(const CLLocationCoordinate2D lhs, const CLLocationCoordinate2D rhs);

@interface MLNShape (Private)

/**
 Returns an `mbgl::GeoJSON` representation of the `MLNShape`.
 */
- (mbgl::GeoJSON)geoJSONObject;

/**
 Returns an `mbgl::Geometry<double>` representation of the `MLNShape`.
 */
- (mbgl::Geometry<double>)geometryObject;

/**
 Returns a dictionary with the GeoJSON geometry member object.
 */
- (NSDictionary *)geoJSONDictionary;

@end
