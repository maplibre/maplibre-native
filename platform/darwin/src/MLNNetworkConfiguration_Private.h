#import "MLNNetworkConfiguration.h"
#include <mbgl/interface/native_apple_interface.h>

NS_ASSUME_NONNULL_BEGIN

@class MLNNetworkConfiguration;
@protocol MLNNetworkConfigurationMetricsDelegate <NSObject>

- (void)networkConfiguration:(MLNNetworkConfiguration *)networkConfiguration didGenerateMetricEvent:(NSDictionary *)metricEvent;

@end

extern NSString * const kMLNDownloadPerformanceEvent;

@interface MLNNetworkConfiguration (Private)

@property (nonatomic, strong) NSMutableDictionary<NSString*, NSDictionary*> *events;
@property (nonatomic, weak) id<MLNNetworkConfigurationMetricsDelegate> metricsDelegate;

- (void)resetNativeNetworkManagerDelegate;
- (void)startDownloadEvent:(NSString *)urlString type:(NSString *)resourceType;
- (void)stopDownloadEventForResponse:(NSURLResponse *)response;
- (void)cancelDownloadEventForResponse:(NSURLResponse *)response;
@end

NS_ASSUME_NONNULL_END
