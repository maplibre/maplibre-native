#import "MGLSettings_Private.h"
#import "NSBundle+MGLAdditions.h"

#if TARGET_OS_OSX
#import "NSProcessInfo+MGLAdditions.h"
#endif

NSString * const MGLMapboxAccountTypeKey = @"MGLMapboxAccountType";

@interface MGLSettings ()

@property (atomic) NSString *accessToken;
@property (nonatomic) NSURL *apiBaseURL;

@end

@implementation MGLSettings

#pragma mark - Internal

+ (void)load {
    // Read the initial configuration from Info.plist.
    NSString *accessToken = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"MGLMapboxAccessToken"];
    if (accessToken.length) {
        self.accessToken = accessToken;
    }

    NSString *apiBaseURL = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"MGLMapboxAPIBaseURL"];
    
    // If apiBaseURL is not a valid URL, [NSURL URLWithString:] will be `nil`.
    if (apiBaseURL.length && [NSURL URLWithString:apiBaseURL]) {
        [self setAPIBaseURL:[NSURL URLWithString:apiBaseURL]];
    }
}

+ (instancetype)sharedSettings {
#if TARGET_OS_OSX
    if (NSProcessInfo.processInfo.mgl_isInterfaceBuilderDesignablesAgent) {
        return nil;
    }
#endif
    
    static dispatch_once_t onceToken;
    static MGLSettings *_sharedSettings;
    void (^setupBlock)(void) = ^{
        dispatch_once(&onceToken, ^{
            _sharedSettings = [[self alloc] init];
        });
    };
    if (![[NSThread currentThread] isMainThread]) {
        dispatch_sync(dispatch_get_main_queue(), ^{
            setupBlock();
        });
    } else {
        setupBlock();
    }
    return _sharedSettings;
}

+ (void)setAccessToken:(NSString *)accessToken {
    accessToken = [accessToken stringByTrimmingCharactersInSet:
                   [NSCharacterSet whitespaceAndNewlineCharacterSet]];
    if (!accessToken.length) {
        return;
    }

    [MGLSettings sharedSettings].accessToken = accessToken;
}

+ (NSString *)accessToken {
    return [MGLSettings sharedSettings].accessToken;
}

+ (void)setAPIBaseURL:(NSURL *)apiBaseURL {
    [MGLSettings sharedSettings].apiBaseURL = apiBaseURL;
}

+ (NSURL *)apiBaseURL {
    return [MGLSettings sharedSettings].apiBaseURL;
}

@end
