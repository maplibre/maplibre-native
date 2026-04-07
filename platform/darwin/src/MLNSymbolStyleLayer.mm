// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNSource.h"
#import "NSPredicate+MLNPrivateAdditions.h"
#import "NSDate+MLNAdditions.h"
#import "MLNStyleLayer_Private.h"
#import "MLNStyleValue_Private.h"
#import "MLNSymbolStyleLayer.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNSymbolStyleLayer_Private.h"

#include <mbgl/style/layers/symbol_layer.hpp>
#include <mbgl/style/transition_options.hpp>


namespace mbgl {

    MBGL_DEFINE_ENUM(MLNIconAnchor, {
        { MLNIconAnchorCenter, "center" },
        { MLNIconAnchorLeft, "left" },
        { MLNIconAnchorRight, "right" },
        { MLNIconAnchorTop, "top" },
        { MLNIconAnchorBottom, "bottom" },
        { MLNIconAnchorTopLeft, "top-left" },
        { MLNIconAnchorTopRight, "top-right" },
        { MLNIconAnchorBottomLeft, "bottom-left" },
        { MLNIconAnchorBottomRight, "bottom-right" },
    });

    MBGL_DEFINE_ENUM(MLNIconPitchAlignment, {
        { MLNIconPitchAlignmentMap, "map" },
        { MLNIconPitchAlignmentViewport, "viewport" },
        { MLNIconPitchAlignmentAuto, "auto" },
    });

    MBGL_DEFINE_ENUM(MLNIconRotationAlignment, {
        { MLNIconRotationAlignmentMap, "map" },
        { MLNIconRotationAlignmentViewport, "viewport" },
        { MLNIconRotationAlignmentAuto, "auto" },
    });

    MBGL_DEFINE_ENUM(MLNIconTextFit, {
        { MLNIconTextFitNone, "none" },
        { MLNIconTextFitWidth, "width" },
        { MLNIconTextFitHeight, "height" },
        { MLNIconTextFitBoth, "both" },
    });

    MBGL_DEFINE_ENUM(MLNSymbolPlacement, {
        { MLNSymbolPlacementPoint, "point" },
        { MLNSymbolPlacementLine, "line" },
        { MLNSymbolPlacementLineCenter, "line-center" },
    });

    MBGL_DEFINE_ENUM(MLNSymbolZOrder, {
        { MLNSymbolZOrderAuto, "auto" },
        { MLNSymbolZOrderViewportY, "viewport-y" },
        { MLNSymbolZOrderSource, "source" },
    });

    MBGL_DEFINE_ENUM(MLNTextAnchor, {
        { MLNTextAnchorCenter, "center" },
        { MLNTextAnchorLeft, "left" },
        { MLNTextAnchorRight, "right" },
        { MLNTextAnchorTop, "top" },
        { MLNTextAnchorBottom, "bottom" },
        { MLNTextAnchorTopLeft, "top-left" },
        { MLNTextAnchorTopRight, "top-right" },
        { MLNTextAnchorBottomLeft, "bottom-left" },
        { MLNTextAnchorBottomRight, "bottom-right" },
    });

    MBGL_DEFINE_ENUM(MLNTextJustification, {
        { MLNTextJustificationAuto, "auto" },
        { MLNTextJustificationLeft, "left" },
        { MLNTextJustificationCenter, "center" },
        { MLNTextJustificationRight, "right" },
    });

    MBGL_DEFINE_ENUM(MLNTextPitchAlignment, {
        { MLNTextPitchAlignmentMap, "map" },
        { MLNTextPitchAlignmentViewport, "viewport" },
        { MLNTextPitchAlignmentAuto, "auto" },
    });

    MBGL_DEFINE_ENUM(MLNTextRotationAlignment, {
        { MLNTextRotationAlignmentMap, "map" },
        { MLNTextRotationAlignmentViewport, "viewport" },
        { MLNTextRotationAlignmentAuto, "auto" },
    });

    MBGL_DEFINE_ENUM(MLNTextTransform, {
        { MLNTextTransformNone, "none" },
        { MLNTextTransformUppercase, "uppercase" },
        { MLNTextTransformLowercase, "lowercase" },
    });

    MBGL_DEFINE_ENUM(MLNTextWritingMode, {
        { MLNTextWritingModeHorizontal, "horizontal" },
        { MLNTextWritingModeVertical, "vertical" },
    });

    MBGL_DEFINE_ENUM(MLNIconTranslationAnchor, {
        { MLNIconTranslationAnchorMap, "map" },
        { MLNIconTranslationAnchorViewport, "viewport" },
    });

    MBGL_DEFINE_ENUM(MLNTextTranslationAnchor, {
        { MLNTextTranslationAnchorMap, "map" },
        { MLNTextTranslationAnchorViewport, "viewport" },
    });

}

@interface MLNSymbolStyleLayer ()

@property (nonatomic, readonly) mbgl::style::SymbolLayer *rawLayer;

@end

@implementation MLNSymbolStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier source:(MLNSource *)source
{
    MLNLogDebug(@"Initializing %@ with identifier: %@ source: %@", NSStringFromClass([self class]), identifier, source);
    auto layer = std::make_unique<mbgl::style::SymbolLayer>(identifier.UTF8String, source.identifier.UTF8String);
    return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::SymbolLayer *)rawLayer
{
    return (mbgl::style::SymbolLayer *)super.rawLayer;
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

- (void)setIconAllowsOverlap:(NSExpression *)iconAllowsOverlap {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconAllowsOverlap: %@", iconAllowsOverlap);

    auto mbglValue = MLNStyleValueTransformer<bool, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<bool>>(iconAllowsOverlap, false);
    self.rawLayer->setIconAllowOverlap(mbglValue);
}

- (NSExpression *)iconAllowsOverlap {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconAllowOverlap();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconAllowOverlap();
    }
    return MLNStyleValueTransformer<bool, NSNumber *>().toExpression(propertyValue);
}

