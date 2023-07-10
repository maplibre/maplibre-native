#import <Mapbox.h>
#import <XCTest/XCTest.h>
#import "MLNOfflinePack_Private.h"
#import "MLNTestAssertionHandler.h"

@interface MLNOfflinePackTests : XCTestCase

@end

@implementation MLNOfflinePackTests

- (void)testInvalidation {
    MLNOfflinePack *invalidPack = [[MLNOfflinePack alloc] init];

    XCTAssertEqual(invalidPack.state, MLNOfflinePackStateInvalid, @"Offline pack should be invalid when initialized independently of MLNOfflineStorage.");

    XCTAssertThrowsSpecificNamed(invalidPack.region, NSException, MLNInvalidOfflinePackException, @"Invalid offline pack should raise an exception when accessing its region.");
    XCTAssertThrowsSpecificNamed(invalidPack.context, NSException, MLNInvalidOfflinePackException, @"Invalid offline pack should raise an exception when accessing its context.");
    XCTAssertThrowsSpecificNamed([invalidPack resume], NSException, MLNInvalidOfflinePackException, @"Invalid offline pack should raise an exception when being resumed.");
    XCTAssertThrowsSpecificNamed([invalidPack suspend], NSException, MLNInvalidOfflinePackException, @"Invalid offline pack should raise an exception when being suspended.");
}

- (void)testInvalidatingAnInvalidPack {
    MLNOfflinePack *invalidPack = [[MLNOfflinePack alloc] init];

    XCTAssertThrowsSpecificNamed([invalidPack invalidate], NSException, NSInternalInconsistencyException, @"Invalid offline pack should raise an exception when being invalidated.");

    // Now try again, without asserts
    NSAssertionHandler *oldHandler = [NSAssertionHandler currentHandler];
    MLNTestAssertionHandler *newHandler = [[MLNTestAssertionHandler alloc] initWithTestCase:self];
    [[[NSThread currentThread] threadDictionary] setValue:newHandler forKey:NSAssertionHandlerKey];

    // Make sure this doesn't crash without asserts
    [invalidPack invalidate];
    
    [[[NSThread currentThread] threadDictionary] setValue:oldHandler forKey:NSAssertionHandlerKey];
}

- (void)testProgressBoxing {
    MLNOfflinePackProgress progress = {
        .countOfResourcesCompleted = 3,
        .countOfResourcesExpected = 2,
        .countOfBytesCompleted = 7,
        .countOfTilesCompleted = 1,
        .countOfTileBytesCompleted = 6,
        .maximumResourcesExpected = UINT64_MAX,
    };
    MLNOfflinePackProgress roundTrippedProgress = [NSValue valueWithMLNOfflinePackProgress:progress].MLNOfflinePackProgressValue;

    XCTAssertEqual(progress.countOfResourcesCompleted, roundTrippedProgress.countOfResourcesCompleted, @"Completed resources should round-trip.");
    XCTAssertEqual(progress.countOfResourcesExpected, roundTrippedProgress.countOfResourcesExpected, @"Expected resources should round-trip.");
    XCTAssertEqual(progress.countOfBytesCompleted, roundTrippedProgress.countOfBytesCompleted, @"Completed bytes should round-trip.");
    XCTAssertEqual(progress.countOfTilesCompleted, roundTrippedProgress.countOfTilesCompleted, @"Completed tiles should round-trip.");
    XCTAssertEqual(progress.countOfTileBytesCompleted, roundTrippedProgress.countOfTileBytesCompleted, @"Completed tile bytes should round-trip.");
    XCTAssertEqual(progress.maximumResourcesExpected, roundTrippedProgress.maximumResourcesExpected, @"Maximum expected resources should round-trip.");
}

@end
