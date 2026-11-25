#import <MapLibre.h>
#import <XCTest/XCTest.h>

@interface MLNOfflineStorageGetPackTests : XCTestCase
@end

@implementation MLNOfflineStorageGetPackTests

- (void)setUp {
  [super setUp];
  [MLNSettings useWellKnownTileServer:MLNMapTiler];

  if (![MLNOfflineStorage sharedOfflineStorage].packs) {
    XCTestExpectation *expectation = [self
        keyValueObservingExpectationForObject:[MLNOfflineStorage sharedOfflineStorage]
                                      keyPath:@"packs"
                                      handler:^BOOL(id _Nonnull observedObject,
                                                    NSDictionary *_Nonnull change) {
                                        const auto changeKind = static_cast<NSKeyValueChange>(
                                            [change[NSKeyValueChangeKindKey] unsignedLongValue]);
                                        return changeKind == NSKeyValueChangeSetting;
                                      }];
    [self waitForExpectations:@[ expectation ] timeout:10];
  }
  XCTAssertNotNil([MLNOfflineStorage sharedOfflineStorage].packs,
                  @"Shared offline storage object should have loaded its packs.");
}

- (void)testGetPackWithRegionId {
  NSURL *styleURL = [[MLNStyle predefinedStyle:@"Bright"] url];
  MLNCoordinateBounds bounds = {
      {.latitude = 39.70358155855172, .longitude = -83.69506472545841},
      {.latitude = 39.703818870225376, .longitude = -83.69420641857361},
  };
  MLNTilePyramidOfflineRegion *region =
      [[MLNTilePyramidOfflineRegion alloc] initWithStyleURL:styleURL
                                                     bounds:bounds
                                              fromZoomLevel:20
                                                toZoomLevel:20];

  __block MLNOfflinePack *addedPack;
  XCTestExpectation *addExpectation = [self expectationWithDescription:@"add pack"];
  [[MLNOfflineStorage sharedOfflineStorage]
       addPackForRegion:region
            withContext:[NSData data]
      completionHandler:^(MLNOfflinePack *_Nullable pack, NSError *_Nullable error) {
        addedPack = pack;
        [addExpectation fulfill];
      }];
  [self waitForExpectations:@[ addExpectation ] timeout:5];
  XCTAssertNotNil(addedPack, @"Pack should have been added.");

  int64_t regionId = addedPack.regionId.longLongValue;

  XCTestExpectation *getExpectation = [self expectationWithDescription:@"get existing pack"];
  [[MLNOfflineStorage sharedOfflineStorage]
      getPackWithRegionId:regionId
        completionHandler:^(MLNOfflinePack *_Nullable pack, NSError *_Nullable error) {
          XCTAssertNil(error, @"Looking up an existing pack should not fail.");
          XCTAssertEqual(pack, addedPack,
                         @"Lookup should return the pack owned by the shared offline storage.");
          [getExpectation fulfill];
        }];
  [self waitForExpectations:@[ getExpectation ] timeout:5];

  XCTestExpectation *removeExpectation = [self expectationWithDescription:@"remove pack"];
  [[MLNOfflineStorage sharedOfflineStorage] removePack:addedPack
                                 withCompletionHandler:^(NSError *_Nullable error) {
                                   [removeExpectation fulfill];
                                 }];
  [self waitForExpectations:@[ removeExpectation ] timeout:5];

  XCTestExpectation *getRemovedExpectation = [self expectationWithDescription:@"get removed pack"];
  [[MLNOfflineStorage sharedOfflineStorage]
      getPackWithRegionId:regionId
        completionHandler:^(MLNOfflinePack *_Nullable pack, NSError *_Nullable error) {
          XCTAssertNil(error, @"Looking up a removed pack should not fail.");
          XCTAssertNil(pack, @"A removed pack should not be found.");
          [getRemovedExpectation fulfill];
        }];
  [self waitForExpectations:@[ getRemovedExpectation ] timeout:5];
}

- (void)testGetPackWithUnknownRegionId {
  XCTestExpectation *expectation = [self expectationWithDescription:@"get unknown pack"];
  [[MLNOfflineStorage sharedOfflineStorage]
      getPackWithRegionId:INT64_MAX
        completionHandler:^(MLNOfflinePack *_Nullable pack, NSError *_Nullable error) {
          XCTAssertNil(error, @"Looking up an unknown region should not fail.");
          XCTAssertNil(pack, @"An unknown region should not have a pack.");
          [expectation fulfill];
        }];
  [self waitForExpectations:@[ expectation ] timeout:5];
}

@end
