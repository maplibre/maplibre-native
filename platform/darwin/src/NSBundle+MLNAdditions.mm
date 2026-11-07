#import "NSBundle+MLNAdditions.h"

#import "MLNSettings.h"

const MLNExceptionName MLNBundleNotFoundException = @"MLNBundleNotFoundException";

@implementation NSBundle (MLNAdditions)

+ (instancetype)mgl_frameworkBundle {
  NSBundle *bundle = [self bundleForClass:[MLNSettings class]];

#ifndef MLN_CUSTOM_COMBINED_BUNDLE
  if (![bundle.infoDictionary[@"CFBundlePackageType"] isEqualToString:@"FMWK"]) {
    // For static frameworks, the bundle is the containing application
    // bundle but the resources are in Mapbox.bundle.
    NSString *bundlePath = [bundle pathForResource:@"Mapbox" ofType:@"bundle"];
    if (bundlePath) {
      bundle = [self bundleWithPath:bundlePath];
    } else {
      [NSException raise:MLNBundleNotFoundException
                  format:@"The MapLibre framework bundle could not be found. If using  MapLibre "
                         @"iOS as a static framework, make sure that MapLibre bundle is copied "
                         @"into the root of the app bundle."];
    }
  }
#endif

  return bundle;
}

+ (nullable NSString *)mgl_frameworkBundleIdentifier {
  return self.mgl_frameworkInfoDictionary[@"CFBundleIdentifier"];
}

+ (nullable NSDictionary<NSString *, id> *)mgl_frameworkInfoDictionary {
  NSBundle *bundle = self.mgl_frameworkBundle;
  return bundle.infoDictionary;
}

+ (nullable NSString *)mgl_applicationBundleIdentifier {
  return [NSBundle mainBundle].bundleIdentifier;
}

@end
