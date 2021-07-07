#import <UIKit/UIKit.h>
#import <CoreLocation/CoreLocation.h>

@interface MGLScaleBar : UIView

// Sets the scale and redraws the scale bar
@property (nonatomic, assign) CLLocationDistance metersPerPoint;

// Sets whether the scale uses styles that make it easier to read on a dark styled map
@property (nonatomic, assign) BOOL shouldShowDarkStyles;

@end
