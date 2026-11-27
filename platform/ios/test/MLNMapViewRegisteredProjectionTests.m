#import <MapLibre.h>
#import <XCTest/XCTest.h>

#import "MLNScaleBar.h"

static NSString *const MLNTestPolarProjectionName = @"test:ios-polar";

// GL JS's test polar stereographic projection: EPSG:3413's true scale at 70 N and central meridian
// at 45 W on a sphere, with EPSG:3413's tile matrix set.
static const double MLNTestPolarScale = 6378137 * (1 + 0.9396926207859083);
static const double MLNTestPolarExtent = 8388608;

// GL JS's meters per point at 70 N, 45 W at zoom 4, as its geolocate control measures them: the
// distance to the point 100 points to the right, over 100. Mercator's would be about 1.6 times as
// much.
static const CLLocationDistance MLNTestMetersPerPointAt70N = 1022.8325840486776;

@interface MLNMapViewRegisteredProjectionTests : XCTestCase

@property (nonatomic) MLNMapView *mapView;

@end

@implementation MLNMapViewRegisteredProjectionTests

- (void)setUp {
    [super setUp];

    MLNCoordinateReferenceSystem *polar = [[MLNCoordinateReferenceSystem alloc]
        initWithName:MLNTestPolarProjectionName
        tileMatrixSet:(MLNTileMatrixSet){{-MLNTestPolarExtent / 2, MLNTestPolarExtent / 2},
                                         MLNTestPolarExtent}
        forward:^MLNCRSPosition(CLLocationCoordinate2D coordinate) {
            double rho = MLNTestPolarScale * tan(M_PI_4 - coordinate.latitude * M_PI / 360);
            double angle = (coordinate.longitude + 45) * M_PI / 180;
            return (MLNCRSPosition){rho * sin(angle), -rho * cos(angle)};
        }
        inverse:^CLLocationCoordinate2D(MLNCRSPosition position) {
            double latitude = 90 - 2 * atan(hypot(position.x, position.y) / MLNTestPolarScale) * 180 / M_PI;
            double longitude = -45 + atan2(position.x, -position.y) * 180 / M_PI;
            return CLLocationCoordinate2DMake(latitude, longitude);
        }];
    NSError *error;
    XCTAssertTrue([MLNStyle addProjection:polar error:&error], @"%@", error);

    NSString *styleJSON = [NSString
        stringWithFormat:@"{\"version\": 8, \"projection\": {\"type\": \"%@\"}, \"sources\": {}, \"layers\": []}",
                         MLNTestPolarProjectionName];
    self.mapView = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 256, 256) styleJSON:styleJSON];
}

- (void)tearDown {
    self.mapView = nil;
    XCTAssertTrue([MLNStyle removeProjectionNamed:MLNTestPolarProjectionName error:nil]);

    [super tearDown];
}

- (void)testCoordinatesRoundTripThroughTheProjection {
    [self.mapView setCenterCoordinate:CLLocationCoordinate2DMake(90, 0) zoomLevel:1 animated:NO];

    CGPoint pole = [self.mapView convertCoordinate:CLLocationCoordinate2DMake(90, 0) toPointToView:self.mapView];
    XCTAssertEqualWithAccuracy(pole.x, 128, 1e-6);
    XCTAssertEqualWithAccuracy(pole.y, 128, 1e-6);

    // The central meridian, 45 W, points down from the pole and 45 E points right.
    CGPoint below = [self.mapView convertCoordinate:CLLocationCoordinate2DMake(70, -45) toPointToView:self.mapView];
    XCTAssertEqualWithAccuracy(below.x, 128, 1e-6);
    XCTAssertGreaterThan(below.y, 128);
    CGPoint right = [self.mapView convertCoordinate:CLLocationCoordinate2DMake(70, 45) toPointToView:self.mapView];
    XCTAssertGreaterThan(right.x, 128);
    XCTAssertEqualWithAccuracy(right.y, 128, 1e-6);

    for (NSValue *value in @[[NSValue valueWithMLNCoordinate:CLLocationCoordinate2DMake(70, -45)],
                             [NSValue valueWithMLNCoordinate:CLLocationCoordinate2DMake(75, 100)],
                             [NSValue valueWithMLNCoordinate:CLLocationCoordinate2DMake(62, -150)]]) {
        CLLocationCoordinate2D coordinate = value.MLNCoordinateValue;
        CGPoint point = [self.mapView convertCoordinate:coordinate toPointToView:self.mapView];
        CLLocationCoordinate2D back = [self.mapView convertPoint:point toCoordinateFromView:self.mapView];
        XCTAssertEqualWithAccuracy(back.latitude, coordinate.latitude, 1e-6);
        XCTAssertEqualWithAccuracy(back.longitude, coordinate.longitude, 1e-6);
    }
}

