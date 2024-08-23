// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNSource.h"
#import "NSPredicate+MLNPrivateAdditions.h"
#import "NSDate+MLNAdditions.h"
#import "MLNStyleLayer_Private.h"
#import "MLNStyleValue_Private.h"
#import "MLNFillExtrusionStyleLayer.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNFillExtrusionStyleLayer_Private.h"

#include <mbgl/style/layers/fill_extrusion_layer.hpp>
#include <mbgl/style/transition_options.hpp>


namespace mbgl {

    MBGL_DEFINE_ENUM(MLNFillExtrusionTranslationAnchor, {
        { MLNFillExtrusionTranslationAnchorMap, "map" },
        { MLNFillExtrusionTranslationAnchorViewport, "viewport" },
    });

}

@interface MLNFillExtrusionStyleLayer ()

@property (nonatomic, readonly) mbgl::style::FillExtrusionLayer *rawLayer;

@end

@implementation MLNFillExtrusionStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier source:(MLNSource *)source
{
    MLNLogDebug(@"Initializing %@ with identifier: %@ source: %@", NSStringFromClass([self class]), identifier, source);
    auto layer = std::make_unique<mbgl::style::FillExtrusionLayer>(identifier.UTF8String, source.identifier.UTF8String);
    return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::FillExtrusionLayer *)rawLayer
{
    return (mbgl::style::FillExtrusionLayer *)super.rawLayer;
}

- (NSString *)sourceIdentifier
{
    MLNAssertStyleLayerIsValid();

    return @(self.rawLayer->getSourceID().c_str());
}

- (NSString *)sourceLayerIdentifier
{
    MLNAssertStyleLayerIsValid();

    auto layerID = self.rawLayer->getSourceLayer();
    return layerID.empty() ? nil : @(layerID.c_str());
}

- (void)setSourceLayerIdentifier:(NSString *)sourceLayerIdentifier
{
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting sourceLayerIdentifier: %@", sourceLayerIdentifier);

    self.rawLayer->setSourceLayer(sourceLayerIdentifier.UTF8String ?: "");
}

- (void)setPredicate:(NSPredicate *)predicate
{
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting predicate: %@", predicate);

    self.rawLayer->setFilter(predicate ? predicate.mgl_filter : mbgl::style::Filter());
}

- (NSPredicate *)predicate
{
    MLNAssertStyleLayerIsValid();

    return [NSPredicate mgl_predicateWithFilter:self.rawLayer->getFilter()];
}

// MARK: - Accessing the Paint Attributes

- (void)setFillExtrusionBase:(NSExpression *)fillExtrusionBase {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionBase: %@", fillExtrusionBase);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(fillExtrusionBase, true);
    self.rawLayer->setFillExtrusionBase(mbglValue);
}

- (NSExpression *)fillExtrusionBase {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillExtrusionBase();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillExtrusionBase();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setFillExtrusionBaseTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionBaseTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setFillExtrusionBaseTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)fillExtrusionBaseTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getFillExtrusionBaseTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setFillExtrusionColor:(NSExpression *)fillExtrusionColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionColor: %@", fillExtrusionColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(fillExtrusionColor, true);
    self.rawLayer->setFillExtrusionColor(mbglValue);
}

- (NSExpression *)fillExtrusionColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillExtrusionColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillExtrusionColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setFillExtrusionColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setFillExtrusionColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)fillExtrusionColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getFillExtrusionColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setFillExtrusionHasVerticalGradient:(NSExpression *)fillExtrusionHasVerticalGradient {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionHasVerticalGradient: %@", fillExtrusionHasVerticalGradient);

    auto mbglValue = MLNStyleValueTransformer<bool, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<bool>>(fillExtrusionHasVerticalGradient, false);
    self.rawLayer->setFillExtrusionVerticalGradient(mbglValue);
}

- (NSExpression *)fillExtrusionHasVerticalGradient {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillExtrusionVerticalGradient();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillExtrusionVerticalGradient();
    }
    return MLNStyleValueTransformer<bool, NSNumber *>().toExpression(propertyValue);
}

- (void)setFillExtrusionVerticalGradient:(NSExpression *)fillExtrusionVerticalGradient {
}

- (NSExpression *)fillExtrusionVerticalGradient {
    return self.fillExtrusionHasVerticalGradient;
}

