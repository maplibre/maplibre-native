#import "MLNFeature.h"
#import "MLNFoundation.h"
#import "MLNShape.h"

#import <mln/style/conversion/geojson.hpp>
#import <mln/util/feature.hpp>
#import <mln/util/geo.hpp>

NS_ASSUME_NONNULL_BEGIN

/**
 Returns an array of ``MLNFeature`` objects converted from the given vector of
 vector tile features.
 */
MLN_EXPORT
NSArray<MLNShape<MLNFeature> *> *MLNFeaturesFromMBGLFeatures(
    const std::vector<mln::Feature> &features);

/**
 Returns an array of ``MLNFeature`` objects converted from the given vector of
 vector tile features.
 */
MLN_EXPORT
NSArray<MLNShape<MLNFeature> *> *MLNFeaturesFromMBGLFeatures(
    const std::vector<mln::GeoJSONFeature> &features);

/**
 Returns an ``MLNFeature`` object converted from the given mln::GeoJSONFeature
 */
MLN_EXPORT
id<MLNFeature> MLNFeatureFromMBGLFeature(const mln::GeoJSONFeature &feature);

/**
 Returns an ``MLNShape`` representing the given geojson. The shape can be
 a feature, a collection of features, or a geometry.
 */
MLNShape *MLNShapeFromGeoJSON(const mapbox::geojson::geojson &geojson);

/**
 Takes an `mln::GeoJSONFeature` object, an identifer, and attributes dictionary and
 returns the feature object with converted `mln::FeatureIdentifier` and
 `mln::PropertyMap` properties.
 */
mln::GeoJSONFeature mbglFeature(mln::GeoJSONFeature feature, id identifier,
                                NSDictionary *attributes);

/**
 Returns an `NSDictionary` representation of an ``MLNFeature``.
 */
NSDictionary<NSString *, id> *NSDictionaryFeatureForGeometry(NSDictionary *geometry,
                                                             NSDictionary *attributes,
                                                             id identifier);

NS_ASSUME_NONNULL_END

#define MLN_DEFINE_FEATURE_INIT_WITH_CODER()                                                \
  -(instancetype)initWithCoder : (NSCoder *)decoder {                                       \
    if (self = [super initWithCoder:decoder]) {                                             \
      NSSet<Class> *identifierClasses =                                                     \
          [NSSet setWithArray:@[ [NSString class], [NSNumber class] ]];                     \
      identifier = [decoder decodeObjectOfClasses:identifierClasses forKey:@"identifier"];  \
      NSSet<Class> *attributesClasses =                                                     \
          [NSSet setWithArray:@[ [NSDictionary class], [NSArray class] ]];                  \
      _attributes = [decoder decodeObjectOfClasses:attributesClasses forKey:@"attributes"]; \
    }                                                                                       \
    return self;                                                                            \
  }                                                                                         \
  +(BOOL)supportsSecureCoding {                                                             \
    return YES;                                                                             \
  }

#define MLN_DEFINE_FEATURE_ENCODE()                        \
  -(void)encodeWithCoder : (NSCoder *)coder {              \
    [super encodeWithCoder:coder];                         \
    [coder encodeObject:identifier forKey:@"identifier"];  \
    [coder encodeObject:_attributes forKey:@"attributes"]; \
  }

#define MLN_DEFINE_FEATURE_IS_EQUAL()                                                     \
  -(BOOL)isEqual : (id)other {                                                            \
    if (other == self) return YES;                                                        \
    if (![other isKindOfClass:[self class]]) return NO;                                   \
    __typeof(self) otherFeature = other;                                                  \
    return [super isEqual:other] && [self geoJSONObject] == [otherFeature geoJSONObject]; \
  }                                                                                       \
  -(NSUInteger)hash {                                                                     \
    return [super hash] + [[self geoJSONDictionary] hash];                                \
  }

#define MLN_DEFINE_FEATURE_ATTRIBUTES_GETTER() \
  -(NSDictionary *)attributes {                \
    if (!_attributes) {                        \
      return @{};                              \
    }                                          \
    return _attributes;                        \
  }
