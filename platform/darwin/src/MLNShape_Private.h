#import "MLNShape.h"

#import <mln/util/geo.hpp>
#import <mln/util/geojson.hpp>
#import <mln/util/geometry.hpp>

bool operator==(const CLLocationCoordinate2D lhs, const CLLocationCoordinate2D rhs);

@interface MLNShape (Private)

/**
 Returns an `mln::GeoJSON` representation of the ``MLNShape``.
 */
- (mln::GeoJSON)geoJSONObject;

/**
 Returns an `mln::Geometry<double>` representation of the ``MLNShape``.
 */
- (mln::Geometry<double>)geometryObject;

/**
 Returns a dictionary with the GeoJSON geometry member object.
 */
- (NSDictionary *)geoJSONDictionary;

@end
