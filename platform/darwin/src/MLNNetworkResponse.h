#import <Foundation/Foundation.h>
#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

MLN_EXPORT
@interface MLNNetworkResponse : NSObject

@property (retain, nullable) NSError *error;
@property (retain, nullable) NSData *data;
@property (retain, nullable) NSURLResponse *response;

+ (MLNNetworkResponse *)responseWithData:(NSData *)data
                             urlResponse:(NSURLResponse *)response
                                   error:(NSError *)error;

@end

NS_ASSUME_NONNULL_END
