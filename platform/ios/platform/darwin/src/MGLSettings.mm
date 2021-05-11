#import "MGLSettings_Private.h"
#import "NSBundle+MGLAdditions.h"

#if TARGET_OS_OSX
#import "NSProcessInfo+MGLAdditions.h"
#endif

@interface MGLSettings ()

@property (atomic) NSString *apiKey;
@property (atomic) mbgl::TileServerOptions *tileServerOptions;
@property (atomic) NSString *tileServerOptionsChangeToken;

@end

@implementation MGLSettings

#pragma mark - Internal

- (instancetype)init {
    if (self = [super init]) {
        self.tileServerOptions = new mbgl::TileServerOptions(mbgl::TileServerOptions::MapboxConfiguration());
    }
    return self;
}

+ (void)load {
    // Read the initial configuration from Info.plist.
    NSString *apiKey = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"MGLApiKey"];
    if (apiKey.length) {
        self.apiKey = apiKey;
    }

    NSString *apiBaseURL = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"MGLTileServerBaseURL"];
    
    // If apiBaseURL is not a valid URL, [NSURL URLWithString:] will be `nil`.
    if (apiBaseURL.length && [NSURL URLWithString:apiBaseURL]) {
        [self setAPIBaseURL:[NSURL URLWithString:apiBaseURL]];
        [self tileServerOptionsChanged];
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

+ (void)setApiKey:(NSString *)apiKey {
    apiKey = [apiKey stringByTrimmingCharactersInSet:
             [NSCharacterSet whitespaceAndNewlineCharacterSet]];
    if (!apiKey.length) {
        return;
    }

    [MGLSettings sharedSettings].apiKey = apiKey;
}

+ (NSString *)apiKey {
    return [MGLSettings sharedSettings].apiKey;
}

+ (void)setAPIBaseURL:(NSURL *)apiBaseURL {
    auto tileServerOptions = [MGLSettings sharedSettings].tileServerOptions;
    NSString *baseUrlNSStr = apiBaseURL.absoluteString;
    auto baseUrl = std::string([baseUrlNSStr UTF8String]);

    [MGLSettings sharedSettings].tileServerOptions = &tileServerOptions->withBaseURL(baseUrl);
}

+ (NSURL *)apiBaseURL {
    auto baseUrl = [MGLSettings sharedSettings].tileServerOptions->baseURL();
    NSString* baseUrlNSStr = [NSString stringWithUTF8String:baseUrl.c_str()];
    NSURL *url = [NSURL URLWithString:baseUrlNSStr];
    return url;
}

+ (void)setTileServerOptions:(mbgl::TileServerOptions)options {
    [MGLSettings sharedSettings].tileServerOptions = &options;
    [self tileServerOptionsChanged];
}

+ (mbgl::TileServerOptions)tileServerOptions {
    auto options = [MGLSettings sharedSettings];
    return options.tileServerOptions->clone();
}

+ (void)tileServerOptionsChanged {
    [MGLSettings sharedSettings].tileServerOptionsChangeToken = [[NSUUID UUID] UUIDString];
}

@end
