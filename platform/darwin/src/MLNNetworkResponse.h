//
//  MLNNetworkResponse.h
//  App
//
//  Created by Malcolm Toon on 9/22/25.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MLNNetworkResponse : NSObject

@property (retain, nullable) NSError *error;
@property (retain, nullable) NSData *data;
@property (retain, nullable) NSURLResponse *response;

+ (MLNNetworkResponse *)responseWithData:(NSData *)data
                             urlResponse:(NSURLResponse *)response
                                   error:(NSError *)error;

@end

NS_ASSUME_NONNULL_END
