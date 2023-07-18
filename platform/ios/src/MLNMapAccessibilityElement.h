#import <UIKit/UIKit.h>

#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

@protocol MLNFeature;

/// Unique identifier representing a single annotation in mbgl.
typedef uint64_t MLNAnnotationTag;

/** An accessibility element representing something that appears on the map. */
MLN_EXPORT
@interface MLNMapAccessibilityElement : UIAccessibilityElement

@end

/** An accessibility element representing a map annotation. */
@interface MLNAnnotationAccessibilityElement : MLNMapAccessibilityElement

/** The tag of the annotation represented by this element. */
@property (nonatomic) MLNAnnotationTag tag;

- (instancetype)initWithAccessibilityContainer:(id)container tag:(MLNAnnotationTag)identifier NS_DESIGNATED_INITIALIZER;

@end

/** An accessibility element representing a map feature. */
MLN_EXPORT
@interface MLNFeatureAccessibilityElement : MLNMapAccessibilityElement

/** The feature represented by this element. */
@property (nonatomic, strong) id <MLNFeature> feature;

- (instancetype)initWithAccessibilityContainer:(id)container feature:(id <MLNFeature>)feature NS_DESIGNATED_INITIALIZER;

@end

/** An accessibility element representing a place feature. */
MLN_EXPORT
@interface MLNPlaceFeatureAccessibilityElement : MLNFeatureAccessibilityElement
@end

/** An accessibility element representing a road feature. */
MLN_EXPORT
@interface MLNRoadFeatureAccessibilityElement : MLNFeatureAccessibilityElement
@end

/** An accessibility element representing the MLNMapView at large. */
MLN_EXPORT
@interface MLNMapViewProxyAccessibilityElement : UIAccessibilityElement
@end

NS_ASSUME_NONNULL_END
