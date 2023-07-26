#import "MLNFoundation_Private.h"
#import "MLNFeature_Private.h"
#import "MLNCluster.h"

#import "MLNPointAnnotation.h"
#import "MLNPolyline.h"
#import "MLNPolygon.h"
#import "MLNValueEvaluator.h"

#import "MLNShape_Private.h"
#import "MLNPointCollection_Private.h"
#import "MLNPolyline_Private.h"
#import "MLNPolygon_Private.h"

#import "NSDictionary+MLNAdditions.h"
#import "NSArray+MLNAdditions.h"
#import "NSExpression+MLNPrivateAdditions.h"
#import "MLNLoggingConfiguration_Private.h"

#import <mbgl/util/geometry.hpp>
#import <mbgl/style/conversion/geojson.hpp>
#import <mapbox/feature.hpp>

// Cluster constants
static NSString * const MLNClusterIdentifierKey = @"cluster_id";
static NSString * const MLNClusterCountKey = @"point_count";
const NSUInteger MLNClusterIdentifierInvalid = NSUIntegerMax;

@interface MLNEmptyFeature ()
@end

@implementation MLNEmptyFeature

@synthesize identifier;
@synthesize attributes = _attributes;

MLN_DEFINE_FEATURE_INIT_WITH_CODER();
MLN_DEFINE_FEATURE_ENCODE();
MLN_DEFINE_FEATURE_IS_EQUAL();
MLN_DEFINE_FEATURE_ATTRIBUTES_GETTER();

- (id)attributeForKey:(NSString *)key {
    MLNLogDebug(@"Retrieving attributeForKey: %@", key);
    return self.attributes[key];
}

- (NSDictionary *)geoJSONDictionary {
    return NSDictionaryFeatureForGeometry([super geoJSONDictionary], self.attributes, self.identifier);
}

- (mbgl::GeoJSON)geoJSONObject {
    return mbglFeature({[self geometryObject]}, identifier, self.attributes);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; identifier = %@, attributes = %@>",
            NSStringFromClass([self class]), (void *)self,
            self.identifier ? [NSString stringWithFormat:@"\"%@\"", self.identifier] : self.identifier,
            self.attributes.count ? self.attributes : @"none"];
}

@end

@interface MLNPointFeature ()
@end

@implementation MLNPointFeature

@synthesize identifier;
@synthesize attributes = _attributes;

MLN_DEFINE_FEATURE_INIT_WITH_CODER();
MLN_DEFINE_FEATURE_ENCODE();
MLN_DEFINE_FEATURE_IS_EQUAL();
MLN_DEFINE_FEATURE_ATTRIBUTES_GETTER();

- (id)attributeForKey:(NSString *)key {
    MLNLogDebug(@"Retrieving attributeForKey: %@", key);
    return self.attributes[key];
}

- (NSDictionary *)geoJSONDictionary {
    return NSDictionaryFeatureForGeometry([super geoJSONDictionary], self.attributes, self.identifier);
}

- (mbgl::GeoJSON)geoJSONObject {
    return mbglFeature({[self geometryObject]}, identifier, self.attributes);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; identifier = %@, coordinate = %f, %f, attributes = %@>",
            NSStringFromClass([self class]), (void *)self,
            self.identifier ? [NSString stringWithFormat:@"\"%@\"", self.identifier] : self.identifier,
            self.coordinate.latitude, self.coordinate.longitude,
            self.attributes.count ? self.attributes : @"none"];
}

@end

@implementation MLNPointFeatureCluster

- (NSUInteger)clusterIdentifier {
    NSNumber *clusterNumber = MLN_OBJC_DYNAMIC_CAST([self attributeForKey:MLNClusterIdentifierKey], NSNumber);
    MLNAssert(clusterNumber, @"Clusters should have a cluster_id");
    
    if (!clusterNumber) {
        return MLNClusterIdentifierInvalid;
    }
    
    NSUInteger clusterIdentifier = [clusterNumber unsignedIntegerValue];
    MLNAssert(clusterIdentifier <= UINT32_MAX, @"Cluster identifiers are 32bit");
    
    return clusterIdentifier;
}

- (NSUInteger)clusterPointCount {
    NSNumber *count = MLN_OBJC_DYNAMIC_CAST([self attributeForKey:MLNClusterCountKey], NSNumber);
    MLNAssert(count, @"Clusters should have a point_count");
    
    return [count unsignedIntegerValue];
}
@end


