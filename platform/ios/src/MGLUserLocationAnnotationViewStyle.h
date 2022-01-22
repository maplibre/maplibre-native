#import <UIKit/UIKit.h>

#import "MGLFoundation.h"

NS_ASSUME_NONNULL_BEGIN

/**
 A class containing information about the default User Location annotation view style.
 */
MGL_EXPORT
@interface MGLUserLocationAnnotationViewStyle : NSObject

/**
 The fill color for the puck view.
 */
@property (nonatomic) UIColor *puckFillColor;
/**
 The shadow color for the puck view.
 */
@property (nonatomic) UIColor *puckShadowColor;
/**
 The shadow opacity for the puck view.
 Set any value between 0.0 and 1.0.
 The default value of this property is equal to `0.25`
 */
@property (nonatomic, assign) CGFloat puckShadowOpacity;
/**
 The fill color for the arrow puck.
 */
@property (nonatomic) UIColor *puckArrowFillColor;
/**
 The fill color for the puck view.
 */
@property (nonatomic) UIColor *haloFillColor;
/**
 The halo fill color for the approximate view.
 */
@property (nonatomic) UIColor *approximateHaloFillColor API_AVAILABLE(ios(14));
/**
 The halo border color for the approximate view.
 */
@property (nonatomic) UIColor *approximateHaloBorderColor API_AVAILABLE(ios(14));
/**
 The halo border width for the approximate view.
 The default value of this property is equal to `2.0`
 */
@property (nonatomic, assign) CGFloat approximateHaloBorderWidth API_AVAILABLE(ios(14));
/**
 The halo opacity for the approximate view.
 Set any value between 0.0 and 1.0
 The default value of this property is equal to `0.15`
 */
@property (nonatomic, assign) CGFloat approximateHaloOpacity API_AVAILABLE(ios(14));

@end

NS_ASSUME_NONNULL_END
