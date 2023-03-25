#import "MLNMapViewIntegrationTest.h"
#import "MLNTestUtility.h"
#import "../../../darwin/src/MLNGeometry_Private.h"

#include <mbgl/map/camera.hpp>

@interface MLNCameraTransitionFinishTests : MLNMapViewIntegrationTest
@end

@implementation MLNCameraTransitionFinishTests

- (void)testEaseToCompletionHandler {
    
    MLNCoordinateBounds bounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(0.0, 0.0),
                                                         CLLocationCoordinate2DMake(1.0, 1.0));
    MLNMapCamera *camera = [self.mapView cameraThatFitsCoordinateBounds:bounds];
    
    XCTestExpectation *expectation = [self expectationWithDescription:@"Completion block should be called"];
    
    [self.mapView setCamera:camera
               withDuration:0.0
    animationTimingFunction:nil
          completionHandler:^{
              [expectation fulfill];
          }];
    
    [self waitForExpectations:@[expectation] timeout:0.5];
}

- (void)testEaseToCompletionHandlerAnimated {
    
    MLNCoordinateBounds bounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(0.0, 0.0),
                                                         CLLocationCoordinate2DMake(1.0, 1.0));
    MLNMapCamera *camera = [self.mapView cameraThatFitsCoordinateBounds:bounds];
    
    XCTestExpectation *expectation = [self expectationWithDescription:@"Completion block should be called"];
    
    [self.mapView setCamera:camera
               withDuration:0.3
    animationTimingFunction:nil
          completionHandler:^{
              [expectation fulfill];
          }];
    
    [self waitForExpectations:@[expectation] timeout:0.5];
}

- (void)testFlyToCompletionHandler {
    
    MLNCoordinateBounds bounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(0.0, 0.0),
                                                         CLLocationCoordinate2DMake(1.0, 1.0));
    MLNMapCamera *camera = [self.mapView cameraThatFitsCoordinateBounds:bounds];
    
    XCTestExpectation *expectation = [self expectationWithDescription:@"Completion block should be called"];
    
    [self.mapView flyToCamera:camera
                 withDuration:0.0
            completionHandler:^{
                [expectation fulfill];
            }];
    
    [self waitForExpectations:@[expectation] timeout:0.5];
}

- (void)testFlyToCompletionHandlerAnimated {
    
    MLNCoordinateBounds bounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(0.0, 0.0),
                                                         CLLocationCoordinate2DMake(1.0, 1.0));
    MLNMapCamera *camera = [self.mapView cameraThatFitsCoordinateBounds:bounds];
    
    XCTestExpectation *expectation = [self expectationWithDescription:@"Completion block should be called"];
    
    [self.mapView flyToCamera:camera
                 withDuration:0.3
            completionHandler:^{
                [expectation fulfill];
            }];
    
    [self waitForExpectations:@[expectation] timeout:0.5];
}
@end

// MARK: - camera transitions with NaN values

@interface MLNMapView (MLNCameraTransitionFinishNaNTests)
- (mbgl::CameraOptions)cameraOptionsObjectForAnimatingToCamera:(MLNMapCamera *)camera edgePadding:(UIEdgeInsets)insets;
@end

@interface MLNCameraTransitionNaNZoomMapView: MLNMapView
@end

@implementation MLNCameraTransitionNaNZoomMapView
- (mbgl::CameraOptions)cameraOptionsObjectForAnimatingToCamera:(MLNMapCamera *)camera edgePadding:(UIEdgeInsets)insets {
    mbgl::CameraOptions options = [super cameraOptionsObjectForAnimatingToCamera:camera edgePadding:insets];
    options.zoom = NAN;
    return options;
}
@end

// Subclass the entire test suite, but with a different MLNMapView subclass
@interface MLNCameraTransitionFinishNaNTests : MLNCameraTransitionFinishTests
@end

@implementation MLNCameraTransitionFinishNaNTests
- (MLNMapView *)mapViewForTestWithFrame:(CGRect)rect styleURL:(NSURL *)styleURL {
    return [[MLNCameraTransitionNaNZoomMapView alloc] initWithFrame:rect styleURL:styleURL];
}
@end

