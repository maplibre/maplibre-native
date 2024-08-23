// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNSource.h"
#import "NSPredicate+MLNPrivateAdditions.h"
#import "NSDate+MLNAdditions.h"
#import "MLNStyleLayer_Private.h"
#import "MLNStyleValue_Private.h"
#import "MLNCircleStyleLayer.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNCircleStyleLayer_Private.h"

#include <mbgl/style/layers/circle_layer.hpp>
#include <mbgl/style/transition_options.hpp>


namespace mbgl {

    MBGL_DEFINE_ENUM(MLNCirclePitchAlignment, {
        { MLNCirclePitchAlignmentMap, "map" },
        { MLNCirclePitchAlignmentViewport, "viewport" },
    });

    MBGL_DEFINE_ENUM(MLNCircleScaleAlignment, {
        { MLNCircleScaleAlignmentMap, "map" },
        { MLNCircleScaleAlignmentViewport, "viewport" },
    });

    MBGL_DEFINE_ENUM(MLNCircleTranslationAnchor, {
        { MLNCircleTranslationAnchorMap, "map" },
        { MLNCircleTranslationAnchorViewport, "viewport" },
    });

}

@interface MLNCircleStyleLayer ()

@property (nonatomic, readonly) mbgl::style::CircleLayer *rawLayer;

@end

@implementation MLNCircleStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier source:(MLNSource *)source
{
    MLNLogDebug(@"Initializing %@ with identifier: %@ source: %@", NSStringFromClass([self class]), identifier, source);
    auto layer = std::make_unique<mbgl::style::CircleLayer>(identifier.UTF8String, source.identifier.UTF8String);
    return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::CircleLayer *)rawLayer
{
    return (mbgl::style::CircleLayer *)super.rawLayer;
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

- (void)setCircleSortKey:(NSExpression *)circleSortKey {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleSortKey: %@", circleSortKey);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(circleSortKey, true);
    self.rawLayer->setCircleSortKey(mbglValue);
}

- (NSExpression *)circleSortKey {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getCircleSortKey();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultCircleSortKey();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

// MARK: - Accessing the Paint Attributes

- (void)setCircleBlur:(NSExpression *)circleBlur {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleBlur: %@", circleBlur);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(circleBlur, true);
    self.rawLayer->setCircleBlur(mbglValue);
}

- (NSExpression *)circleBlur {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getCircleBlur();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultCircleBlur();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setCircleBlurTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleBlurTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setCircleBlurTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)circleBlurTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getCircleBlurTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setCircleColor:(NSExpression *)circleColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleColor: %@", circleColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(circleColor, true);
    self.rawLayer->setCircleColor(mbglValue);
}

- (NSExpression *)circleColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getCircleColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultCircleColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setCircleColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setCircleColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)circleColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getCircleColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setCircleOpacity:(NSExpression *)circleOpacity {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleOpacity: %@", circleOpacity);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(circleOpacity, true);
    self.rawLayer->setCircleOpacity(mbglValue);
}

- (NSExpression *)circleOpacity {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getCircleOpacity();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultCircleOpacity();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setCircleOpacityTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleOpacityTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setCircleOpacityTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)circleOpacityTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getCircleOpacityTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setCirclePitchAlignment:(NSExpression *)circlePitchAlignment {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circlePitchAlignment: %@", circlePitchAlignment);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::AlignmentType, NSValue *, mbgl::style::AlignmentType, MLNCirclePitchAlignment>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::AlignmentType>>(circlePitchAlignment, false);
    self.rawLayer->setCirclePitchAlignment(mbglValue);
}

- (NSExpression *)circlePitchAlignment {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getCirclePitchAlignment();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultCirclePitchAlignment();
    }
    return MLNStyleValueTransformer<mbgl::style::AlignmentType, NSValue *, mbgl::style::AlignmentType, MLNCirclePitchAlignment>().toExpression(propertyValue);
}

- (void)setCircleRadius:(NSExpression *)circleRadius {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleRadius: %@", circleRadius);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(circleRadius, true);
    self.rawLayer->setCircleRadius(mbglValue);
}

- (NSExpression *)circleRadius {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getCircleRadius();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultCircleRadius();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setCircleRadiusTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleRadiusTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setCircleRadiusTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)circleRadiusTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getCircleRadiusTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setCircleScaleAlignment:(NSExpression *)circleScaleAlignment {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleScaleAlignment: %@", circleScaleAlignment);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::CirclePitchScaleType, NSValue *, mbgl::style::CirclePitchScaleType, MLNCircleScaleAlignment>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::CirclePitchScaleType>>(circleScaleAlignment, false);
    self.rawLayer->setCirclePitchScale(mbglValue);
}

- (NSExpression *)circleScaleAlignment {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getCirclePitchScale();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultCirclePitchScale();
    }
    return MLNStyleValueTransformer<mbgl::style::CirclePitchScaleType, NSValue *, mbgl::style::CirclePitchScaleType, MLNCircleScaleAlignment>().toExpression(propertyValue);
}

