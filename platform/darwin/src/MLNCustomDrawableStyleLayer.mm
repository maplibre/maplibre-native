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

#include <mbgl/style/layer.hpp>

@interface MLNCustomDrawableStyleLayer (Internal)
- (instancetype)initWithPendingLayer:(std::unique_ptr<mbgl::style::Layer>)pendingLayer;
@end

@implementation MLNCustomDrawableStyleLayer

- (instancetype)initWithRawLayer:(mbgl::style::Layer *)rawLayer {
    return [super initWithRawLayer:rawLayer];
}

- (instancetype)initWithPendingLayer:(std::unique_ptr<mbgl::style::Layer>)pendingLayer {
    return [super initWithPendingLayer:std::move(pendingLayer)];
}

@end

namespace mbgl {

MLNStyleLayer* CustomDrawableStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNCustomDrawableStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