@interface MLNPolylineFeature ()
@end

@implementation MLNPolylineFeature

@synthesize identifier;
@synthesize attributes = _attributes;

MLN_DEFINE_FEATURE_INIT_WITH_CODER();
MLN_DEFINE_FEATURE_ENCODE();
MLN_DEFINE_FEATURE_IS_EQUAL();
MLN_DEFINE_FEATURE_ATTRIBUTES_GETTER();

- (id)attributeForKey:(NSString *)key {
    MLNLogDebug(@"Retrieving attributeForKey: %@", key);
    return self.attributes[key];
}

- (NSDictionary *)geoJSONDictionary {
    return NSDictionaryFeatureForGeometry([super geoJSONDictionary], self.attributes, self.identifier);
}

- (mbgl::GeoJSON)geoJSONObject {
    return mbglFeature({[self geometryObject]}, identifier, self.attributes);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; identifier = %@, count = %lu, bounds = %@, attributes = %@>",
            NSStringFromClass([self class]), (void *)self,
            self.identifier ? [NSString stringWithFormat:@"\"%@\"", self.identifier] : self.identifier,
            (unsigned long)[self pointCount],
            MLNStringFromCoordinateBounds(self.overlayBounds),
            self.attributes.count ? self.attributes : @"none"];
}

@end

@interface MLNPolygonFeature ()
@end

@implementation MLNPolygonFeature

@synthesize identifier;
@synthesize attributes = _attributes;

MLN_DEFINE_FEATURE_INIT_WITH_CODER();
MLN_DEFINE_FEATURE_ENCODE();
MLN_DEFINE_FEATURE_IS_EQUAL();
MLN_DEFINE_FEATURE_ATTRIBUTES_GETTER();

- (id)attributeForKey:(NSString *)key {
    MLNLogDebug(@"Retrieving attributeForKey: %@", key);
    return self.attributes[key];
}

- (NSDictionary *)geoJSONDictionary {
    return NSDictionaryFeatureForGeometry([super geoJSONDictionary], self.attributes, self.identifier);
}

- (mbgl::GeoJSON)geoJSONObject {
    return mbglFeature({[self geometryObject]}, identifier, self.attributes);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; identifier = %@, count = %lu, bounds = %@, attributes = %@>",
            NSStringFromClass([self class]), (void *)self,
            self.identifier ? [NSString stringWithFormat:@"\"%@\"", self.identifier] : self.identifier,
            (unsigned long)[self pointCount],
            MLNStringFromCoordinateBounds(self.overlayBounds),
            self.attributes.count ? self.attributes : @"none"];
}

@end

@interface MLNPointCollectionFeature ()
@end

@implementation MLNPointCollectionFeature

@synthesize identifier;
@synthesize attributes = _attributes;

MLN_DEFINE_FEATURE_INIT_WITH_CODER();
MLN_DEFINE_FEATURE_ENCODE();
MLN_DEFINE_FEATURE_IS_EQUAL();
MLN_DEFINE_FEATURE_ATTRIBUTES_GETTER();

- (id)attributeForKey:(NSString *)key {
    MLNLogDebug(@"Retrieving attributeForKey: %@", key);
    return self.attributes[key];
}

- (NSDictionary *)geoJSONDictionary {
    return NSDictionaryFeatureForGeometry([super geoJSONDictionary], self.attributes, self.identifier);
}

- (mbgl::GeoJSON)geoJSONObject {
    return mbglFeature({[self geometryObject]}, identifier, self.attributes);
}

@end

@interface MLNMultiPolylineFeature ()
@end

@implementation MLNMultiPolylineFeature

@synthesize identifier;
@synthesize attributes = _attributes;

MLN_DEFINE_FEATURE_INIT_WITH_CODER();
MLN_DEFINE_FEATURE_ENCODE();
MLN_DEFINE_FEATURE_IS_EQUAL();
MLN_DEFINE_FEATURE_ATTRIBUTES_GETTER();

- (id)attributeForKey:(NSString *)key {
    MLNLogDebug(@"Retrieving attributeForKey: %@", key);
    return self.attributes[key];
}

- (NSDictionary *)geoJSONDictionary {
    return NSDictionaryFeatureForGeometry([super geoJSONDictionary], self.attributes, self.identifier);
}

