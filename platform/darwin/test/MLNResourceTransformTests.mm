#import <MapLibre.h>
#import <XCTest/XCTest.h>

#import "MLNOfflineStorage_Private.h"

#include <mln/storage/resource.hpp>
#include <mln/util/run_loop.hpp>

#include <memory>
#include <string>

#pragma clang diagnostic ignored "-Wshadow"

/// Covers the resource transform that ``MLNOfflineStorage`` installs for its delegate.
///
/// The transform derives the URL it hands to the delegate with `[NSURL URLWithString:]`, which
/// returns nil when the resource identifier is not a valid UTF-8 string. The delegate protocol
/// declares that URL as non-null, so callers that bridge it into a non-optional value (such as a
/// Swift `URL`) trap when it is nil.
@interface MLNResourceTransformTests : XCTestCase <MLNOfflineStorageDelegate>
@end

@implementation MLNResourceTransformTests {
  BOOL _didReceiveNilURL;
}

- (void)tearDown {
  [MLNOfflineStorage sharedOfflineStorage].delegate = nil;
  [super tearDown];
}

- (NSURL *)offlineStorage:(MLNOfflineStorage *)storage
     URLForResourceOfKind:(MLNResourceKind)kind
                  withURL:(NSURL *)url {
  if (!url) {
    _didReceiveNilURL = YES;
    // A URL with an unknown scheme keeps the request finishing deterministically.
    return [NSURL URLWithString:@"test://nil-url"];
  }

  return url;
}

- (void)testResourceWithAnUnparseableURLIsPassedThroughUnchanged {
  MLNOfflineStorage *offlineStorage = [MLNOfflineStorage sharedOfflineStorage];
  offlineStorage.delegate = self;

  std::shared_ptr<mln::FileSource> fileSource = offlineStorage.mbglOnlineFileSource;

  // Not valid UTF-8, so `[NSURL URLWithString:]` returns nil for it.
  const std::string unparseableURL("\xFF\xFE", 2);

  BOOL completed = NO;
  std::unique_ptr<mln::AsyncRequest> request;
  request =
      fileSource->request({mln::Resource::Unknown, unparseableURL}, [&](mln::Response response) {
        request.reset();
        completed = YES;
        XCTAssertTrue(response.error.get(), @"A resource with an unparseable URL cannot be "
                                            @"loaded");
        CFRunLoopStop(CFRunLoopGetCurrent());
      });

  CFRunLoopRunInMode(kCFRunLoopDefaultMode, 10.0, false);

  XCTAssertTrue(completed, @"The request should complete instead of trapping");
  XCTAssertFalse(_didReceiveNilURL, @"The delegate must not be handed a nil URL");
}

@end
