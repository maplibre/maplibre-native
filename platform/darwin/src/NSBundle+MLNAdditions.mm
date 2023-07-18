#import "NSBundle+MLNAdditions.h"

#import "MLNSettings.h"

const MLNExceptionName MLNBundleNotFoundException = @"MLNBundleNotFoundException";

@implementation NSBundle (MLNAdditions)

+ (instancetype)mgl_frameworkBundle {
    NSBundle *bundle = [self bundleForClass:[MLNSettings class]];

    if (![bundle.infoDictionary[@"CFBundlePackageType"] isEqualToString:@"FMWK"]) {
        // For static frameworks, the bundle is the containing application
        // bundle but the resources are in Mapbox.bundle.
        NSString *bundlePath = [bundle pathForResource:@"Mapbox" ofType:@"bundle"];
        if (bundlePath) {
            bundle = [self bundleWithPath:bundlePath];
        } else {
            [NSException raise:MLNBundleNotFoundException
                        format:@"The Mapbox framework bundle could not be found. If using the Mapbox Maps SDK for iOS as a static framework, make sure that Mapbox.bundle is copied into the root of the app bundle."];
        }
    }

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
    NSString *bundleIdentifier = [NSBundle mainBundle].bundleIdentifier;
    
#if (defined(__IPHONE_OS_VERSION_MAX_ALLOWED) && (__IPHONE_OS_VERSION_MAX_ALLOWED < 120200)) || \
    (defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED < 101404))
    // Before SDK 12.2 (bundled with Xcode 10.2): There’s no main bundle identifier when running in a unit test bundle.
    // 12.2 and after: the above bundle identifier is: com.apple.dt.xctest.tool
    if (!bundleIdentifier) {
        bundleIdentifier = [NSBundle bundleForClass:[MLNSettings class]].bundleIdentifier;
    }
#endif
    return bundleIdentifier;
}

@end
