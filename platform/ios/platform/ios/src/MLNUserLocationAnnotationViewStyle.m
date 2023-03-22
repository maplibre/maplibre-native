#import "MLNUserLocationAnnotationViewStyle.h"
#import "MLNLoggingConfiguration_Private.h"

@implementation MLNUserLocationAnnotationViewStyle

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
    MLNLogDebug(@"Setting puckFillColor: %@", puckFillColor);
    _puckFillColor = puckFillColor;
}

- (void)setPuckShadowColor:(UIColor *)puckShadowColor {
    MLNLogDebug(@"Setting puckShadowColor: %@", puckShadowColor);
    _puckShadowColor = puckShadowColor;
}

- (void)setPuckShadowOpacity:(CGFloat)puckShadowOpacity {
    MLNLogDebug(@"Setting puckShadowOpacity: %.2f", puckShadowOpacity);
    _puckShadowOpacity = puckShadowOpacity;
}

- (void)setPuckArrowFillColor:(UIColor *)puckArrowFillColor {
    MLNLogDebug(@"Setting puckArrowFillColor: %@", puckArrowFillColor);
    _puckArrowFillColor = puckArrowFillColor;
}

- (void)setHaloFillColor:(UIColor *)haloFillColor {
    MLNLogDebug(@"Setting haloFillColor: %@", haloFillColor);
    _haloFillColor = haloFillColor;
}

- (void)setApproximateHaloFillColor:(UIColor *)approximateHaloFillColor {
    MLNLogDebug(@"Setting approximateHaloFillColor: %@", approximateHaloFillColor);
    _approximateHaloFillColor = approximateHaloFillColor;
}

- (void)setApproximateHaloBorderColor:(UIColor *)approximateHaloBorderColor {
    MLNLogDebug(@"Setting approximateHaloBorderColor: %@", approximateHaloBorderColor);
    _approximateHaloBorderColor = approximateHaloBorderColor;
}

- (void)setApproximateHaloBorderWidth:(CGFloat)approximateHaloBorderWidth {
    MLNLogDebug(@"Setting approximateHaloBorderWidth: %.2f", approximateHaloBorderWidth);
    _approximateHaloBorderWidth = approximateHaloBorderWidth;
}

- (void)setApproximateHaloOpacity:(CGFloat)approximateHaloOpacity {
    MLNLogDebug(@"Setting approximateHaloOpacity: %.2f", approximateHaloOpacity);
    _approximateHaloOpacity = approximateHaloOpacity;
}

@end