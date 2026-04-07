// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MLNSource.h"
#import "NSPredicate+MLNPrivateAdditions.h"
#import "NSDate+MLNAdditions.h"
#import "MLNStyleLayer_Private.h"
#import "MLNStyleValue_Private.h"
#import "MLNRasterStyleLayer.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNRasterStyleLayer_Private.h"

#include <mbgl/style/layers/raster_layer.hpp>
#include <mbgl/style/transition_options.hpp>


namespace mbgl {

    MBGL_DEFINE_ENUM(MLNRasterResamplingMode, {
        { MLNRasterResamplingModeLinear, "linear" },
        { MLNRasterResamplingModeNearest, "nearest" },
    });

}

@interface MLNRasterStyleLayer ()

@property (nonatomic, readonly) mbgl::style::RasterLayer *rawLayer;

@end

@implementation MLNRasterStyleLayer

- (instancetype)initWithIdentifier:(NSString *)identifier source:(MLNSource *)source
{
    MLNLogDebug(@"Initializing %@ with identifier: %@ source: %@", NSStringFromClass([self class]), identifier, source);
    auto layer = std::make_unique<mbgl::style::RasterLayer>(identifier.UTF8String, source.identifier.UTF8String);
    return self = [super initWithPendingLayer:std::move(layer)];
}

- (mbgl::style::RasterLayer *)rawLayer
{
    return (mbgl::style::RasterLayer *)super.rawLayer;
}

- (NSString *)sourceIdentifier
{
    MLNAssertStyleLayerIsValid();

    return @(self.rawLayer->getSourceID().c_str());
}

// MARK: - Accessing the Paint Attributes

- (void)setMaximumRasterBrightness:(NSExpression *)maximumRasterBrightness {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting maximumRasterBrightness: %@", maximumRasterBrightness);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(maximumRasterBrightness, false);
    self.rawLayer->setRasterBrightnessMax(mbglValue);
}

- (NSExpression *)maximumRasterBrightness {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getRasterBrightnessMax();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultRasterBrightnessMax();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setMaximumRasterBrightnessTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting maximumRasterBrightnessTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setRasterBrightnessMaxTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)maximumRasterBrightnessTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getRasterBrightnessMaxTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setRasterBrightnessMax:(NSExpression *)rasterBrightnessMax {
}

- (NSExpression *)rasterBrightnessMax {
    return self.maximumRasterBrightness;
}

- (void)setMinimumRasterBrightness:(NSExpression *)minimumRasterBrightness {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting minimumRasterBrightness: %@", minimumRasterBrightness);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(minimumRasterBrightness, false);
    self.rawLayer->setRasterBrightnessMin(mbglValue);
}

- (NSExpression *)minimumRasterBrightness {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getRasterBrightnessMin();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultRasterBrightnessMin();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setMinimumRasterBrightnessTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting minimumRasterBrightnessTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setRasterBrightnessMinTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)minimumRasterBrightnessTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getRasterBrightnessMinTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setRasterBrightnessMin:(NSExpression *)rasterBrightnessMin {
}

- (NSExpression *)rasterBrightnessMin {
    return self.minimumRasterBrightness;
}

- (void)setRasterContrast:(NSExpression *)rasterContrast {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting rasterContrast: %@", rasterContrast);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(rasterContrast, false);
    self.rawLayer->setRasterContrast(mbglValue);
}

- (NSExpression *)rasterContrast {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getRasterContrast();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultRasterContrast();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setRasterContrastTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting rasterContrastTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setRasterContrastTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)rasterContrastTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getRasterContrastTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setRasterFadeDuration:(NSExpression *)rasterFadeDuration {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting rasterFadeDuration: %@", rasterFadeDuration);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(rasterFadeDuration, false);
    self.rawLayer->setRasterFadeDuration(mbglValue);
}