- (mbgl::GeoJSON)geoJSONObject {
    return mbglFeature({[self geometryObject]}, identifier, self.attributes);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; identifier = %@, count = %lu, bounds = %@, attributes = %@>",
            NSStringFromClass([self class]), (void *)self,
            self.identifier ? [NSString stringWithFormat:@"\"%@\"", self.identifier] : self.identifier,
            (unsigned long)self.polylines.count,
            MLNStringFromCoordinateBounds(self.overlayBounds),
            self.attributes.count ? self.attributes : @"none"];
}

@end

@interface MLNMultiPolygonFeature ()
@end

@implementation MLNMultiPolygonFeature

@synthesize identifier;
@synthesize attributes = _attributes;

MLN_DEFINE_FEATURE_INIT_WITH_CODER();
MLN_DEFINE_FEATURE_ENCODE();
MLN_DEFINE_FEATURE_IS_EQUAL();
MLN_DEFINE_FEATURE_ATTRIBUTES_GETTER();

- (id)attributeForKey:(NSString *)key {
    MLNLogDebug(@"Retrieving attributeForKey: %@", key);
    return self.attributes[key];
}

- (NSDictionary *)geoJSONDictionary {
    return NSDictionaryFeatureForGeometry([super geoJSONDictionary], self.attributes, self.identifier);
}

- (mbgl::GeoJSON)geoJSONObject {
    return mbglFeature({[self geometryObject]}, identifier, self.attributes);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; identifier = %@, count = %lu, bounds = %@, attributes = %@>",
            NSStringFromClass([self class]), (void *)self,
            self.identifier ? [NSString stringWithFormat:@"\"%@\"", self.identifier] : self.identifier,
            (unsigned long)self.polygons.count,
            MLNStringFromCoordinateBounds(self.overlayBounds),
            self.attributes.count ? self.attributes : @"none"];
}

@end

@interface MLNShapeCollectionFeature ()
@end

@implementation MLNShapeCollectionFeature

@synthesize identifier;
@synthesize attributes = _attributes;

@dynamic shapes;

+ (instancetype)shapeCollectionWithShapes:(NSArray<MLNShape<MLNFeature> *> *)shapes {
    return [super shapeCollectionWithShapes:shapes];
}

MLN_DEFINE_FEATURE_INIT_WITH_CODER();
MLN_DEFINE_FEATURE_ENCODE();
MLN_DEFINE_FEATURE_IS_EQUAL();
MLN_DEFINE_FEATURE_ATTRIBUTES_GETTER();

- (id)attributeForKey:(NSString *)key {
    return self.attributes[key];
}

- (NSDictionary *)geoJSONDictionary {
    return NSDictionaryFeatureForGeometry([super geoJSONDictionary], self.attributes, self.identifier);
}

- (mbgl::GeoJSON)geoJSONObject {
    mbgl::FeatureCollection featureCollection;
    featureCollection.reserve(self.shapes.count);
    for (MLNShape <MLNFeature> *feature in self.shapes) {
        auto geoJSONObject = feature.geoJSONObject;
        MLNAssert(geoJSONObject.is<mbgl::GeoJSONFeature>(), @"Feature collection must only contain features.");
        featureCollection.push_back(geoJSONObject.get<mbgl::GeoJSONFeature>());
    }
    return featureCollection;
}

@end

/**
 Transforms an `mbgl::geometry::geometry` type into an instance of the
 corresponding Objective-C geometry class.
 */
template <typename T>
class GeometryEvaluator {
private:
    const mbgl::PropertyMap *shared_properties;
    const bool is_in_feature;
    
public:
    GeometryEvaluator(const mbgl::PropertyMap *properties = nullptr, const bool isInFeature = false):
        shared_properties(properties),
        is_in_feature(isInFeature)
    {}
    
    MLNShape * operator()(const mbgl::EmptyGeometry &) const {
        return is_in_feature ? [[MLNEmptyFeature alloc] init] : [[MLNShape alloc] init];
    }

    MLNShape * operator()(const mbgl::Point<T> &geometry) const {
        Class shapeClass = is_in_feature ? [MLNPointFeature class] : [MLNPointAnnotation class];
        
        // If we're dealing with a cluster, we should change the class type.
        // This could be generic and build the subclass at runtime if it turns
        // out we need to support more than point clusters.
        if (shared_properties) {
            auto clusterIt = shared_properties->find("cluster");
            if (clusterIt != shared_properties->end()) {
                auto clusterValue = clusterIt->second;
                if (clusterValue.template is<bool>()) {
                    if (clusterValue.template get<bool>()) {
                        shapeClass = [MLNPointFeatureCluster class];
                    }
                }
            }
        }
        
        MLNPointAnnotation *shape = [[shapeClass alloc] init];
        shape.coordinate = toLocationCoordinate2D(geometry);
        return shape;
    }

