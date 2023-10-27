#import "MLNCustomDrawableStyleLayer.h"
#import "MLNStyleLayer.h"

#import "MLNCustomDrawableStyleLayer_Private.h"
#import "MLNStyle_Private.h"
#import "MLNStyleLayer_Private.h"
#import "MLNGeometry_Private.h"

#include <mbgl/layermanager/custom_drawable_layer_factory.hpp>
#include <mbgl/style/layers/custom_drawable_layer.hpp>

#include <memory>
#include <cmath>

@implementation MLNCustomDrawableStyleLayer

/// @note
/// Inherit MLNCustomDrawableStyleLayer class and override initWithIdentifier method to create and attach a valid CustomDrawableLayerHost instance
/// Example:
/// - (instancetype)initWithIdentifier:(NSString *)identifier {
///    auto layer = std::make_unique<mbgl::style::CustomDrawableLayer>(identifier.UTF8String,
///                                                            std::make_unique<ExampleCustomDrawableStyleLayerHost>(self));
///    return self = [super initWithPendingLayer:std::move(layer)];
/// }
///
- (instancetype)initWithIdentifier:(NSString *)identifier {
    auto layer = std::make_unique<mbgl::style::CustomDrawableLayer>(identifier.UTF8String, nullptr);
    return self = [super initWithPendingLayer:std::move(layer)];
}

@end

namespace mbgl {

MLNStyleLayer* CustomDrawableStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNCustomDrawableStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
