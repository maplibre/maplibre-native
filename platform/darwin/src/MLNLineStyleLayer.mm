// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNSource.h"
#import "NSPredicate+MLNPrivateAdditions.h"
#import "NSDate+MLNAdditions.h"
#import "MLNStyleLayer_Private.h"
#import "MLNStyleValue_Private.h"
#import "MLNLineStyleLayer.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNLineStyleLayer_Private.h"

#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/transition_options.hpp>


namespace mbgl {

    MBGL_DEFINE_ENUM(MLNLineCap, {
        { MLNLineCapButt, "butt" },
        { MLNLineCapRound, "round" },
        { MLNLineCapSquare, "square" },
    });

    MBGL_DEFINE_ENUM(MLNLineJoin, {
        { MLNLineJoinBevel, "bevel" },
        { MLNLineJoinRound, "round" },
        { MLNLineJoinMiter, "miter" },
    });

    MBGL_DEFINE_ENUM(MLNLineTranslationAnchor, {
        { MLNLineTranslationAnchorMap, "map" },
        { MLNLineTranslationAnchorViewport, "viewport" },
    });

}

@interface MLNLineStyleLayer ()

@property (nonatomic, readonly) mbgl::style::LineLayer *rawLayer;

@end

@implementation MLNLineStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier source:(MLNSource *)source
{
    MLNLogDebug(@"Initializing %@ with identifier: %@ source: %@", NSStringFromClass([self class]), identifier, source);
    auto layer = std::make_unique<mbgl::style::LineLayer>(identifier.UTF8String, source.identifier.UTF8String);
    return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::LineLayer *)rawLayer
{
    return (mbgl::style::LineLayer *)super.rawLayer;
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

- (void)setLineCap:(NSExpression *)lineCap {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineCap: %@", lineCap);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::LineCapType, NSValue *, mbgl::style::LineCapType, MLNLineCap>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::LineCapType>>(lineCap, false);
    self.rawLayer->setLineCap(mbglValue);
}

- (NSExpression *)lineCap {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineCap();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineCap();
    }
    return MLNStyleValueTransformer<mbgl::style::LineCapType, NSValue *, mbgl::style::LineCapType, MLNLineCap>().toExpression(propertyValue);
}

- (void)setLineJoin:(NSExpression *)lineJoin {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineJoin: %@", lineJoin);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::LineJoinType, NSValue *, mbgl::style::LineJoinType, MLNLineJoin>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::LineJoinType>>(lineJoin, true);
    self.rawLayer->setLineJoin(mbglValue);
}

- (NSExpression *)lineJoin {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineJoin();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineJoin();
    }
    return MLNStyleValueTransformer<mbgl::style::LineJoinType, NSValue *, mbgl::style::LineJoinType, MLNLineJoin>().toExpression(propertyValue);
}

- (void)setLineMiterLimit:(NSExpression *)lineMiterLimit {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineMiterLimit: %@", lineMiterLimit);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(lineMiterLimit, false);
    self.rawLayer->setLineMiterLimit(mbglValue);
}

- (NSExpression *)lineMiterLimit {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineMiterLimit();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineMiterLimit();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setLineRoundLimit:(NSExpression *)lineRoundLimit {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineRoundLimit: %@", lineRoundLimit);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(lineRoundLimit, false);
    self.rawLayer->setLineRoundLimit(mbglValue);
}

- (NSExpression *)lineRoundLimit {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineRoundLimit();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineRoundLimit();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setLineSortKey:(NSExpression *)lineSortKey {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineSortKey: %@", lineSortKey);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(lineSortKey, true);
    self.rawLayer->setLineSortKey(mbglValue);
}

- (NSExpression *)lineSortKey {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineSortKey();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineSortKey();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

// MARK: - Accessing the Paint Attributes

- (void)setLineBlur:(NSExpression *)lineBlur {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineBlur: %@", lineBlur);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(lineBlur, true);
    self.rawLayer->setLineBlur(mbglValue);
}

- (NSExpression *)lineBlur {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineBlur();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineBlur();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setLineBlurTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineBlurTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setLineBlurTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)lineBlurTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getLineBlurTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setLineColor:(NSExpression *)lineColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineColor: %@", lineColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(lineColor, true);
    self.rawLayer->setLineColor(mbglValue);
}

- (NSExpression *)lineColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setLineColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setLineColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)lineColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getLineColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setLineDashPattern:(NSExpression *)lineDashPattern {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineDashPattern: %@", lineDashPattern);

    auto mbglValue = MLNStyleValueTransformer<std::vector<float>, NSArray<NSNumber *> *, float>().toPropertyValue<mbgl::style::PropertyValue<std::vector<float>>>(lineDashPattern, false);
    self.rawLayer->setLineDasharray(mbglValue);
}

- (NSExpression *)lineDashPattern {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineDasharray();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineDasharray();
    }
    return MLNStyleValueTransformer<std::vector<float>, NSArray<NSNumber *> *, float>().toExpression(propertyValue);
}

- (void)setLineDashPatternTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineDashPatternTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setLineDasharrayTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)lineDashPatternTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getLineDasharrayTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setLineDasharray:(NSExpression *)lineDasharray {
}

- (NSExpression *)lineDasharray {
    return self.lineDashPattern;
}