    MLNShape * operator()(const mbgl::LineString<T> &geometry) const {
        std::vector<CLLocationCoordinate2D> coordinates = toLocationCoordinates2D(geometry);
        Class shapeClass = is_in_feature ? [MLNPolylineFeature class] : [MLNPolyline class];
        return [shapeClass polylineWithCoordinates:&coordinates[0] count:coordinates.size()];
    }

    MLNShape * operator()(const mbgl::Polygon<T> &geometry) const {
        return toShape<MLNPolygon, MLNPolygonFeature>(geometry, is_in_feature);
    }

    MLNShape * operator()(const mbgl::MultiPoint<T> &geometry) const {
        std::vector<CLLocationCoordinate2D> coordinates = toLocationCoordinates2D(geometry);
        Class shapeClass = is_in_feature ? [MLNPointCollectionFeature class] : [MLNPointCollection class];
        return [[shapeClass alloc] initWithCoordinates:&coordinates[0] count:coordinates.size()];
    }

    MLNShape * operator()(const mbgl::MultiLineString<T> &geometry) const {
        NSMutableArray *polylines = [NSMutableArray arrayWithCapacity:geometry.size()];
        for (auto &lineString : geometry) {
            std::vector<CLLocationCoordinate2D> coordinates = toLocationCoordinates2D(lineString);
            MLNPolyline *polyline = [MLNPolyline polylineWithCoordinates:&coordinates[0] count:coordinates.size()];
            [polylines addObject:polyline];
        }

        Class shapeClass = is_in_feature ? [MLNMultiPolylineFeature class] : [MLNMultiPolyline class];
        return [shapeClass multiPolylineWithPolylines:polylines];
    }

    MLNShape * operator()(const mbgl::MultiPolygon<T> &geometry) const {
        NSMutableArray *polygons = [NSMutableArray arrayWithCapacity:geometry.size()];
        for (auto &polygon : geometry) {
            [polygons addObject:toShape(polygon, false)];
        }

        Class shapeClass = is_in_feature ? [MLNMultiPolygonFeature class] : [MLNMultiPolygon class];
        return [shapeClass multiPolygonWithPolygons:polygons];
    }

    MLNShape * operator()(const mapbox::geometry::geometry_collection<T> &collection) const {
        NSMutableArray *shapes = [NSMutableArray arrayWithCapacity:collection.size()];
        for (auto &geometry : collection) {
            // This is very much like the transformation that happens in MLNFeaturesFromMBGLFeatures(), but these are raw geometries with no associated feature IDs or attributes.
            MLNShape *shape = mapbox::geometry::geometry<T>::visit(geometry, *this);
            [shapes addObject:shape];
        }
        Class shapeClass = is_in_feature ? [MLNShapeCollectionFeature class] : [MLNShapeCollection class];
        return [shapeClass shapeCollectionWithShapes:shapes];
    }

private:
    static CLLocationCoordinate2D toLocationCoordinate2D(const mbgl::Point<T> &point) {
        return CLLocationCoordinate2DMake(point.y, point.x);
    }

    static std::vector<CLLocationCoordinate2D> toLocationCoordinates2D(const std::vector<mbgl::Point<T>> &points) {
        std::vector<CLLocationCoordinate2D> coordinates;
        coordinates.reserve(points.size());
        std::transform(points.begin(), points.end(), std::back_inserter(coordinates), toLocationCoordinate2D);
        return coordinates;
    }

