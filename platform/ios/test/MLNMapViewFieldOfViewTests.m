#import <MapLibre.h>
#import <XCTest/XCTest.h>

@interface MLNMapViewFieldOfViewTests : XCTestCase
@property (nonatomic) MLNMapView *mapView;
@end

@implementation MLNMapViewFieldOfViewTests

- (void)setUp {
    [super setUp];

    [MLNSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    self.mapView = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 400, 400) styleURL:styleURL];
}

- (void)tearDown {
    self.mapView = nil;
    [MLNSettings setApiKey:nil];
    [super tearDown];
}

- (void)testDefaultFieldOfView {
    // 2 * atan((height / 2) / (height * 1.5))
    XCTAssertEqualWithAccuracy(self.mapView.fieldOfView, 36.8699, 0.001);
}

- (void)testFieldOfViewKeepsTheRestOfTheCamera {
    [self.mapView setCenterCoordinate:CLLocationCoordinate2DMake(38.7, -120.07) zoomLevel:12 animated:NO];
    MLNMapCamera *camera = self.mapView.camera;
    camera.pitch = 50;
    camera.heading = 30;
    self.mapView.camera = camera;
    double zoomLevel = self.mapView.zoomLevel;
    CLLocationCoordinate2D center = self.mapView.centerCoordinate;

    self.mapView.fieldOfView = 70;

    XCTAssertEqualWithAccuracy(self.mapView.fieldOfView, 70, 0.001);
    XCTAssertEqualWithAccuracy(self.mapView.zoomLevel, zoomLevel, 0.0001);
    XCTAssertEqualWithAccuracy(self.mapView.centerCoordinate.latitude, center.latitude, 0.000001);
    XCTAssertEqualWithAccuracy(self.mapView.centerCoordinate.longitude, center.longitude, 0.000001);
    XCTAssertEqualWithAccuracy(self.mapView.camera.pitch, 50, 0.001);
    XCTAssertEqualWithAccuracy(self.mapView.direction, 30, 0.001);
}

- (void)testAWiderFieldOfViewShowsMoreOfTheDistance {
    [self.mapView setCenterCoordinate:CLLocationCoordinate2DMake(38.7, -120.07) zoomLevel:12 animated:NO];
    MLNMapCamera *camera = self.mapView.camera;
    camera.pitch = 50;
    self.mapView.camera = camera;
    // A point a quarter of the way down from the top edge, on the center line.
    CGPoint point = CGPointMake(200, 100);
    CLLocationCoordinate2D narrow = [self.mapView convertPoint:point toCoordinateFromView:self.mapView];

    self.mapView.fieldOfView = 70;
    CLLocationCoordinate2D wide = [self.mapView convertPoint:point toCoordinateFromView:self.mapView];

    // The scale at the center is unchanged, so the same screen point is
    // farther from the center on the ground with the wider field of view.
    CLLocationCoordinate2D center = self.mapView.centerCoordinate;
    XCTAssertGreaterThan(narrow.latitude, center.latitude);
    XCTAssertGreaterThan(wide.latitude - center.latitude, (narrow.latitude - center.latitude) * 1.2);
}

- (void)testCameraFromAnEyeLooksDownAtTheCenter {
    // 0.02 degrees of latitude is 2,224 m along the ground, and the eye is 2,000 m up.
    CLLocationCoordinate2D eye = CLLocationCoordinate2DMake(38.70, -120.07);
    CLLocationCoordinate2D center = CLLocationCoordinate2DMake(38.72, -120.07);
    [self.mapView setCenterCoordinate:center centerAltitude:1000 fromEyeCoordinate:eye eyeAltitude:3000];

    XCTAssertEqualWithAccuracy(self.mapView.centerCoordinate.latitude, center.latitude, 0.000001);
    XCTAssertEqualWithAccuracy(self.mapView.centerCoordinate.longitude, center.longitude, 0.000001);
    XCTAssertEqualWithAccuracy(self.mapView.centerAltitude, 1000, 0.001);
    XCTAssertEqualWithAccuracy(self.mapView.direction, 0, 0.001);
    XCTAssertEqualWithAccuracy(self.mapView.camera.pitch, 48.03, 0.1);
    // The camera is 600 points from the center of a 400 point view at the default field of
    // view, and that is the 2,991 m line of sight.
    CLLocationDistance lineOfSight = [self.mapView metersPerPointAtLatitude:center.latitude] * 600;
    XCTAssertEqualWithAccuracy(lineOfSight, 2991, 15);
}

- (void)testCameraFromAnEyeFollowsTheFieldOfView {
    CLLocationCoordinate2D eye = CLLocationCoordinate2DMake(38.70, -120.07);
    CLLocationCoordinate2D center = CLLocationCoordinate2DMake(38.72, -120.07);
    self.mapView.fieldOfView = 90;
    [self.mapView setCenterCoordinate:center centerAltitude:1000 fromEyeCoordinate:eye eyeAltitude:3000];

    // At 90 degrees the camera is 200 points from the center of a 400 point view.
    CLLocationDistance lineOfSight = [self.mapView metersPerPointAtLatitude:center.latitude] * 200;
    XCTAssertEqualWithAccuracy(lineOfSight, 2991, 15);
    XCTAssertEqualWithAccuracy(self.mapView.camera.pitch, 48.03, 0.1);
}

- (void)testCameraFromAnEyeLooksUpAtACenterAboveIt {
    CLLocationCoordinate2D eye = CLLocationCoordinate2DMake(38.70, -120.07);
    CLLocationCoordinate2D center = CLLocationCoordinate2DMake(38.70, -120.05);
    self.mapView.maximumPitch = 180;
    [self.mapView setCenterCoordinate:center centerAltitude:3000 fromEyeCoordinate:eye eyeAltitude:1000];

    XCTAssertEqualWithAccuracy(self.mapView.centerAltitude, 3000, 0.001);
    XCTAssertEqualWithAccuracy(self.mapView.direction, 90, 0.001);
    // 0.02 degrees of longitude at this latitude is 1,736 m: 49 degrees above level.
    XCTAssertEqualWithAccuracy(self.mapView.camera.pitch, 90 + 49.04, 0.1);
}

- (void)testFieldOfViewIsLimited {
    self.mapView.fieldOfView = 200;
    XCTAssertEqualWithAccuracy(self.mapView.fieldOfView, 150, 0.001);
    self.mapView.fieldOfView = 0;
    XCTAssertEqualWithAccuracy(self.mapView.fieldOfView, 0.1, 0.001);
}

@end