- (void)setLineGapWidth:(NSExpression *)lineGapWidth {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineGapWidth: %@", lineGapWidth);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(lineGapWidth, true);
    self.rawLayer->setLineGapWidth(mbglValue);
}

- (NSExpression *)lineGapWidth {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineGapWidth();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineGapWidth();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setLineGapWidthTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineGapWidthTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setLineGapWidthTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)lineGapWidthTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getLineGapWidthTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setLineGradient:(NSExpression *)lineGradient {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineGradient: %@", lineGradient);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::ColorRampPropertyValue>(lineGradient);
    self.rawLayer->setLineGradient(mbglValue);
}

- (NSExpression *)lineGradient {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineGradient();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineGradient();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setLineOffset:(NSExpression *)lineOffset {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineOffset: %@", lineOffset);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(lineOffset, true);
    self.rawLayer->setLineOffset(mbglValue);
}

- (NSExpression *)lineOffset {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineOffset();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineOffset();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setLineOffsetTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineOffsetTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setLineOffsetTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)lineOffsetTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getLineOffsetTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setLineOpacity:(NSExpression *)lineOpacity {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineOpacity: %@", lineOpacity);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(lineOpacity, true);
    self.rawLayer->setLineOpacity(mbglValue);
}

- (NSExpression *)lineOpacity {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineOpacity();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineOpacity();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setLineOpacityTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineOpacityTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setLineOpacityTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)lineOpacityTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getLineOpacityTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setLinePattern:(NSExpression *)linePattern {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting linePattern: %@", linePattern);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::expression::Image, NSString *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::expression::Image>>(linePattern, true);
    self.rawLayer->setLinePattern(mbglValue);
}

- (NSExpression *)linePattern {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLinePattern();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLinePattern();
    }
    return MLNStyleValueTransformer<mbgl::style::expression::Image, NSString *>().toExpression(propertyValue);
}

- (void)setLinePatternTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting linePatternTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setLinePatternTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)linePatternTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getLinePatternTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setLineTranslation:(NSExpression *)lineTranslation {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineTranslation: %@", lineTranslation);

    auto mbglValue = MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toPropertyValue<mbgl::style::PropertyValue<std::array<float, 2>>>(lineTranslation, false);
    self.rawLayer->setLineTranslate(mbglValue);
}

- (NSExpression *)lineTranslation {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineTranslate();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineTranslate();
    }
    return MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toExpression(propertyValue);
}

- (void)setLineTranslationTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineTranslationTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setLineTranslateTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)lineTranslationTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getLineTranslateTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setLineTranslate:(NSExpression *)lineTranslate {
}

- (NSExpression *)lineTranslate {
    return self.lineTranslation;
}

- (void)setLineTranslationAnchor:(NSExpression *)lineTranslationAnchor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineTranslationAnchor: %@", lineTranslationAnchor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::TranslateAnchorType, NSValue *, mbgl::style::TranslateAnchorType, MLNLineTranslationAnchor>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::TranslateAnchorType>>(lineTranslationAnchor, false);
    self.rawLayer->setLineTranslateAnchor(mbglValue);
}

- (NSExpression *)lineTranslationAnchor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineTranslateAnchor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineTranslateAnchor();
    }
    return MLNStyleValueTransformer<mbgl::style::TranslateAnchorType, NSValue *, mbgl::style::TranslateAnchorType, MLNLineTranslationAnchor>().toExpression(propertyValue);
}

- (void)setLineTranslateAnchor:(NSExpression *)lineTranslateAnchor {
}

- (NSExpression *)lineTranslateAnchor {
    return self.lineTranslationAnchor;
}

- (void)setLineWidth:(NSExpression *)lineWidth {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineWidth: %@", lineWidth);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(lineWidth, true);
    self.rawLayer->setLineWidth(mbglValue);
}

- (NSExpression *)lineWidth {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getLineWidth();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultLineWidth();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setLineWidthTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting lineWidthTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setLineWidthTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)lineWidthTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getLineWidthTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

@end

@implementation NSValue (MLNLineStyleLayerAdditions)

+ (NSValue *)valueWithMLNLineCap:(MLNLineCap)lineCap {
    return [NSValue value:&lineCap withObjCType:@encode(MLNLineCap)];
}

- (MLNLineCap)MLNLineCapValue {
    MLNLineCap lineCap;
    [self getValue:&lineCap];
    return lineCap;
}

+ (NSValue *)valueWithMLNLineJoin:(MLNLineJoin)lineJoin {
    return [NSValue value:&lineJoin withObjCType:@encode(MLNLineJoin)];
}

- (MLNLineJoin)MLNLineJoinValue {
    MLNLineJoin lineJoin;
    [self getValue:&lineJoin];
    return lineJoin;
}

+ (NSValue *)valueWithMLNLineTranslationAnchor:(MLNLineTranslationAnchor)lineTranslationAnchor {
    return [NSValue value:&lineTranslationAnchor withObjCType:@encode(MLNLineTranslationAnchor)];
}

- (MLNLineTranslationAnchor)MLNLineTranslationAnchorValue {
    MLNLineTranslationAnchor lineTranslationAnchor;
    [self getValue:&lineTranslationAnchor];
    return lineTranslationAnchor;
}

@end

namespace mbgl {

MLNStyleLayer* LineStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNLineStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
