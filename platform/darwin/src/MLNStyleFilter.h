#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MLNStyleFilter : NSObject

// This will filter the data passed in
- (NSData *)filterData:(NSData *)data;

@end

NS_ASSUME_NONNULL_END