- (void)testMetersPerPointIsTheProjectionsScale {
    [self.mapView setCenterCoordinate:CLLocationCoordinate2DMake(70, -45) zoomLevel:4 animated:NO];

    XCTAssertEqualWithAccuracy([self.mapView metersPerPointAtLatitude:70], MLNTestMetersPerPointAt70N,
                               MLNTestMetersPerPointAt70N * 1e-9);
}

- (void)testScaleBarShowsTheProjectionsScale {
    self.mapView.showsScale = YES;
    [self.mapView setCenterCoordinate:CLLocationCoordinate2DMake(70, -45) zoomLevel:4 animated:NO];
    [self.mapView layoutIfNeeded];

    MLNScaleBar *scaleBar = (MLNScaleBar *)self.mapView.scaleBar;
    XCTAssertEqualWithAccuracy(scaleBar.metersPerPoint, MLNTestMetersPerPointAt70N,
                               MLNTestMetersPerPointAt70N * 1e-9);
}

- (void)testMapProjectionMeasuresAsTheMapView {
    [self.mapView setCenterCoordinate:CLLocationCoordinate2DMake(70, -45) zoomLevel:4 animated:NO];
    MLNMapProjection *projection = self.mapView.mapProjection;

    XCTAssertEqualWithAccuracy(projection.metersPerPoint, MLNTestMetersPerPointAt70N,
                               MLNTestMetersPerPointAt70N * 1e-9);
    CLLocationDistance altitude = self.mapView.camera.altitude;
    XCTAssertEqualWithAccuracy(projection.camera.altitude, altitude, altitude * 1e-9);

    MLNMapCamera *camera = projection.camera;
    camera.altitude *= 4;
    [projection setCamera:camera withEdgeInsets:UIEdgeInsetsZero];
    XCTAssertEqualWithAccuracy(projection.camera.altitude, camera.altitude, camera.altitude * 1e-9);
    [self.mapView setCamera:camera animated:NO];
    XCTAssertEqualWithAccuracy(self.mapView.zoomLevel, 2, 1e-9);
    CLLocationDistance metersPerPoint = [self.mapView metersPerPointAtLatitude:70];
    XCTAssertEqualWithAccuracy(projection.metersPerPoint, metersPerPoint, metersPerPoint * 1e-9);
}

- (void)testMetersPerPointAcrossTheAntimeridian {
    // The projection's inverse answers longitudes down to -225, so the center comes back as -190.
    [self.mapView setCenterCoordinate:CLLocationCoordinate2DMake(75, 170) zoomLevel:4 animated:NO];
    XCTAssertEqualWithAccuracy(self.mapView.centerCoordinate.longitude, -190, 1e-6);

    CLLocationDistance metersPerPoint = [self.mapView metersPerPointAtLatitude:75];
    CLLocationDistance mercator = 2 * M_PI * 6378137 * cos(75 * M_PI / 180) / (512 * pow(2, 4));
    XCTAssertLessThan(metersPerPoint, mercator * 0.9);
    MLNMapProjection *projection = self.mapView.mapProjection;
    XCTAssertEqualWithAccuracy(projection.metersPerPoint, metersPerPoint, metersPerPoint * 1e-9);
}

- (void)testCameraAltitudeCountsTheProjectionsUnitsAsMeters {
    [self.mapView setCenterCoordinate:CLLocationCoordinate2DMake(70, -45) zoomLevel:3 animated:NO];
    CLLocationDistance altitude = self.mapView.camera.altitude;

    // The projection's units per point stand where Mercator has its meters per point at the equator.
    CLLocationDistance expected = MLNAltitudeForZoomLevel(3, 0, 0, self.mapView.frame.size) *
                                  MLNTestPolarExtent / (2 * M_PI * 6378137);
    XCTAssertEqualWithAccuracy(altitude, expected, expected * 1e-9);

    // Unlike Mercator's, the altitude does not depend on the latitude.
    [self.mapView setCenterCoordinate:CLLocationCoordinate2DMake(85, -45) zoomLevel:3 animated:NO];
    XCTAssertEqualWithAccuracy(self.mapView.camera.altitude, expected, expected * 1e-9);

    MLNMapCamera *camera = self.mapView.camera;
    self.mapView.zoomLevel = 5;
    [self.mapView setCamera:camera animated:NO];
    XCTAssertEqualWithAccuracy(self.mapView.zoomLevel, 3, 1e-9);
}

@end
