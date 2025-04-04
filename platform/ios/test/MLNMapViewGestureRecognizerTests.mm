#import <Mapbox.h>
#import <XCTest/XCTest.h>

#import "../../darwin/src/MLNGeometry_Private.h"
#import "MLNMockGestureRecognizers.h"
#import "MLNMapView_Private.h"

#include <mbgl/map/map.hpp>
#include <mbgl/map/camera.hpp>

@interface MLNMapView (MLNMapViewGestureRecognizerTests)

- (void)handlePinchGesture:(UIPinchGestureRecognizer *)pinch;
- (void)handleRotateGesture:(UIRotationGestureRecognizer *)rotate;
- (void)handleDoubleTapGesture:(UITapGestureRecognizer *)doubleTap;
- (void)handleTwoFingerTapGesture:(UITapGestureRecognizer *)twoFingerTap;
- (void)handleQuickZoomGesture:(UILongPressGestureRecognizer *)quickZoom;
- (void)handleTwoFingerDragGesture:(UIPanGestureRecognizer *)twoFingerDrag;

@end

@interface MLNMapViewGestureRecognizerTests : XCTestCase <MLNMapViewDelegate>

@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) UIWindow *window;
@property (nonatomic) UIViewController *viewController;
@property (nonatomic) XCTestExpectation *styleLoadingExpectation;
@property (nonatomic) XCTestExpectation *twoFingerExpectation;
@property (nonatomic) XCTestExpectation *quickZoomExpectation;
@property (nonatomic) XCTestExpectation *doubleTapExpectation;
@property (nonatomic) XCTestExpectation *twoFingerDragExpectation;
@property (assign) CGRect screenBounds;

@end

@implementation MLNMapViewGestureRecognizerTests

- (void)setUp {
    [super setUp];

    [MLNSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    self.screenBounds = UIScreen.mainScreen.bounds;
    self.mapView = [[MLNMapView alloc] initWithFrame:self.screenBounds styleURL:styleURL];
    self.mapView.zoomLevel = 16;
    self.mapView.delegate = self;

    self.viewController = [[UIViewController alloc] init];
    self.viewController.view = [[UIView alloc] initWithFrame:self.screenBounds];
    [self.viewController.view addSubview:self.mapView];
    self.window = [[UIWindow alloc] initWithFrame:self.screenBounds];
    [self.window addSubview:self.viewController.view];
    [self.window makeKeyAndVisible];

    if (!self.mapView.style) {
        _styleLoadingExpectation = [self expectationWithDescription:@"Map view should finish loading style."];
        [self waitForExpectationsWithTimeout:10 handler:nil];
    }
}

- (void)mapView:(MLNMapView *)mapView didFinishLoadingStyle:(MLNStyle *)style {
    XCTAssertNotNil(mapView.style);
    XCTAssertEqual(mapView.style, style);

    [_styleLoadingExpectation fulfill];
}

- (void)testHandlePinchGestureContentInset {
    UIEdgeInsets contentInset = UIEdgeInsetsMake(1, 1, 1, 1);
    self.mapView.contentInset = contentInset;
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(self.mapView.contentInset);
    auto cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(padding, cameraPadding, @"MLNMapView's contentInset property should match camera's padding.");
    XCTAssertTrue(UIEdgeInsetsEqualToEdgeInsets(self.mapView.contentInset, contentInset));

    contentInset = UIEdgeInsetsMake(20, 20, 20, 20);
    [self.mapView setCamera:self.mapView.camera withDuration:0.0 animationTimingFunction:nil edgePadding:contentInset completionHandler:nil];
    XCTAssertFalse(UIEdgeInsetsEqualToEdgeInsets(self.mapView.contentInset, contentInset));

    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertNotEqual(padding, cameraPadding);

    UIPinchGestureRecognizerMock *pinchGesture = [[UIPinchGestureRecognizerMock alloc] initWithTarget:nil action:nil];
    pinchGesture.state = UIGestureRecognizerStateBegan;
    pinchGesture.scale = 1.0;
    [self.mapView handlePinchGesture:pinchGesture];
    XCTAssertNotEqual(padding, cameraPadding);

    mbgl::EdgeInsets edgePadding = MLNEdgeInsetsFromNSEdgeInsets(contentInset) + padding;

    pinchGesture.state = UIGestureRecognizerStateChanged;
    [self.mapView handlePinchGesture:pinchGesture];
    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(edgePadding, cameraPadding, @"When a gesture recognizer is performed camera paddings must not be changed.");

    pinchGesture.state = UIGestureRecognizerStateEnded;
    [self.mapView handlePinchGesture:pinchGesture];
    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(edgePadding, cameraPadding, @"When a gesture recognizer is performed camera paddings must not be changed.");
}

- (void)testHandleRotateGestureContentInset {
    UIEdgeInsets contentInset = UIEdgeInsetsMake(1, 1, 1, 1);
    self.mapView.contentInset = contentInset;
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(self.mapView.contentInset);
    auto cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(padding, cameraPadding, @"MLNMapView's contentInset property should match camera's padding.");
    XCTAssertTrue(UIEdgeInsetsEqualToEdgeInsets(self.mapView.contentInset, contentInset));

    contentInset = UIEdgeInsetsMake(20, 20, 20, 20);
    [self.mapView setCamera:self.mapView.camera withDuration:0.0 animationTimingFunction:nil edgePadding:contentInset completionHandler:nil];
    XCTAssertFalse(UIEdgeInsetsEqualToEdgeInsets(self.mapView.contentInset, contentInset));

    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertNotEqual(padding, cameraPadding);

    UIRotationGestureRecognizerMock *rotateGesture = [[UIRotationGestureRecognizerMock alloc] initWithTarget:nil action:nil];
    rotateGesture.state = UIGestureRecognizerStateBegan;
    rotateGesture.rotation = 1;
    [self.mapView handleRotateGesture:rotateGesture];
    XCTAssertNotEqual(padding, cameraPadding);

    mbgl::EdgeInsets edgePadding = MLNEdgeInsetsFromNSEdgeInsets(contentInset) + padding;

    rotateGesture.state = UIGestureRecognizerStateChanged;
    [self.mapView handleRotateGesture:rotateGesture];
    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(edgePadding, cameraPadding, @"When a gesture recognizer is performed camera paddings must not be changed.");

    rotateGesture.state = UIGestureRecognizerStateEnded;
    [self.mapView handleRotateGesture:rotateGesture];
    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(edgePadding, cameraPadding, @"When a gesture recognizer is performed camera paddings must not be changed.");
}

- (void)testHandleDoubleTapGestureContentInset {
    UIEdgeInsets contentInset = UIEdgeInsetsMake(1, 1, 1, 1);
    self.mapView.contentInset = contentInset;
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(self.mapView.contentInset);
    auto cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(padding, cameraPadding, @"MLNMapView's contentInset property should match camera's padding.");
    XCTAssertTrue(UIEdgeInsetsEqualToEdgeInsets(self.mapView.contentInset, contentInset));

    contentInset = UIEdgeInsetsMake(20, 20, 20, 20);
    [self.mapView setCamera:self.mapView.camera withDuration:0.0 animationTimingFunction:nil edgePadding:contentInset completionHandler:nil];
    XCTAssertFalse(UIEdgeInsetsEqualToEdgeInsets(self.mapView.contentInset, contentInset));

    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertNotEqual(padding, cameraPadding);

    UITapGestureRecognizerMock *doubleTapGesture = [[UITapGestureRecognizerMock alloc] initWithTarget:nil action:nil];
    doubleTapGesture.mockTappedView = self.mapView;
    doubleTapGesture.mockTappedPoint = CGPointMake(1.0, 1.0);

    mbgl::EdgeInsets edgePadding = MLNEdgeInsetsFromNSEdgeInsets(contentInset) + padding;

    [self.mapView handleDoubleTapGesture:doubleTapGesture];
    _doubleTapExpectation = [self expectationWithDescription:@"Double tap gesture animation."];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.4 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self->_doubleTapExpectation fulfill];
    });
    [self waitForExpectationsWithTimeout:10 handler:nil];

    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(edgePadding, cameraPadding, @"When a gesture recognizer is performed camera paddings must not be changed.");
}

