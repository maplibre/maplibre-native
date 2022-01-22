#import "MGLRendererConfiguration.h"

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

static NSString * const MGLCollisionBehaviorPre4_0Key = @"MGLCollisionBehaviorPre4_0";
static NSString * const MGLIdeographicFontFamilyNameKey = @"MGLIdeographicFontFamilyName";

@implementation MGLRendererConfiguration

+ (instancetype)currentConfiguration {
    return [[self alloc] init];
}

- (const float)scaleFactor {
#if TARGET_OS_IPHONE
    return [UIScreen instancesRespondToSelector:@selector(nativeScale)] ? [[UIScreen mainScreen] nativeScale] : [[UIScreen mainScreen] scale];
#else
    return [NSScreen mainScreen].backingScaleFactor;
#endif
}

- (nullable NSString *)localFontFamilyName {
    id infoDictionaryObject = [NSBundle.mainBundle objectForInfoDictionaryKey:MGLIdeographicFontFamilyNameKey];
    return [self localFontFamilyNameWithInfoDictionaryObject:infoDictionaryObject];
}

- (nullable NSString *)localFontFamilyNameWithInfoDictionaryObject:(nullable id)infoDictionaryObject {
    if ([infoDictionaryObject isKindOfClass:[NSNumber class]] && ![infoDictionaryObject boolValue]) {
        // NO means don’t use local fonts.
        return nil;
    } else if ([infoDictionaryObject isKindOfClass:[NSString class]]) {
        return infoDictionaryObject;
    } else if ([infoDictionaryObject isKindOfClass:[NSArray class]]) {
        // mbgl::LocalGlyphRasterizer::Impl accepts only a single string, but form a cascade list with one font on each line.
        return [infoDictionaryObject componentsJoinedByString:@"\n"];
    }
    
#if TARGET_OS_IPHONE
    return [UIFont systemFontOfSize:0 weight:UIFontWeightRegular].familyName;
#else
    return [NSFont systemFontOfSize:0 weight:NSFontWeightRegular].familyName;
#endif
}

- (BOOL)perSourceCollisions {
    id infoDictionaryObject = [NSBundle.mainBundle objectForInfoDictionaryKey:MGLCollisionBehaviorPre4_0Key];
    return [self perSourceCollisionsWithInfoDictionaryObject:infoDictionaryObject];
}

- (BOOL)perSourceCollisionsWithInfoDictionaryObject:(nullable id)infoDictionaryObject {
    // Set the collision behaviour. A value set in `NSUserDefaults.standardUserDefaults`
    // should override anything in the application's info.plist
    if ([NSUserDefaults.standardUserDefaults objectForKey:MGLCollisionBehaviorPre4_0Key]) {
        return [NSUserDefaults.standardUserDefaults boolForKey:MGLCollisionBehaviorPre4_0Key];
    } else if ([infoDictionaryObject isKindOfClass:[NSNumber class]] || [infoDictionaryObject isKindOfClass:[NSString class]]) {
        // Also support NSString to correspond with the behavior of `-[NSUserDefaults boolForKey:]`
        return [infoDictionaryObject boolValue];
    }
    return NO;
}

@end
