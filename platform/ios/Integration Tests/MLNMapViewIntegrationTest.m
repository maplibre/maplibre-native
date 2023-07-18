#import "MLNMapViewIntegrationTest.h"

@interface MLNMapView (MLNMapViewIntegrationTest)
- (void)updateFromDisplayLink:(CADisplayLink *)displayLink;
- (void)setNeedsRerender;
@end

@implementation MLNMapViewIntegrationTest

- (MLNMapView *)mapViewForTestWithFrame:(CGRect)rect styleURL:(NSURL *)styleURL {
    return [[MLNMapView alloc] initWithFrame:UIScreen.mainScreen.bounds styleURL:styleURL];
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
    [MLNSettings setApiKey:nil];

    [super tearDown];
}

// MARK: - MLNMapViewDelegate

- (MLNAnnotationView*)mapView:(MLNMapView *)mapView viewForAnnotation:(id<MLNAnnotation>)annotation {
    if (self.viewForAnnotation) {
        return self.viewForAnnotation(mapView, annotation);
    }
    
    return nil;
}

- (void)mapView:(MLNMapView *)mapView didFinishLoadingStyle:(MLNStyle *)style {
    XCTAssertNotNil(mapView.style);
    XCTAssertEqual(mapView.style, style);

    [self.styleLoadingExpectation fulfill];
}

- (void)mapViewDidFinishRenderingFrame:(MLNMapView *)mapView fullyRendered:(__unused BOOL)fullyRendered {
    [self.renderFinishedExpectation fulfill];
    self.renderFinishedExpectation = nil;
}

- (void)mapView:(MLNMapView *)mapView regionWillChangeAnimated:(BOOL)animated {
    if (self.regionWillChange) {
        self.regionWillChange(mapView, animated);
    }
}

- (void)mapViewRegionIsChanging:(MLNMapView *)mapView {
    if (self.regionIsChanging) {
        self.regionIsChanging(mapView);
    }
}

- (void)mapView:(MLNMapView *)mapView regionDidChangeWithReason:(MLNCameraChangeReason)reason animated:(BOOL)animated {
    if (self.regionDidChange) {
        self.regionDidChange(mapView, reason, animated);
    }
}

- (CGPoint)mapViewUserLocationAnchorPoint:(MLNMapView *)mapView {
    if (self.mapViewUserLocationAnchorPoint) {
        return self.mapViewUserLocationAnchorPoint(mapView);
    }
    return CGPointZero;
}

- (BOOL)mapView:(MLNMapView *)mapView annotationCanShowCallout:(id<MLNAnnotation>)annotation {
    if (self.mapViewAnnotationCanShowCalloutForAnnotation) {
        return self.mapViewAnnotationCanShowCalloutForAnnotation(mapView, annotation);
    }
    return NO;
}

- (id<MLNCalloutView>)mapView:(MLNMapView *)mapView calloutViewForAnnotation:(id<MLNAnnotation>)annotation {
    if (self.mapViewCalloutViewForAnnotation) {
        return self.mapViewCalloutViewForAnnotation(mapView, annotation);
    }
    return nil;
}

// MARK: - Utilities

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

- (MLNStyle *)style {
    return self.mapView.style;
}

@end