- (void)testHandleTwoFingerTapGesture {
    UIEdgeInsets contentInset = UIEdgeInsetsMake(1, 1, 1, 1);
    self.mapView.contentInset = contentInset;
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(self.mapView.contentInset);
    auto cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(padding, cameraPadding, @"MLNMapView's contentInset property should match camera's padding.");
    XCTAssertTrue(UIEdgeInsetsEqualToEdgeInsets(self.mapView.contentInset, contentInset));

    contentInset = UIEdgeInsetsMake(20, 20, 20, 20);
    [self.mapView setCamera:self.mapView.camera withDuration:0.0 animationTimingFunction:nil edgePadding:contentInset completionHandler:nil];
    XCTAssertFalse(UIEdgeInsetsEqualToEdgeInsets(self.mapView.contentInset, contentInset));

    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertNotEqual(padding, cameraPadding);

    UITapGestureRecognizerMock *twoFingerTap = [[UITapGestureRecognizerMock alloc] initWithTarget:nil action:nil];
    twoFingerTap.mockTappedView = self.mapView;
    twoFingerTap.mockTappedPoint = CGPointMake(1.0, 1.0);

    [self.mapView handleTwoFingerTapGesture:twoFingerTap];
    _twoFingerExpectation = [self expectationWithDescription:@"Two Finger tap gesture animation."];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.4 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self->_twoFingerExpectation fulfill];
    });
    [self waitForExpectationsWithTimeout:10 handler:nil];

    mbgl::EdgeInsets edgePadding = MLNEdgeInsetsFromNSEdgeInsets(contentInset) + padding;
    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(edgePadding, cameraPadding, @"When a gesture recognizer is performed camera paddings must not be changed.");
}

