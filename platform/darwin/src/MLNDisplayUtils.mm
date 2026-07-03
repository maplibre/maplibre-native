#import "MLNDisplayUtils.h"

CGFloat MLNEffectiveScaleFactor(CGSize nativePixelBounds, CGSize logicalPointBounds) {
  // `nativeBounds` is always portrait-up and does not rotate; `bounds` rotates with the interface
  // orientation. Comparing the `.width` values directly gives the wrong ratio in landscape
  // (e.g. 1668 / 1194 = ~1.40 instead of 2.0 on iPad Pro 11), under-sizing the drawable and
  // rendering the map below native resolution. Pair short-side with short-side so the ratio is
  // orientation independent and still correct for external / CarPlay screens.
  CGFloat nativeShortSide = MIN(nativePixelBounds.width, nativePixelBounds.height);
  CGFloat pointShortSide = MIN(logicalPointBounds.width, logicalPointBounds.height);
  return pointShortSide > 0.0 ? nativeShortSide / pointShortSide : 0.0;
}

CGFloat MLNEffectiveScaleFactorForView(id viewOrNil) {
#if TARGET_OS_IPHONE || TARGET_OS_TV

  UIView *view = (UIView *)viewOrNil;
  UIScreen *screen = view.window.screen ?: [UIScreen mainScreen];

  CGFloat logicalToPixelRatio = MLNEffectiveScaleFactor(screen.nativeBounds.size, screen.bounds.size);
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
