#import "MLNIntegrationTestCase.h"

@interface MLNMapViewIntegrationTest : MLNIntegrationTestCase <MLNMapViewDelegate>
@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) UIWindow *window;
@property (nonatomic) MLNStyle *style;
@property (nonatomic) XCTestExpectation *styleLoadingExpectation;
@property (nonatomic) XCTestExpectation *renderFinishedExpectation;
@property (nonatomic) MLNAnnotationView * (^viewForAnnotation)(MLNMapView *mapView, id<MLNAnnotation> annotation);
@property (nonatomic) void (^regionWillChange)(MLNMapView *mapView, BOOL animated);
@property (nonatomic) void (^regionIsChanging)(MLNMapView *mapView);
@property (nonatomic) void (^regionDidChange)(MLNMapView *mapView, MLNCameraChangeReason reason, BOOL animated);
@property (nonatomic) CGPoint (^mapViewUserLocationAnchorPoint)(MLNMapView *mapView);
@property (nonatomic) BOOL (^mapViewAnnotationCanShowCalloutForAnnotation)(MLNMapView *mapView, id<MLNAnnotation> annotation);
@property (nonatomic) id<MLNCalloutView> (^mapViewCalloutViewForAnnotation)(MLNMapView *mapView, id<MLNAnnotation> annotation);

// Utility methods
- (void)waitForMapViewToFinishLoadingStyleWithTimeout:(NSTimeInterval)timeout;
- (void)waitForMapViewToBeRenderedWithTimeout:(NSTimeInterval)timeout;
- (MLNMapView *)mapViewForTestWithFrame:(CGRect)rect styleURL:(NSURL *)styleURL;
@end
