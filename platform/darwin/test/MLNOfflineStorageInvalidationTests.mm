#import <MapLibre.h>
#import <XCTest/XCTest.h>

#import "MLNOfflineStorage_Private.h"

#include <mln/storage/resource_options.hpp>
#include <mln/util/client_options.hpp>

#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

namespace {

class HeldInvalidationFileSource final : public mln::DatabaseFileSource {
public:
  HeldInvalidationFileSource()
      : DatabaseFileSource(mln::ResourceOptions().withCachePath(":memory:"), mln::ClientOptions()) {
  }

  void invalidateOfflineRegion(const mln::OfflineRegion &region,
                               std::function<void(std::exception_ptr)> callback) override {
    regionID = region.getID();
    pending = std::move(callback);
  }

  void complete(std::exception_ptr exception) {
    auto callback = std::move(pending);
    callback(exception);
  }

  int64_t regionID = -1;
  std::function<void(std::exception_ptr)> pending;
};

}  // namespace

@interface MLNOfflineStorageInvalidationTests : XCTestCase
- (void)assertInvalidationCompletionWithException:(std::exception_ptr)exception;
- (MLNOfflinePack *)addPackToStorage:(MLNOfflineStorage *)storage;
- (void)removePack:(MLNOfflinePack *)pack fromStorage:(MLNOfflineStorage *)storage;
@end

@implementation MLNOfflineStorageInvalidationTests

- (void)testInvalidatePackWaitsForDatabaseSuccess {
  [self assertInvalidationCompletionWithException:nullptr];
}

- (void)testInvalidatePackPropagatesDelayedDatabaseFailure {
  [self assertInvalidationCompletionWithException:std::make_exception_ptr(
                                                      std::runtime_error("invalidation failed"))];
}

- (void)testInvalidateRemovedPackCompletesAsynchronouslyWithoutDatabaseAccess {
  MLNOfflineStorage *storage = [MLNOfflineStorage sharedOfflineStorage];
  MLNOfflinePack *pack = [self addPackToStorage:storage];
  if (!pack) {
    return;
  }
  [self removePack:pack fromStorage:storage];
  XCTAssertEqual(pack.state, MLNOfflinePackStateInvalid);

  auto originalFileSource = storage.mbglDatabaseFileSource;
  auto fileSource = std::make_shared<HeldInvalidationFileSource>();
  storage.mbglDatabaseFileSource = fileSource;
  @try {
    __block BOOL returned = NO;
    __block NSUInteger completionCount = 0;
    XCTestExpectation *completed = [self expectationWithDescription:@"removed pack completed"];
    completed.assertForOverFulfill = YES;
    dispatch_async(dispatch_get_main_queue(), ^{
      [storage invalidatePack:pack
          withCompletionHandler:^(NSError *error) {
            XCTAssertTrue(returned, @"Completion must be asynchronous for a removed pack too.");
            XCTAssertTrue([NSThread isMainThread]);
            XCTAssertNil(error);
            ++completionCount;
            [completed fulfill];
          }];
      returned = YES;
    });
    [self waitForExpectations:@[ completed ] timeout:5];
    XCTAssertEqual(completionCount, 1u);
    XCTAssertFalse(static_cast<bool>(fileSource->pending));
  } @finally {
    storage.mbglDatabaseFileSource = originalFileSource;
  }
}

- (void)assertInvalidationCompletionWithException:(std::exception_ptr)exception {
  MLNOfflineStorage *storage = [MLNOfflineStorage sharedOfflineStorage];
  MLNOfflinePack *pack = [self addPackToStorage:storage];
  XCTAssertNotNil(pack);
  if (!pack) {
    return;
  }

  auto originalFileSource = storage.mbglDatabaseFileSource;
  auto fileSource = std::make_shared<HeldInvalidationFileSource>();
  storage.mbglDatabaseFileSource = fileSource;
  @try {
    __block NSUInteger completionCount = 0;
    __block NSError *completionError;
    __block XCTestExpectation *completed;
    [storage invalidatePack:pack
        withCompletionHandler:^(NSError *error) {
          XCTAssertTrue([NSThread isMainThread]);
          ++completionCount;
          completionError = error;
          [completed fulfill];
        }];
    XCTAssertEqual(fileSource->regionID, pack.regionId.longLongValue);
    XCTAssertTrue(static_cast<bool>(fileSource->pending));
    if (!fileSource->pending) {
      return;
    }

    XCTestExpectation *mainQueueFence = [self expectationWithDescription:@"main queue drained"];
    dispatch_async(dispatch_get_main_queue(), ^{
      XCTAssertEqual(completionCount, 0u,
                     @"Invalidation must not complete while the database callback is held.");
      [mainQueueFence fulfill];
    });
    [self waitForExpectations:@[ mainQueueFence ] timeout:5];
    if (completionCount != 0) {
      // Do not invoke the old implementation's callback after its stack captures expired.
      return;
    }

    completed = [self expectationWithDescription:@"invalidation completed"];
    completed.assertForOverFulfill = YES;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
      fileSource->complete(exception);
    });
    [self waitForExpectations:@[ completed ] timeout:5];
    XCTAssertEqual(completionCount, 1u);
    if (exception) {
      XCTAssertEqualObjects(completionError.domain, MLNErrorDomain);
      XCTAssertEqual(completionError.code, MLNErrorCodeModifyingOfflineStorageFailed);
      XCTAssertEqualObjects(completionError.localizedDescription, @"invalidation failed");
    } else {
      XCTAssertNil(completionError);
    }
  } @finally {
    storage.mbglDatabaseFileSource = originalFileSource;
    [self removePack:pack fromStorage:storage];
  }
}

- (MLNOfflinePack *)addPackToStorage:(MLNOfflineStorage *)storage {
  MLNCoordinateBounds bounds = {{51, 0}, {51.001, 0.001}};
  MLNTilePyramidOfflineRegion *region = [[MLNTilePyramidOfflineRegion alloc]
      initWithStyleURL:[NSURL URLWithString:@"https://example.com/style.json"]
                bounds:bounds
         fromZoomLevel:0
           toZoomLevel:0];
  XCTestExpectation *added = [self expectationWithDescription:@"pack added"];
  __block MLNOfflinePack *pack;
  [storage addPackForRegion:region
                withContext:[[NSUUID UUID].UUIDString dataUsingEncoding:NSUTF8StringEncoding]
          completionHandler:^(MLNOfflinePack *result, NSError *error) {
            XCTAssertNil(error);
            pack = result;
            [added fulfill];
          }];
  [self waitForExpectations:@[ added ] timeout:5];
  XCTAssertNotNil(pack);
  return pack;
}

- (void)removePack:(MLNOfflinePack *)pack fromStorage:(MLNOfflineStorage *)storage {
  XCTestExpectation *removed = [self expectationWithDescription:@"pack removed"];
  [storage removePack:pack
      withCompletionHandler:^(NSError *error) {
        XCTAssertNil(error);
        [removed fulfill];
      }];
  [self waitForExpectations:@[ removed ] timeout:5];
}

@end
