#import "MLNCustomDrawableStyleLayer.h"
#import "MLNStyleLayer.h"

#import "MLNCustomDrawableStyleLayer_Private.h"
#import "MLNGeometry_Private.h"
#import "MLNStyleLayer_Private.h"
#import "MLNStyle_Private.h"

#include <mln/layermanager/custom_drawable_layer_factory.hpp>
#include <mln/style/layers/custom_drawable_layer.hpp>

#include <cmath>
#include <memory>

#include <mln/style/layer.hpp>

@interface MLNCustomDrawableStyleLayer (Internal)
- (instancetype)initWithPendingLayer:(std::unique_ptr<mln::style::Layer>)pendingLayer;
@end

@implementation MLNCustomDrawableStyleLayer

- (instancetype)initWithRawLayer:(mln::style::Layer*)rawLayer {
  return [super initWithRawLayer:rawLayer];
}

- (instancetype)initWithPendingLayer:(std::unique_ptr<mln::style::Layer>)pendingLayer {
  return [super initWithPendingLayer:std::move(pendingLayer)];
}

@end

namespace mln {

MLNStyleLayer* CustomDrawableStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
  return [[MLNCustomDrawableStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mln
