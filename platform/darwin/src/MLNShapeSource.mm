#import "MLNFoundation_Private.h"
#import "MLNShapeSource_Private.h"

#import "MLNLoggingConfiguration_Private.h"
#import "MLNStyle_Private.h"
#import "MLNStyleValue_Private.h"
#import "MLNMapView_Private.h"
#import "MLNSource_Private.h"
#import "MLNFeature_Private.h"
#import "MLNShape_Private.h"
#import "MLNCluster.h"

#import "NSPredicate+MLNPrivateAdditions.h"
#import "NSURL+MLNAdditions.h"

#include <mbgl/map/map.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/renderer/renderer.hpp>

const MLNShapeSourceOption MLNShapeSourceOptionBuffer = @"MLNShapeSourceOptionBuffer";
const MLNShapeSourceOption MLNShapeSourceOptionClusterRadius = @"MLNShapeSourceOptionClusterRadius";
const MLNShapeSourceOption MLNShapeSourceOptionClusterMinPoints = @"MLNShapeSourceOptionClusterMinPoints";
const MLNShapeSourceOption MLNShapeSourceOptionClustered = @"MLNShapeSourceOptionClustered";
const MLNShapeSourceOption MLNShapeSourceOptionClusterProperties = @"MLNShapeSourceOptionClusterProperties";
const MLNShapeSourceOption MLNShapeSourceOptionMaximumZoomLevel = @"MLNShapeSourceOptionMaximumZoomLevel";
const MLNShapeSourceOption MLNShapeSourceOptionMaximumZoomLevelForClustering = @"MLNShapeSourceOptionMaximumZoomLevelForClustering";
const MLNShapeSourceOption MLNShapeSourceOptionMinimumZoomLevel = @"MLNShapeSourceOptionMinimumZoomLevel";
const MLNShapeSourceOption MLNShapeSourceOptionSimplificationTolerance = @"MLNShapeSourceOptionSimplificationTolerance";
const MLNShapeSourceOption MLNShapeSourceOptionLineDistanceMetrics = @"MLNShapeSourceOptionLineDistanceMetrics";
const MLNShapeSourceOption MLNShapeSourceOptionSynchronousUpdate = @"MLNShapeSourceOptionSynchronousUpdate";

