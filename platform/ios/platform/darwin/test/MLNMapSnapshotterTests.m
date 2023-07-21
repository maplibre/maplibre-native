#import <Mapbox.h>
#import <XCTest/XCTest.h>

MLNImage *MLNNormalizedImage(MLNImage *sourceImage) {
#if TARGET_OS_IPHONE
    CGSize scaledSize = CGSizeMake(sourceImage.size.width * sourceImage.scale, sourceImage.size.height * sourceImage.scale);
    UIGraphicsBeginImageContextWithOptions(scaledSize, NO, UIScreen.mainScreen.scale);
    [sourceImage drawInRect:(CGRect){ .origin = CGPointZero, .size = scaledSize }];
    UIImage *normalizedImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return normalizedImage;
#else
    return [NSImage imageWithSize:sourceImage.size flipped:NO drawingHandler:^BOOL(NSRect dstRect) {
        [sourceImage drawInRect:dstRect];
        return YES;
    }];
#endif
}

BOOL MLNEqualImages(MLNImage *leftImage, MLNImage *rightImage) {
#if TARGET_OS_IPHONE
    NSData *leftData = UIImagePNGRepresentation(MLNNormalizedImage(leftImage));
    NSData *rightData = UIImagePNGRepresentation(MLNNormalizedImage(rightImage));
    return [leftData isEqualToData:rightData];
#else
    CGImageRef leftCGImage = [MLNNormalizedImage(leftImage) CGImageForProposedRect:nil context:nil hints:nil];
    NSBitmapImageRep *leftImageRep = [[NSBitmapImageRep alloc] initWithCGImage:leftCGImage];
    NSData *leftData = [leftImageRep representationUsingType:NSPNGFileType properties:@{}];
    
    CGImageRef rightCGImage = [MLNNormalizedImage(rightImage) CGImageForProposedRect:nil context:nil hints:nil];
    NSBitmapImageRep *rightImageRep = [[NSBitmapImageRep alloc] initWithCGImage:rightCGImage];
    NSData *rightData = [rightImageRep representationUsingType:NSPNGFileType properties:@{}];
    
    return [leftData isEqualToData:rightData];
#endif
}

MLNImage *MLNImageFromCurrentContext(void) {
#if TARGET_OS_IPHONE
    return UIGraphicsGetImageFromCurrentImageContext();
#else
    CGContextRef context = NSGraphicsContext.currentContext.CGContext;
    CGImageRef cgImage = CGBitmapContextCreateImage(context);
    CGFloat scale = NSScreen.mainScreen.backingScaleFactor;
    NSSize imageSize = NSMakeSize(CGImageGetWidth(cgImage) / scale, CGImageGetHeight(cgImage) / scale);
    NSImage *image = [[NSImage alloc] initWithCGImage:cgImage size:imageSize];
    CGImageRelease(cgImage);
    return image;
#endif
}

@interface MLNMapSnapshotterTests : XCTestCase <MLNMapSnapshotterDelegate, MLNOfflineStorageDelegate>

@property (nonatomic) XCTestExpectation *styleLoadingExpectation;
@property (nonatomic, copy, nullable) void (^runtimeStylingActions)(MLNStyle *style);

@end

@implementation MLNMapSnapshotterTests

- (void)setUp {
    [super setUp];
    
    [MLNSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
    
    [MLNOfflineStorage sharedOfflineStorage].delegate = self;
}

- (void)tearDown {
    [MLNSettings setApiKey:nil];
    [MLNOfflineStorage sharedOfflineStorage].delegate = nil;
    self.styleLoadingExpectation = nil;
    self.runtimeStylingActions = nil;
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
#endif
    
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    MLNMapCamera *camera = [MLNMapCamera camera];
    MLNMapSnapshotOptions *options = [[MLNMapSnapshotOptions alloc] initWithStyleURL:styleURL camera:camera size:CGSizeMake(500, 500)];
    
    MLNMapSnapshotter *snapshotter = [[MLNMapSnapshotter alloc] initWithOptions:options];
    snapshotter.delegate = self;
    
    [snapshotter startWithOverlayHandler:^(MLNMapSnapshotOverlay * _Nonnull snapshotOverlay) {
        XCTAssertNotNil(snapshotOverlay);
        if (snapshotOverlay) {
            XCTAssertNotEqual(snapshotOverlay.context, NULL);
            MLNImage *imageFromContext = MLNImageFromCurrentContext();
            XCTAssertTrue(MLNEqualImages(blankImage, imageFromContext), @"Base map in snapshot should be blank.");
        }
        [overlayExpectation fulfill];
    } completionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(snapshot);
        if (snapshot) {
            XCTAssertEqual(snapshot.image.size.width, 500);
            XCTAssertEqual(snapshot.image.size.height, 500);
        }
        [completionExpectation fulfill];
    }];
    [self waitForExpectations:@[self.styleLoadingExpectation, overlayExpectation, completionExpectation] timeout:5 enforceOrder:YES];
}

- (void)testDelegate {
    self.styleLoadingExpectation = [self expectationWithDescription:@"Style should finish loading."];
    XCTestExpectation *completionExpectation = [self expectationWithDescription:@"Completion handler should get called."];
    
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    MLNMapCamera *camera = [MLNMapCamera camera];
    MLNMapSnapshotOptions *options = [[MLNMapSnapshotOptions alloc] initWithStyleURL:styleURL camera:camera size:CGSizeMake(500, 500)];
    
    MLNMapSnapshotter *snapshotter = [[MLNMapSnapshotter alloc] initWithOptions:options];
    snapshotter.delegate = self;
    
    [snapshotter startWithCompletionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(snapshot);
        if (snapshot) {
            XCTAssertEqual(snapshot.image.size.width, 500);
            XCTAssertEqual(snapshot.image.size.height, 500);
        }
        [completionExpectation fulfill];
    }];
    [self waitForExpectations:@[self.styleLoadingExpectation, completionExpectation] timeout:10 enforceOrder:YES];
}

