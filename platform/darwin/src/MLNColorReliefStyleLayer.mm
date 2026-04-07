// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNColorReliefStyleLayer.h"
#import "MLNColorReliefStyleLayer_Private.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNSource.h"
#import "MLNStyleLayer_Private.h"
#import "MLNStyleValue_Private.h"
#import "NSDate+MLNAdditions.h"
#import "NSPredicate+MLNPrivateAdditions.h"

#include <mbgl/style/layers/color_relief_layer.hpp>
#include <mbgl/style/transition_options.hpp>

@interface MLNColorReliefStyleLayer ()

@property (nonatomic, readonly) mbgl::style::ColorReliefLayer *rawLayer;

@end

@implementation MLNColorReliefStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier source:(MLNSource *)source {
  MLNLogDebug(@"Initializing %@ with identifier: %@ source: %@", NSStringFromClass([self class]),
              identifier, source);
  auto layer = std::make_unique<mbgl::style::ColorReliefLayer>(identifier.UTF8String,
                                                               source.identifier.UTF8String);
  return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::ColorReliefLayer *)rawLayer {
  return (mbgl::style::ColorReliefLayer *)super.rawLayer;
}

- (NSString *)sourceIdentifier {
  MLNAssertStyleLayerIsValid();

  return @(self.rawLayer->getSourceID().c_str());
}

// MARK: - Accessing the Paint Attributes

- (void)setColorReliefColor:(NSExpression *)colorReliefColor {
  MLNAssertStyleLayerIsValid();
  MLNLogDebug(@"Setting colorReliefColor: %@", colorReliefColor);

  auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>()
                       .toPropertyValue<mbgl::style::ColorRampPropertyValue>(colorReliefColor);
  self.rawLayer->setColorReliefColor(mbglValue);
}

- (NSExpression *)colorReliefColor {
  MLNAssertStyleLayerIsValid();

  auto propertyValue = self.rawLayer->getColorReliefColor();
  if (propertyValue.isUndefined()) {
    propertyValue = self.rawLayer->getDefaultColorReliefColor();
  }
  return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setColorReliefOpacity:(NSExpression *)colorReliefOpacity {
  MLNAssertStyleLayerIsValid();
  MLNLogDebug(@"Setting colorReliefOpacity: %@", colorReliefOpacity);

  auto mbglValue =
      MLNStyleValueTransformer<float, NSNumber *>()
          .toPropertyValue<mbgl::style::PropertyValue<float>>(colorReliefOpacity, false);
  self.rawLayer->setColorReliefOpacity(mbglValue);
}

- (NSExpression *)colorReliefOpacity {
  MLNAssertStyleLayerIsValid();

  auto propertyValue = self.rawLayer->getColorReliefOpacity();
  if (propertyValue.isUndefined()) {
    propertyValue = self.rawLayer->getDefaultColorReliefOpacity();
  }
  return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setColorReliefOpacityTransition:(MLNTransition)transition {
  MLNAssertStyleLayerIsValid();
  MLNLogDebug(@"Setting colorReliefOpacityTransition: %@", MLNStringFromMLNTransition(transition));

  self.rawLayer->setColorReliefOpacityTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)colorReliefOpacityTransition {
  MLNAssertStyleLayerIsValid();

  mbgl::style::TransitionOptions transitionOptions =
      self.rawLayer->getColorReliefOpacityTransition();

  return MLNTransitionFromOptions(transitionOptions);
}

@end

namespace mbgl {

MLNStyleLayer *ColorReliefStyleLayerPeerFactory::createPeer(style::Layer *rawLayer) {
  return [[MLNColorReliefStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