mbgl::Immutable<mbgl::style::GeoJSONOptions> MLNGeoJSONOptionsFromDictionary(NSDictionary<MLNShapeSourceOption, id> *options) {
    auto geoJSONOptions = mbgl::makeMutable<mbgl::style::GeoJSONOptions>();

    if (NSNumber *value = options[MLNShapeSourceOptionMinimumZoomLevel]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNShapeSourceOptionMaximumZoomLevel must be an NSNumber."];
        }
        geoJSONOptions->minzoom = value.integerValue;
    }

    if (NSNumber *value = options[MLNShapeSourceOptionMaximumZoomLevel]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNShapeSourceOptionMaximumZoomLevel must be an NSNumber."];
        }
        geoJSONOptions->maxzoom = value.integerValue;
    }

    if (NSNumber *value = options[MLNShapeSourceOptionBuffer]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNShapeSourceOptionBuffer must be an NSNumber."];
        }
        geoJSONOptions->buffer = value.integerValue;
    }

    if (NSNumber *value = options[MLNShapeSourceOptionSimplificationTolerance]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNShapeSourceOptionSimplificationTolerance must be an NSNumber."];
        }
        geoJSONOptions->tolerance = value.doubleValue;
    }

    if (NSNumber *value = options[MLNShapeSourceOptionClusterRadius]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNShapeSourceOptionClusterRadius must be an NSNumber."];
        }
        geoJSONOptions->clusterRadius = value.integerValue;
    }

    if (NSNumber *value = options[MLNShapeSourceOptionClusterMinPoints]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNShapeSourceOptionClusterMinPoints must be an NSNumber."];
        }
        geoJSONOptions->clusterMinPoints = value.integerValue;
    }

    if (NSNumber *value = options[MLNShapeSourceOptionMaximumZoomLevelForClustering]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNShapeSourceOptionMaximumZoomLevelForClustering must be an NSNumber."];
        }
        geoJSONOptions->clusterMaxZoom = value.integerValue;
    }

    if (NSNumber *value = options[MLNShapeSourceOptionClustered]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNShapeSourceOptionClustered must be an NSNumber."];
        }
        geoJSONOptions->cluster = value.boolValue;
    }

    if (NSDictionary *value = options[MLNShapeSourceOptionClusterProperties]) {
        if (![value isKindOfClass:[NSDictionary<NSString *, NSArray *> class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNShapeSourceOptionClusterProperties must be an NSDictionary with an NSString as a key and an array containing two NSExpression objects as a value."];
        }

        NSEnumerator *stringEnumerator = [value keyEnumerator];
        NSString *key;

        while (key = [stringEnumerator nextObject]) {
            NSArray *expressionsArray = value[key];
            if (![expressionsArray isKindOfClass:[NSArray class]]) {
                [NSException raise:NSInvalidArgumentException
                            format:@"MLNShapeSourceOptionClusterProperties dictionary member value must be an array containing two objects."];
            }
            // Check that the array has 2 values. One should be a the reduce expression and one should be the map expression.
            if ([expressionsArray count] != 2) {
                [NSException raise:NSInvalidArgumentException
                            format:@"MLNShapeSourceOptionClusterProperties member value requires array of two objects."];
            }

            // reduceExpression should be a valid NSExpression
            NSExpression *reduceExpression = expressionsArray[0];
            if (![reduceExpression isKindOfClass:[NSExpression class]]) {
                [NSException raise:NSInvalidArgumentException
                format:@"MLNShapeSourceOptionClusterProperties array value requires two expression objects."];
            }
            auto reduce = MLNClusterPropertyFromNSExpression(reduceExpression);
            if (!reduce) {
                [NSException raise:NSInvalidArgumentException
                            format:@"Failed to convert MLNShapeSourceOptionClusterProperties reduce expression."];
            }

            // mapExpression should be a valid NSExpression
            NSExpression *mapExpression = expressionsArray[1];
            if (![mapExpression isKindOfClass:[NSExpression class]]) {
                [NSException raise:NSInvalidArgumentException
                            format:@"MLNShapeSourceOptionClusterProperties member value must contain a valid NSExpression."];
            }
            auto map = MLNClusterPropertyFromNSExpression(mapExpression);
            if (!map) {
                [NSException raise:NSInvalidArgumentException
                            format:@"Failed to convert MLNShapeSourceOptionClusterProperties map expression."];
            }

            std::string keyString = std::string([key UTF8String]);

            geoJSONOptions->clusterProperties.emplace(keyString, std::make_pair(std::move(map), std::move(reduce)));
        }
    }

    if (NSNumber *value = options[MLNShapeSourceOptionLineDistanceMetrics]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNShapeSourceOptionLineDistanceMetrics must be an NSNumber."];
        }
        geoJSONOptions->lineMetrics = value.boolValue;
    }

    if (NSNumber *value = options[MLNShapeSourceOptionSynchronousUpdate]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            [NSException raise:NSInvalidArgumentException
                        format:@"MLNShapeSourceOptionSynchronousUpdate must be an NSNumber."];
        }
        geoJSONOptions->synchronousUpdate = value.boolValue;
    }

    return geoJSONOptions;
}

@interface MLNShapeSource ()

@property (nonatomic, readwrite) NSDictionary *options;
@property (nonatomic, readonly) mbgl::style::GeoJSONSource *rawSource;

@end

@implementation MLNShapeSource

