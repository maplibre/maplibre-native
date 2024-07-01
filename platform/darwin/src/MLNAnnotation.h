#import <CoreLocation/CoreLocation.h>
#import <Foundation/Foundation.h>
#import <TargetConditionals.h>

NS_ASSUME_NONNULL_BEGIN

/**
 The `MLNAnnotation` protocol is used to provide annotation-related information
 to a map view. To use this protocol, you adopt it in any custom objects that
 store or represent annotation data. Each object then serves as the source of
 information about a single map annotation and provides critical information,
 such as the annotation’s location on the map. Annotation objects do not provide
 the visual representation of the annotation but typically coordinate (in
 conjunction with the map view’s delegate) the creation of an appropriate
 objects to handle the display.

 An object that adopts this protocol must implement the `coordinate` property.
 The other methods of this protocol are optional.

 #### Related examples
 TODO: Annotation models
 TODO: Annotation views, learn how to add objects that follow the
 `MLNAnnotation` protocol
 */
@protocol MLNAnnotation <NSObject>

// MARK: Position Attributes

/**
 The center point (specified as a map coordinate) of the annotation. (required)
 (read-only)
 */
@property (nonatomic, readonly) CLLocationCoordinate2D coordinate;

@optional

// MARK: Title Attributes

/**
 The string containing the annotation’s title.

 Although this property is optional, if you support the selection of annotations
 in your map view, you are expected to provide this property. This string is
 displayed in the callout for the associated annotation.
 */
@property (nonatomic, readonly, copy, nullable) NSString *title;

/**
 The string containing the annotation’s subtitle.

 This string is displayed in the callout for the associated annotation.
 */
@property (nonatomic, readonly, copy, nullable) NSString *subtitle;

#if !TARGET_OS_IPHONE

/** The string containing the annotation’s tooltip. */
@property (nonatomic, readonly, copy, nullable) NSString *toolTip;

#endif

@end

NS_ASSUME_NONNULL_END
