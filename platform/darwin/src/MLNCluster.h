#import "MLNFoundation.h"

@protocol MLNFeature;

NS_ASSUME_NONNULL_BEGIN

/**
 An `NSUInteger` constant used to indicate an invalid cluster identifier.
 This indicates a missing cluster feature.
 */
FOUNDATION_EXTERN MLN_EXPORT const NSUInteger MLNClusterIdentifierInvalid;

/**
 A protocol that feature subclasses (i.e. those already conforming to
 the `MLNFeature` protocol) conform to if they represent clusters.

 Currently the only class that conforms to `MLNCluster` is
 `MLNPointFeatureCluster` (a subclass of `MLNPointFeature`).

 To check if a feature is a cluster, check conformity to `MLNCluster`, for
 example:

 ```swift
 let shape = try! MLNShape(data: clusterShapeData, encoding: String.Encoding.utf8.rawValue)

 guard let pointFeature = shape as? MLNPointFeature else {
     throw ExampleError.unexpectedFeatureType
 }

 // Check for cluster conformance
 guard let cluster = pointFeature as? MLNCluster else {
     throw ExampleError.featureIsNotACluster
 }

 // Currently the only supported class that conforms to `MLNCluster` is
 // `MLNPointFeatureCluster`
 guard cluster is MLNPointFeatureCluster else {
     throw ExampleError.unexpectedFeatureType
 }
 ```
 */
MLN_EXPORT
@protocol MLNCluster <MLNFeature>

/** The identifier for the cluster. */
@property (nonatomic, readonly) NSUInteger clusterIdentifier;

/** The number of points within this cluster */
@property (nonatomic, readonly) NSUInteger clusterPointCount;

@end

NS_ASSUME_NONNULL_END
