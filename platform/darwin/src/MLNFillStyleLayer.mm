// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNSource.h"
#import "NSPredicate+MLNPrivateAdditions.h"
#import "NSDate+MLNAdditions.h"
#import "MLNStyleLayer_Private.h"
#import "MLNStyleValue_Private.h"
#import "MLNFillStyleLayer.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNFillStyleLayer_Private.h"

#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/transition_options.hpp>


namespace mbgl {

    MBGL_DEFINE_ENUM(MLNFillTranslationAnchor, {
        { MLNFillTranslationAnchorMap, "map" },
        { MLNFillTranslationAnchorViewport, "viewport" },
    });

}

@interface MLNFillStyleLayer ()

@property (nonatomic, readonly) mbgl::style::FillLayer *rawLayer;

@end

@implementation MLNFillStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier source:(MLNSource *)source
{
    MLNLogDebug(@"Initializing %@ with identifier: %@ source: %@", NSStringFromClass([self class]), identifier, source);
    auto layer = std::make_unique<mbgl::style::FillLayer>(identifier.UTF8String, source.identifier.UTF8String);
    return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::FillLayer *)rawLayer
{
    return (mbgl::style::FillLayer *)super.rawLayer;
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

// MARK: - Accessing the Layout Attributes

- (void)setFillSortKey:(NSExpression *)fillSortKey {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillSortKey: %@", fillSortKey);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(fillSortKey, true);
    self.rawLayer->setFillSortKey(mbglValue);
}

- (NSExpression *)fillSortKey {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillSortKey();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillSortKey();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

// MARK: - Accessing the Paint Attributes

- (void)setFillAntialiased:(NSExpression *)fillAntialiased {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillAntialiased: %@", fillAntialiased);

    auto mbglValue = MLNStyleValueTransformer<bool, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<bool>>(fillAntialiased, false);
    self.rawLayer->setFillAntialias(mbglValue);
}

- (NSExpression *)isFillAntialiased {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillAntialias();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillAntialias();
    }
    return MLNStyleValueTransformer<bool, NSNumber *>().toExpression(propertyValue);
}

- (void)setFillAntialias:(NSExpression *)fillAntialias {
}

- (NSExpression *)fillAntialias {
    return self.isFillAntialiased;
}

- (void)setFillColor:(NSExpression *)fillColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillColor: %@", fillColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(fillColor, true);
    self.rawLayer->setFillColor(mbglValue);
}

- (NSExpression *)fillColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setFillColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setFillColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)fillColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getFillColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setFillOpacity:(NSExpression *)fillOpacity {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillOpacity: %@", fillOpacity);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(fillOpacity, true);
    self.rawLayer->setFillOpacity(mbglValue);
}

- (NSExpression *)fillOpacity {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillOpacity();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillOpacity();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setFillOpacityTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillOpacityTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setFillOpacityTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)fillOpacityTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getFillOpacityTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setFillOutlineColor:(NSExpression *)fillOutlineColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillOutlineColor: %@", fillOutlineColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(fillOutlineColor, true);
    self.rawLayer->setFillOutlineColor(mbglValue);
}

- (NSExpression *)fillOutlineColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillOutlineColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillOutlineColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setFillOutlineColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillOutlineColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setFillOutlineColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)fillOutlineColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getFillOutlineColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setFillPattern:(NSExpression *)fillPattern {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillPattern: %@", fillPattern);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::expression::Image, NSString *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::expression::Image>>(fillPattern, true);
    self.rawLayer->setFillPattern(mbglValue);
}

- (NSExpression *)fillPattern {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillPattern();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillPattern();
    }
    return MLNStyleValueTransformer<mbgl::style::expression::Image, NSString *>().toExpression(propertyValue);
}

- (void)setFillPatternTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillPatternTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setFillPatternTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)fillPatternTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getFillPatternTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setFillTranslation:(NSExpression *)fillTranslation {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillTranslation: %@", fillTranslation);

    auto mbglValue = MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toPropertyValue<mbgl::style::PropertyValue<std::array<float, 2>>>(fillTranslation, false);
    self.rawLayer->setFillTranslate(mbglValue);
}

- (NSExpression *)fillTranslation {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillTranslate();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillTranslate();
    }
    return MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toExpression(propertyValue);
}

- (void)setFillTranslationTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillTranslationTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setFillTranslateTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)fillTranslationTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getFillTranslateTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setFillTranslate:(NSExpression *)fillTranslate {
}

- (NSExpression *)fillTranslate {
    return self.fillTranslation;
}

- (void)setFillTranslationAnchor:(NSExpression *)fillTranslationAnchor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting fillTranslationAnchor: %@", fillTranslationAnchor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::TranslateAnchorType, NSValue *, mbgl::style::TranslateAnchorType, MLNFillTranslationAnchor>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::TranslateAnchorType>>(fillTranslationAnchor, false);
    self.rawLayer->setFillTranslateAnchor(mbglValue);
}

- (NSExpression *)fillTranslationAnchor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getFillTranslateAnchor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultFillTranslateAnchor();
    }
    return MLNStyleValueTransformer<mbgl::style::TranslateAnchorType, NSValue *, mbgl::style::TranslateAnchorType, MLNFillTranslationAnchor>().toExpression(propertyValue);
}

- (void)setFillTranslateAnchor:(NSExpression *)fillTranslateAnchor {
}

- (NSExpression *)fillTranslateAnchor {
    return self.fillTranslationAnchor;
}

@end

@implementation NSValue (MLNFillStyleLayerAdditions)

+ (NSValue *)valueWithMLNFillTranslationAnchor:(MLNFillTranslationAnchor)fillTranslationAnchor {
    return [NSValue value:&fillTranslationAnchor withObjCType:@encode(MLNFillTranslationAnchor)];
}

- (MLNFillTranslationAnchor)MLNFillTranslationAnchorValue {
    MLNFillTranslationAnchor fillTranslationAnchor;
    [self getValue:&fillTranslationAnchor];
    return fillTranslationAnchor;
}

@end

namespace mbgl {

MLNStyleLayer* FillStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNFillStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
