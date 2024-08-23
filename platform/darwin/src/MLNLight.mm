// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.
// test

#import "MLNLight.h"

#import "MLNTypes.h"
#import "MLNStyleValue_Private.h"
#import "NSValue+MLNAdditions.h"
#import "MLNLoggingConfiguration_Private.h"

#import <mbgl/style/light.hpp>
#import <mbgl/style/types.hpp>

namespace mbgl {

    MBGL_DEFINE_ENUM(MLNLightAnchor, {
        { MLNLightAnchorMap, "map" },
        { MLNLightAnchorViewport, "viewport" },
    });

}

@interface MLNLight()

@end

@implementation MLNLight

- (instancetype)initWithMBGLLight:(const mbgl::style::Light *)mbglLight
{
    if (self = [super init]) {
        MLNLogInfo(@"Initializing %@.", NSStringFromClass([self class]));
        auto anchor = mbglLight->getAnchor();
        NSExpression *anchorExpression;
        if (anchor.isUndefined()) {
            mbgl::style::PropertyValue<mbgl::style::LightAnchorType> defaultAnchor = mbglLight->getDefaultAnchor();
            anchorExpression = MLNStyleValueTransformer<mbgl::style::LightAnchorType, NSValue *, mbgl::style::LightAnchorType, MLNLightAnchor>().toExpression(defaultAnchor);
        } else {
            anchorExpression = MLNStyleValueTransformer<mbgl::style::LightAnchorType, NSValue *, mbgl::style::LightAnchorType, MLNLightAnchor>().toExpression(anchor);
        }

        _anchor = anchorExpression;

        auto positionValue = mbglLight->getPosition();
        if (positionValue.isUndefined()) {
            _position = MLNStyleValueTransformer<mbgl::style::Position, NSValue *>().toExpression(mbglLight->getDefaultPosition());
        } else {
            _position = MLNStyleValueTransformer<mbgl::style::Position, NSValue *>().toExpression(positionValue);
        }
        _positionTransition = MLNTransitionFromOptions(mbglLight->getPositionTransition());
        auto colorValue = mbglLight->getColor();
        if (colorValue.isUndefined()) {
            _color = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(mbglLight->getDefaultColor());
        } else {
            _color = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toExpression(colorValue);
        }
        _colorTransition = MLNTransitionFromOptions(mbglLight->getColorTransition());
        auto intensityValue = mbglLight->getIntensity();
        if (intensityValue.isUndefined()) {
            _intensity = MLNStyleValueTransformer<float, NSNumber *>().toExpression(mbglLight->getDefaultIntensity());
        } else {
            _intensity = MLNStyleValueTransformer<float, NSNumber *>().toExpression(intensityValue);
        }
        _intensityTransition = MLNTransitionFromOptions(mbglLight->getIntensityTransition());
    }

    return self;
}

- (mbgl::style::Light)mbglLight
{
    mbgl::style::Light mbglLight;
    auto anchor = MLNStyleValueTransformer<mbgl::style::LightAnchorType, NSValue *, mbgl::style::LightAnchorType, MLNLightAnchor>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::LightAnchorType>>(self.anchor, false);
    mbglLight.setAnchor(anchor);

    auto position = MLNStyleValueTransformer<mbgl::style::Position, NSValue *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::style::Position>>(self.position, false);
    mbglLight.setPosition(position);

    mbglLight.setPositionTransition(MLNOptionsFromTransition(self.positionTransition));

    auto color = MLNStyleValueTransformer<mbgl::Color, MLNColor *>().toPropertyValue<mbgl::style::PropertyValue<mbgl::Color>>(self.color, false);
    mbglLight.setColor(color);

    mbglLight.setColorTransition(MLNOptionsFromTransition(self.colorTransition));

    auto intensity = MLNStyleValueTransformer<float, NSNumber *>().toPropertyValue<mbgl::style::PropertyValue<float>>(self.intensity, false);
    mbglLight.setIntensity(intensity);

    mbglLight.setIntensityTransition(MLNOptionsFromTransition(self.intensityTransition));


    return mbglLight;
}



- (void)setAnchor:(NSExpression *)anchor {
    MLNLogDebug(@"Setting anchor: %@", anchor);
    _anchor = anchor;
}

- (void)setPosition:(NSExpression *)position {
    MLNLogDebug(@"Setting position: %@", position);
    _position = position;
}

- (void)setPositionTransition:(MLNTransition)transition {
    MLNLogDebug(@"Setting positionTransition: %@", MLNStringFromMLNTransition(transition));
    _positionTransition = transition;
}

- (void)setColor:(NSExpression *)color {
    MLNLogDebug(@"Setting color: %@", color);
    _color = color;
}

- (void)setColorTransition:(MLNTransition)transition {
    MLNLogDebug(@"Setting colorTransition: %@", MLNStringFromMLNTransition(transition));
    _colorTransition = transition;
}

- (void)setIntensity:(NSExpression *)intensity {
    MLNLogDebug(@"Setting intensity: %@", intensity);
    _intensity = intensity;
}

- (void)setIntensityTransition:(MLNTransition)transition {
    MLNLogDebug(@"Setting intensityTransition: %@", MLNStringFromMLNTransition(transition));
    _intensityTransition = transition;
}

@end
