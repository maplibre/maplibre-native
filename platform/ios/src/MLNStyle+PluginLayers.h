#import <Foundation/Foundation.h>

#import "MLNStyle.h"

NS_ASSUME_NONNULL_BEGIN

/// Runtime access to style layers that have no Objective-C class, such as the C-ABI plugin layers
/// registered through `MLNPuckPlugin`. `MLNPluginStyleLayer` has no initializer, so these layers
/// can otherwise only come from a style JSON document.
@interface MLNStyle (PluginLayers)

/// Adds a layer described by its style-spec JSON, e.g. `{"id": "puck", "type": "puck", ...}`.
///
/// - parameter layerJSON: The layer object, as it would appear in the style's `layers` array.
/// - parameter siblingIdentifier: Identifier of the layer to insert below, or `nil` to add on top.
/// An
///   unknown identifier is an error.
/// - parameter error: Set when the layer type is not registered, the JSON is invalid, or the
/// identifier
///   is already in use.
///
/// - returns: `YES` when the layer was added.
- (BOOL)addLayerWithJSON:(NSDictionary<NSString *, id> *)layerJSON
    belowLayerWithIdentifier:(nullable NSString *)siblingIdentifier
                       error:(NSError *_Nullable *_Nullable)error;

/// Sets one paint or layout property of an existing layer and schedules a repaint (or a re-layout
/// for a layout property).
///
/// - parameter name: The property name, e.g. `puck-bearing` or `model-heading`. The layer decides
/// whether
///   it is a paint or a layout property.
/// - parameter value: A JSON value (`NSNumber`, `NSString`, or an expression array).
/// - parameter layerIdentifier: Identifier of the layer to change.
/// - parameter error: Set when the layer does not exist or the property rejects the value.
///
/// - returns: `YES` when the value was applied.
- (BOOL)setProperty:(NSString *)name
                     value:(id)value
    forLayerWithIdentifier:(NSString *)layerIdentifier
                     error:(NSError *_Nullable *_Nullable)error;

@end

NS_ASSUME_NONNULL_END
