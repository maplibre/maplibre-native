#import <UIKit/UIKit.h>
#import <CoreLocation/CoreLocation.h>

@interface MLNScaleBar : UIView

// Sets the scale and redraws the scale bar
@property (nonatomic, assign) CLLocationDistance metersPerPoint;

// Sets whether the scale uses styles that make it easier to read on a dark styled map
@property (nonatomic, assign) BOOL shouldShowDarkStyles;

// Sets whether the scale uses metric
@property (nonatomic, assign) BOOL usesMetricSystem;

@end
