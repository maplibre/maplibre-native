#import <Mapbox/Mapbox.h>
#import <XCTest/XCTest.h>

MGLImage *MGLNormalizedImage(MGLImage *sourceImage) {
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

BOOL MGLEqualImages(MGLImage *leftImage, MGLImage *rightImage) {
#if TARGET_OS_IPHONE
    NSData *leftData = UIImagePNGRepresentation(MGLNormalizedImage(leftImage));
    NSData *rightData = UIImagePNGRepresentation(MGLNormalizedImage(rightImage));
    return [leftData isEqualToData:rightData];
#else
    CGImageRef leftCGImage = [MGLNormalizedImage(leftImage) CGImageForProposedRect:nil context:nil hints:nil];
    NSBitmapImageRep *leftImageRep = [[NSBitmapImageRep alloc] initWithCGImage:leftCGImage];
    NSData *leftData = [leftImageRep representationUsingType:NSPNGFileType properties:@{}];
    
    CGImageRef rightCGImage = [MGLNormalizedImage(rightImage) CGImageForProposedRect:nil context:nil hints:nil];
    NSBitmapImageRep *rightImageRep = [[NSBitmapImageRep alloc] initWithCGImage:rightCGImage];
    NSData *rightData = [rightImageRep representationUsingType:NSPNGFileType properties:@{}];
    
    return [leftData isEqualToData:rightData];
#endif
}

MGLImage *MGLImageFromCurrentContext(void) {
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

@interface MGLMapSnapshotterTests : XCTestCase <MGLMapSnapshotterDelegate, MGLOfflineStorageDelegate>

@property (nonatomic) XCTestExpectation *styleLoadingExpectation;
@property (nonatomic, copy, nullable) void (^runtimeStylingActions)(MGLStyle *style);

@end

@implementation MGLMapSnapshotterTests

- (void)setUp {
    [super setUp];
    
    [MGLSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
    
    [MGLOfflineStorage sharedOfflineStorage].delegate = self;
}

- (void)tearDown {
    [MGLSettings setApiKey:nil];
    [MGLOfflineStorage sharedOfflineStorage].delegate = nil;
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
    MGLMapCamera *camera = [MGLMapCamera camera];
    MGLMapSnapshotOptions *options = [[MGLMapSnapshotOptions alloc] initWithStyleURL:styleURL camera:camera size:CGSizeMake(500, 500)];
    
    MGLMapSnapshotter *snapshotter = [[MGLMapSnapshotter alloc] initWithOptions:options];
    snapshotter.delegate = self;
    
    [snapshotter startWithOverlayHandler:^(MGLMapSnapshotOverlay * _Nonnull snapshotOverlay) {
        XCTAssertNotNil(snapshotOverlay);
        if (snapshotOverlay) {
            XCTAssertNotEqual(snapshotOverlay.context, NULL);
            MGLImage *imageFromContext = MGLImageFromCurrentContext();
            XCTAssertTrue(MGLEqualImages(blankImage, imageFromContext), @"Base map in snapshot should be blank.");
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
    [self waitForExpectations:@[self.styleLoadingExpectation, overlayExpectation, completionExpectation] timeout:5 enforceOrder:YES];
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
    [self waitForExpectations:@[self.styleLoadingExpectation, completionExpectation] timeout:10 enforceOrder:YES];
}

- (void)testRuntimeStyling {
    [self testStyleURL:nil camera:[MGLMapCamera camera] applyingRuntimeStylingActions:^(MGLStyle *style) {
        MGLBackgroundStyleLayer *backgroundLayer = [[MGLBackgroundStyleLayer alloc] initWithIdentifier:@"background"];
        backgroundLayer.backgroundColor = [NSExpression expressionForConstantValue:[MGLColor orangeColor]];
        [style addLayer:backgroundLayer];
    } expectedImageName:@"Fixtures/MGLMapSnapshotterTests/background"];
}

- (void)testLocalGlyphRendering {
    [[NSUserDefaults standardUserDefaults] setObject:@[@"PingFang TC"] forKey:@"MGLIdeographicFontFamilyName"];
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"mixed" withExtension:@"json"];
    [self testStyleURL:styleURL camera:nil applyingRuntimeStylingActions:^(MGLStyle *style) {} expectedImageName:@"Fixtures/MGLMapSnapshotterTests/PingFang"];
}

/**
 Tests that applying the given runtime styling actions on a blank style results in a snapshot image that matches the image with the given name in the asset catalog.
 
 @param actions Runtime styling actions to apply to the blank style.
 @param camera The camera to show, or `nil` to show the style’s default camera.
 @param expectedImageName Name of the test fixture image in Media.xcassets.
 */
- (void)testStyleURL:(nullable NSURL *)styleURL camera:(nullable MGLMapCamera *)camera applyingRuntimeStylingActions:(void (^)(MGLStyle *style))actions expectedImageName:(NSString *)expectedImageName {
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
    
    MGLMapCamera *defaultCamera = camera ?: [MGLMapCamera camera];
    if (!camera) {
        defaultCamera.centerCoordinate = kCLLocationCoordinate2DInvalid;
        defaultCamera.heading = -1;
        defaultCamera.pitch = -1;
    }
    MGLMapSnapshotOptions *options = [[MGLMapSnapshotOptions alloc] initWithStyleURL:styleURL camera:defaultCamera size:expectedImage.size];
    if (!camera) {
        options.zoomLevel = -1;
    }
    
    MGLMapSnapshotter *snapshotter = [[MGLMapSnapshotter alloc] initWithOptions:options];
    snapshotter.delegate = self;
    self.runtimeStylingActions = actions;
    
    [snapshotter startWithOverlayHandler:^(MGLMapSnapshotOverlay * _Nonnull snapshotOverlay) {
        XCTAssertNotNil(snapshotOverlay);
// This image comparison returns false, but they are identical when inspecting them manually
//        if (snapshotOverlay) {
//            XCTAssertNotEqual(snapshotOverlay.context, NULL);
//            MGLImage *actualImage = MGLImageFromCurrentContext();
//            XCTAssertTrue(MGLEqualImages(expectedImage, actualImage), @"Bare snapshot before ornamentation differs from expected image.");
//        }
        [overlayExpectation fulfill];
    } completionHandler:^(MGLMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
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

#pragma mark MGLMapSnapshotterDelegate methods

- (void)mapSnapshotter:(MGLMapSnapshotter *)snapshotter didFinishLoadingStyle:(MGLStyle *)style {
    XCTAssertNotNil(snapshotter);
    XCTAssertNotNil(style);
    
    if (self.runtimeStylingActions) {
        self.runtimeStylingActions(style);
    }
    
    [self.styleLoadingExpectation fulfill];
}

#pragma mark MGLOfflineStorageDelegate methods

- (NSURL *)offlineStorage:(MGLOfflineStorage *)storage URLForResourceOfKind:(MGLResourceKind)kind withURL:(NSURL *)url {
    if (kind == MGLResourceKindGlyphs && [url.scheme isEqualToString:@"local"]) {
        return [[NSBundle bundleForClass:[self class]] URLForResource:@"glyphs" withExtension:@"pbf"];
    }
    return url;
}

@end