- (void)setIconAllowOverlap:(NSExpression *)iconAllowOverlap {
}

- (NSExpression *)iconAllowOverlap {
    return self.iconAllowsOverlap;
}

- (void)setIconAnchor:(NSExpression *)iconAnchor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconAnchor: %@", iconAnchor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::SymbolAnchorType, NSValue *, mbgl::style::SymbolAnchorType, MLNIconAnchor>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::SymbolAnchorType>>(iconAnchor, true);
    self.rawLayer->setIconAnchor(mbglValue);
}

- (NSExpression *)iconAnchor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconAnchor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconAnchor();
    }
    return MLNStyleValueTransformer<mbgl::style::SymbolAnchorType, NSValue *, mbgl::style::SymbolAnchorType, MLNIconAnchor>().toExpression(propertyValue);
}

- (void)setIconIgnoresPlacement:(NSExpression *)iconIgnoresPlacement {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconIgnoresPlacement: %@", iconIgnoresPlacement);

    auto mbglValue = MLNStyleValueTransformer<bool, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<bool>>(iconIgnoresPlacement, false);
    self.rawLayer->setIconIgnorePlacement(mbglValue);
}

- (NSExpression *)iconIgnoresPlacement {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconIgnorePlacement();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconIgnorePlacement();
    }
    return MLNStyleValueTransformer<bool, NSNumber *>().toExpression(propertyValue);
}

- (void)setIconIgnorePlacement:(NSExpression *)iconIgnorePlacement {
}

- (NSExpression *)iconIgnorePlacement {
    return self.iconIgnoresPlacement;
}

- (void)setIconImageName:(NSExpression *)iconImageName {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconImageName: %@", iconImageName);

    if (iconImageName && iconImageName.expressionType == NSConstantValueExpressionType) {
        std::string string = ((NSString *)iconImageName.constantValue).UTF8String;
        if (mbgl::style::conversion::hasTokens(string)) {
            self.rawLayer->setIconImage(mbgl::style::PropertyValue<mbgl::style::expression::Image>(
                mbgl::style::conversion::convertTokenStringToImageExpression(string)));
            return;
        }
    }
    auto mbglValue = MLNStyleValueTransformer<mbgl::style::expression::Image, NSString *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::expression::Image>>(iconImageName, true);
    self.rawLayer->setIconImage(mbglValue);
}

- (NSExpression *)iconImageName {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconImage();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconImage();
    }
    return MLNStyleValueTransformer<mbgl::style::expression::Image, NSString *>().toExpression(propertyValue);
}

- (void)setIconImage:(NSExpression *)iconImage {
}

- (NSExpression *)iconImage {
    return self.iconImageName;
}

- (void)setIconOffset:(NSExpression *)iconOffset {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconOffset: %@", iconOffset);

    auto mbglValue = MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toPropertyValue<mbgl::style::PropertyValue<std::array<float, 2>>>(iconOffset, true);
    self.rawLayer->setIconOffset(mbglValue);
}

- (NSExpression *)iconOffset {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconOffset();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconOffset();
    }
    return MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toExpression(propertyValue);
}

- (void)setIconOptional:(NSExpression *)iconOptional {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconOptional: %@", iconOptional);

    auto mbglValue = MLNStyleValueTransformer<bool, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<bool>>(iconOptional, false);
    self.rawLayer->setIconOptional(mbglValue);
}

- (NSExpression *)isIconOptional {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconOptional();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconOptional();
    }
    return MLNStyleValueTransformer<bool, NSNumber *>().toExpression(propertyValue);
}

- (void)setIconPadding:(NSExpression *)iconPadding {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconPadding: %@", iconPadding);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Padding, NSValue *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Padding>>(iconPadding, true);
    self.rawLayer->setIconPadding(mbglValue);
}

- (NSExpression *)iconPadding {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconPadding();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconPadding();
    }
    return MLNStyleValueTransformer<mbgl::Padding, NSValue *>().toExpression(propertyValue);
}

- (void)setIconPitchAlignment:(NSExpression *)iconPitchAlignment {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconPitchAlignment: %@", iconPitchAlignment);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::AlignmentType, NSValue *, mbgl::style::AlignmentType, MLNIconPitchAlignment>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::AlignmentType>>(iconPitchAlignment, false);
    self.rawLayer->setIconPitchAlignment(mbglValue);
}

- (NSExpression *)iconPitchAlignment {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconPitchAlignment();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconPitchAlignment();
    }
    return MLNStyleValueTransformer<mbgl::style::AlignmentType, NSValue *, mbgl::style::AlignmentType, MLNIconPitchAlignment>().toExpression(propertyValue);
}

- (void)setIconRotation:(NSExpression *)iconRotation {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconRotation: %@", iconRotation);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(iconRotation, true);
    self.rawLayer->setIconRotate(mbglValue);
}