- (void)setCirclePitchScale:(NSExpression *)circlePitchScale {
}

- (NSExpression *)circlePitchScale {
    return self.circleScaleAlignment;
}

- (void)setCircleStrokeColor:(NSExpression *)circleStrokeColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleStrokeColor: %@", circleStrokeColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(circleStrokeColor, true);
    self.rawLayer->setCircleStrokeColor(mbglValue);
}

- (NSExpression *)circleStrokeColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getCircleStrokeColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultCircleStrokeColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setCircleStrokeColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleStrokeColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setCircleStrokeColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)circleStrokeColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getCircleStrokeColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setCircleStrokeOpacity:(NSExpression *)circleStrokeOpacity {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleStrokeOpacity: %@", circleStrokeOpacity);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(circleStrokeOpacity, true);
    self.rawLayer->setCircleStrokeOpacity(mbglValue);
}

- (NSExpression *)circleStrokeOpacity {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getCircleStrokeOpacity();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultCircleStrokeOpacity();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setCircleStrokeOpacityTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleStrokeOpacityTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setCircleStrokeOpacityTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)circleStrokeOpacityTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getCircleStrokeOpacityTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setCircleStrokeWidth:(NSExpression *)circleStrokeWidth {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleStrokeWidth: %@", circleStrokeWidth);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(circleStrokeWidth, true);
    self.rawLayer->setCircleStrokeWidth(mbglValue);
}

- (NSExpression *)circleStrokeWidth {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getCircleStrokeWidth();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultCircleStrokeWidth();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setCircleStrokeWidthTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleStrokeWidthTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setCircleStrokeWidthTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)circleStrokeWidthTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getCircleStrokeWidthTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setCircleTranslation:(NSExpression *)circleTranslation {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleTranslation: %@", circleTranslation);

    auto mbglValue = MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toPropertyValue<mbgl::style::PropertyValue<std::array<float, 2>>>(circleTranslation, false);
    self.rawLayer->setCircleTranslate(mbglValue);
}

- (NSExpression *)circleTranslation {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getCircleTranslate();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultCircleTranslate();
    }
    return MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toExpression(propertyValue);
}

- (void)setCircleTranslationTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleTranslationTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setCircleTranslateTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)circleTranslationTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getCircleTranslateTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setCircleTranslate:(NSExpression *)circleTranslate {
}

- (NSExpression *)circleTranslate {
    return self.circleTranslation;
}

- (void)setCircleTranslationAnchor:(NSExpression *)circleTranslationAnchor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting circleTranslationAnchor: %@", circleTranslationAnchor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::TranslateAnchorType, NSValue *, mbgl::style::TranslateAnchorType, MLNCircleTranslationAnchor>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::TranslateAnchorType>>(circleTranslationAnchor, false);
    self.rawLayer->setCircleTranslateAnchor(mbglValue);
}

- (NSExpression *)circleTranslationAnchor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getCircleTranslateAnchor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultCircleTranslateAnchor();
    }
    return MLNStyleValueTransformer<mbgl::style::TranslateAnchorType, NSValue *, mbgl::style::TranslateAnchorType, MLNCircleTranslationAnchor>().toExpression(propertyValue);
}

- (void)setCircleTranslateAnchor:(NSExpression *)circleTranslateAnchor {
}

- (NSExpression *)circleTranslateAnchor {
    return self.circleTranslationAnchor;
}

@end

@implementation NSValue (MLNCircleStyleLayerAdditions)

+ (NSValue *)valueWithMLNCirclePitchAlignment:(MLNCirclePitchAlignment)circlePitchAlignment {
    return [NSValue value:&circlePitchAlignment withObjCType:@encode(MLNCirclePitchAlignment)];
}

- (MLNCirclePitchAlignment)MLNCirclePitchAlignmentValue {
    MLNCirclePitchAlignment circlePitchAlignment;
    [self getValue:&circlePitchAlignment];
    return circlePitchAlignment;
}

+ (NSValue *)valueWithMLNCircleScaleAlignment:(MLNCircleScaleAlignment)circleScaleAlignment {
    return [NSValue value:&circleScaleAlignment withObjCType:@encode(MLNCircleScaleAlignment)];
}

- (MLNCircleScaleAlignment)MLNCircleScaleAlignmentValue {
    MLNCircleScaleAlignment circleScaleAlignment;
    [self getValue:&circleScaleAlignment];
    return circleScaleAlignment;
}

+ (NSValue *)valueWithMLNCircleTranslationAnchor:(MLNCircleTranslationAnchor)circleTranslationAnchor {
    return [NSValue value:&circleTranslationAnchor withObjCType:@encode(MLNCircleTranslationAnchor)];
}

- (MLNCircleTranslationAnchor)MLNCircleTranslationAnchorValue {
    MLNCircleTranslationAnchor circleTranslationAnchor;
    [self getValue:&circleTranslationAnchor];
    return circleTranslationAnchor;
}

@end

namespace mbgl {

MLNStyleLayer* CircleStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNCircleStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
