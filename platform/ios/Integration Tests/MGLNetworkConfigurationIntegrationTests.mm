#import <XCTest/XCTest.h>
@import Mapbox;
#import "MGLMapViewIntegrationTest.h"
#import "MGLNetworkConfiguration_Private.h"
#import "MGLOfflineStorage_Private.h"
#import "MGLFoundation_Private.h"

@interface MGLNetworkConfiguration (Testing)
+ (void)testing_clearNativeNetworkManagerDelegate;
+ (id)testing_nativeNetworkManagerDelegate;
@end

@interface MGLNetworkConfigurationTestDelegate: NSObject <MGLNetworkConfigurationDelegate>
@property (nonatomic) NSURLSession *(^handler)();
@end

@interface MGLNetworkConfigurationSessionDelegate: NSObject <NSURLSessionDelegate>
@property (nonatomic) dispatch_block_t authHandler;
@end

@interface MGLNetworkConfigurationSessionDataDelegate: NSObject <NSURLSessionDataDelegate>
@property (nonatomic) void (^dataHandler)(NSURLSessionDataTask *, NSData *);
@end


@interface MGLNetworkConfigurationIntegrationTests : MGLIntegrationTestCase
@end

#define ASSERT_NATIVE_DELEGATE_IS_NIL() \
    XCTAssertNil([MGLNetworkConfiguration testing_nativeNetworkManagerDelegate])

#define ASSERT_NATIVE_DELEGATE_IS_NOT_NIL() \
    XCTAssertNotNil([MGLNetworkConfiguration testing_nativeNetworkManagerDelegate])

// NOTE: These tests are currently assumed to run in this specific order.
@implementation MGLNetworkConfigurationIntegrationTests

- (void)setUp {
    [super setUp];

    // Reset before each test
    [MGLNetworkConfiguration testing_clearNativeNetworkManagerDelegate];
}

- (void)test0_NativeNetworkManagerDelegateIsSet
{
    ASSERT_NATIVE_DELEGATE_IS_NIL();
    MGLNetworkConfiguration *config = [[MGLNetworkConfiguration alloc] init];
    [config resetNativeNetworkManagerDelegate];

    id delegate = [MGLNetworkConfiguration testing_nativeNetworkManagerDelegate];

    id<MGLNativeNetworkDelegate> manager = MGL_OBJC_DYNAMIC_CAST_AS_PROTOCOL(delegate, MGLNativeNetworkDelegate);
    XCTAssertNotNil(manager);

    // Expected properties
    XCTAssertNotNil([manager sessionConfiguration]);

    [MGLNetworkConfiguration sharedManager];
    id delegate2 = [MGLNetworkConfiguration testing_nativeNetworkManagerDelegate];
    XCTAssert(delegate != delegate2);
}

- (void)test1_NativeNetworkManagerDelegateIsSetBySharedManager
{
    ASSERT_NATIVE_DELEGATE_IS_NIL();

    // Just calling the shared manager is also sufficient (even though, it's a
    // singleton and created with a dispatch_once, the delegate is re-set for
    // each call.
    [MGLNetworkConfiguration sharedManager];
    ASSERT_NATIVE_DELEGATE_IS_NOT_NIL();

    id delegate = [MGLNetworkConfiguration testing_nativeNetworkManagerDelegate];
    id<MGLNativeNetworkDelegate> manager = MGL_OBJC_DYNAMIC_CAST_AS_PROTOCOL(delegate, MGLNativeNetworkDelegate);
    XCTAssertNotNil(manager);

    // Expected properties
    XCTAssertNotNil([manager sessionConfiguration]);
}

- (void)test2_NativeNetworkManagerDelegateIsSet
{
    ASSERT_NATIVE_DELEGATE_IS_NIL();
    [MGLNetworkConfiguration sharedManager];
    id delegate = [MGLNetworkConfiguration testing_nativeNetworkManagerDelegate];

    [[MGLNetworkConfiguration sharedManager] resetNativeNetworkManagerDelegate];
    id delegate2 = [MGLNetworkConfiguration testing_nativeNetworkManagerDelegate];
    XCTAssert(delegate == delegate2);
}

- (void)test3_NativeNetworkManagerDelegateIsSetBySharedOfflineStorage
{
    ASSERT_NATIVE_DELEGATE_IS_NIL();

    // Similar to `[MGLNetworkConfiguration sharedManager]`,
    // `[MGLOfflineStorage sharedOfflineStorage]` also sets the delegate.
    [MGLOfflineStorage sharedOfflineStorage];
    ASSERT_NATIVE_DELEGATE_IS_NOT_NIL();
}

- (void)test4_NativeNetworkManagerDelegateIsSetBySharedOfflineStorageASecondTime
{
    // Testing a second time...
    ASSERT_NATIVE_DELEGATE_IS_NIL();
    [MGLOfflineStorage sharedOfflineStorage];
    ASSERT_NATIVE_DELEGATE_IS_NOT_NIL();
}