- (NSExpression *)rasterFadeDuration {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getRasterFadeDuration();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultRasterFadeDuration();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setRasterHueRotation:(NSExpression *)rasterHueRotation {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting rasterHueRotation: %@", rasterHueRotation);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(rasterHueRotation, false);
    self.rawLayer->setRasterHueRotate(mbglValue);
}

- (NSExpression *)rasterHueRotation {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getRasterHueRotate();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultRasterHueRotate();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setRasterHueRotationTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting rasterHueRotationTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setRasterHueRotateTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)rasterHueRotationTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getRasterHueRotateTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setRasterHueRotate:(NSExpression *)rasterHueRotate {
}

- (NSExpression *)rasterHueRotate {
    return self.rasterHueRotation;
}

- (void)setRasterOpacity:(NSExpression *)rasterOpacity {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting rasterOpacity: %@", rasterOpacity);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(rasterOpacity, false);
    self.rawLayer->setRasterOpacity(mbglValue);
}

- (NSExpression *)rasterOpacity {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getRasterOpacity();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultRasterOpacity();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setRasterOpacityTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting rasterOpacityTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setRasterOpacityTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)rasterOpacityTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getRasterOpacityTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setRasterResamplingMode:(NSExpression *)rasterResamplingMode {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting rasterResamplingMode: %@", rasterResamplingMode);

    auto mbglValue = MLNStyleValueTransformer<mbgl::style::RasterResamplingType, NSValue *, mbgl::style::RasterResamplingType, MLNRasterResamplingMode>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::RasterResamplingType>>(rasterResamplingMode, false);
    self.rawLayer->setRasterResampling(mbglValue);
}

- (NSExpression *)rasterResamplingMode {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getRasterResampling();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultRasterResampling();
    }
    return MLNStyleValueTransformer<mbgl::style::RasterResamplingType, NSValue *, mbgl::style::RasterResamplingType, MLNRasterResamplingMode>().toExpression(propertyValue);
}

- (void)setRasterResampling:(NSExpression *)rasterResampling {
}

- (NSExpression *)rasterResampling {
    return self.rasterResamplingMode;
}

- (void)setRasterSaturation:(NSExpression *)rasterSaturation {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting rasterSaturation: %@", rasterSaturation);

    auto mbglValue = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(rasterSaturation, false);
    self.rawLayer->setRasterSaturation(mbglValue);
}

- (NSExpression *)rasterSaturation {
    MLNAssertStyleLayerIsValid();

    auto propertyValue = self.rawLayer->getRasterSaturation();
    if (propertyValue.isUndefined()) {
        propertyValue = self.rawLayer->getDefaultRasterSaturation();
    }
    return MLNStyleValueTransformer<float, NSNumber *>().toExpression(propertyValue);
}

- (void)setRasterSaturationTransition:(MLNTransition )transition {
    MLNAssertStyleLayerIsValid();
    MLNLogDebug(@"Setting rasterSaturationTransition: %@", MLNStringFromMLNTransition(transition));

    self.rawLayer->setRasterSaturationTransition(MLNOptionsFromTransition(transition));
}

- (MLNTransition)rasterSaturationTransition {
    MLNAssertStyleLayerIsValid();

    mbgl::style::TransitionOptions transitionOptions = self.rawLayer->getRasterSaturationTransition();

    return MLNTransitionFromOptions(transitionOptions);
}

@end

@implementation NSValue (MLNRasterStyleLayerAdditions)

+ (NSValue *)valueWithMLNRasterResamplingMode:(MLNRasterResamplingMode)rasterResamplingMode {
    return [NSValue value:&rasterResamplingMode withObjCType:@encode(MLNRasterResamplingMode)];
}

- (MLNRasterResamplingMode)MLNRasterResamplingModeValue {
    MLNRasterResamplingMode rasterResamplingMode;
    [self getValue:&rasterResamplingMode];
    return rasterResamplingMode;
}

@end

namespace mbgl {

MLNStyleLayer* RasterStyleLayerPeerFactory::createPeer(style::Layer* rawLayer) {
    return [[MLNRasterStyleLayer alloc] initWithRawLayer:rawLayer];
}

}  // namespace mbgl