- (NSExpression *)iconRotation {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconRotate();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconRotate();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setIconRotate:(NSExpression *)iconRotate {
}

- (NSExpression *)iconRotate {
    return self.iconRotation;
}

- (void)setIconRotationAlignment:(NSExpression *)iconRotationAlignment {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconRotationAlignment: %@", iconRotationAlignment);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::AlignmentType, NSValue *, mbgl::style::AlignmentType, MLNIconRotationAlignment>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::AlignmentType>>(iconRotationAlignment, false);
    self.rawLayer->setIconRotationAlignment(mbglValue);
}

- (NSExpression *)iconRotationAlignment {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconRotationAlignment();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconRotationAlignment();
    }
    return MLNStyleValueTransformer<mbgl::style::AlignmentType, NSValue *, mbgl::style::AlignmentType, MLNIconRotationAlignment>().toExpression(propertyValue);
}

- (void)setIconScale:(NSExpression *)iconScale {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconScale: %@", iconScale);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(iconScale, true);
    self.rawLayer->setIconSize(mbglValue);
}

- (NSExpression *)iconScale {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconSize();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconSize();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setIconSize:(NSExpression *)iconSize {
}

- (NSExpression *)iconSize {
    return self.iconScale;
}

- (void)setIconTextFit:(NSExpression *)iconTextFit {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconTextFit: %@", iconTextFit);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::IconTextFitType, NSValue *, mbgl::style::IconTextFitType, MLNIconTextFit>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::IconTextFitType>>(iconTextFit, false);
    self.rawLayer->setIconTextFit(mbglValue);
}

- (NSExpression *)iconTextFit {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconTextFit();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconTextFit();
    }
    return MLNStyleValueTransformer<mbgl::style::IconTextFitType, NSValue *, mbgl::style::IconTextFitType, MLNIconTextFit>().toExpression(propertyValue);
}

- (void)setIconTextFitPadding:(NSExpression *)iconTextFitPadding {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconTextFitPadding: %@", iconTextFitPadding);

    auto mbglValue = MLNStyleValueTransformer<std::array<float, 4>, NSValue *>().toPropertyValue<mbgl::style::PropertyValue<std::array<float, 4>>>(iconTextFitPadding, false);
    self.rawLayer->setIconTextFitPadding(mbglValue);
}

- (NSExpression *)iconTextFitPadding {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconTextFitPadding();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconTextFitPadding();
    }
    return MLNStyleValueTransformer<std::array<float, 4>, NSValue *>().toExpression(propertyValue);
}

- (void)setKeepsIconUpright:(NSExpression *)keepsIconUpright {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting keepsIconUpright: %@", keepsIconUpright);

    auto mbglValue = MLNStyleValueTransformer<bool, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<bool>>(keepsIconUpright, false);
    self.rawLayer->setIconKeepUpright(mbglValue);
}

- (NSExpression *)keepsIconUpright {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconKeepUpright();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconKeepUpright();
    }
    return MLNStyleValueTransformer<bool, NSNumber *>().toExpression(propertyValue);
}

- (void)setIconKeepUpright:(NSExpression *)iconKeepUpright {
}

- (NSExpression *)iconKeepUpright {
    return self.keepsIconUpright;
}

- (void)setKeepsTextUpright:(NSExpression *)keepsTextUpright {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting keepsTextUpright: %@", keepsTextUpright);

    auto mbglValue = MLNStyleValueTransformer<bool, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<bool>>(keepsTextUpright, false);
    self.rawLayer->setTextKeepUpright(mbglValue);
}

- (NSExpression *)keepsTextUpright {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextKeepUpright();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextKeepUpright();
    }
    return MLNStyleValueTransformer<bool, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextKeepUpright:(NSExpression *)textKeepUpright {
}

- (NSExpression *)textKeepUpright {
    return self.keepsTextUpright;
}

- (void)setMaximumTextAngle:(NSExpression *)maximumTextAngle {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting maximumTextAngle: %@", maximumTextAngle);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(maximumTextAngle, false);
    self.rawLayer->setTextMaxAngle(mbglValue);
}

- (NSExpression *)maximumTextAngle {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextMaxAngle();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextMaxAngle();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextMaxAngle:(NSExpression *)textMaxAngle {
}

- (NSExpression *)textMaxAngle {
    return self.maximumTextAngle;
}

- (void)setMaximumTextWidth:(NSExpression *)maximumTextWidth {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting maximumTextWidth: %@", maximumTextWidth);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(maximumTextWidth, true);
    self.rawLayer->setTextMaxWidth(mbglValue);
}

- (NSExpression *)maximumTextWidth {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextMaxWidth();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextMaxWidth();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextMaxWidth:(NSExpression *)textMaxWidth {
}

- (NSExpression *)textMaxWidth {
    return self.maximumTextWidth;
}

- (void)setSymbolAvoidsEdges:(NSExpression *)symbolAvoidsEdges {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting symbolAvoidsEdges: %@", symbolAvoidsEdges);

    auto mbglValue = MLNStyleValueTransformer<bool, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<bool>>(symbolAvoidsEdges, false);
    self.rawLayer->setSymbolAvoidEdges(mbglValue);
}

- (NSExpression *)symbolAvoidsEdges {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getSymbolAvoidEdges();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultSymbolAvoidEdges();
    }
    return MLNStyleValueTransformer<bool, NSNumber *>().toExpression(propertyValue);
}

- (void)setSymbolAvoidEdges:(NSExpression *)symbolAvoidEdges {
}

- (NSExpression *)symbolAvoidEdges {
    return self.symbolAvoidsEdges;
}

- (void)setSymbolPlacement:(NSExpression *)symbolPlacement {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting symbolPlacement: %@", symbolPlacement);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::SymbolPlacementType, NSValue *, mbgl::style::SymbolPlacementType, MLNSymbolPlacement>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::SymbolPlacementType>>(symbolPlacement, false);
    self.rawLayer->setSymbolPlacement(mbglValue);
}

- (NSExpression *)symbolPlacement {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getSymbolPlacement();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultSymbolPlacement();
    }
    return MLNStyleValueTransformer<mbgl::style::SymbolPlacementType, NSValue *, mbgl::style::SymbolPlacementType, MLNSymbolPlacement>().toExpression(propertyValue);
}

