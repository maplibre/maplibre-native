#import "MLNDisplayUtils.h"

CGFloat MLNEffectiveScaleFactorForView(id viewOrNil) {
#if TARGET_OS_IPHONE || TARGET_OS_TV

    UIView *view = (UIView *)viewOrNil;
    UIScreen *screen = view.window.screen ?: [UIScreen mainScreen];

    CGFloat logicalToPixelRatio = screen.nativeBounds.size.width / screen.bounds.size.width;
    if (logicalToPixelRatio > 0.0) {
        return logicalToPixelRatio;
    }

    return [screen respondsToSelector:@selector(nativeScale)] ? screen.nativeScale : screen.scale;

#elif TARGET_OS_OSX

    NSView *view = (NSView *)viewOrNil;
    NSScreen *screen = view ? (view.window.screen ?: [NSScreen mainScreen]) : [NSScreen mainScreen];
    return screen ? screen.backingScaleFactor : 1.0;

#else

    return 1.0;

#endif
}