- (void)test5_NativeNetworkManagerDelegateIsSetByMapViewInit
{
    ASSERT_NATIVE_DELEGATE_IS_NIL();
    (void)[[MGLMapView alloc] init];
    ASSERT_NATIVE_DELEGATE_IS_NOT_NIL();
}

- (void)testNetworkConfigurationDelegateIsNil
{
    MGLNetworkConfiguration *manager = [MGLNetworkConfiguration sharedManager];
    XCTAssertNil(manager.delegate);
}

- (void)internalTestNetworkConfigurationWithSession:(NSURLSession*)session shouldDownload:(BOOL)shouldDownload {

    __block BOOL didCallSessionDelegate = NO;
    __block BOOL isMainThread = YES;

    // Setup delegate object that provides a NSURLSession
    MGLNetworkConfiguration *manager = [MGLNetworkConfiguration sharedManager];
    MGLNetworkConfigurationTestDelegate *delegate = [[MGLNetworkConfigurationTestDelegate alloc] init];
    delegate.handler = ^{
        NSURLSession *internalSession;
        @synchronized (self) {
            didCallSessionDelegate = YES;
            isMainThread = [NSThread isMainThread];
            internalSession = session;
        }
        return internalSession;
    };

    manager.delegate = delegate;

    // The following is modified/taken from MGLOfflineStorageTests as we do not yet have a
    // good mechanism to test FileSource (in this SDK)
    //
    // Want to ensure we download from the network; nuclear option
    {
        XCTestExpectation *expectation = [self expectationWithDescription:@"Expect database to be reset without an error."];
        [[MGLOfflineStorage sharedOfflineStorage] resetDatabaseWithCompletionHandler:^(NSError * _Nullable error) {
            XCTAssertNil(error);
            [expectation fulfill];
        }];
        [self waitForExpectationsWithTimeout:10 handler:nil];
    }

    // Boston
    MGLCoordinateBounds bounds = {
        { .latitude = 42.360, .longitude = -71.056 },
        { .latitude = 42.358, .longitude = -71.053 },
    };
    NSURL *styleURL = [[MGLStyle predefinedStyle:@"Bright"] url];
    MGLTilePyramidOfflineRegion *region = [[MGLTilePyramidOfflineRegion alloc] initWithStyleURL:styleURL
                                                                                         bounds:bounds
                                                                                  fromZoomLevel:20
                                                                                    toZoomLevel:20];

    NSData *context = [NSKeyedArchiver archivedDataWithRootObject:@{
        @"Name": @"Faneuil Hall"
    }];

    __block MGLOfflinePack *pack = nil;

    // Add pack
    {
        XCTestExpectation *additionCompletionHandlerExpectation = [self expectationWithDescription:@"add pack completion handler"];

        [[MGLOfflineStorage sharedOfflineStorage] addPackForRegion:region
                                                       withContext:context
                                                 completionHandler:^(MGLOfflinePack * _Nullable completionHandlerPack, NSError * _Nullable error) {
            pack = completionHandlerPack;
            [additionCompletionHandlerExpectation fulfill];
        }];
        [self waitForExpectationsWithTimeout:5 handler:nil];
    }

    XCTAssert(pack.state == MGLOfflinePackStateInactive);

    // Download
    {
        XCTestExpectation *expectation = [self expectationForNotification:MGLOfflinePackProgressChangedNotification object:pack handler:^BOOL(NSNotification * _Nonnull notification) {
            return pack.state == MGLOfflinePackStateComplete;
        }];

        expectation.inverted = !shouldDownload;
        [pack resume];

        [self waitForExpectations:@[expectation] timeout:15];
    }

    XCTAssert(didCallSessionDelegate);
    XCTAssertFalse(isMainThread);

    // Remove pack, so we don't affect other tests
    {
        XCTestExpectation *expectation = [self expectationWithDescription:@"remove pack completion handler"];
        [[MGLOfflineStorage sharedOfflineStorage] removePack:pack withCompletionHandler:^(NSError * _Nullable error) {
            [expectation fulfill];
        }];
        [self waitForExpectationsWithTimeout:5 handler:nil];
    }
}

- (void)testNetworkConfigurationWithSharedSession🔒 {
    NSURLSession *session = [NSURLSession sharedSession];
    [self internalTestNetworkConfigurationWithSession:session shouldDownload:YES];
}

- (void)testNetworkConfigurationWithBackgroundSessionConfiguration {
    // Background session configurations are NOT supported, we expect this test
    // trigger an exception
    NSURLSessionConfiguration *sessionConfig = [NSURLSessionConfiguration backgroundSessionConfigurationWithIdentifier:NSStringFromSelector(_cmd)];
    NSURLSession *session = [NSURLSession sessionWithConfiguration:sessionConfig];

    XCTAssert([session isKindOfClass:NSClassFromString(@"__NSURLBackgroundSession")]);

    // We cannot do this yet, as it requires intecepting the exception in gl-native
    // It makes more sense to support background configs (requiring delegation
    // rather than blocks in gl-native)
    //  [self internalTestNetworkConfigurationWithSession:session], NSException, NSInvalidArgumentException);
}

