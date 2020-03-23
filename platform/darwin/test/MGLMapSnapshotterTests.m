#import <Mapbox/Mapbox.h>
#import <XCTest/XCTest.h>

@interface MGLMapSnapshotterTests : XCTestCase <MGLMapSnapshotterDelegate>

@property (nonatomic) XCTestExpectation *styleLoadingExpectation;

@end

@implementation MGLMapSnapshotterTests

- (void)setUp {
    [super setUp];
    
    [MGLAccountManager setAccessToken:@"pk.feedcafedeadbeefbadebede"];
    
    // Initialize file system and run loop before snapshotting.
    // https://github.com/mapbox/mapbox-gl-native-ios/issues/227
    (void)[MGLOfflineStorage sharedOfflineStorage];
}

- (void)tearDown {
    [MGLAccountManager setAccessToken:nil];
    self.styleLoadingExpectation = nil;
    [super tearDown];
}

- (void)testOverlayHandler {
    self.styleLoadingExpectation = [self expectationWithDescription:@"Style should finish loading."];
    XCTestExpectation *overlayExpectation = [self expectationWithDescription:@"Overlay handler should get called."];
    XCTestExpectation *completionExpectation = [self expectationWithDescription:@"Completion handler should get called."];
    
#if TARGET_OS_IPHONE
    CGRect rect = CGRectMake(0, 0, 500, 500);
    UIGraphicsBeginImageContextWithOptions(rect.size, NO, UIScreen.mainScreen.scale);
    UIImage *blankImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
#else
    NSImage *blankImage = [NSImage imageWithSize:NSMakeSize(500, 500) flipped:NO drawingHandler:^BOOL(NSRect dstRect) {
        return YES;
    }];
    CGImageRef blankCGImage = [blankImage CGImageForProposedRect:nil context:nil hints:nil];
    NSBitmapImageRep *blankImageRep = [[NSBitmapImageRep alloc] initWithCGImage:blankCGImage];
    NSData *blankImageData = [blankImageRep representationUsingType:NSPNGFileType properties:@{}];
#endif
    
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    MGLMapCamera *camera = [MGLMapCamera camera];
    MGLMapSnapshotOptions *options = [[MGLMapSnapshotOptions alloc] initWithStyleURL:styleURL camera:camera size:CGSizeMake(500, 500)];
    
    MGLMapSnapshotter *snapshotter = [[MGLMapSnapshotter alloc] initWithOptions:options];
    snapshotter.delegate = self;
    
    [snapshotter startWithOverlayHandler:^(MGLMapSnapshotOverlay * _Nonnull snapshotOverlay) {
        XCTAssertNotNil(snapshotOverlay);
        if (snapshotOverlay) {
            XCTAssertNotEqual(snapshotOverlay.context, NULL);
#if TARGET_OS_IPHONE
            UIImage *imageFromContext = UIGraphicsGetImageFromCurrentImageContext();
            XCTAssertEqualObjects(UIImagePNGRepresentation(blankImage), UIImagePNGRepresentation(imageFromContext), @"Base map in snapshot should be blank.");
#else
            CGImageRef imageFromContext = CGBitmapContextCreateImage(snapshotOverlay.context);
            NSBitmapImageRep *imageRep = [[NSBitmapImageRep alloc] initWithCGImage:imageFromContext];
            NSData *imageData = [imageRep representationUsingType:NSPNGFileType properties:@{}];
            XCTAssertEqualObjects(blankImageData, imageData, @"Base map in snapshot should be blank.");
#endif
        }
        [overlayExpectation fulfill];
    } completionHandler:^(MGLMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(snapshot);
        if (snapshot) {
            XCTAssertEqual(snapshot.image.size.width, 500);
            XCTAssertEqual(snapshot.image.size.height, 500);
        }
        [completionExpectation fulfill];
    }];
    [self waitForExpectations:@[self.styleLoadingExpectation, overlayExpectation, completionExpectation] timeout:2 enforceOrder:YES];
}

- (void)testDelegate {
    self.styleLoadingExpectation = [self expectationWithDescription:@"Style should finish loading."];
    XCTestExpectation *completionExpectation = [self expectationWithDescription:@"Completion handler should get called."];
    
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    MGLMapCamera *camera = [MGLMapCamera camera];
    MGLMapSnapshotOptions *options = [[MGLMapSnapshotOptions alloc] initWithStyleURL:styleURL camera:camera size:CGSizeMake(500, 500)];
    
    MGLMapSnapshotter *snapshotter = [[MGLMapSnapshotter alloc] initWithOptions:options];
    snapshotter.delegate = self;
    
    [snapshotter startWithCompletionHandler:^(MGLMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(snapshot);
        if (snapshot) {
            XCTAssertEqual(snapshot.image.size.width, 500);
            XCTAssertEqual(snapshot.image.size.height, 500);
        }
        [completionExpectation fulfill];
    }];
    [self waitForExpectations:@[self.styleLoadingExpectation, completionExpectation] timeout:2 enforceOrder:YES];
}

#pragma mark MGLMapSnapshotterDelegate methods

- (void)mapSnapshotter:(MGLMapSnapshotter *)snapshotter didFinishLoadingStyle:(MGLStyle *)style {
    XCTAssertNotNil(snapshotter);
    XCTAssertNotNil(style);
    [self.styleLoadingExpectation fulfill];
}

@end
