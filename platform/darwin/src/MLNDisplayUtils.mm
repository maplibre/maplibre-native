#import "MLNDisplayUtils.h"

CGFloat MLNEffectiveScaleFactor(CGSize nativePixelBounds, CGSize logicalPointBounds) {
  // `nativeBounds` is always portrait-up while `bounds` rotates with the interface orientation,
  // so pair short side with short side to keep the ratio orientation independent.
  CGFloat nativeShortSide = MIN(nativePixelBounds.width, nativePixelBounds.height);
  CGFloat pointShortSide = MIN(logicalPointBounds.width, logicalPointBounds.height);
  return pointShortSide > 0.0 ? nativeShortSide / pointShortSide : 0.0;
}

CGFloat MLNEffectiveScaleFactorForView(id viewOrNil) {
#if TARGET_OS_IPHONE || TARGET_OS_TV

  UIView *view = (UIView *)viewOrNil;
  UIScreen *screen = view.window.screen ?: [UIScreen mainScreen];

  // `nativeScale` is orientation-independent and accounts for Display Zoom (unlike `scale`).
  if ([screen respondsToSelector:@selector(nativeScale)] && screen.nativeScale > 0.0) {
    return screen.nativeScale;
  }

  CGFloat logicalToPixelRatio =
      MLNEffectiveScaleFactor(screen.nativeBounds.size, screen.bounds.size);
  if (logicalToPixelRatio > 0.0) {
    return logicalToPixelRatio;
  }

  return screen.scale;

#elif TARGET_OS_OSX

  NSView *view = (NSView *)viewOrNil;
  NSScreen *screen = view ? (view.window.screen ?: [NSScreen mainScreen]) : [NSScreen mainScreen];
  return screen ? screen.backingScaleFactor : 1.0;

#else

  return 1.0;

#endif
}
