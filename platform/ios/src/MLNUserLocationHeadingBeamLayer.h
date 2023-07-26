#import <QuartzCore/QuartzCore.h>
#import "MLNUserLocationAnnotationView.h"
#import "MLNUserLocationHeadingIndicator.h"

@interface MLNUserLocationHeadingBeamLayer : CALayer <MLNUserLocationHeadingIndicator>

- (MLNUserLocationHeadingBeamLayer *)initWithUserLocationAnnotationView:(MLNUserLocationAnnotationView *)userLocationView;
- (void)updateHeadingAccuracy:(CLLocationDirection)accuracy;
- (void)updateTintColor:(CGColorRef)color;

@end
