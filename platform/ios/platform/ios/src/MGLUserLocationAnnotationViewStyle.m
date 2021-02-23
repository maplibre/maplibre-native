#import "MGLUserLocationAnnotationViewStyle.h"
#import "MGLLoggingConfiguration_Private.h"

@implementation MGLUserLocationAnnotationViewStyle

- (instancetype)init {
    if ((self = [super init])) {
        self.puckShadowOpacity = 0.25;
        if (@available(iOS 14, *)) {
            self.approximateHaloBorderWidth = 2.0;
            self.approximateHaloOpacity = 0.15;
        }
    }
    return self;
}

- (void)setPuckFillColor:(UIColor *)puckFillColor {
    MGLLogDebug(@"Setting puckFillColor: %@", puckFillColor);
    _puckFillColor = puckFillColor;
}

- (void)setPuckShadowColor:(UIColor *)puckShadowColor {
    MGLLogDebug(@"Setting puckShadowColor: %@", puckShadowColor);
    _puckShadowColor = puckShadowColor;
}

- (void)setPuckShadowOpacity:(CGFloat)puckShadowOpacity {
    MGLLogDebug(@"Setting puckShadowOpacity: %.2f", puckShadowOpacity);
    _puckShadowOpacity = puckShadowOpacity;
}

- (void)setPuckArrowFillColor:(UIColor *)puckArrowFillColor {
    MGLLogDebug(@"Setting puckArrowFillColor: %@", puckArrowFillColor);
    _puckArrowFillColor = puckArrowFillColor;
}

- (void)setHaloFillColor:(UIColor *)haloFillColor {
    MGLLogDebug(@"Setting haloFillColor: %@", haloFillColor);
    _haloFillColor = haloFillColor;
}

- (void)setApproximateHaloFillColor:(UIColor *)approximateHaloFillColor {
    MGLLogDebug(@"Setting approximateHaloFillColor: %@", approximateHaloFillColor);
    _approximateHaloFillColor = approximateHaloFillColor;
}

- (void)setApproximateHaloBorderColor:(UIColor *)approximateHaloBorderColor {
    MGLLogDebug(@"Setting approximateHaloBorderColor: %@", approximateHaloBorderColor);
    _approximateHaloBorderColor = approximateHaloBorderColor;
}

- (void)setApproximateHaloBorderWidth:(CGFloat)approximateHaloBorderWidth {
    MGLLogDebug(@"Setting approximateHaloBorderWidth: %.2f", approximateHaloBorderWidth);
    _approximateHaloBorderWidth = approximateHaloBorderWidth;
}

- (void)setApproximateHaloOpacity:(CGFloat)approximateHaloOpacity {
    MGLLogDebug(@"Setting approximateHaloOpacity: %.2f", approximateHaloOpacity);
    _approximateHaloOpacity = approximateHaloOpacity;
}

@end