- (instancetype)initWithIdentifier:(NSString *)identifier URL:(NSURL *)url options:(NSDictionary<NSString *, id> *)options {
    auto geoJSONOptions = MLNGeoJSONOptionsFromDictionary(options);
    auto source = std::make_unique<mbgl::style::GeoJSONSource>(identifier.UTF8String, std::move(geoJSONOptions));
    if (self = [super initWithPendingSource:std::move(source)]) {
        self.URL = url;
    }
    return self;
}

- (instancetype)initWithIdentifier:(NSString *)identifier shape:(nullable MLNShape *)shape options:(NSDictionary<MLNShapeSourceOption, id> *)options {
    auto geoJSONOptions = MLNGeoJSONOptionsFromDictionary(options);
    auto source = std::make_unique<mbgl::style::GeoJSONSource>(identifier.UTF8String, std::move(geoJSONOptions));
    if (self = [super initWithPendingSource:std::move(source)]) {
        if ([shape isMemberOfClass:[MLNShapeCollection class]]) {
            static dispatch_once_t onceToken;
            dispatch_once(&onceToken, ^{
                NSLog(@"MLNShapeCollection initialized with MLNFeatures will not retain attributes."
                        @"Use MLNShapeCollectionFeature to retain attributes instead."
                        @"This will be logged only once.");
            });
        }
        self.shape = shape;
    }
    return self;
}

- (instancetype)initWithIdentifier:(NSString *)identifier features:(NSArray<MLNShape<MLNFeature> *> *)features options:(nullable NSDictionary<MLNShapeSourceOption, id> *)options {
    for (id <MLNFeature> feature in features) {
        if (![feature conformsToProtocol:@protocol(MLNFeature)]) {
            [NSException raise:NSInvalidArgumentException format:@"The object %@ included in the features argument does not conform to the MLNFeature protocol.", feature];
        }
    }
    MLNShapeCollectionFeature *shapeCollectionFeature = [MLNShapeCollectionFeature shapeCollectionWithShapes:features];
    return [self initWithIdentifier:identifier shape:shapeCollectionFeature options:options];
}

- (instancetype)initWithIdentifier:(NSString *)identifier shapes:(NSArray<MLNShape *> *)shapes options:(nullable NSDictionary<MLNShapeSourceOption, id> *)options {
    MLNShapeCollection *shapeCollection = [MLNShapeCollection shapeCollectionWithShapes:shapes];
    return [self initWithIdentifier:identifier shape:shapeCollection options:options];
}

- (mbgl::style::GeoJSONSource *)rawSource {
    return (mbgl::style::GeoJSONSource *)super.rawSource;
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
        _shape = nil;
    } else {
        self.shape = nil;
    }
}

- (void)setShape:(MLNShape *)shape {
    MLNAssertStyleSourceIsValid();
    self.rawSource->setGeoJSON({ shape.geoJSONObject });
    _shape = shape;
}

- (NSString *)description {
    if (self.rawSource) {
        return [NSString stringWithFormat:@"<%@: %p; identifier = %@; URL = %@; shape = %@>",
                NSStringFromClass([self class]), (void *)self, self.identifier, self.URL, self.shape];
    }
    else {
        return [NSString stringWithFormat:@"<%@: %p; identifier = %@; URL = <unknown>; shape = %@>",
                NSStringFromClass([self class]), (void *)self, self.identifier, self.shape];
    }
}

- (NSArray<id <MLNFeature>> *)featuresMatchingPredicate:(nullable NSPredicate *)predicate {
    MLNAssertStyleSourceIsValid();
    std::optional<mbgl::style::Filter> optionalFilter;
    if (predicate) {
        optionalFilter = predicate.mgl_filter;
    }

    std::vector<mbgl::Feature> features;
    if ([self.stylable isKindOfClass:[MLNMapView class]]) {
        MLNMapView *mapView = (MLNMapView *)self.stylable;
        features = mapView.renderer->querySourceFeatures(self.rawSource->getID(), { {}, optionalFilter });
    }
    return MLNFeaturesFromMBGLFeatures(features);
}

// MARK: - MLNCluster management

