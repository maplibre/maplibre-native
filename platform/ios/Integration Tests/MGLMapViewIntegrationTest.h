#import "MGLIntegrationTestCase.h"

@interface MGLMapViewIntegrationTest : MGLIntegrationTestCase <MGLMapViewDelegate>
@property (nonatomic) MGLMapView *mapView;
@property (nonatomic) UIWindow *window;
@property (nonatomic) MGLStyle *style;
@property (nonatomic) XCTestExpectation *styleLoadingExpectation;
@property (nonatomic) XCTestExpectation *renderFinishedExpectation;
@property (nonatomic) MGLAnnotationView * (^viewForAnnotation)(MGLMapView *mapView, id<MGLAnnotation> annotation);
@property (nonatomic) void (^regionWillChange)(MGLMapView *mapView, BOOL animated);
@property (nonatomic) void (^regionIsChanging)(MGLMapView *mapView);
@property (nonatomic) void (^regionDidChange)(MGLMapView *mapView, MGLCameraChangeReason reason, BOOL animated);
@property (nonatomic) CGPoint (^mapViewUserLocationAnchorPoint)(MGLMapView *mapView);
@property (nonatomic) BOOL (^mapViewAnnotationCanShowCalloutForAnnotation)(MGLMapView *mapView, id<MGLAnnotation> annotation);
@property (nonatomic) id<MGLCalloutView> (^mapViewCalloutViewForAnnotation)(MGLMapView *mapView, id<MGLAnnotation> annotation);

// Utility methods
- (void)waitForMapViewToFinishLoadingStyleWithTimeout:(NSTimeInterval)timeout;
- (void)waitForMapViewToBeRenderedWithTimeout:(NSTimeInterval)timeout;
- (MGLMapView *)mapViewForTestWithFrame:(CGRect)rect styleURL:(NSURL *)styleURL;
@end
