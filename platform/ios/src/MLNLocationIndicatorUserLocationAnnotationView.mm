#import "MLNLocationIndicatorUserLocationAnnotationView.h"
#include <CoreLocation/CoreLocation.h>
#include <Foundation/Foundation.h>
#include <QuartzCore/QuartzCore.h>

#import "MLNLocationIndicatorStyleLayer.h"
#import "MLNUserLocation.h"
#import "UIImage+MLNAdditions.h"
#import "MLNMapView.h"
#import "NSExpression+MLNAdditions.h"

const CGFloat MLNUserLocationAnnotationPuckSize = 45.0;
const CGFloat MLNUserLocationAnnotationArrowSize = MLNUserLocationAnnotationPuckSize * 0.5;


@implementation MLNLocationIndicatorUserLocationAnnotationView {
    MLNLocationIndicatorStyleLayer* location_layer;

    CLLocation* _oldLocation;
    MLNRenderMode _renderMode;
}

- (void)update {
    if (self.mapView.style && !self->location_layer) {
        // Find existing location indicator layer before creating a new one
        self->location_layer = (MLNLocationIndicatorStyleLayer*)[self.mapView.style layerWithIdentifier:@"location_indicator_layer"];

        if (!self->location_layer) {
            self->_renderMode = MLNRenderModeNone;

            self->location_layer = [[MLNLocationIndicatorStyleLayer alloc] initWithIdentifier:@"location_indicator_layer"];

            [self.mapView.style setImage:[UIImage mgl_resourceImageNamed:@"user_bearing_icon"] forName:@"user_bearing_icon"];
            [self.mapView.style setImage:[UIImage mgl_resourceImageNamed:@"user_icon_stale"] forName:@"user_icon_stale"];
            [self.mapView.style setImage:[UIImage mgl_resourceImageNamed:@"user_icon"] forName:@"user_icon"];
            [self.mapView.style setImage:[UIImage mgl_resourceImageNamed:@"user_puck_icon"] forName:@"user_puck_icon"];
            [self.mapView.style setImage:[UIImage mgl_resourceImageNamed:@"user_stroke_icon"] forName:@"user_stroke_icon"];
            [self.mapView.style setImage:[UIImage mgl_resourceImageNamed:@"user_icon_shadow"] forName:@"user_icon_shadow"];

            NSDictionary *opacityStops = @{@(self.mapView.minimumZoomLevel): @0.6f,
                @(self.mapView.maximumZoomLevel): @1.0f };
            NSExpression *stops = [NSExpression expressionForConstantValue:opacityStops];
            NSExpression *styleScaling = [NSExpression mgl_expressionForInterpolatingExpression:NSExpression.zoomLevelVariableExpression withCurveType:MLNExpressionInterpolationModeLinear parameters:nil stops:stops];
            self->location_layer.topImageSize = styleScaling;
            self->location_layer.bearingImageSize = styleScaling;

            NSDictionary *backgroundOpacityStops = @{@(self.mapView.minimumZoomLevel): @1.2f,
                @(self.mapView.maximumZoomLevel): @1.5f };
            NSExpression *backgroundStops = [NSExpression expressionForConstantValue:backgroundOpacityStops];
            NSExpression *backgroundStyleScaling = [NSExpression mgl_expressionForInterpolatingExpression:NSExpression.zoomLevelVariableExpression withCurveType:MLNExpressionInterpolationModeLinear parameters:nil stops:backgroundStops];
            self->location_layer.shadowImageSize = backgroundStyleScaling;
            self->location_layer.imageTiltDisplacement = [NSExpression expressionForConstantValue:@2.0];
            self->location_layer.perspectiveCompensation = [NSExpression expressionForConstantValue:@0.9];

            self->location_layer.bearingTransition = MLNTransitionMake(1.0, 0, kCAMediaTimingFunctionLinear);

            [self.mapView.style addLayer:self->location_layer];
        }
    }

    CLLocation* newLocation = self.userLocation.location;

    if (self->location_layer) {
        if (self.mapView.userTrackingMode == MLNUserTrackingModeFollowWithCourse) {
            if (self.userLocation.location.course >= 0) {
                self->location_layer.bearing = [NSExpression expressionForConstantValue:@(self.userLocation.location.course)];
            }

            if (self->_renderMode != MLNRenderModeGps) {
                self->location_layer.bearingImage = [NSExpression expressionForConstantValue:@"user_puck_icon"];
                self->location_layer.topImage = [NSExpression expressionForConstantValue:@"user_puck_icon"];
                self->location_layer.shadowImage = [NSExpression expressionForConstantValue:@"user_stroke_icon"];

                self->_renderMode = MLNRenderModeGps;
            }
        } else {
            if (self->_renderMode != MLNRenderModeNormal && self.mapView.userTrackingMode == MLNUserTrackingModeFollow) {
                self->location_layer.bearingImage = self.mapView.showsUserHeadingIndicator ? [NSExpression expressionForConstantValue:@"user_bearing_icon"] : [NSExpression expressionForConstantValue:@"user_icon"];
                self->location_layer.topImage = [NSExpression expressionForConstantValue:@"user_icon"];
                self->location_layer.shadowImage = [NSExpression expressionForConstantValue:@"user_stroke_icon"];

                self->_renderMode = MLNRenderModeNormal;
            } else if (self->_renderMode != MLNRenderModeCompass && self.mapView.userTrackingMode == MLNUserTrackingModeFollowWithHeading) {
                self->location_layer.bearingImage = [NSExpression expressionForConstantValue:@"user_bearing_icon"];
                self->location_layer.topImage = [NSExpression expressionForConstantValue:@"user_icon"];
                self->location_layer.shadowImage = [NSExpression expressionForConstantValue:@"user_stroke_icon"];

                self->_renderMode = MLNRenderModeCompass;
            }
            bool showHeading = self.mapView.showsUserHeadingIndicator || self.mapView.userTrackingMode == MLNUserTrackingModeFollowWithHeading;
            if (showHeading) {
                CLLocationDirection headingDirection = (self.userLocation.heading.trueHeading >= 0 ? self.userLocation.heading.trueHeading : self.userLocation.heading.magneticHeading);
                if (headingDirection >= 0) {
                    self->location_layer.bearing = [NSExpression expressionForConstantValue:@(headingDirection)];
                }
            }
        }

        NSTimeInterval duration = 0.3;
        if(_oldLocation) {
            duration = MIN([newLocation.timestamp timeIntervalSinceDate:_oldLocation.timestamp], 1.0);
        }
        if (duration > 0) {
            MLNTransition transition = MLNTransitionMake(duration, 0, kCAMediaTimingFunctionLinear);
            self->location_layer.locationTransition = transition;
        }

        NSArray *location = @[@(newLocation.coordinate.latitude), @(newLocation.coordinate.longitude), @0];
        self->location_layer.location = [NSExpression expressionForConstantValue:location];
    }

    _oldLocation = newLocation;
}

- (void)removeLayer {
    if (self->location_layer) {
        if ([self.mapView.style layerWithIdentifier:@"location_indicator_layer"]) {
            [self.mapView.style removeLayer:self->location_layer];
        }
        self->location_layer = nil;
    }

    [super removeFromSuperview];
}

- (void)removeFromSuperview {
    [self removeLayer];
    [super removeFromSuperview];
}

@end