- (void)testNetworkConfigurationWithDefaultSessionConfiguration🔒 {
    NSURLSessionConfiguration *sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
    NSURLSession *session = [NSURLSession sessionWithConfiguration:sessionConfig];
    [self internalTestNetworkConfigurationWithSession:session shouldDownload:YES];
}

- (void)testNetworkConfigurationWithEmphemeralSessionConfiguration🔒 {
    NSURLSessionConfiguration *sessionConfig = [NSURLSessionConfiguration ephemeralSessionConfiguration];
    NSURLSession *session = [NSURLSession sessionWithConfiguration:sessionConfig];
    [self internalTestNetworkConfigurationWithSession:session shouldDownload:YES];
}

- (void)testNetworkConfigurationWithSessionConfigurationWithDelegate🔒 {
    __block BOOL didCallAuthChallenge = NO;
    __block BOOL isMainThread = YES;

    NSURLSessionConfiguration *sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
    MGLNetworkConfigurationSessionDelegate *delegate = [[MGLNetworkConfigurationSessionDelegate alloc] init];
    delegate.authHandler = ^{
        @synchronized (self) {
            didCallAuthChallenge = YES;
            isMainThread = [NSThread isMainThread];
        }
    };

    NSURLSession *session = [NSURLSession sessionWithConfiguration:sessionConfig
                                                          delegate:delegate
                                                     delegateQueue:nil];
    [self internalTestNetworkConfigurationWithSession:session shouldDownload:NO];

    [session finishTasksAndInvalidate];

    XCTAssertFalse(isMainThread);
    XCTAssert(didCallAuthChallenge);
}

- (void)testFailureForNetworkConfigurationWithSessionWithDataDelegate🔒 {
    __block BOOL didCallReceiveData = NO;

    NSURLSessionConfiguration *sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
    MGLNetworkConfigurationSessionDataDelegate *delegate = [[MGLNetworkConfigurationSessionDataDelegate alloc] init];
    delegate.dataHandler = ^(NSURLSessionDataTask *task, NSData *data) {
        @synchronized (self) {
            didCallReceiveData = YES;
        }
    };

    // NOTE: Sessions with a delegate that conforms to NSURLSessionDataDelegate
    // are NOT supported.
    NSURLSession *session = [NSURLSession sessionWithConfiguration:sessionConfig
                                                          delegate:delegate
                                                     delegateQueue:nil];

    BOOL conforms = [session.delegate conformsToProtocol:@protocol(NSURLSessionDataDelegate)];
    XCTAssert(conforms);
#ifdef DEBUG
    if (conforms) {
        NSLog(@"Session delegates conforming to NSURLSessionDataDelegate are not supported");
    }
#else
    [self internalTestNetworkConfigurationWithSession:session shouldDownload:YES];
#endif
    [session finishTasksAndInvalidate];

    XCTAssertFalse(didCallReceiveData);
}

- (void)testNetworkConfigurationWithSessionConfigurationWithCustomHeaders🔒 {
    // Custom session configuration, based on `MGLNetworkConfiguration.defaultSessionConfiguration`
    NSURLSessionConfiguration *sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
    sessionConfig.HTTPAdditionalHeaders = @{ @"testing" : @YES };
    sessionConfig.HTTPMaximumConnectionsPerHost = 1;
    sessionConfig.timeoutIntervalForResource = 30;
    sessionConfig.requestCachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
    sessionConfig.URLCache = nil;

    NSURLSession *session = [NSURLSession sessionWithConfiguration:sessionConfig];

    [self internalTestNetworkConfigurationWithSession:session shouldDownload:YES];
}

@end

#pragma mark - MGLNetworkConfiguration delegate

@implementation MGLNetworkConfigurationTestDelegate
- (NSURLSession *)sessionForNetworkConfiguration:(MGLNetworkConfiguration *)configuration {
    if (self.handler) {
        return self.handler();
    }

    return nil;
}
@end

#pragma mark - NSURLSession delegate

@implementation MGLNetworkConfigurationSessionDelegate
- (void)URLSession:(NSURLSession *)session didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential * _Nullable credential))completionHandler {
    if (self.authHandler) {
        self.authHandler();
    }

    // Cancel the challenge
    completionHandler(NSURLSessionAuthChallengeCancelAuthenticationChallenge, nil);
}
@end

#pragma mark - NSURLSession data delegate

@implementation MGLNetworkConfigurationSessionDataDelegate
- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask didReceiveData:(NSData *)data {
    if (self.dataHandler) {
        self.dataHandler(dataTask, data);
    }
}
@end




