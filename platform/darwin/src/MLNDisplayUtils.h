#import <Foundation/Foundation.h>
#import <TargetConditionals.h>

#if TARGET_OS_IPHONE || TARGET_OS_TV
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

FOUNDATION_EXTERN CGFloat MLNEffectiveScaleFactorForView(id viewOrNil);
