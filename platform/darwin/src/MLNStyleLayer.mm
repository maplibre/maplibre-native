#import "MLNStyleLayer_Private.h"
#import "MLNStyle_Private.h"

#include <mln/style/layer.hpp>
#include <mln/style/style.hpp>

const MLNExceptionName MLNInvalidStyleLayerException = @"MLNInvalidStyleLayerException";

@interface MLNStyleLayer ()

@property (nonatomic, readonly) mln::style::Layer *rawLayer;

@end

@implementation MLNStyleLayer {
  std::unique_ptr<mln::style::Layer> _pendingLayer;
  mapbox::base::WeakPtr<mln::style::Layer> _weakLayer;
}

- (instancetype)initWithRawLayer:(mln::style::Layer *)rawLayer {
  if (self = [super init]) {
    _identifier = @(rawLayer->getID().c_str());
    _weakLayer = rawLayer->makeWeakPtr();
    rawLayer->peer = LayerWrapper{self};
  }
  return self;
}

- (instancetype)initWithPendingLayer:(std::unique_ptr<mln::style::Layer>)pendingLayer {
  if (self = [self initWithRawLayer:pendingLayer.get()]) {
    _pendingLayer = std::move(pendingLayer);
  }
  return self;
}

- (mln::style::Layer *)rawLayer {
  return _weakLayer.get();
}

- (void)addToStyle:(MLNStyle *)style belowLayer:(MLNStyleLayer *)otherLayer {
  if (_pendingLayer == nullptr) {
    [NSException raise:MLNRedundantLayerException
                format:@"This instance %@ was already added to %@. Adding the same layer instance "
                        "to the style more than once is invalid.",
                       self, style];
  }

  if (otherLayer) {
    const std::optional<std::string> belowLayerId{otherLayer.identifier.UTF8String};
    style.rawStyle->addLayer(std::move(_pendingLayer), belowLayerId);
  } else {
    style.rawStyle->addLayer(std::move(_pendingLayer));
  }
}

- (void)removeFromStyle:(MLNStyle *)style {
  if (self.rawLayer == style.rawStyle->getLayer(self.identifier.UTF8String)) {
    _pendingLayer = style.rawStyle->removeLayer(self.identifier.UTF8String);
  }
}

- (void)setVisible:(BOOL)visible {
  MLNAssertStyleLayerIsValid();

  mln::style::VisibilityType v =
      visible ? mln::style::VisibilityType::Visible : mln::style::VisibilityType::None;
  self.rawLayer->setVisibility(v);
}

- (BOOL)isVisible {
  MLNAssertStyleLayerIsValid();

  mln::style::VisibilityType v = self.rawLayer->getVisibility();
  return (v == mln::style::VisibilityType::Visible);
}

- (void)setMaximumZoomLevel:(float)maximumZoomLevel {
  MLNAssertStyleLayerIsValid();

  self.rawLayer->setMaxZoom(maximumZoomLevel);
}

- (float)maximumZoomLevel {
  MLNAssertStyleLayerIsValid();

  return self.rawLayer->getMaxZoom();
}

- (void)setMinimumZoomLevel:(float)minimumZoomLevel {
  MLNAssertStyleLayerIsValid();

  self.rawLayer->setMinZoom(minimumZoomLevel);
}

- (float)minimumZoomLevel {
  MLNAssertStyleLayerIsValid();

  return self.rawLayer->getMinZoom();
}

- (NSString *)description {
  if (self.rawLayer) {
    return [NSString stringWithFormat:@"<%@: %p; identifier = %@; visible = %@>",
                                      NSStringFromClass([self class]), (void *)self,
                                      self.identifier, self.visible ? @"YES" : @"NO"];
  } else {
    return
        [NSString stringWithFormat:@"<%@: %p; identifier = %@; visible = NO>",
                                   NSStringFromClass([self class]), (void *)self, self.identifier];
  }
}

@end
