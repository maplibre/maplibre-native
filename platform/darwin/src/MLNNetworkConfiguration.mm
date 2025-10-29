#import "MLNNetworkConfiguration_Private.h"
#import "MLNLoggingConfiguration_Private.h"
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#import "MLNSettings_Private.h"
#endif

#import "MLNReachability.h"

static NSString * const MLNStartTime = @"start_time";
static NSString * const MLNResourceType = @"resource_type";
NSString * const kMLNDownloadPerformanceEvent = @"mobile.performance_trace";

@interface MLNNetworkConfiguration () <MLNNativeNetworkDelegate>

@property (nonatomic, strong) NSMutableDictionary<NSString *, NSDictionary*> *events;
@property (nonatomic, weak) id<MLNNetworkConfigurationMetricsDelegate> metricsDelegate;
@property (nonatomic) dispatch_queue_t eventsQueue;

@end

@implementation MLNNetworkConfiguration
{
    NSURLSessionConfiguration *_sessionConfig;
}

- (instancetype)init {
    if (self = [super init]) {
        self.sessionConfiguration = nil;
        _events = [NSMutableDictionary dictionary];
        _eventsQueue = dispatch_queue_create("org.maplibre.network-configuration", DISPATCH_QUEUE_CONCURRENT);
    }

    return self;
}

+ (instancetype)sharedManager {
    static dispatch_once_t onceToken;
    static MLNNetworkConfiguration *_sharedManager;
    dispatch_once(&onceToken, ^{
        _sharedManager = [[self alloc] init];
    });

    // Notice, this is reset for each call. This is primarily for testing purposes.
    // TODO: Consider only calling this for testing?
    [_sharedManager resetNativeNetworkManagerDelegate];

    return _sharedManager;
}

- (void)resetNativeNetworkManagerDelegate {
    // Tell core about our network integration. `delegate` here is not (yet)
    // intended to be set to nil, except for testing.
    [MLNNativeNetworkManager sharedManager].delegate = self;
}

+ (NSURLSessionConfiguration *)defaultSessionConfiguration {
    NSURLSessionConfiguration* sessionConfiguration = [NSURLSessionConfiguration defaultSessionConfiguration];

    sessionConfiguration.timeoutIntervalForResource = 30;
    sessionConfiguration.HTTPMaximumConnectionsPerHost = 8;
    sessionConfiguration.requestCachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
    sessionConfiguration.URLCache = nil;

    return sessionConfiguration;
}

// MARK: - MLNNativeNetworkDelegate

- (NSURLSession *)sessionForNetworkManager:(MLNNativeNetworkManager *)networkManager {
    // Note: this method is NOT called on the main thread.
    NSURLSession *session;
    if ([self.delegate respondsToSelector:@selector(sessionForNetworkConfiguration:)]) {
        session = [self.delegate sessionForNetworkConfiguration:self];
    }

    // Check for a background session; string checking is fragile, but this is not
    // a deal breaker as we're only doing this to provide more clarity to the
    // developer
    NSAssert(![session isKindOfClass:NSClassFromString(@"__NSURLBackgroundSession")],
             @"Background NSURLSessions are not yet supported");

    if (session.delegate) {
        NSAssert(![session.delegate conformsToProtocol:@protocol(NSURLSessionDataDelegate)],
                 @"Session delegates conforming to NSURLSessionDataDelegate are not yet supported");
    }

    return session;
}

- (NSMutableURLRequest *)willSendRequest:(NSMutableURLRequest *)request {

    if ([self.delegate respondsToSelector:@selector(willSendRequest:)]) {
        return [self.delegate willSendRequest:request];
    }

    return request;

}

- (MLNInternalNetworkResponse *)didReceiveResponse:(MLNInternalNetworkResponse *)response {

    if ([self.delegate respondsToSelector:@selector(didReceiveResponse:)]) {

        MLNNetworkResponse *tempResponse = [MLNNetworkResponse responseWithData:response.data
                                                                    urlResponse:response.response
                                                                          error:response.error];
        if (tempResponse) {
            MLNInternalNetworkResponse *internalResponse = [MLNInternalNetworkResponse responseWithData:tempResponse.data
                                                                                            urlResponse:tempResponse.response
                                                                                                  error:tempResponse.error];
            return internalResponse;
        } else {
            return nil;
        }

    }

    return response;

}


- (NSURLSessionConfiguration *)sessionConfiguration {
    NSURLSessionConfiguration *sessionConfig = nil;
    @synchronized (self) {
        sessionConfig = _sessionConfig;
    }
    return sessionConfig;
}

- (void)setSessionConfiguration:(NSURLSessionConfiguration *)sessionConfiguration {
    @synchronized (self) {
        if (sessionConfiguration == nil) {
            _sessionConfig = [MLNNetworkConfiguration defaultSessionConfiguration];
        } else {
            _sessionConfig = sessionConfiguration;
        }
    }
}

