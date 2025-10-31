#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class MLNNativeNetworkManager;

@interface MLNInternalNetworkResponse : NSObject

@property (retain, nullable) NSError *error;
@property (retain, nullable) NSData *data;
@property (retain, nullable) NSURLResponse *response;

+ (MLNInternalNetworkResponse *)responseWithData:(NSData *)data
                                     urlResponse:(NSURLResponse *)response
                                           error:(NSError *)error;

@end

@protocol MLNNativeNetworkDelegate <NSObject>

@optional

- (NSURLSession *)sessionForNetworkManager:(MLNNativeNetworkManager *)networkManager;

- (NSMutableURLRequest *)willSendRequest:(NSMutableURLRequest *)request;

- (MLNInternalNetworkResponse *)didReceiveResponse:(MLNInternalNetworkResponse *)response;

@required

- (NSURLSessionConfiguration *)sessionConfiguration;

- (void)startDownloadEvent:(NSString *)event type:(NSString *)type;

- (void)cancelDownloadEventForResponse:(NSURLResponse *)response;

- (void)stopDownloadEventForResponse:(NSURLResponse *)response;

- (void)debugLog:(NSString *)message;

- (void)errorLog:(NSString *)message;

@end

#define MLN_APPLE_EXPORT __attribute__((visibility("default")))

@interface MLNNativeNetworkManager : NSObject

+ (MLNNativeNetworkManager *)sharedManager;

@property (nonatomic, weak) id<MLNNativeNetworkDelegate> delegate;

@property (nonatomic, readonly) NSURLSessionConfiguration *sessionConfiguration;

- (void)startDownloadEvent:(NSString *)event type:(NSString *)type;

- (void)cancelDownloadEventForResponse:(NSURLResponse *)response;

- (void)stopDownloadEventForResponse:(NSURLResponse *)response;

- (void)debugLog:(NSString *)format, ...;

- (void)errorLog:(NSString *)format, ...;

@end

NS_ASSUME_NONNULL_END
