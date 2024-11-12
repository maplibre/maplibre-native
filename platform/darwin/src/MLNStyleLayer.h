#import <Foundation/Foundation.h>

#import "MLNFoundation.h"
#import "MLNTypes.h"

NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXTERN MLN_EXPORT MLNExceptionName const MLNInvalidStyleLayerException;

/**
 ``MLNStyleLayer`` is an abstract base class for style layers. A style layer
 manages the layout and appearance of content at a specific z-index in a style.
 An ``MLNStyle`` object consists of one or more ``MLNStyle`` objects.

 Each style layer defined by the style JSON file is represented at runtime by an
 ``MLNStyleLayer`` object, which you can use to refine the map’s appearance. You
 can also add and remove style layers dynamically.

 Create instances of ``MLNBackgroundStyleLayer`` and the concrete subclasses of
 ``MLNForegroundStyleLayer`` in order to use ``MLNForegroundStyleLayer``'s properties and methods.
 You do not create instances of ``MLNStyleLayer`` directly, and do not
 create your own subclasses of this class.

 Do not add ``MLNStyleLayer`` objects to the `style` property of a ``MLNStyleLayer`` before
 ``MLNMapViewDelegate/mapView:didFinishLoadingStyle:`` is called.
 */
MLN_EXPORT
@interface MLNStyleLayer : NSObject

// MARK: Initializing a Style Layer

- (instancetype)init
    __attribute__((unavailable("Use -init methods of concrete subclasses instead.")));

// MARK: Identifying a Style Layer

/**
 A string that uniquely identifies the style layer in the style to which it is
 added.
 */
@property (nonatomic, copy, readonly) NSString *identifier;

// MARK: Configuring a Style Layer’s Visibility

/**
 Whether this layer is displayed. A value of `NO` hides the layer.

 #### Related examples
 TODO: Show and hide a layer, learn how to toggle an ``MLNStyleLayer``
 object's visibility.
 */
@property (nonatomic, assign, getter=isVisible) BOOL visible;

/**
 The maximum zoom level at which the layer gets parsed and appears. This value is a floating-point
 number.
 */
@property (nonatomic, assign) float maximumZoomLevel;

/**
 The minimum zoom level at which the layer gets parsed and appears. This value is a floating-point
 number.
 */
@property (nonatomic, assign) float minimumZoomLevel;

@end

NS_ASSUME_NONNULL_END
