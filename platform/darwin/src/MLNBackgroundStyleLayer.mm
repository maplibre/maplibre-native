// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNSource.h"
#import "NSPredicate+MLNPrivateAdditions.h"
#import "NSDate+MLNAdditions.h"
#import "MLNStyleLayer_Private.h"
#import "MLNStyleValue_Private.h"
#import "MLNBackgroundStyleLayer.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNBackgroundStyleLayer_Private.h"

#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/transition_options.hpp>


@interface MLNBackgroundStyleLayer ()

@property (nonatomic, readonly) mbgl::style::BackgroundLayer *rawLayer;

@end

@implementation MLNBackgroundStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier
{
    MLNLogDebug(@"Initializing %@ with identifier: %@", NSStringFromClass([self class]), identifier);
    auto layer = std::make_unique<mbgl::style::BackgroundLayer>(identifier.UTF8String);
    return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::BackgroundLayer *)rawLayer
{
    return (mbgl::style::BackgroundLayer *)super.rawLayer;
}

// MARK: - Accessing the Paint Attributes

- (void)setBackgroundColor:(NSExpression *)backgroundColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting backgroundColor: %@", backgroundColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(backgroundColor, false);
    self.rawLayer->setBackgroundColor(mbglValue);
}

- (NSExpression *)backgroundColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getBackgroundColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultBackgroundColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setBackgroundColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting backgroundColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setBackgroundColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)backgroundColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getBackgroundColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setBackgroundOpacity:(NSExpression *)backgroundOpacity {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting backgroundOpacity: %@", backgroundOpacity);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(backgroundOpacity, false);
    self.rawLayer->setBackgroundOpacity(mbglValue);
}

- (NSExpression *)backgroundOpacity {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getBackgroundOpacity();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultBackgroundOpacity();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setBackgroundOpacityTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting backgroundOpacityTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setBackgroundOpacityTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)backgroundOpacityTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getBackgroundOpacityTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setBackgroundPattern:(NSExpression *)backgroundPattern {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting backgroundPattern: %@", backgroundPattern);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::expression::Image, NSString *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::expression::Image>>(backgroundPattern, false);
    self.rawLayer->setBackgroundPattern(mbglValue);
}

- (NSExpression *)backgroundPattern {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getBackgroundPattern();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultBackgroundPattern();
    }
    return MLNStyleValueTransformer<mbgl::style::expression::Image, NSString *>().toExpression(propertyValue);
}

- (void)setBackgroundPatternTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting backgroundPatternTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setBackgroundPatternTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)backgroundPatternTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getBackgroundPatternTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

@end

namespace mbgl {

MLNStyleLayer* BackgroundStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNBackgroundStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