- (void)setSymbolScreenSpace:(NSExpression *)symbolScreenSpace {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting symbolScreenSpace: %@", symbolScreenSpace);

    auto mbglValue = MLNStyleValueTransformer<bool, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<bool>>(symbolScreenSpace, false);
    self.rawLayer->setSymbolScreenSpace(mbglValue);
}

- (NSExpression *)symbolScreenSpace {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getSymbolScreenSpace();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultSymbolScreenSpace();
    }
    return MLNStyleValueTransformer<bool, NSNumber *>().toExpression(propertyValue);
}

- (void)setSymbolSortKey:(NSExpression *)symbolSortKey {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting symbolSortKey: %@", symbolSortKey);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(symbolSortKey, true);
    self.rawLayer->setSymbolSortKey(mbglValue);
}

- (NSExpression *)symbolSortKey {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getSymbolSortKey();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultSymbolSortKey();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setSymbolSpacing:(NSExpression *)symbolSpacing {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting symbolSpacing: %@", symbolSpacing);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(symbolSpacing, false);
    self.rawLayer->setSymbolSpacing(mbglValue);
}

- (NSExpression *)symbolSpacing {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getSymbolSpacing();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultSymbolSpacing();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setSymbolZOrder:(NSExpression *)symbolZOrder {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting symbolZOrder: %@", symbolZOrder);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::SymbolZOrderType, NSValue *, mbgl::style::SymbolZOrderType, MLNSymbolZOrder>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::SymbolZOrderType>>(symbolZOrder, false);
    self.rawLayer->setSymbolZOrder(mbglValue);
}

- (NSExpression *)symbolZOrder {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getSymbolZOrder();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultSymbolZOrder();
    }
    return MLNStyleValueTransformer<mbgl::style::SymbolZOrderType, NSValue *, mbgl::style::SymbolZOrderType, MLNSymbolZOrder>().toExpression(propertyValue);
}

- (void)setText:(NSExpression *)text {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting text: %@", text);

    if (text && text.expressionType == NSConstantValueExpressionType) {
        std::string string = ((NSString *)text.constantValue).UTF8String;
        if (mbgl::style::conversion::hasTokens(string)) {
            self.rawLayer->setTextField(mbgl::style::PropertyValue<mbgl::style::expression::Formatted>(
                mbgl::style::conversion::convertTokenStringToFormatExpression(string)));
            return;
        }
    }
    auto mbglValue = MLNStyleValueTransformer<mbgl::style::expression::Formatted, NSString *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::expression::Formatted>>(text, true);
    self.rawLayer->setTextField(mbglValue);
}

- (NSExpression *)text {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextField();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextField();
    }
    return MLNStyleValueTransformer<mbgl::style::expression::Formatted, NSString *>().toExpression(propertyValue);
}

- (void)setTextField:(NSExpression *)textField {
}

- (NSExpression *)textField {
    return self.text;
}

- (void)setTextAllowsOverlap:(NSExpression *)textAllowsOverlap {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textAllowsOverlap: %@", textAllowsOverlap);

    auto mbglValue = MLNStyleValueTransformer<bool, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<bool>>(textAllowsOverlap, false);
    self.rawLayer->setTextAllowOverlap(mbglValue);
}

- (NSExpression *)textAllowsOverlap {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextAllowOverlap();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextAllowOverlap();
    }
    return MLNStyleValueTransformer<bool, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextAllowOverlap:(NSExpression *)textAllowOverlap {
}

- (NSExpression *)textAllowOverlap {
    return self.textAllowsOverlap;
}

- (void)setTextAnchor:(NSExpression *)textAnchor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textAnchor: %@", textAnchor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::SymbolAnchorType, NSValue *, mbgl::style::SymbolAnchorType, MLNTextAnchor>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::SymbolAnchorType>>(textAnchor, true);
    self.rawLayer->setTextAnchor(mbglValue);
}

- (NSExpression *)textAnchor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextAnchor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextAnchor();
    }
    return MLNStyleValueTransformer<mbgl::style::SymbolAnchorType, NSValue *, mbgl::style::SymbolAnchorType, MLNTextAnchor>().toExpression(propertyValue);
}

- (void)setTextFontNames:(NSExpression *)textFontNames {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textFontNames: %@", textFontNames);

    auto mbglValue = MLNStyleValueTransformer<std::vector<std::string>, NSArray<NSString *> *, std::string>().toPropertyValue<mbgl::style::PropertyValue<std::vector<std::string>>>(textFontNames, true);
    self.rawLayer->setTextFont(mbglValue);
}

- (NSExpression *)textFontNames {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextFont();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextFont();
    }
    return MLNStyleValueTransformer<std::vector<std::string>, NSArray<NSString *> *, std::string>().toExpression(propertyValue);
}

- (void)setTextFont:(NSExpression *)textFont {
}

