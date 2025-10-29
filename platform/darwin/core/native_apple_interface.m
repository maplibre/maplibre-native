#import <Foundation/Foundation.h>
#import <mbgl/interface/native_apple_interface.h>

@implementation MLNInternalNetworkResponse

+(MLNInternalNetworkResponse *)responseWithData:(NSData *)data
                         urlResponse:(NSURLResponse *)response
                               error:(NSError *)error {

    MLNInternalNetworkResponse *tempResult = [[MLNInternalNetworkResponse alloc] init];
    tempResult.data = data;
    tempResult.response = response;
    tempResult.error = error;
    return tempResult;
}

@end



@implementation MLNNativeNetworkManager

static MLNNativeNetworkManager *instance = nil;

+ (MLNNativeNetworkManager *)sharedManager {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[MLNNativeNetworkManager alloc] init];
    });
    return instance;
}

+ (NSURLSessionConfiguration *)testSessionConfiguration {
    NSURLSessionConfiguration* sessionConfiguration = [NSURLSessionConfiguration defaultSessionConfiguration];

    sessionConfiguration.timeoutIntervalForResource = 30;
    sessionConfiguration.HTTPMaximumConnectionsPerHost = 8;
    sessionConfiguration.requestCachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
    sessionConfiguration.URLCache = nil;

    return sessionConfiguration;
}

// MARK: - Required

- (NSURLSessionConfiguration *)sessionConfiguration {
    NSURLSessionConfiguration *configuration = [_delegate sessionConfiguration];

    if (!configuration) {
        // For testing. Since we get a `nil` return when SDK is modular, we use
        // this for testing requests.
        // Same as `[MLNNetworkConfiguration defaultSessionConfiguration]` in
        // ``MLNNetworkConfiguration/mm``.
        configuration = [MLNNativeNetworkManager testSessionConfiguration];
    }

    return configuration;
}

- (void)startDownloadEvent:(NSString *)event type:(NSString *)type {
    [self.delegate startDownloadEvent:event type:type];
}

- (void)cancelDownloadEventForResponse:(NSURLResponse *)response {
    [self.delegate cancelDownloadEventForResponse:response];
}

- (void)stopDownloadEventForResponse:(NSURLResponse *)response {
    [self.delegate stopDownloadEventForResponse:response];
}

- (void)debugLog:(NSString *)format, ... {
    va_list formatList;
    va_start(formatList, format);
    NSString *formattedMessage = [[NSString alloc] initWithFormat:format arguments:formatList];
    va_end(formatList);

    [self.delegate debugLog:formattedMessage];
}

- (void)errorLog:(NSString *)format, ... {
    va_list formatList;
    va_start(formatList, format);
    NSString *formattedMessage = [[NSString alloc] initWithFormat:format arguments:formatList];
    va_end(formatList);

    [self.delegate errorLog:formattedMessage];
}

@end
