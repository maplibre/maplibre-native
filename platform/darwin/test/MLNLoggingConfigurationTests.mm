#import <XCTest/XCTest.h>

#import "MLNLoggingConfiguration_Private.h"
#import "MLNNetworkConfiguration_Private.h"

#include <mln/util/logging.hpp>

#ifndef MLN_LOGGING_DISABLED

@interface MLNLoggingConfigurationTests : XCTestCase
@property (nonatomic) MLNLoggingLevel previousLoggingLevel;
@property (nonatomic, copy) MLNLoggingBlockHandler previousHandler;
@end

@implementation MLNLoggingConfigurationTests

- (void)setUp {
  [super setUp];
  MLNLoggingConfiguration *configuration = [MLNLoggingConfiguration sharedConfiguration];
  self.previousLoggingLevel = configuration.loggingLevel;
  self.previousHandler = [configuration valueForKey:@"handler"];
  configuration.loggingLevel = MLNLoggingLevelVerbose;
}

- (void)tearDown {
  MLNLoggingConfiguration *configuration = [MLNLoggingConfiguration sharedConfiguration];
  configuration.handler = self.previousHandler;
  configuration.loggingLevel = self.previousLoggingLevel;
  [super tearDown];
}

// Regression tests for https://github.com/maplibre/maplibre-native/pull/694.
- (void)testNetworkErrorLogPreservesPercentEncodedURL {
  NSString *message = @"Failed to load https://example.com/map%20data.json — 地図";
  __block NSUInteger callCount = 0;
  [MLNLoggingConfiguration sharedConfiguration].handler =
      ^(MLNLoggingLevel level, NSString *function, NSUInteger line, NSString *loggedMessage) {
        callCount++;
        XCTAssertEqual(level, MLNLoggingLevelError);
        XCTAssertTrue([function containsString:@"errorLog:"]);
        XCTAssertGreaterThan(line, 0u);
        XCTAssertEqualObjects(loggedMessage, message);
      };

  MLNNetworkConfiguration *configuration = [[MLNNetworkConfiguration alloc] init];
  MLNNativeNetworkManager *networkManager = [[MLNNativeNetworkManager alloc] init];
  networkManager.delegate = (id<MLNNativeNetworkDelegate>)configuration;
  [networkManager errorLog:@"%@", message];
  XCTAssertEqual(callCount, 1u);
}

#if MLN_LOGGING_ENABLE_DEBUG
- (void)testNetworkDebugLogPreservesPercentEncodedURL {
  NSString *message = @"Requesting https://example.com/map%20data.json — 地図";
  __block NSUInteger callCount = 0;
  [MLNLoggingConfiguration sharedConfiguration].handler =
      ^(MLNLoggingLevel level, NSString *function, NSUInteger line, NSString *loggedMessage) {
        callCount++;
        XCTAssertEqual(level, MLNLoggingLevelDebug);
        XCTAssertTrue([function containsString:@"debugLog:"]);
        XCTAssertGreaterThan(line, 0u);
        XCTAssertEqualObjects(loggedMessage, message);
      };

  MLNNetworkConfiguration *configuration = [[MLNNetworkConfiguration alloc] init];
  MLNNativeNetworkManager *networkManager = [[MLNNativeNetworkManager alloc] init];
  networkManager.delegate = (id<MLNNativeNetworkDelegate>)configuration;
  [networkManager debugLog:@"%@", message];
  XCTAssertEqual(callCount, 1u);
}
#endif

- (void)testNetworkLogPreservesLiteralPercentSigns {
  NSString *message = @"Response contains literal %% and 100% complete — 地図";
  __block NSUInteger callCount = 0;
  [MLNLoggingConfiguration sharedConfiguration].handler =
      ^(MLNLoggingLevel level, NSString *function, NSUInteger line, NSString *loggedMessage) {
        callCount++;
        XCTAssertEqualObjects(loggedMessage, message);
      };

  MLNNetworkConfiguration *configuration = [[MLNNetworkConfiguration alloc] init];
  [(id<MLNNativeNetworkDelegate>)configuration errorLog:message];
  XCTAssertEqual(callCount, 1u);
}

- (void)testNetworkLogRespectsLoggingLevel {
  __block NSUInteger callCount = 0;
  MLNLoggingConfiguration *logging = [MLNLoggingConfiguration sharedConfiguration];
  logging.handler =
      ^(MLNLoggingLevel level, NSString *function, NSUInteger line, NSString *message) {
        callCount++;
      };
  MLNNetworkConfiguration *configuration = [[MLNNetworkConfiguration alloc] init];

  logging.loggingLevel = MLNLoggingLevelNone;
  [(id<MLNNativeNetworkDelegate>)configuration errorLog:@"https://example.com/map%20data.json"];
#if MLN_LOGGING_ENABLE_DEBUG
  logging.loggingLevel = MLNLoggingLevelError;
  [(id<MLNNativeNetworkDelegate>)configuration debugLog:@"https://example.com/map%20data.json"];
#endif
  XCTAssertEqual(callCount, 0u);
}

- (void)testCoreLogPreservesPercentEncodedURLAndMetadata {
  NSString *message = @"Failed to load https://example.com/map%20data.json — 地図";
  NSString *expectedMessage =
      [NSString stringWithFormat:@"[event]:HttpRequest [code]:404 [message]:%@", message];
  NSMutableArray<NSNumber *> *levels = [NSMutableArray array];
  [MLNLoggingConfiguration sharedConfiguration].handler =
      ^(MLNLoggingLevel level, NSString *function, NSUInteger line, NSString *loggedMessage) {
        [levels addObject:@(level)];
        XCTAssertEqualObjects(loggedMessage, expectedMessage);
      };

  // Exercise the installed Darwin observer synchronously and restore it afterwards.
  auto observer = mln::Log::removeObserver();
  XCTAssertNotEqual(observer.get(), nullptr);
  if (!observer) {
    return;
  }
  const std::string coreMessage = message.UTF8String;
#if MLN_LOGGING_ENABLE_DEBUG
  XCTAssertTrue(
      observer->onRecord(mln::EventSeverity::Debug, mln::Event::HttpRequest, 404, coreMessage));
#endif
  XCTAssertTrue(
      observer->onRecord(mln::EventSeverity::Info, mln::Event::HttpRequest, 404, coreMessage));
  XCTAssertTrue(
      observer->onRecord(mln::EventSeverity::Warning, mln::Event::HttpRequest, 404, coreMessage));
  XCTAssertTrue(
      observer->onRecord(mln::EventSeverity::Error, mln::Event::HttpRequest, 404, coreMessage));
  mln::Log::setObserver(std::move(observer));

  NSArray<NSNumber *> *expectedLevels = @[
#if MLN_LOGGING_ENABLE_DEBUG
    @(MLNLoggingLevelDebug),
#endif
    @(MLNLoggingLevelInfo), @(MLNLoggingLevelWarning), @(MLNLoggingLevelError)
  ];
  XCTAssertEqualObjects(levels, expectedLevels);
}

- (void)testFormattedLogStillSubstitutesArguments {
  __block NSUInteger callCount = 0;
  [MLNLoggingConfiguration sharedConfiguration].handler =
      ^(MLNLoggingLevel level, NSString *function, NSUInteger line, NSString *message) {
        callCount++;
        XCTAssertEqualObjects(message, @"HTTP 404: https://example.com/map%20data.json");
      };

  MLNLogError(@"HTTP %ld: %@", (long)404, @"https://example.com/map%20data.json");
  XCTAssertEqual(callCount, 1u);
}

@end
#endif
