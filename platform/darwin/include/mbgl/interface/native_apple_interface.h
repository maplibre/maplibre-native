#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class MLNNativeNetworkManager;

@protocol MLNNativeNetworkDelegate <NSObject>

@optional

- (NSURLSession *)sessionForNetworkManager:(MLNNativeNetworkManager *)networkManager;

@required

- (NSURLSessionConfiguration *)sessionConfiguration;

- (void)startDownloadEvent:(NSString *)event type:(NSString *)type;

- (void)cancelDownloadEventForResponse:(NSURLResponse *)response;

- (void)stopDownloadEventForResponse:(NSURLResponse *)response;

- (void)debugLog:(NSString *)message;

- (void)errorLog:(NSString *)message;

@end

#define MLN_APPLE_EXPORT __attribute__((visibility ("default")))

@interface MLNNativeNetworkManager: NSObject

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
