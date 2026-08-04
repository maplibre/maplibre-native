#import <Mapbox.h>
#import <XCTest/XCTest.h>
#import "MLNNetworkConfiguration_Private.h"

@interface MLNTestNetworkConfigurationDelegate : NSObject <MLNNetworkConfigurationDelegate>
@property (nonatomic) NSUInteger receivedResponseCount;
@property (nonatomic, nullable) NSURLResponse *replacementResponse;
@end

@implementation MLNTestNetworkConfigurationDelegate

- (MLNNetworkResponse *)didReceiveResponse:(MLNNetworkResponse *)response {
    self.receivedResponseCount += 1;
    if (self.replacementResponse) {
        response.response = self.replacementResponse;
    }
    return response;
}

@end

@interface MLNNetworkConfigurationTests : XCTestCase
@end

@implementation MLNNetworkConfigurationTests

// Regression test for https://github.com/mapbox/mapbox-gl-native/issues/14982
- (void)testAccessingEventsFromMultipleThreads {
    MLNNetworkConfiguration *configuration = [[MLNNetworkConfiguration alloc] init];

    // Concurrent
    dispatch_queue_t queue = dispatch_queue_create("org.maplibre.testAccessingEventsFromMultipleThreads", DISPATCH_QUEUE_CONCURRENT);

    NSUInteger numberOfConcurrentBlocks = 20;

    XCTestExpectation *expectation = [self expectationWithDescription:@"wait-for-threads"];
    expectation.expectedFulfillmentCount = numberOfConcurrentBlocks;

    for (NSUInteger i = 0; i < numberOfConcurrentBlocks; i++) {

        NSString *event = [NSString stringWithFormat:@"test://event-%ld", i];
        NSString *resourceType = @"test";

        dispatch_async(queue, ^{
            [configuration startDownloadEvent:event type:resourceType];

            NSURL *url = [NSURL URLWithString:event];
            NSURLResponse *response = [[NSURLResponse alloc] initWithURL:url MIMEType:nil expectedContentLength:0 textEncodingName:nil];

            [configuration stopDownloadEventForResponse:response];

            dispatch_async(dispatch_get_main_queue(), ^{
                [expectation fulfill];
            });
        });
    }

    [self waitForExpectations:@[expectation] timeout:10.0];
}

// Regression test: received responses were converted for the delegate but the delegate was
// never actually called, so `didReceiveResponse:` implementations had no effect.
- (void)testDidReceiveResponseIsForwardedToTheDelegate {
    MLNNetworkConfiguration *configuration = [[MLNNetworkConfiguration alloc] init];
    MLNTestNetworkConfigurationDelegate *delegate = [[MLNTestNetworkConfigurationDelegate alloc] init];
    configuration.delegate = delegate;

    NSURL *url = [NSURL URLWithString:@"test://tile"];
    delegate.replacementResponse = [[NSHTTPURLResponse alloc] initWithURL:url
                                                               statusCode:404
                                                              HTTPVersion:@"HTTP/1.1"
                                                             headerFields:nil];

    MLNInternalNetworkResponse *internalResponse = [[MLNInternalNetworkResponse alloc] init];
    internalResponse.response = [[NSHTTPURLResponse alloc] initWithURL:url
                                                            statusCode:403
                                                           HTTPVersion:@"HTTP/1.1"
                                                          headerFields:nil];
    internalResponse.data = [@"forbidden" dataUsingEncoding:NSUTF8StringEncoding];

    MLNInternalNetworkResponse *processedResponse =
        [(id<MLNNativeNetworkDelegate>)configuration didReceiveResponse:internalResponse];

    XCTAssertEqual(delegate.receivedResponseCount, 1);
    XCTAssertEqual(((NSHTTPURLResponse *)processedResponse.response).statusCode, 404);
    XCTAssertEqualObjects(processedResponse.data, internalResponse.data);
}

- (void)testDidReceiveResponseWithoutDelegateReturnsResponseUnchanged {
    MLNNetworkConfiguration *configuration = [[MLNNetworkConfiguration alloc] init];

    MLNInternalNetworkResponse *internalResponse = [[MLNInternalNetworkResponse alloc] init];
    internalResponse.data = [@"payload" dataUsingEncoding:NSUTF8StringEncoding];

    MLNInternalNetworkResponse *processedResponse =
        [(id<MLNNativeNetworkDelegate>)configuration didReceiveResponse:internalResponse];

    XCTAssertEqual(processedResponse, internalResponse);
}
@end
