#import "SMCalloutView.h"
#import "MLNCalloutView.h"

/**
 A concrete implementation of `MLNCalloutView` based on
 <a href="https://github.com/nfarina/calloutview">SMCalloutView</a>. This
 callout view displays the represented annotation’s title, subtitle, and
 accessory views in a compact, two-line layout.
 */
@interface MLNCompactCalloutView : MLNSMCalloutView <MLNCalloutView>

+ (instancetype)platformCalloutView;

@end
