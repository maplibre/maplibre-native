#import "MLNFoundation.h"
#import "MLNFeature.h"
#import "MLNShape.h"

#import <mbgl/util/geo.hpp>
#import <mbgl/util/feature.hpp>
#import <mbgl/style/conversion/geojson.hpp>

NS_ASSUME_NONNULL_BEGIN

/**
 Returns an array of `MLNFeature` objects converted from the given vector of
 vector tile features.
 */
MLN_EXPORT
NSArray<MLNShape <MLNFeature> *> *MLNFeaturesFromMBGLFeatures(const std::vector<mbgl::Feature> &features);

/**
 Returns an array of `MLNFeature` objects converted from the given vector of
 vector tile features.
 */
MLN_EXPORT
NSArray<MLNShape <MLNFeature> *> *MLNFeaturesFromMBGLFeatures(const std::vector<mbgl::GeoJSONFeature> &features);

/**
 Returns an `MLNFeature` object converted from the given mbgl::GeoJSONFeature
 */
MLN_EXPORT
id <MLNFeature> MLNFeatureFromMBGLFeature(const mbgl::GeoJSONFeature &feature);

/**
 Returns an `MLNShape` representing the given geojson. The shape can be
 a feature, a collection of features, or a geometry.
 */
MLNShape* MLNShapeFromGeoJSON(const mapbox::geojson::geojson &geojson);

/**
 Takes an `mbgl::GeoJSONFeature` object, an identifer, and attributes dictionary and
 returns the feature object with converted `mbgl::FeatureIdentifier` and
 `mbgl::PropertyMap` properties.
 */
mbgl::GeoJSONFeature mbglFeature(mbgl::GeoJSONFeature feature, id identifier, NSDictionary * attributes);

/**
 Returns an `NSDictionary` representation of an `MLNFeature`.
 */
NSDictionary<NSString *, id> *NSDictionaryFeatureForGeometry(NSDictionary *geometry, NSDictionary *attributes, id identifier);

NS_ASSUME_NONNULL_END

#define MLN_DEFINE_FEATURE_INIT_WITH_CODER() \
    - (instancetype)initWithCoder:(NSCoder *)decoder { \
        if (self = [super initWithCoder:decoder]) { \
            NSSet<Class> *identifierClasses = [NSSet setWithArray:@[[NSString class], [NSNumber class]]]; \
            identifier = [decoder decodeObjectOfClasses:identifierClasses forKey:@"identifier"]; \
            _attributes = [decoder decodeObjectOfClass:[NSDictionary class] forKey:@"attributes"]; \
        } \
        return self; \
    }

#define MLN_DEFINE_FEATURE_ENCODE() \
    - (void)encodeWithCoder:(NSCoder *)coder { \
        [super encodeWithCoder:coder]; \
        [coder encodeObject:identifier forKey:@"identifier"]; \
        [coder encodeObject:_attributes forKey:@"attributes"]; \
    }

#define MLN_DEFINE_FEATURE_IS_EQUAL() \
    - (BOOL)isEqual:(id)other { \
        if (other == self) return YES; \
        if (![other isKindOfClass:[self class]]) return NO; \
        __typeof(self) otherFeature = other; \
        return [super isEqual:other] && [self geoJSONObject] == [otherFeature geoJSONObject]; \
    } \
    - (NSUInteger)hash { \
        return [super hash] + [[self geoJSONDictionary] hash]; \
    }

#define MLN_DEFINE_FEATURE_ATTRIBUTES_GETTER() \
    - (NSDictionary *) attributes { \
        if (!_attributes) { \
            return @{}; \
        } \
        return _attributes; \
    }
