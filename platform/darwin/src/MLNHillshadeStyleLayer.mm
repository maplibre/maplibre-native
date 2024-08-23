// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNSource.h"
#import "NSPredicate+MLNPrivateAdditions.h"
#import "NSDate+MLNAdditions.h"
#import "MLNStyleLayer_Private.h"
#import "MLNStyleValue_Private.h"
#import "MLNHillshadeStyleLayer.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNHillshadeStyleLayer_Private.h"

#include <mbgl/style/layers/hillshade_layer.hpp>
#include <mbgl/style/transition_options.hpp>


namespace mbgl {

    MBGL_DEFINE_ENUM(MLNHillshadeIlluminationAnchor, {
        { MLNHillshadeIlluminationAnchorMap, "map" },
        { MLNHillshadeIlluminationAnchorViewport, "viewport" },
    });

}

@interface MLNHillshadeStyleLayer ()

@property (nonatomic, readonly) mbgl::style::HillshadeLayer *rawLayer;

@end

@implementation MLNHillshadeStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier source:(MLNSource *)source
{
    MLNLogDebug(@"Initializing %@ with identifier: %@ source: %@", NSStringFromClass([self class]), identifier, source);
    auto layer = std::make_unique<mbgl::style::HillshadeLayer>(identifier.UTF8String, source.identifier.UTF8String);
    return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::HillshadeLayer *)rawLayer
{
    return (mbgl::style::HillshadeLayer *)super.rawLayer;
}

- (NSString *)sourceIdentifier
{
    MLNAssertStyleLayerIsValid();

    return @(self.rawLayer->getSourceID().c_str());
}

// MARK: - Accessing the Paint Attributes

- (void)setHillshadeAccentColor:(NSExpression *)hillshadeAccentColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting hillshadeAccentColor: %@", hillshadeAccentColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(hillshadeAccentColor, false);
    self.rawLayer->setHillshadeAccentColor(mbglValue);
}

- (NSExpression *)hillshadeAccentColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getHillshadeAccentColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultHillshadeAccentColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setHillshadeAccentColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting hillshadeAccentColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setHillshadeAccentColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)hillshadeAccentColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getHillshadeAccentColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setHillshadeExaggeration:(NSExpression *)hillshadeExaggeration {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting hillshadeExaggeration: %@", hillshadeExaggeration);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(hillshadeExaggeration, false);
    self.rawLayer->setHillshadeExaggeration(mbglValue);
}

- (NSExpression *)hillshadeExaggeration {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getHillshadeExaggeration();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultHillshadeExaggeration();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setHillshadeExaggerationTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting hillshadeExaggerationTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setHillshadeExaggerationTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)hillshadeExaggerationTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getHillshadeExaggerationTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setHillshadeHighlightColor:(NSExpression *)hillshadeHighlightColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting hillshadeHighlightColor: %@", hillshadeHighlightColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(hillshadeHighlightColor, false);
    self.rawLayer->setHillshadeHighlightColor(mbglValue);
}

- (NSExpression *)hillshadeHighlightColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getHillshadeHighlightColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultHillshadeHighlightColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setHillshadeHighlightColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting hillshadeHighlightColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setHillshadeHighlightColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)hillshadeHighlightColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getHillshadeHighlightColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setHillshadeIlluminationAnchor:(NSExpression *)hillshadeIlluminationAnchor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting hillshadeIlluminationAnchor: %@", hillshadeIlluminationAnchor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::HillshadeIlluminationAnchorType, NSValue *, mbgl::style::HillshadeIlluminationAnchorType, MLNHillshadeIlluminationAnchor>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::HillshadeIlluminationAnchorType>>(hillshadeIlluminationAnchor, false);
    self.rawLayer->setHillshadeIlluminationAnchor(mbglValue);
}

- (NSExpression *)hillshadeIlluminationAnchor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getHillshadeIlluminationAnchor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultHillshadeIlluminationAnchor();
    }
    return MLNStyleValueTransformer<mbgl::style::HillshadeIlluminationAnchorType, NSValue *, mbgl::style::HillshadeIlluminationAnchorType, MLNHillshadeIlluminationAnchor>().toExpression(propertyValue);
}

- (void)setHillshadeIlluminationDirection:(NSExpression *)hillshadeIlluminationDirection {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting hillshadeIlluminationDirection: %@", hillshadeIlluminationDirection);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(hillshadeIlluminationDirection, false);
    self.rawLayer->setHillshadeIlluminationDirection(mbglValue);
}

- (NSExpression *)hillshadeIlluminationDirection {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getHillshadeIlluminationDirection();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultHillshadeIlluminationDirection();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setHillshadeShadowColor:(NSExpression *)hillshadeShadowColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting hillshadeShadowColor: %@", hillshadeShadowColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(hillshadeShadowColor, false);
    self.rawLayer->setHillshadeShadowColor(mbglValue);
}

- (NSExpression *)hillshadeShadowColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getHillshadeShadowColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultHillshadeShadowColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setHillshadeShadowColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting hillshadeShadowColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setHillshadeShadowColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)hillshadeShadowColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getHillshadeShadowColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

@end

@implementation NSValue (MLNHillshadeStyleLayerAdditions)

+ (NSValue *)valueWithMLNHillshadeIlluminationAnchor:(MLNHillshadeIlluminationAnchor)hillshadeIlluminationAnchor {
    return [NSValue value:&hillshadeIlluminationAnchor withObjCType:@encode(MLNHillshadeIlluminationAnchor)];
}

- (MLNHillshadeIlluminationAnchor)MLNHillshadeIlluminationAnchorValue {
    MLNHillshadeIlluminationAnchor hillshadeIlluminationAnchor;
    [self getValue:&hillshadeIlluminationAnchor];
    return hillshadeIlluminationAnchor;
}

@end

namespace mbgl {

MLNStyleLayer* HillshadeStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNHillshadeStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
