#import <XCTest/XCTest.h>
#import <CoreLocation/CoreLocation.h>
#import <Mapbox.h>
#import <MapKit/MapKit.h>

@interface MLNMapCameraTests : XCTestCase

@end

@implementation MLNMapCameraTests

- (void)testEyeCoordinateInitialization {
    CLLocationCoordinate2D fountainSquare = CLLocationCoordinate2DMake(39.10152215, -84.5124439696089);
    CLLocationCoordinate2D unionTerminal = CLLocationCoordinate2DMake(39.10980955, -84.5352778794236);

    MLNMapCamera *camera = [MLNMapCamera cameraLookingAtCenterCoordinate:fountainSquare
                                                       fromEyeCoordinate:fountainSquare
                                                             eyeAltitude:1000];
    MKMapCamera *mkCamera = [MKMapCamera cameraLookingAtCenterCoordinate:fountainSquare
                                                       fromEyeCoordinate:fountainSquare
                                                             eyeAltitude:1000];
    XCTAssertEqual(camera.centerCoordinate.latitude, fountainSquare.latitude);
    XCTAssertEqual(camera.centerCoordinate.longitude, fountainSquare.longitude);
    XCTAssertEqual(camera.centerCoordinate.latitude, mkCamera.centerCoordinate.latitude);
    XCTAssertEqual(camera.centerCoordinate.longitude, mkCamera.centerCoordinate.longitude);
    XCTAssertEqual(camera.altitude, 1000, @"Eye altitude should be equivalent to altitude in untilted camera.");
    XCTAssertEqual(camera.altitude, mkCamera.altitude, @"Eye altitude in untilted camera should match MapKit.");
    XCTAssertEqual(camera.pitch, 0, @"Camera directly over center coordinate should be untilted.");
    XCTAssertEqual(camera.pitch, mkCamera.pitch, @"Camera directly over center coordinate should have same pitch as MapKit.");

    XCTAssertEqual(camera.heading, 0, @"Camera directly over center coordinate should be unrotated.");
    XCTAssertEqual(camera.heading, mkCamera.heading, @"Camera directly over center coordinate should have same heading as MapKit.");

    camera = [MLNMapCamera cameraLookingAtCenterCoordinate:fountainSquare
                                         fromEyeCoordinate:unionTerminal
                                               eyeAltitude:1000];
    mkCamera = [MKMapCamera cameraLookingAtCenterCoordinate:fountainSquare
                                          fromEyeCoordinate:unionTerminal
                                                eyeAltitude:1000];
    XCTAssertEqual(camera.centerCoordinate.latitude, fountainSquare.latitude);
    XCTAssertEqual(camera.centerCoordinate.longitude, fountainSquare.longitude);
    XCTAssertEqual(camera.centerCoordinate.latitude, mkCamera.centerCoordinate.latitude);
    XCTAssertEqual(camera.centerCoordinate.longitude, mkCamera.centerCoordinate.longitude);
    XCTAssertEqual(camera.altitude, 1000);
    XCTAssertEqual(camera.altitude, mkCamera.altitude, @"Eye altitude in tilted camera should match MapKit.");
    XCTAssertEqualWithAccuracy(camera.pitch, 65.3469146074, 0.01);
    XCTAssertEqual(camera.pitch, mkCamera.pitch);
    XCTAssertEqualWithAccuracy(camera.heading, 115.066396383, 0.01);
    XCTAssertEqualWithAccuracy(camera.heading, mkCamera.heading, 0.01);
}

- (void)testViewingDistanceInitialization {
    CLLocationCoordinate2D fountainSquare = CLLocationCoordinate2DMake(39.10152215, -84.5124439696089);
    MLNMapCamera *camera = [MLNMapCamera cameraLookingAtCenterCoordinate:fountainSquare
                                                          acrossDistance:10000
                                                                   pitch:0
                                                                 heading:0];
    MKMapCamera *mkCamera = [MKMapCamera cameraLookingAtCenterCoordinate:fountainSquare
                                                            fromDistance:10000
                                                                   pitch:0
                                                                 heading:0];
    XCTAssertEqualWithAccuracy(camera.altitude, 10000, 0.01, @"Untilted camera should use distance verbatim.");
    XCTAssertEqualWithAccuracy(camera.altitude, mkCamera.altitude, 0.01, @"Untilted camera altitude should match MapKit.");

    camera = [MLNMapCamera cameraLookingAtCenterCoordinate:fountainSquare
                                                  altitude:10000
                                                     pitch:0
                                                   heading:0];
    XCTAssertEqual(camera.altitude, 10000, @"Untilted camera should use altitude verbatim.");

    camera = [MLNMapCamera cameraLookingAtCenterCoordinate:fountainSquare
                                            acrossDistance:10000
                                                     pitch:60
                                                   heading:0];
    mkCamera = [MKMapCamera cameraLookingAtCenterCoordinate:fountainSquare
                                               fromDistance:10000
                                                      pitch:60
                                                    heading:0];
    XCTAssertEqualWithAccuracy(camera.altitude, 5000, 0.01, @"Tilted camera altitude should account for pitch.");
    XCTAssertEqualWithAccuracy(camera.altitude, mkCamera.altitude, 0.01, @"Tilted camera altitude should match MapKit.");

    camera = [MLNMapCamera cameraLookingAtCenterCoordinate:fountainSquare
                                                  altitude:10000
                                                     pitch:60
                                                   heading:0];
    XCTAssertEqual(camera.altitude, 10000, @"Tilted camera should use altitude verbatim.");
}