- (NSExpression *)textFont {
    return self.textFontNames;
}

- (void)setTextFontSize:(NSExpression *)textFontSize {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textFontSize: %@", textFontSize);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(textFontSize, true);
    self.rawLayer->setTextSize(mbglValue);
}

- (NSExpression *)textFontSize {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextSize();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextSize();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextSize:(NSExpression *)textSize {
}

- (NSExpression *)textSize {
    return self.textFontSize;
}

- (void)setTextIgnoresPlacement:(NSExpression *)textIgnoresPlacement {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textIgnoresPlacement: %@", textIgnoresPlacement);

    auto mbglValue = MLNStyleValueTransformer<bool, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<bool>>(textIgnoresPlacement, false);
    self.rawLayer->setTextIgnorePlacement(mbglValue);
}

- (NSExpression *)textIgnoresPlacement {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextIgnorePlacement();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextIgnorePlacement();
    }
    return MLNStyleValueTransformer<bool, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextIgnorePlacement:(NSExpression *)textIgnorePlacement {
}

- (NSExpression *)textIgnorePlacement {
    return self.textIgnoresPlacement;
}

- (void)setTextJustification:(NSExpression *)textJustification {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textJustification: %@", textJustification);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::TextJustifyType, NSValue *, mbgl::style::TextJustifyType, MLNTextJustification>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::TextJustifyType>>(textJustification, true);
    self.rawLayer->setTextJustify(mbglValue);
}

- (NSExpression *)textJustification {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextJustify();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextJustify();
    }
    return MLNStyleValueTransformer<mbgl::style::TextJustifyType, NSValue *, mbgl::style::TextJustifyType, MLNTextJustification>().toExpression(propertyValue);
}

- (void)setTextJustify:(NSExpression *)textJustify {
}

- (NSExpression *)textJustify {
    return self.textJustification;
}

- (void)setTextLetterSpacing:(NSExpression *)textLetterSpacing {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textLetterSpacing: %@", textLetterSpacing);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(textLetterSpacing, true);
    self.rawLayer->setTextLetterSpacing(mbglValue);
}

- (NSExpression *)textLetterSpacing {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextLetterSpacing();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextLetterSpacing();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextLineHeight:(NSExpression *)textLineHeight {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textLineHeight: %@", textLineHeight);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(textLineHeight, false);
    self.rawLayer->setTextLineHeight(mbglValue);
}

- (NSExpression *)textLineHeight {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextLineHeight();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextLineHeight();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextOffset:(NSExpression *)textOffset {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textOffset: %@", textOffset);

    auto mbglValue = MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toPropertyValue<mbgl::style::PropertyValue<std::array<float, 2>>>(textOffset, true);
    self.rawLayer->setTextOffset(mbglValue);
}

- (NSExpression *)textOffset {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextOffset();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextOffset();
    }
    return MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toExpression(propertyValue);
}

- (void)setTextOptional:(NSExpression *)textOptional {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textOptional: %@", textOptional);

    auto mbglValue = MLNStyleValueTransformer<bool, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<bool>>(textOptional, false);
    self.rawLayer->setTextOptional(mbglValue);
}

- (NSExpression *)isTextOptional {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextOptional();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextOptional();
    }
    return MLNStyleValueTransformer<bool, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextPadding:(NSExpression *)textPadding {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textPadding: %@", textPadding);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(textPadding, false);
    self.rawLayer->setTextPadding(mbglValue);
}

- (NSExpression *)textPadding {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextPadding();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextPadding();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextPitchAlignment:(NSExpression *)textPitchAlignment {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textPitchAlignment: %@", textPitchAlignment);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::AlignmentType, NSValue *, mbgl::style::AlignmentType, MLNTextPitchAlignment>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::AlignmentType>>(textPitchAlignment, false);
    self.rawLayer->setTextPitchAlignment(mbglValue);
}

- (NSExpression *)textPitchAlignment {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextPitchAlignment();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextPitchAlignment();
    }
    return MLNStyleValueTransformer<mbgl::style::AlignmentType, NSValue *, mbgl::style::AlignmentType, MLNTextPitchAlignment>().toExpression(propertyValue);
}

- (void)setTextRadialOffset:(NSExpression *)textRadialOffset {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textRadialOffset: %@", textRadialOffset);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(textRadialOffset, true);
    self.rawLayer->setTextRadialOffset(mbglValue);
}

- (NSExpression *)textRadialOffset {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextRadialOffset();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextRadialOffset();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextRotation:(NSExpression *)textRotation {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textRotation: %@", textRotation);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(textRotation, true);
    self.rawLayer->setTextRotate(mbglValue);
}

- (NSExpression *)textRotation {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextRotate();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextRotate();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextRotate:(NSExpression *)textRotate {
}

- (NSExpression *)textRotate {
    return self.textRotation;
}

- (void)setTextRotationAlignment:(NSExpression *)textRotationAlignment {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textRotationAlignment: %@", textRotationAlignment);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::AlignmentType, NSValue *, mbgl::style::AlignmentType, MLNTextRotationAlignment>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::AlignmentType>>(textRotationAlignment, false);
    self.rawLayer->setTextRotationAlignment(mbglValue);
}

- (NSExpression *)textRotationAlignment {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextRotationAlignment();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextRotationAlignment();
    }
    return MLNStyleValueTransformer<mbgl::style::AlignmentType, NSValue *, mbgl::style::AlignmentType, MLNTextRotationAlignment>().toExpression(propertyValue);
}

- (void)setTextTransform:(NSExpression *)textTransform {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textTransform: %@", textTransform);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::TextTransformType, NSValue *, mbgl::style::TextTransformType, MLNTextTransform>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::TextTransformType>>(textTransform, true);
    self.rawLayer->setTextTransform(mbglValue);
}

- (NSExpression *)textTransform {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextTransform();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextTransform();
    }
    return MLNStyleValueTransformer<mbgl::style::TextTransformType, NSValue *, mbgl::style::TextTransformType, MLNTextTransform>().toExpression(propertyValue);
}

- (void)setTextVariableAnchor:(NSExpression *)textVariableAnchor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textVariableAnchor: %@", textVariableAnchor);

    auto mbglValue = MLNStyleValueTransformer<std::vector<mbgl::style::SymbolAnchorType>, NSArray<NSValue *> *, mbgl::style::SymbolAnchorType, MLNTextAnchor>().toPropertyValue<mbgl::style::PropertyValue<std::vector<mbgl::style::SymbolAnchorType>>>(textVariableAnchor, false);
    self.rawLayer->setTextVariableAnchor(mbglValue);
}