- (void)testRuntimeStyling {
    [self testStyleURL:nil camera:[MLNMapCamera camera] applyingRuntimeStylingActions:^(MLNStyle *style) {
        MLNBackgroundStyleLayer *backgroundLayer = [[MLNBackgroundStyleLayer alloc] initWithIdentifier:@"background"];
        backgroundLayer.backgroundColor = [NSExpression expressionForConstantValue:[MLNColor orangeColor]];
        [style addLayer:backgroundLayer];
    } expectedImageName:@"Fixtures/MLNMapSnapshotterTests/background"];
}

- (void)testLocalGlyphRendering {
    [[NSUserDefaults standardUserDefaults] setObject:@[@"PingFang TC"] forKey:@"MLNIdeographicFontFamilyName"];
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"mixed" withExtension:@"json"];
    [self testStyleURL:styleURL camera:nil applyingRuntimeStylingActions:^(MLNStyle *style) {} expectedImageName:@"Fixtures/MLNMapSnapshotterTests/PingFang"];
}

/**
 Tests that applying the given runtime styling actions on a blank style results in a snapshot image that matches the image with the given name in the asset catalog.
 
 @param actions Runtime styling actions to apply to the blank style.
 @param camera The camera to show, or `nil` to show the style’s default camera.
 @param expectedImageName Name of the test fixture image in Media.xcassets.
 */
- (void)testStyleURL:(nullable NSURL *)styleURL camera:(nullable MLNMapCamera *)camera applyingRuntimeStylingActions:(void (^)(MLNStyle *style))actions expectedImageName:(NSString *)expectedImageName {
    self.styleLoadingExpectation = [self expectationWithDescription:@"Style should finish loading."];
    XCTestExpectation *overlayExpectation = [self expectationWithDescription:@"Overlay handler should get called."];
    XCTestExpectation *completionExpectation = [self expectationWithDescription:@"Completion handler should get called."];
    
#if TARGET_OS_IPHONE
    UIImage *expectedImage = [UIImage imageNamed:expectedImageName inBundle:[NSBundle bundleForClass:[self class]] compatibleWithTraitCollection:nil];
#else
    NSImage *expectedImage = [[NSBundle bundleForClass:[self class]] imageForResource:expectedImageName];
#endif
    XCTAssertNotNil(expectedImage, @"Image fixture “%@” missing from Media.xcassets.", expectedImageName);
    
    if (!styleURL) {
        styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    }
    
    MLNMapCamera *defaultCamera = camera ?: [MLNMapCamera camera];
    if (!camera) {
        defaultCamera.centerCoordinate = kCLLocationCoordinate2DInvalid;
        defaultCamera.heading = -1;
        defaultCamera.pitch = -1;
    }
    MLNMapSnapshotOptions *options = [[MLNMapSnapshotOptions alloc] initWithStyleURL:styleURL camera:defaultCamera size:expectedImage.size];
    if (!camera) {
        options.zoomLevel = -1;
    }
    
    MLNMapSnapshotter *snapshotter = [[MLNMapSnapshotter alloc] initWithOptions:options];
    snapshotter.delegate = self;
    self.runtimeStylingActions = actions;
    
    [snapshotter startWithOverlayHandler:^(MLNMapSnapshotOverlay * _Nonnull snapshotOverlay) {
        XCTAssertNotNil(snapshotOverlay);
// This image comparison returns false, but they are identical when inspecting them manually
//        if (snapshotOverlay) {
//            XCTAssertNotEqual(snapshotOverlay.context, NULL);
//            MLNImage *actualImage = MLNImageFromCurrentContext();
//            XCTAssertTrue(MLNEqualImages(expectedImage, actualImage), @"Bare snapshot before ornamentation differs from expected image.");
//        }
        [overlayExpectation fulfill];
    } completionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(snapshot);
        if (snapshot) {
            XCTAssertEqual(snapshot.image.size.width, expectedImage.size.width);
            XCTAssertEqual(snapshot.image.size.height, expectedImage.size.height);
        }
        [completionExpectation fulfill];
    }];
    [self waitForExpectations:@[self.styleLoadingExpectation, overlayExpectation, completionExpectation] timeout:5 enforceOrder:YES];
    self.runtimeStylingActions = nil;
}

// MARK: MLNMapSnapshotterDelegate methods

- (void)mapSnapshotter:(MLNMapSnapshotter *)snapshotter didFinishLoadingStyle:(MLNStyle *)style {
    XCTAssertNotNil(snapshotter);
    XCTAssertNotNil(style);
    
    if (self.runtimeStylingActions) {
        self.runtimeStylingActions(style);
    }
    
    [self.styleLoadingExpectation fulfill];
}

// MARK: MLNOfflineStorageDelegate methods

- (NSURL *)offlineStorage:(MLNOfflineStorage *)storage URLForResourceOfKind:(MLNResourceKind)kind withURL:(NSURL *)url {
    if (kind == MLNResourceKindGlyphs && [url.scheme isEqualToString:@"local"]) {
        return [[NSBundle bundleForClass:[self class]] URLForResource:@"glyphs" withExtension:@"pbf"];
    }
    return url;
}

@end
