#import <UIKit/UIKit.h>
#import "MLNUserLocationAnnotationView.h"

extern const CGFloat MLNUserLocationAnnotationDotSize;
extern const CGFloat MLNUserLocationAnnotationHaloSize;

extern const CGFloat MLNUserLocationAnnotationPuckSize;
extern const CGFloat MLNUserLocationAnnotationArrowSize;

// Threshold in radians between heading indicator rotation updates.
extern const CGFloat MLNUserLocationHeadingUpdateThreshold;

@interface MLNFaux3DUserLocationAnnotationView : MLNUserLocationAnnotationView

@end