- (void)testViewingDistance {
    MLNMapCamera *camera = [MLNMapCamera camera];
    camera.altitude = 10000;
    XCTAssertEqual(camera.altitude, 10000);
    XCTAssertEqual(camera.viewingDistance, 10000);
    camera.viewingDistance = 10000;
    XCTAssertEqual(camera.altitude, 10000);
    XCTAssertEqual(camera.viewingDistance, 10000);

    camera.pitch = 60;
    camera.altitude = 10000;
    XCTAssertEqual(camera.altitude, 10000);
    XCTAssertEqualWithAccuracy(camera.viewingDistance, 20000, 0.01);
    camera.viewingDistance = 10000;
    XCTAssertEqualWithAccuracy(camera.altitude, 5000, 0.01);
    XCTAssertEqual(camera.viewingDistance, 10000);
}

- (void)testRollProperty {
    MLNMapCamera *camera = [MLNMapCamera camera];
    XCTAssertEqual(camera.roll, 0, @"Default camera should have zero roll.");

    camera.roll = 45;
    XCTAssertEqual(camera.roll, 45, @"Roll should be set correctly.");

    camera.roll = -30;
    XCTAssertEqual(camera.roll, -30, @"Roll should accept negative values.");

    camera.roll = 180;
    XCTAssertEqual(camera.roll, 180, @"Roll should accept 180 degrees.");
}

- (void)testRollInCopy {
    MLNMapCamera *camera = [MLNMapCamera camera];
    camera.roll = 30;

    MLNMapCamera *copy = [camera copy];
    XCTAssertEqual(copy.roll, 30, @"Copied camera should preserve roll value.");
}

- (void)testRollInEncoding {
    CLLocationCoordinate2D coord = CLLocationCoordinate2DMake(40.0, -80.0);
    MLNMapCamera *camera = [MLNMapCamera cameraLookingAtCenterCoordinate:coord
                                                                altitude:1000
                                                                   pitch:45
                                                                 heading:90];
    camera.roll = 25;

    NSData *data = [NSKeyedArchiver archivedDataWithRootObject:camera requiringSecureCoding:YES error:NULL];
    MLNMapCamera *decodedCamera = [NSKeyedUnarchiver unarchivedObjectOfClass:[MLNMapCamera class] fromData:data error:NULL];

    XCTAssertEqual(decodedCamera.roll, 25, @"Decoded camera should preserve roll value.");
}

- (void)testRollInEquality {
    CLLocationCoordinate2D coord = CLLocationCoordinate2DMake(40.0, -80.0);
    MLNMapCamera *camera1 = [MLNMapCamera cameraLookingAtCenterCoordinate:coord
                                                                 altitude:1000
                                                                    pitch:45
                                                                  heading:90];
    camera1.roll = 30;

    MLNMapCamera *camera2 = [MLNMapCamera cameraLookingAtCenterCoordinate:coord
                                                                 altitude:1000
                                                                    pitch:45
                                                                  heading:90];
    camera2.roll = 30;

    XCTAssertTrue([camera1 isEqual:camera2], @"Cameras with same roll should be equal.");
    XCTAssertTrue([camera1 isEqualToMapCamera:camera2], @"Cameras with same roll should be equal (isEqualToMapCamera).");

    camera2.roll = 35;
    XCTAssertFalse([camera1 isEqual:camera2], @"Cameras with different roll should not be equal.");
    XCTAssertFalse([camera1 isEqualToMapCamera:camera2], @"Cameras with different roll should not be equal (isEqualToMapCamera).");
}

- (void)testRollInHash {
    MLNMapCamera *camera1 = [MLNMapCamera camera];
    camera1.roll = 45;

    MLNMapCamera *camera2 = [MLNMapCamera camera];
    camera2.roll = 45;

    XCTAssertEqual(camera1.hash, camera2.hash, @"Cameras with same properties including roll should have same hash.");
}

@end