- (NSExpression *)textVariableAnchor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextVariableAnchor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextVariableAnchor();
    }
    return MLNStyleValueTransformer<std::vector<mbgl::style::SymbolAnchorType>, NSArray<NSValue *> *, mbgl::style::SymbolAnchorType, MLNTextAnchor>().toExpression(propertyValue);
}

- (void)setTextVariableAnchorOffset:(NSExpression *)textVariableAnchorOffset {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textVariableAnchorOffset: %@", textVariableAnchorOffset);

    auto mbglValue = MLNStyleValueTransformer<mbgl::VariableAnchorOffsetCollection, NSArray<NSValue *> *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::VariableAnchorOffsetCollection>>(textVariableAnchorOffset, true);
    self.rawLayer->setTextVariableAnchorOffset(mbglValue);
}

- (NSExpression *)textVariableAnchorOffset {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextVariableAnchorOffset();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextVariableAnchorOffset();
    }
    return MLNStyleValueTransformer<mbgl::VariableAnchorOffsetCollection, NSArray<NSValue *> *>().toExpression(propertyValue);
}

- (void)setTextWritingModes:(NSExpression *)textWritingModes {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textWritingModes: %@", textWritingModes);

    auto mbglValue = MLNStyleValueTransformer<std::vector<mbgl::style::TextWritingModeType>, NSArray<NSValue *> *, mbgl::style::TextWritingModeType, MLNTextWritingMode>().toPropertyValue<mbgl::style::PropertyValue<std::vector<mbgl::style::TextWritingModeType>>>(textWritingModes, false);
    self.rawLayer->setTextWritingMode(mbglValue);
}

- (NSExpression *)textWritingModes {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextWritingMode();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextWritingMode();
    }
    return MLNStyleValueTransformer<std::vector<mbgl::style::TextWritingModeType>, NSArray<NSValue *> *, mbgl::style::TextWritingModeType, MLNTextWritingMode>().toExpression(propertyValue);
}

- (void)setTextWritingMode:(NSExpression *)textWritingMode {
}

- (NSExpression *)textWritingMode {
    return self.textWritingModes;
}

// MARK: - Accessing the Paint Attributes

- (void)setIconColor:(NSExpression *)iconColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconColor: %@", iconColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(iconColor, true);
    self.rawLayer->setIconColor(mbglValue);
}

- (NSExpression *)iconColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setIconColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setIconColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)iconColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getIconColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setIconHaloBlur:(NSExpression *)iconHaloBlur {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconHaloBlur: %@", iconHaloBlur);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(iconHaloBlur, true);
    self.rawLayer->setIconHaloBlur(mbglValue);
}

- (NSExpression *)iconHaloBlur {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconHaloBlur();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconHaloBlur();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setIconHaloBlurTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconHaloBlurTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setIconHaloBlurTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)iconHaloBlurTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getIconHaloBlurTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setIconHaloColor:(NSExpression *)iconHaloColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconHaloColor: %@", iconHaloColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(iconHaloColor, true);
    self.rawLayer->setIconHaloColor(mbglValue);
}

- (NSExpression *)iconHaloColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconHaloColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconHaloColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setIconHaloColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconHaloColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setIconHaloColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)iconHaloColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getIconHaloColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setIconHaloWidth:(NSExpression *)iconHaloWidth {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconHaloWidth: %@", iconHaloWidth);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(iconHaloWidth, true);
    self.rawLayer->setIconHaloWidth(mbglValue);
}

- (NSExpression *)iconHaloWidth {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconHaloWidth();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconHaloWidth();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setIconHaloWidthTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconHaloWidthTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setIconHaloWidthTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)iconHaloWidthTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getIconHaloWidthTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setIconOpacity:(NSExpression *)iconOpacity {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconOpacity: %@", iconOpacity);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(iconOpacity, true);
    self.rawLayer->setIconOpacity(mbglValue);
}

- (NSExpression *)iconOpacity {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconOpacity();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconOpacity();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setIconOpacityTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconOpacityTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setIconOpacityTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)iconOpacityTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getIconOpacityTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setIconTranslation:(NSExpression *)iconTranslation {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconTranslation: %@", iconTranslation);

    auto mbglValue = MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toPropertyValue<mbgl::style::PropertyValue<std::array<float, 2>>>(iconTranslation, false);
    self.rawLayer->setIconTranslate(mbglValue);
}