- (void)testHandleQuickZoomGesture {
    UIEdgeInsets contentInset = UIEdgeInsetsMake(1, 1, 1, 1);
    self.mapView.contentInset = contentInset;
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(self.mapView.contentInset);
    auto cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(padding, cameraPadding, @"MLNMapView's contentInset property should match camera's padding.");
    XCTAssertTrue(UIEdgeInsetsEqualToEdgeInsets(self.mapView.contentInset, contentInset));

    contentInset = UIEdgeInsetsMake(20, 20, 20, 20);
    [self.mapView setCamera:self.mapView.camera withDuration:0.0 animationTimingFunction:nil edgePadding:contentInset completionHandler:nil];
    XCTAssertFalse(UIEdgeInsetsEqualToEdgeInsets(self.mapView.contentInset, contentInset));

    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertNotEqual(padding, cameraPadding);

    UILongPressGestureRecognizerMock *quickZoom = [[UILongPressGestureRecognizerMock alloc] initWithTarget:nil action:nil];
    quickZoom.state = UIGestureRecognizerStateBegan;
    [self.mapView handleQuickZoomGesture:quickZoom];
    XCTAssertNotEqual(padding, cameraPadding);

    quickZoom.state = UIGestureRecognizerStateChanged;
    quickZoom.mockTappedPoint = CGPointMake(self.mapView.frame.size.width / 2, self.mapView.frame.size.height / 2);
    [self.mapView handleQuickZoomGesture:quickZoom];
    _quickZoomExpectation = [self expectationWithDescription:@"Quick zoom gesture animation."];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.4 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self->_quickZoomExpectation fulfill];
    });
    [self waitForExpectationsWithTimeout:10 handler:nil];

    mbgl::EdgeInsets edgePadding = MLNEdgeInsetsFromNSEdgeInsets(contentInset) + padding;

    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(edgePadding, cameraPadding, @"When a gesture recognizer is performed camera paddings must not be changed.");

    quickZoom.state = UIGestureRecognizerStateEnded;
    [self.mapView handleQuickZoomGesture:quickZoom];
    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(edgePadding, cameraPadding, @"When a gesture recognizer is performed camera paddings must not be changed.");
}

- (void)testHandleTwoFingerDragGesture {
    UIEdgeInsets contentInset = UIEdgeInsetsMake(1, 1, 1, 1);
    self.mapView.contentInset = contentInset;
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(self.mapView.contentInset);
    auto cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(padding, cameraPadding, @"MLNMapView's contentInset property should match camera's padding.");
    XCTAssertTrue(UIEdgeInsetsEqualToEdgeInsets(self.mapView.contentInset, contentInset));

    contentInset = UIEdgeInsetsMake(20, 20, 20, 20);
    [self.mapView setCamera:self.mapView.camera withDuration:0.0 animationTimingFunction:nil edgePadding:contentInset completionHandler:nil];
    XCTAssertFalse(UIEdgeInsetsEqualToEdgeInsets(self.mapView.contentInset, contentInset));

    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertNotEqual(padding, cameraPadding);

    UIPanGestureRecognizerMock *twoFingerDrag = [[UIPanGestureRecognizerMock alloc] initWithTarget:nil action:nil];
    twoFingerDrag.state = UIGestureRecognizerStateBegan;
    twoFingerDrag.firstFingerPoint = CGPointMake(self.mapView.frame.size.width / 3, self.mapView.frame.size.height/2);
    twoFingerDrag.secondFingerPoint = CGPointMake((self.mapView.frame.size.width / 2), self.mapView.frame.size.height/2);
    twoFingerDrag.numberOfTouches = 2;
    [self.mapView handleTwoFingerDragGesture:twoFingerDrag];
    XCTAssertNotEqual(padding, cameraPadding);

    twoFingerDrag.state = UIGestureRecognizerStateChanged;
    twoFingerDrag.firstFingerPoint = CGPointMake(self.mapView.frame.size.width / 3, (self.mapView.frame.size.height/2)-10);
    twoFingerDrag.secondFingerPoint = CGPointMake((self.mapView.frame.size.width / 2), (self.mapView.frame.size.height/2)-10);
    [self.mapView handleTwoFingerDragGesture:twoFingerDrag];
    _twoFingerDragExpectation = [self expectationWithDescription:@"Quick zoom gesture animation."];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.4 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self->_twoFingerDragExpectation fulfill];
    });
    [self waitForExpectationsWithTimeout:10 handler:nil];

    mbgl::EdgeInsets edgePadding = MLNEdgeInsetsFromNSEdgeInsets(contentInset) + padding;

    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(edgePadding, cameraPadding, @"When a gesture recognizer is performed camera paddings must not be changed.");

    twoFingerDrag.state = UIGestureRecognizerStateEnded;
    [self.mapView handleTwoFingerDragGesture:twoFingerDrag];
    cameraPadding = self.mapView.mbglMap.getCameraOptions().padding;
    XCTAssertEqual(edgePadding, cameraPadding, @"When a gesture recognizer is performed camera paddings must not be changed.");
}

@end