- (std::optional<mbgl::FeatureExtensionValue>)featureExtensionValueOfCluster:(MLNShape<MLNCluster> *)cluster extension:(std::string)extension options:(const std::map<std::string, mbgl::Value>)options {
    MLNAssertStyleSourceIsValid();
    std::optional<mbgl::FeatureExtensionValue> extensionValue;

    // Check parameters
    if (!self.rawSource || !self.stylable || !cluster) {
        return extensionValue;
    }

    auto geoJSON = [cluster geoJSONObject];

    if (!geoJSON.is<mbgl::GeoJSONFeature>()) {
        MLNAssert(0, @"cluster geoJSON object is not a feature.");
        return extensionValue;
    }

    auto clusterFeature = geoJSON.get<mbgl::GeoJSONFeature>();

    if ([self.stylable isKindOfClass:[MLNMapView class]]) {
        MLNMapView *mapView = (MLNMapView *)self.stylable;
        extensionValue = mapView.renderer->queryFeatureExtensions(self.rawSource->getID(),
                                                                  clusterFeature,
                                                                  "supercluster",
                                                                  extension,
                                                                  options);
    }
    return extensionValue;
}

- (NSArray<id <MLNFeature>> *)leavesOfCluster:(MLNPointFeatureCluster *)cluster offset:(NSUInteger)offset limit:(NSUInteger)limit {
    const std::map<std::string, mbgl::Value> options = {
        { "limit", static_cast<uint64_t>(limit) },
        { "offset", static_cast<uint64_t>(offset) }
    };

    auto featureExtension = [self featureExtensionValueOfCluster:cluster extension:"leaves" options:options];

    if (!featureExtension) {
        return @[];
    }

    if (!featureExtension->is<mbgl::FeatureCollection>()) {
        return @[];
    }

    std::vector<mbgl::GeoJSONFeature> leaves = featureExtension->get<mbgl::FeatureCollection>();
    return MLNFeaturesFromMBGLFeatures(leaves);
}

- (NSArray<id <MLNFeature>> *)childrenOfCluster:(MLNPointFeatureCluster *)cluster {
    auto featureExtension = [self featureExtensionValueOfCluster:cluster extension:"children" options:{}];

    if (!featureExtension) {
        return @[];
    }

    if (!featureExtension->is<mbgl::FeatureCollection>()) {
        return @[];
    }

    std::vector<mbgl::GeoJSONFeature> leaves = featureExtension->get<mbgl::FeatureCollection>();
    return MLNFeaturesFromMBGLFeatures(leaves);
}

- (double)zoomLevelForExpandingCluster:(MLNPointFeatureCluster *)cluster {
    auto featureExtension = [self featureExtensionValueOfCluster:cluster extension:"expansion-zoom" options:{}];

    if (!featureExtension) {
        return -1.0;
    }

    if (!featureExtension->is<mbgl::Value>()) {
        return -1.0;
    }

    auto value = featureExtension->get<mbgl::Value>();
    if (value.is<uint64_t>()) {
        auto zoom = value.get<uint64_t>();
        return static_cast<double>(zoom);
    }

    return -1.0;
}

- (void)debugRecursiveLogForFeature:(id <MLNFeature>)feature indent:(NSUInteger)indent {
    NSString *description = feature.description;

    // Want our recursive log on a single line
    NSString *log = [description stringByReplacingOccurrencesOfString:@"\\s+"
                                                           withString:@" "
                                                              options:NSRegularExpressionSearch
                                                                range:NSMakeRange(0, description.length)];

    printf("%*s%s\n", (int)indent, "", log.UTF8String);

    MLNPointFeatureCluster *cluster = MLN_OBJC_DYNAMIC_CAST(feature, MLNPointFeatureCluster);

    if (cluster) {
        for (id <MLNFeature> child in [self childrenOfCluster:cluster]) {
            [self debugRecursiveLogForFeature:child indent:indent + 4];
        }
    }
}

@end
