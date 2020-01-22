#import <XCTest/XCTest.h>
#import "MGLNetworkIntegrationManager.h"
#import "MGLNetworkConfiguration.h"
#import "MGLNetworkConfiguration_Private.h"
#import "MGLOfflineStorage_Private.h"
#import "MGLMapView.h"
#import "MGLFoundation_Private.h"

@interface MGLNetworkConfiguration (Testing)
+ (void)testing_clearNativeNetworkManagerDelegate;
+ (id)testing_nativeNetworkManagerDelegate;
@end

@interface MGLNetworkConfigurationIntegrationTests : XCTestCase
@end

#define ASSERT_DELEGATE_IS_NIL() \
    XCTAssertNil([MGLNetworkConfiguration testing_nativeNetworkManagerDelegate])

#define ASSERT_DELEGATE_IS_NOT_NIL() \
    XCTAssertNotNil([MGLNetworkConfiguration testing_nativeNetworkManagerDelegate])

// NOTE: These tests are currently assumed to run in this specific order.
@implementation MGLNetworkConfigurationIntegrationTests

- (void)setUp {
    [super setUp];

    // Reset before each test
    [MGLNetworkConfiguration testing_clearNativeNetworkManagerDelegate];
}

- (void)test1_NativeNetworkManagerDelegateIsSet
{
    ASSERT_DELEGATE_IS_NIL();
    [MGLNetworkConfiguration setNativeNetworkManagerDelegateToDefault];

    id delegate = [MGLNetworkConfiguration testing_nativeNetworkManagerDelegate];

    id<MGLNativeNetworkDelegate> manager = MGL_OBJC_DYNAMIC_CAST_AS_PROTOCOL(delegate, MGLNativeNetworkDelegate);
    XCTAssertNotNil(manager);

    // Expected properties
    XCTAssertNotNil([manager skuToken]);
    XCTAssertNotNil([manager sessionConfiguration]);
}

- (void)test2_NativeNetworkManagerDelegateIsSetBySharedManager
{
    ASSERT_DELEGATE_IS_NIL();

    // Just calling the shared manager is also sufficient (even though, it's a
    // singleton and created with a dispatch_once, the delegate is re-set for
    // each call.
    [MGLNetworkConfiguration sharedManager];
    ASSERT_DELEGATE_IS_NOT_NIL();
}

- (void)test3_NativeNetworkManagerDelegateIsSetBySharedOfflineStorage
{
    ASSERT_DELEGATE_IS_NIL();

    // Similar to `[MGLNetworkConfiguration sharedManager]`,
    // `[MGLOfflineStorage sharedOfflineStorage]` also sets the delegate.
    [MGLOfflineStorage sharedOfflineStorage];
    ASSERT_DELEGATE_IS_NOT_NIL();
}

- (void)test4_NativeNetworkManagerDelegateIsSetBySharedOfflineStorageASecondTime
{
    // Testing a second time...
    ASSERT_DELEGATE_IS_NIL();
    [MGLOfflineStorage sharedOfflineStorage];
    ASSERT_DELEGATE_IS_NOT_NIL();
}

- (void)test5_NativeNetworkManagerDelegateIsSetByMapViewInit
{
    ASSERT_DELEGATE_IS_NIL();
    (void)[[MGLMapView alloc] init];
    ASSERT_DELEGATE_IS_NOT_NIL();
}

@end