- (NSExpression *)iconTranslation {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconTranslate();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconTranslate();
    }
    return MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toExpression(propertyValue);
}

- (void)setIconTranslationTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconTranslationTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setIconTranslateTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)iconTranslationTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getIconTranslateTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setIconTranslate:(NSExpression *)iconTranslate {
}

- (NSExpression *)iconTranslate {
    return self.iconTranslation;
}

- (void)setIconTranslationAnchor:(NSExpression *)iconTranslationAnchor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting iconTranslationAnchor: %@", iconTranslationAnchor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::TranslateAnchorType, NSValue *, mbgl::style::TranslateAnchorType, MLNIconTranslationAnchor>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::TranslateAnchorType>>(iconTranslationAnchor, false);
    self.rawLayer->setIconTranslateAnchor(mbglValue);
}

- (NSExpression *)iconTranslationAnchor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getIconTranslateAnchor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultIconTranslateAnchor();
    }
    return MLNStyleValueTransformer<mbgl::style::TranslateAnchorType, NSValue *, mbgl::style::TranslateAnchorType, MLNIconTranslationAnchor>().toExpression(propertyValue);
}

- (void)setIconTranslateAnchor:(NSExpression *)iconTranslateAnchor {
}

- (NSExpression *)iconTranslateAnchor {
    return self.iconTranslationAnchor;
}

- (void)setTextColor:(NSExpression *)textColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textColor: %@", textColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(textColor, true);
    self.rawLayer->setTextColor(mbglValue);
}

- (NSExpression *)textColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setTextColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setTextColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)textColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getTextColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setTextHaloBlur:(NSExpression *)textHaloBlur {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textHaloBlur: %@", textHaloBlur);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(textHaloBlur, true);
    self.rawLayer->setTextHaloBlur(mbglValue);
}

- (NSExpression *)textHaloBlur {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextHaloBlur();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextHaloBlur();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextHaloBlurTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textHaloBlurTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setTextHaloBlurTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)textHaloBlurTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getTextHaloBlurTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setTextHaloColor:(NSExpression *)textHaloColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textHaloColor: %@", textHaloColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(textHaloColor, true);
    self.rawLayer->setTextHaloColor(mbglValue);
}

- (NSExpression *)textHaloColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextHaloColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextHaloColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setTextHaloColorTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textHaloColorTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setTextHaloColorTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)textHaloColorTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getTextHaloColorTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setTextHaloWidth:(NSExpression *)textHaloWidth {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textHaloWidth: %@", textHaloWidth);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(textHaloWidth, true);
    self.rawLayer->setTextHaloWidth(mbglValue);
}

- (NSExpression *)textHaloWidth {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextHaloWidth();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextHaloWidth();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextHaloWidthTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textHaloWidthTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setTextHaloWidthTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)textHaloWidthTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getTextHaloWidthTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setTextOpacity:(NSExpression *)textOpacity {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textOpacity: %@", textOpacity);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(textOpacity, true);
    self.rawLayer->setTextOpacity(mbglValue);
}

- (NSExpression *)textOpacity {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextOpacity();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextOpacity();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setTextOpacityTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textOpacityTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setTextOpacityTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)textOpacityTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getTextOpacityTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setTextTranslation:(NSExpression *)textTranslation {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textTranslation: %@", textTranslation);

    auto mbglValue = MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toPropertyValue<mbgl::style::PropertyValue<std::array<float, 2>>>(textTranslation, false);
    self.rawLayer->setTextTranslate(mbglValue);
}

- (NSExpression *)textTranslation {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextTranslate();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextTranslate();
    }
    return MLNStyleValueTransformer<std::array<float, 2>, NSValue *>().toExpression(propertyValue);
}

- (void)setTextTranslationTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textTranslationTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setTextTranslateTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)textTranslationTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getTextTranslateTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setTextTranslate:(NSExpression *)textTranslate {
}

- (NSExpression *)textTranslate {
    return self.textTranslation;
}

- (void)setTextTranslationAnchor:(NSExpression *)textTranslationAnchor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting textTranslationAnchor: %@", textTranslationAnchor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::TranslateAnchorType, NSValue *, mbgl::style::TranslateAnchorType, MLNTextTranslationAnchor>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::TranslateAnchorType>>(textTranslationAnchor, false);
    self.rawLayer->setTextTranslateAnchor(mbglValue);
}

- (NSExpression *)textTranslationAnchor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getTextTranslateAnchor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultTextTranslateAnchor();
    }
    return MLNStyleValueTransformer<mbgl::style::TranslateAnchorType, NSValue *, mbgl::style::TranslateAnchorType, MLNTextTranslationAnchor>().toExpression(propertyValue);
}

- (void)setTextTranslateAnchor:(NSExpression *)textTranslateAnchor {
}

- (NSExpression *)textTranslateAnchor {
    return self.textTranslationAnchor;
}

@end

@implementation NSValue (MLNSymbolStyleLayerAdditions)

+ (NSValue *)valueWithMLNIconAnchor:(MLNIconAnchor)iconAnchor {
    return [NSValue value:&iconAnchor withObjCType:@encode(MLNIconAnchor)];
}

- (MLNIconAnchor)MLNIconAnchorValue {
    MLNIconAnchor iconAnchor;
    [self getValue:&iconAnchor];
    return iconAnchor;
}

+ (NSValue *)valueWithMLNIconPitchAlignment:(MLNIconPitchAlignment)iconPitchAlignment {
    return [NSValue value:&iconPitchAlignment withObjCType:@encode(MLNIconPitchAlignment)];
}

