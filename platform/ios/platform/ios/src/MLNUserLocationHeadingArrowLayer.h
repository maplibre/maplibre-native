#import <QuartzCore/QuartzCore.h>
#import "MLNUserLocationAnnotationView.h"
#import "MLNUserLocationHeadingIndicator.h"

@interface MLNUserLocationHeadingArrowLayer : CAShapeLayer <MLNUserLocationHeadingIndicator>

- (instancetype)initWithUserLocationAnnotationView:(MLNUserLocationAnnotationView *)userLocationView;
- (void)updateHeadingAccuracy:(CLLocationDirection)accuracy;
- (void)updateTintColor:(CGColorRef)color;

@end
