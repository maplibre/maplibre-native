#import "MGLNetworkConfiguration_Private.h"
#import "MGLLoggingConfiguration_Private.h"
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#import "MGLSettings_Private.h"
#endif

#import "MGLReachability.h"

static NSString * const MGLStartTime = @"start_time";
static NSString * const MGLResourceType = @"resource_type";
NSString * const kMGLDownloadPerformanceEvent = @"mobile.performance_trace";

@interface MGLNetworkConfiguration () <MGLNativeNetworkDelegate>

@property (nonatomic, strong) NSMutableDictionary<NSString *, NSDictionary*> *events;
@property (nonatomic, weak) id<MGLNetworkConfigurationMetricsDelegate> metricsDelegate;
@property (nonatomic) dispatch_queue_t eventsQueue;

@end

@implementation MGLNetworkConfiguration
{
    NSURLSessionConfiguration *_sessionConfig;
}

- (instancetype)init {
    if (self = [super init]) {
        self.sessionConfiguration = nil;
        _events = [NSMutableDictionary dictionary];
        _eventsQueue = dispatch_queue_create("com.mapbox.network-configuration", DISPATCH_QUEUE_CONCURRENT);
    }
    
    return self;
}

+ (instancetype)sharedManager {
    static dispatch_once_t onceToken;
    static MGLNetworkConfiguration *_sharedManager;
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
    [MGLNativeNetworkManager sharedManager].delegate = self;
}

+ (NSURLSessionConfiguration *)defaultSessionConfiguration {
    NSURLSessionConfiguration* sessionConfiguration = [NSURLSessionConfiguration defaultSessionConfiguration];

    sessionConfiguration.timeoutIntervalForResource = 30;
    sessionConfiguration.HTTPMaximumConnectionsPerHost = 8;
    sessionConfiguration.requestCachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
    sessionConfiguration.URLCache = nil;

    return sessionConfiguration;
}

// MARK: - MGLNativeNetworkDelegate

- (NSURLSession *)sessionForNetworkManager:(MGLNativeNetworkManager *)networkManager {
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
            _sessionConfig = [MGLNetworkConfiguration defaultSessionConfiguration];
        } else {
            _sessionConfig = sessionConfiguration;
        }
    }
}

- (void)startDownloadEvent:(NSString *)urlString type:(NSString *)resourceType {
    if (urlString && resourceType && ![self eventDictionaryForKey:urlString]) {
        NSDate *startDate = [NSDate date];
        [self setEventDictionary:@{ MGLStartTime: startDate, MGLResourceType: resourceType }
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
    MGLLogDebug(message);
}

- (void)errorLog:(NSString *)message {
    MGLLogError(message);
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
    NSDate *startDate = [parameters objectForKey:MGLStartTime];
    NSDate *endDate = [NSDate date];
    NSTimeInterval elapsedTime = [endDate timeIntervalSinceDate:startDate];
    NSDateFormatter* iso8601Formatter = [[NSDateFormatter alloc] init];
    iso8601Formatter.dateFormat = @"yyyy-MM-dd'T'HH:mm:ssZ";
    NSString *createdDate = [iso8601Formatter stringFromDate:[NSDate date]];
    
    NSMutableArray *attributes = [NSMutableArray array];
    [attributes addObject:@{ @"name" : @"requestUrl" , @"value" : urlString }];
    [attributes addObject:@{ @"name" : MGLResourceType , @"value" : [parameters objectForKey:MGLResourceType] }];
    
    if ([response isKindOfClass:[NSHTTPURLResponse class]]) {
        NSInteger responseCode = [(NSHTTPURLResponse *)response statusCode];
        [attributes addObject:@{ @"name" : @"responseCode", @"value" : @(responseCode)}];
    }
    
    BOOL isWIFIOn = [[MGLReachability reachabilityWithHostName:response.URL.host] isReachableViaWiFi];
    [attributes addObject:@{ @"name" : @"wifiOn", @"value" : @(isWIFIOn)}];
    
    if (action) {
        [attributes addObject:@{ @"name" : @"action" , @"value" : action }];
    }
    
    double elapsedTimeInMS = elapsedTime * 1000.0;
    
    return @{
             @"event" : kMGLDownloadPerformanceEvent,
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

@implementation MGLNetworkConfiguration (ForTesting)

+ (void)testing_clearNativeNetworkManagerDelegate {
    [MGLNativeNetworkManager sharedManager].delegate = nil;
}

+ (id)testing_nativeNetworkManagerDelegate {
    return [MGLNativeNetworkManager sharedManager].delegate;
}

@end