- (MLNIconPitchAlignment)MLNIconPitchAlignmentValue {
    MLNIconPitchAlignment iconPitchAlignment;
    [self getValue:&iconPitchAlignment];
    return iconPitchAlignment;
}

+ (NSValue *)valueWithMLNIconRotationAlignment:(MLNIconRotationAlignment)iconRotationAlignment {
    return [NSValue value:&iconRotationAlignment withObjCType:@encode(MLNIconRotationAlignment)];
}

- (MLNIconRotationAlignment)MLNIconRotationAlignmentValue {
    MLNIconRotationAlignment iconRotationAlignment;
    [self getValue:&iconRotationAlignment];
    return iconRotationAlignment;
}

+ (NSValue *)valueWithMLNIconTextFit:(MLNIconTextFit)iconTextFit {
    return [NSValue value:&iconTextFit withObjCType:@encode(MLNIconTextFit)];
}

- (MLNIconTextFit)MLNIconTextFitValue {
    MLNIconTextFit iconTextFit;
    [self getValue:&iconTextFit];
    return iconTextFit;
}

+ (NSValue *)valueWithMLNSymbolPlacement:(MLNSymbolPlacement)symbolPlacement {
    return [NSValue value:&symbolPlacement withObjCType:@encode(MLNSymbolPlacement)];
}

- (MLNSymbolPlacement)MLNSymbolPlacementValue {
    MLNSymbolPlacement symbolPlacement;
    [self getValue:&symbolPlacement];
    return symbolPlacement;
}

+ (NSValue *)valueWithMLNSymbolZOrder:(MLNSymbolZOrder)symbolZOrder {
    return [NSValue value:&symbolZOrder withObjCType:@encode(MLNSymbolZOrder)];
}

- (MLNSymbolZOrder)MLNSymbolZOrderValue {
    MLNSymbolZOrder symbolZOrder;
    [self getValue:&symbolZOrder];
    return symbolZOrder;
}

+ (NSValue *)valueWithMLNTextAnchor:(MLNTextAnchor)textAnchor {
    return [NSValue value:&textAnchor withObjCType:@encode(MLNTextAnchor)];
}

- (MLNTextAnchor)MLNTextAnchorValue {
    MLNTextAnchor textAnchor;
    [self getValue:&textAnchor];
    return textAnchor;
}

+ (NSValue *)valueWithMLNTextJustification:(MLNTextJustification)textJustification {
    return [NSValue value:&textJustification withObjCType:@encode(MLNTextJustification)];
}

- (MLNTextJustification)MLNTextJustificationValue {
    MLNTextJustification textJustification;
    [self getValue:&textJustification];
    return textJustification;
}

+ (NSValue *)valueWithMLNTextPitchAlignment:(MLNTextPitchAlignment)textPitchAlignment {
    return [NSValue value:&textPitchAlignment withObjCType:@encode(MLNTextPitchAlignment)];
}

- (MLNTextPitchAlignment)MLNTextPitchAlignmentValue {
    MLNTextPitchAlignment textPitchAlignment;
    [self getValue:&textPitchAlignment];
    return textPitchAlignment;
}

+ (NSValue *)valueWithMLNTextRotationAlignment:(MLNTextRotationAlignment)textRotationAlignment {
    return [NSValue value:&textRotationAlignment withObjCType:@encode(MLNTextRotationAlignment)];
}

- (MLNTextRotationAlignment)MLNTextRotationAlignmentValue {
    MLNTextRotationAlignment textRotationAlignment;
    [self getValue:&textRotationAlignment];
    return textRotationAlignment;
}

+ (NSValue *)valueWithMLNTextTransform:(MLNTextTransform)textTransform {
    return [NSValue value:&textTransform withObjCType:@encode(MLNTextTransform)];
}

- (MLNTextTransform)MLNTextTransformValue {
    MLNTextTransform textTransform;
    [self getValue:&textTransform];
    return textTransform;
}

+ (NSValue *)valueWithMLNTextWritingMode:(MLNTextWritingMode)textWritingModes {
    return [NSValue value:&textWritingModes withObjCType:@encode(MLNTextWritingMode)];
}

- (MLNTextWritingMode)MLNTextWritingModeValue {
    MLNTextWritingMode textWritingModes;
    [self getValue:&textWritingModes];
    return textWritingModes;
}

+ (NSValue *)valueWithMLNIconTranslationAnchor:(MLNIconTranslationAnchor)iconTranslationAnchor {
    return [NSValue value:&iconTranslationAnchor withObjCType:@encode(MLNIconTranslationAnchor)];
}

- (MLNIconTranslationAnchor)MLNIconTranslationAnchorValue {
    MLNIconTranslationAnchor iconTranslationAnchor;
    [self getValue:&iconTranslationAnchor];
    return iconTranslationAnchor;
}

+ (NSValue *)valueWithMLNTextTranslationAnchor:(MLNTextTranslationAnchor)textTranslationAnchor {
    return [NSValue value:&textTranslationAnchor withObjCType:@encode(MLNTextTranslationAnchor)];
}

- (MLNTextTranslationAnchor)MLNTextTranslationAnchorValue {
    MLNTextTranslationAnchor textTranslationAnchor;
    [self getValue:&textTranslationAnchor];
    return textTranslationAnchor;
}

@end

namespace mbgl {

MLNStyleLayer* SymbolStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNSymbolStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
