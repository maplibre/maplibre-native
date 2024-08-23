// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNSource.h"
#import "NSPredicate+MLNPrivateAdditions.h"
#import "NSDate+MLNAdditions.h"
#import "MLNStyleLayer_Private.h"
#import "MLNStyleValue_Private.h"
#import "MLNHeatmapStyleLayer.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNHeatmapStyleLayer_Private.h"

#include <mbgl/style/layers/heatmap_layer.hpp>
#include <mbgl/style/transition_options.hpp>


@interface MLNHeatmapStyleLayer ()

@property (nonatomic, readonly) mbgl::style::HeatmapLayer *rawLayer;

@end

@implementation MLNHeatmapStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier source:(MLNSource *)source
{
    MLNLogDebug(@"Initializing %@ with identifier: %@ source: %@", NSStringFromClass([self class]), identifier, source);
    auto layer = std::make_unique<mbgl::style::HeatmapLayer>(identifier.UTF8String, source.identifier.UTF8String);
    return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::HeatmapLayer *)rawLayer
{
    return (mbgl::style::HeatmapLayer *)super.rawLayer;
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

- (void)setHeatmapColor:(NSExpression *)heatmapColor {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting heatmapColor: %@", heatmapColor);

    auto mbglValue = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::ColorRampPropertyValue>(heatmapColor);
    self.rawLayer->setHeatmapColor(mbglValue);
}

- (NSExpression *)heatmapColor {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getHeatmapColor();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultHeatmapColor();
    }
    return MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(propertyValue);
}

- (void)setHeatmapIntensity:(NSExpression *)heatmapIntensity {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting heatmapIntensity: %@", heatmapIntensity);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(heatmapIntensity, false);
    self.rawLayer->setHeatmapIntensity(mbglValue);
}

- (NSExpression *)heatmapIntensity {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getHeatmapIntensity();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultHeatmapIntensity();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setHeatmapIntensityTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting heatmapIntensityTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setHeatmapIntensityTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)heatmapIntensityTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getHeatmapIntensityTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setHeatmapOpacity:(NSExpression *)heatmapOpacity {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting heatmapOpacity: %@", heatmapOpacity);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(heatmapOpacity, false);
    self.rawLayer->setHeatmapOpacity(mbglValue);
}

- (NSExpression *)heatmapOpacity {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getHeatmapOpacity();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultHeatmapOpacity();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setHeatmapOpacityTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting heatmapOpacityTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setHeatmapOpacityTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)heatmapOpacityTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getHeatmapOpacityTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setHeatmapRadius:(NSExpression *)heatmapRadius {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting heatmapRadius: %@", heatmapRadius);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(heatmapRadius, true);
    self.rawLayer->setHeatmapRadius(mbglValue);
}

- (NSExpression *)heatmapRadius {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getHeatmapRadius();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultHeatmapRadius();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setHeatmapRadiusTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting heatmapRadiusTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setHeatmapRadiusTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)heatmapRadiusTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getHeatmapRadiusTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setHeatmapWeight:(NSExpression *)heatmapWeight {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting heatmapWeight: %@", heatmapWeight);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(heatmapWeight, true);
    self.rawLayer->setHeatmapWeight(mbglValue);
}

- (NSExpression *)heatmapWeight {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getHeatmapWeight();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultHeatmapWeight();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

@end

namespace mbgl {

MLNStyleLayer* HeatmapStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNHeatmapStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
