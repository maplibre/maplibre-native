#import <UIKit/UIKit.h>
#import "MLNUserLocationAnnotationView.h"

typedef NS_ENUM(NSUInteger, MLNRenderMode) {
  MLNRenderModeNone = 0,
  MLNRenderModeCompass,
  MLNRenderModeGps,
  MLNRenderModeNormal,
};

@interface MLNLocationIndicatorUserLocationAnnotationView : MLNUserLocationAnnotationView

- (void)removeLayer;

@end