- (void)startDownloadEvent:(NSString *)urlString type:(NSString *)resourceType {
    if (urlString && resourceType && ![self eventDictionaryForKey:urlString]) {
        NSDate *startDate = [NSDate date];
        [self setEventDictionary:@{ MLNStartTime: startDate, MLNResourceType: resourceType }
                          forKey:urlString];
    }
}

- (void)stopDownloadEventForResponse:(NSURLResponse *)response {
    [self sendEventForURLResponse:response withAction:nil];
}

- (void)cancelDownloadEventForResponse:(NSURLResponse *)response {
    [self sendEventForURLResponse:response withAction:@"cancel"];
}

- (void)debugLog:(NSString *)message {
    MLNLogDebug(message);
}

- (void)errorLog:(NSString *)message {
    MLNLogError(message);
}

// MARK: - Event management

- (void)sendEventForURLResponse:(NSURLResponse *)response withAction:(NSString *)action
{
    if ([response isKindOfClass:[NSURLResponse class]]) {
        NSString *urlString = response.URL.relativePath;
        if (urlString && [self eventDictionaryForKey:urlString]) {
            NSDictionary *eventAttributes = [self eventAttributesForURL:response withAction:action];
            [self removeEventDictionaryForKey:urlString];

            dispatch_async(dispatch_get_main_queue(), ^{
                [self.metricsDelegate networkConfiguration:self didGenerateMetricEvent:eventAttributes];
            });
        }
    }
}

- (NSDictionary *)eventAttributesForURL:(NSURLResponse *)response withAction:(NSString *)action
{
    NSString *urlString = response.URL.relativePath;
    NSDictionary *parameters = [self eventDictionaryForKey:urlString];
    NSDate *startDate = [parameters objectForKey:MLNStartTime];
    NSDate *endDate = [NSDate date];
    NSTimeInterval elapsedTime = [endDate timeIntervalSinceDate:startDate];
    NSDateFormatter* iso8601Formatter = [[NSDateFormatter alloc] init];
    iso8601Formatter.dateFormat = @"yyyy-MM-dd'T'HH:mm:ssZ";
    NSString *createdDate = [iso8601Formatter stringFromDate:[NSDate date]];

    NSMutableArray *attributes = [NSMutableArray array];
    [attributes addObject:@{ @"name" : @"requestUrl" , @"value" : urlString }];
    [attributes addObject:@{ @"name" : MLNResourceType , @"value" : [parameters objectForKey:MLNResourceType] }];

    if ([response isKindOfClass:[NSHTTPURLResponse class]]) {
        NSInteger responseCode = [(NSHTTPURLResponse *)response statusCode];
        [attributes addObject:@{ @"name" : @"responseCode", @"value" : @(responseCode)}];
    }

    BOOL isWIFIOn = [[MLNReachability reachabilityWithHostName:response.URL.host] isReachableViaWiFi];
    [attributes addObject:@{ @"name" : @"wifiOn", @"value" : @(isWIFIOn)}];

    if (action) {
        [attributes addObject:@{ @"name" : @"action" , @"value" : action }];
    }

    double elapsedTimeInMS = elapsedTime * 1000.0;

    return @{
             @"event" : kMLNDownloadPerformanceEvent,
             @"created" : createdDate,
             @"sessionId" : [NSUUID UUID].UUIDString,
             @"counters" : @[ @{ @"name" : @"elapsedMS" , @"value" : @(elapsedTimeInMS) } ],
             @"attributes" : attributes
             };
}

// MARK: - Events dictionary access

- (nullable NSDictionary*)eventDictionaryForKey:(nonnull NSString*)key {
    __block NSDictionary *dictionary;

    dispatch_sync(self.eventsQueue, ^{
        dictionary = [self.events objectForKey:key];
    });

    return dictionary;
}

- (void)setEventDictionary:(nonnull NSDictionary*)dictionary forKey:(nonnull NSString*)key {
    dispatch_barrier_async(self.eventsQueue, ^{
        [self.events setObject:dictionary forKey:key];
    });
}

- (void)removeEventDictionaryForKey:(nonnull NSString*)key {
    dispatch_barrier_async(self.eventsQueue, ^{
        [self.events removeObjectForKey:key];
    });
}

@end

@implementation MLNNetworkConfiguration (ForTesting)

+ (void)testing_clearNativeNetworkManagerDelegate {
    [MLNNativeNetworkManager sharedManager].delegate = nil;
}

+ (id)testing_nativeNetworkManagerDelegate {
    return [MLNNativeNetworkManager sharedManager].delegate;
}

@end
