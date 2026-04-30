#import <UIKit/UIKit.h>

#import "MLNTypes.h"

NS_ASSUME_NONNULL_BEGIN

/**
 A specialized view that displays the current compass heading for its associated map.
 */
MLN_EXPORT
@interface MLNCompassButton : UIImageView

/**
 The visibility of the compass button.

 You can configure a compass button to be visible all the time or only when the compass heading
 changes.
 */
@property (nonatomic, assign) MLNOrnamentVisibility compassVisibility;

/**
 Sets whether the compass uses white style.
 */
@property (nonatomic, assign) BOOL shouldWhiteStyle;

@end

NS_ASSUME_NONNULL_END