- (void)setFillExtrusionHeight:(NSExpression *)fillExtrusionHeight {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionHeight: %@", fillExtrusionHeight);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(fillExtrusionHeight, true);
    self.rawLayer->setFillExtrusionHeight(mbglValue);
}

- (NSExpression *)fillExtrusionHeight {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillExtrusionHeight();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillExtrusionHeight();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setFillExtrusionHeightTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionHeightTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setFillExtrusionHeightTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)fillExtrusionHeightTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getFillExtrusionHeightTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setFillExtrusionOpacity:(NSExpression *)fillExtrusionOpacity {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionOpacity: %@", fillExtrusionOpacity);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(fillExtrusionOpacity, false);
    self.rawLayer->setFillExtrusionOpacity(mbglValue);
}

- (NSExpression *)fillExtrusionOpacity {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillExtrusionOpacity();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillExtrusionOpacity();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setFillExtrusionOpacityTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionOpacityTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setFillExtrusionOpacityTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)fillExtrusionOpacityTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getFillExtrusionOpacityTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setFillExtrusionPattern:(NSExpression *)fillExtrusionPattern {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionPattern: %@", fillExtrusionPattern);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::expression::Image, NSString *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::expression::Image>>(fillExtrusionPattern, true);
    self.rawLayer->setFillExtrusionPattern(mbglValue);
}

- (NSExpression *)fillExtrusionPattern {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillExtrusionPattern();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillExtrusionPattern();
    }
    return MLNStyleValueTransformer<mbgl::style::expression::Image, NSString *>().toExpression(propertyValue);
}

- (void)setFillExtrusionPatternTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionPatternTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setFillExtrusionPatternTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)fillExtrusionPatternTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getFillExtrusionPatternTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setFillExtrusionTranslation:(NSExpression *)fillExtrusionTranslation {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionTranslation: %@", fillExtrusionTranslation);

    auto mbglValue = MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toPropertyValue<mbgl::style::PropertyValue<std::array<float, 2>>>(fillExtrusionTranslation, false);
    self.rawLayer->setFillExtrusionTranslate(mbglValue);
}

- (NSExpression *)fillExtrusionTranslation {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillExtrusionTranslate();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillExtrusionTranslate();
    }
    return MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toExpression(propertyValue);
}

- (void)setFillExtrusionTranslationTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionTranslationTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setFillExtrusionTranslateTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)fillExtrusionTranslationTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getFillExtrusionTranslateTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setFillExtrusionTranslate:(NSExpression *)fillExtrusionTranslate {
}

- (NSExpression *)fillExtrusionTranslate {
    return self.fillExtrusionTranslation;
}

- (void)setFillExtrusionTranslationAnchor:(NSExpression *)fillExtrusionTranslationAnchor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillExtrusionTranslationAnchor: %@", fillExtrusionTranslationAnchor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::TranslateAnchorType, NSValue *, mbgl::style::TranslateAnchorType, MLNFillExtrusionTranslationAnchor>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::TranslateAnchorType>>(fillExtrusionTranslationAnchor, false);
    self.rawLayer->setFillExtrusionTranslateAnchor(mbglValue);
}

- (NSExpression *)fillExtrusionTranslationAnchor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillExtrusionTranslateAnchor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillExtrusionTranslateAnchor();
    }
    return MLNStyleValueTransformer<mbgl::style::TranslateAnchorType, NSValue *, mbgl::style::TranslateAnchorType, MLNFillExtrusionTranslationAnchor>().toExpression(propertyValue);
}

- (void)setFillExtrusionTranslateAnchor:(NSExpression *)fillExtrusionTranslateAnchor {
}

- (NSExpression *)fillExtrusionTranslateAnchor {
    return self.fillExtrusionTranslationAnchor;
}

@end

@implementation NSValue (MLNFillExtrusionStyleLayerAdditions)

+ (NSValue *)valueWithMLNFillExtrusionTranslationAnchor:(MLNFillExtrusionTranslationAnchor)fillExtrusionTranslationAnchor {
    return [NSValue value:&fillExtrusionTranslationAnchor withObjCType:@encode(MLNFillExtrusionTranslationAnchor)];
}

- (MLNFillExtrusionTranslationAnchor)MLNFillExtrusionTranslationAnchorValue {
    MLNFillExtrusionTranslationAnchor fillExtrusionTranslationAnchor;
    [self getValue:&fillExtrusionTranslationAnchor];
    return fillExtrusionTranslationAnchor;
}

@end

namespace mbgl {

MLNStyleLayer* FillExtrusionStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNFillExtrusionStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
