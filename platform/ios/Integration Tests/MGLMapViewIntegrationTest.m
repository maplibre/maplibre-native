#import "MGLMapViewIntegrationTest.h"

@interface MGLMapView (MGLMapViewIntegrationTest)
- (void)updateFromDisplayLink:(CADisplayLink *)displayLink;
- (void)setNeedsRerender;
@end

@implementation MGLMapViewIntegrationTest

- (MGLMapView *)mapViewForTestWithFrame:(CGRect)rect styleURL:(NSURL *)styleURL {
    return [[MGLMapView alloc] initWithFrame:UIScreen.mainScreen.bounds styleURL:styleURL];
}

- (void)setUp {
    [super setUp];

    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];

    self.mapView = [self mapViewForTestWithFrame:UIScreen.mainScreen.bounds styleURL:styleURL];
    self.mapView.delegate = self;

    UIView *superView = [[UIView alloc] initWithFrame:UIScreen.mainScreen.bounds];
    [superView addSubview:self.mapView];
    self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    [self.window addSubview:superView];
    [self.window makeKeyAndVisible];

    if (!self.mapView.style) {
        [self waitForMapViewToFinishLoadingStyleWithTimeout:10];
    }
}

- (void)tearDown {
    self.styleLoadingExpectation = nil;
    self.renderFinishedExpectation = nil;
    self.mapView = nil;
    self.style = nil;
    self.window = nil;
    [MGLSettings setApiKey:nil];

    [super tearDown];
}

#pragma mark - MGLMapViewDelegate

- (MGLAnnotationView*)mapView:(MGLMapView *)mapView viewForAnnotation:(id<MGLAnnotation>)annotation {
    if (self.viewForAnnotation) {
        return self.viewForAnnotation(mapView, annotation);
    }
    
    return nil;
}

- (void)mapView:(MGLMapView *)mapView didFinishLoadingStyle:(MGLStyle *)style {
    XCTAssertNotNil(mapView.style);
    XCTAssertEqual(mapView.style, style);

    [self.styleLoadingExpectation fulfill];
}

- (void)mapViewDidFinishRenderingFrame:(MGLMapView *)mapView fullyRendered:(__unused BOOL)fullyRendered {
    [self.renderFinishedExpectation fulfill];
    self.renderFinishedExpectation = nil;
}

- (void)mapView:(MGLMapView *)mapView regionWillChangeAnimated:(BOOL)animated {
    if (self.regionWillChange) {
        self.regionWillChange(mapView, animated);
    }
}

- (void)mapViewRegionIsChanging:(MGLMapView *)mapView {
    if (self.regionIsChanging) {
        self.regionIsChanging(mapView);
    }
}

- (void)mapView:(MGLMapView *)mapView regionDidChangeWithReason:(MGLCameraChangeReason)reason animated:(BOOL)animated {
    if (self.regionDidChange) {
        self.regionDidChange(mapView, reason, animated);
    }
}

- (CGPoint)mapViewUserLocationAnchorPoint:(MGLMapView *)mapView {
    if (self.mapViewUserLocationAnchorPoint) {
        return self.mapViewUserLocationAnchorPoint(mapView);
    }
    return CGPointZero;
}

- (BOOL)mapView:(MGLMapView *)mapView annotationCanShowCallout:(id<MGLAnnotation>)annotation {
    if (self.mapViewAnnotationCanShowCalloutForAnnotation) {
        return self.mapViewAnnotationCanShowCalloutForAnnotation(mapView, annotation);
    }
    return NO;
}

- (id<MGLCalloutView>)mapView:(MGLMapView *)mapView calloutViewForAnnotation:(id<MGLAnnotation>)annotation {
    if (self.mapViewCalloutViewForAnnotation) {
        return self.mapViewCalloutViewForAnnotation(mapView, annotation);
    }
    return nil;
}

#pragma mark - Utilities

- (void)waitForMapViewToFinishLoadingStyleWithTimeout:(NSTimeInterval)timeout {
    XCTAssertNil(self.styleLoadingExpectation);
    self.styleLoadingExpectation = [self expectationWithDescription:@"Map view should finish loading style."];
    [self waitForExpectations:@[self.styleLoadingExpectation] timeout:timeout];
    self.styleLoadingExpectation = nil;
}

- (void)waitForMapViewToBeRenderedWithTimeout:(NSTimeInterval)timeout {
    XCTAssertNil(self.renderFinishedExpectation);
    [self.mapView setNeedsRerender];
    self.renderFinishedExpectation = [self expectationWithDescription:@"Map view should be rendered"];
    [self waitForExpectations:@[self.renderFinishedExpectation] timeout:timeout];
    self.renderFinishedExpectation = nil;
}

- (void)waitForExpectations:(NSArray<XCTestExpectation *> *)expectations timeout:(NSTimeInterval)seconds {
    NSTimer *timer;

    if (self.mapView) {
        timer = [NSTimer scheduledTimerWithTimeInterval:1.0/30.0
                                                 target:self
                                               selector:@selector(updateMapViewDisplayLinkFromTimer:)
                                               userInfo:nil
                                                repeats:YES];
    }

    [super waitForExpectations:expectations timeout:seconds];
    [timer invalidate];
}

- (void)updateMapViewDisplayLinkFromTimer:(NSTimer *)timer {
    if (@available(iOS 10.0, *)) {
        // This is required for iOS 13.?, where dispatch blocks were not being
        // called - after being issued with
        // dispatch_async(dispatch_get_main_queue(), ...)
    } else {
        // Before iOS 10 it seems that the display link is not called during the
        // waitForExpectations below
        [self.mapView updateFromDisplayLink:nil];
    }
}

- (MGLStyle *)style {
    return self.mapView.style;
}

@end
