#import <Foundation/Foundation.h>
#import "MLNNetworkResponse.h"

@implementation MLNNetworkResponse

+(MLNNetworkResponse *)responseWithData:(NSData *)data
                         urlResponse:(NSURLResponse *)response
                               error:(NSError *)error {

    MLNNetworkResponse *tempResult = [[MLNNetworkResponse alloc] init];
    tempResult.data = data;
    tempResult.response = response;
    tempResult.error = error;
    return tempResult;
}

@end