    template<typename U = MLNPolygon, typename V = MLNPolygonFeature>
    static U *toShape(const mbgl::Polygon<T> &geometry, const bool isInFeature) {
        auto &linearRing = geometry.front();
        std::vector<CLLocationCoordinate2D> coordinates = toLocationCoordinates2D(linearRing);
        NSMutableArray *innerPolygons;
        if (geometry.size() > 1) {
            innerPolygons = [NSMutableArray arrayWithCapacity:geometry.size() - 1];
            for (auto iter = geometry.begin() + 1; iter != geometry.end(); iter++) {
                auto &innerRing = *iter;
                std::vector<CLLocationCoordinate2D> innerCoordinates = toLocationCoordinates2D(innerRing);
                MLNPolygon *innerPolygon = [MLNPolygon polygonWithCoordinates:&innerCoordinates[0] count:innerCoordinates.size()];
                [innerPolygons addObject:innerPolygon];
            }
        }

        Class shapeClass = isInFeature ? [V class] : [U class];
        return [shapeClass polygonWithCoordinates:&coordinates[0] count:coordinates.size() interiorPolygons:innerPolygons];
    }
};

template <typename T>
class GeoJSONEvaluator {
public:
    MLNShape * operator()(const mbgl::Geometry<T> &geometry) const {
        GeometryEvaluator<T> evaluator;
        MLNShape *shape = mapbox::geometry::geometry<T>::visit(geometry, evaluator);
        return shape;
    }

    MLNShape <MLNFeature> * operator()(const mbgl::GeoJSONFeature &feature) const {
        MLNShape <MLNFeature> *shape = (MLNShape <MLNFeature> *)MLNFeatureFromMBGLFeature(feature);
        return shape;
    }

    MLNShape <MLNFeature> * operator()(const mbgl::FeatureCollection &collection) const {
        NSMutableArray *shapes = [NSMutableArray arrayWithCapacity:collection.size()];
        for (const auto &feature : collection) {
            [shapes addObject:MLNFeatureFromMBGLFeature(feature)];
        }
        return [MLNShapeCollectionFeature shapeCollectionWithShapes:shapes];
    }
};

NSArray<MLNShape <MLNFeature> *> *MLNFeaturesFromMBGLFeatures(const std::vector<mbgl::Feature> &features) {
    NSMutableArray *shapes = [NSMutableArray arrayWithCapacity:features.size()];
    for (const auto &feature : features) {
        [shapes addObject:MLNFeatureFromMBGLFeature(static_cast<mbgl::GeoJSONFeature>(feature))];
    }
    return shapes;
}

NSArray<MLNShape <MLNFeature> *> *MLNFeaturesFromMBGLFeatures(const std::vector<mbgl::GeoJSONFeature> &features) {
    NSMutableArray *shapes = [NSMutableArray arrayWithCapacity:features.size()];
    for (const auto &feature : features) {
        [shapes addObject:MLNFeatureFromMBGLFeature(feature)];
    }
    return shapes;
}

id <MLNFeature> MLNFeatureFromMBGLFeature(const mbgl::GeoJSONFeature &feature) {
    NSMutableDictionary *attributes = [NSMutableDictionary dictionaryWithCapacity:feature.properties.size()];
    for (auto &pair : feature.properties) {
        auto &value = pair.second;
        ValueEvaluator evaluator;
        attributes[@(pair.first.c_str())] = mbgl::Value::visit(value, evaluator);
    }
    GeometryEvaluator<double> evaluator(&feature.properties, true);
    MLNShape <MLNFeature> *shape = (MLNShape <MLNFeature> *)mapbox::geometry::geometry<double>::visit(feature.geometry, evaluator);
    if (!feature.id.is<mapbox::feature::null_value_t>()) {
        shape.identifier = mbgl::FeatureIdentifier::visit(feature.id, ValueEvaluator());
    }
    shape.attributes = attributes;

    return shape;
}

MLNShape* MLNShapeFromGeoJSON(const mapbox::geojson::geojson &geojson) {
    GeoJSONEvaluator<double> evaluator;
    MLNShape *shape = mapbox::geojson::geojson::visit(geojson, evaluator);
    return shape;
}

mbgl::GeoJSONFeature mbglFeature(mbgl::GeoJSONFeature feature, id identifier, NSDictionary *attributes)
{
    if (identifier) {
        NSExpression *identifierExpression = [NSExpression expressionForConstantValue:identifier];
        feature.id = [identifierExpression mgl_featureIdentifier];
    }
    feature.properties = [attributes mgl_propertyMap];
    return feature;
}

NSDictionary<NSString *, id> *NSDictionaryFeatureForGeometry(NSDictionary *geometry, NSDictionary *attributes, id identifier) {
    NSMutableDictionary *feature = [@{@"type": @"Feature",
                                      @"properties": attributes,
                                      @"geometry": geometry} mutableCopy];
    feature[@"id"] = identifier;
    return [feature copy];
}
