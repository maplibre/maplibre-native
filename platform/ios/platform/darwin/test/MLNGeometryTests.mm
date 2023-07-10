#import <Mapbox.h>
#import <XCTest/XCTest.h>

#import "../../darwin/src/MLNGeometry_Private.h"

@interface MLNGeometryTests : XCTestCase
@end

@implementation MLNGeometryTests

- (void)testCoordinateBoundsIsEmpty {
    MLNCoordinateBounds emptyBounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(0, 0), CLLocationCoordinate2DMake(10, 0));
    XCTAssertTrue(MLNCoordinateBoundsIsEmpty(emptyBounds));
    XCTAssertFalse(MLNCoordinateSpanEqualToCoordinateSpan(MLNCoordinateSpanZero, MLNCoordinateBoundsGetCoordinateSpan(emptyBounds)));
}

- (void)testAngleConversions {
    XCTAssertEqualWithAccuracy(-180, MLNDegreesFromRadians(-M_PI), 1e-5);
    XCTAssertEqual(0, MLNDegreesFromRadians(0));
    XCTAssertEqualWithAccuracy(45, MLNDegreesFromRadians(M_PI_4), 1e-5);
    XCTAssertEqualWithAccuracy(90, MLNDegreesFromRadians(M_PI_2), 1e-5);
    XCTAssertEqualWithAccuracy(180, MLNDegreesFromRadians(M_PI), 1e-5);
    XCTAssertEqualWithAccuracy(360, MLNDegreesFromRadians(2 * M_PI), 1e-5);
    XCTAssertEqualWithAccuracy(720, MLNDegreesFromRadians(4 * M_PI), 1e-5);
    
    XCTAssertEqualWithAccuracy(-360, MLNDegreesFromRadians(MLNRadiansFromDegrees(-360)), 1e-4);
    XCTAssertEqualWithAccuracy(-180, MLNDegreesFromRadians(MLNRadiansFromDegrees(-180)), 1e-5);
    XCTAssertEqualWithAccuracy(-90, MLNDegreesFromRadians(MLNRadiansFromDegrees(-90)), 1e-5);
    XCTAssertEqualWithAccuracy(-45, MLNDegreesFromRadians(MLNRadiansFromDegrees(-45)), 1e-5);
    XCTAssertEqualWithAccuracy(0, MLNDegreesFromRadians(MLNRadiansFromDegrees(0)), 1e-5);
    XCTAssertEqualWithAccuracy(45, MLNDegreesFromRadians(MLNRadiansFromDegrees(45)), 1e-5);
    XCTAssertEqualWithAccuracy(90, MLNDegreesFromRadians(MLNRadiansFromDegrees(90)), 1e-5);
    XCTAssertEqualWithAccuracy(180, MLNDegreesFromRadians(MLNRadiansFromDegrees(180)), 1e-5);
    XCTAssertEqualWithAccuracy(360, MLNDegreesFromRadians(MLNRadiansFromDegrees(360)), 1e-4);
}

- (void)testAltitudeConversions {
    CGSize tallSize = CGSizeMake(600, 1200);
    CGSize midSize = CGSizeMake(600, 800);
    CGSize shortSize = CGSizeMake(600, 400);
    
    XCTAssertEqualWithAccuracy(1800, MLNAltitudeForZoomLevel(MLNZoomLevelForAltitude(1800, 0, 0, midSize), 0, 0, midSize), 1e-8);
    XCTAssertLessThan(MLNZoomLevelForAltitude(1800, 0, 0, midSize), MLNZoomLevelForAltitude(1800, 0, 0, tallSize));
    XCTAssertGreaterThan(MLNZoomLevelForAltitude(1800, 0, 0, midSize), MLNZoomLevelForAltitude(1800, 0, 0, shortSize));
    
    XCTAssertEqualWithAccuracy(0, MLNZoomLevelForAltitude(MLNAltitudeForZoomLevel(0, 0, 0, midSize), 0, 0, midSize), 1e-8);
    XCTAssertEqualWithAccuracy(18, MLNZoomLevelForAltitude(MLNAltitudeForZoomLevel(18, 0, 0, midSize), 0, 0, midSize), 1e-8);
    
    XCTAssertEqualWithAccuracy(0, MLNZoomLevelForAltitude(MLNAltitudeForZoomLevel(0, 0, 40, midSize), 0, 40, midSize), 1e-8);
    XCTAssertEqualWithAccuracy(18, MLNZoomLevelForAltitude(MLNAltitudeForZoomLevel(18, 0, 40, midSize), 0, 40, midSize), 1e-8);
    
    XCTAssertEqualWithAccuracy(0, MLNZoomLevelForAltitude(MLNAltitudeForZoomLevel(0, 60, 40, midSize), 60, 40, midSize), 1e-8);
    XCTAssertEqualWithAccuracy(18, MLNZoomLevelForAltitude(MLNAltitudeForZoomLevel(18, 60, 40, midSize), 60, 40, midSize), 1e-8);
}

- (void)testGeometryBoxing {
    CLLocationCoordinate2D coordinate = CLLocationCoordinate2DMake(38.9131982, -77.0325453144239);
    CLLocationCoordinate2D roundTrippedCoordinate = [NSValue valueWithMLNCoordinate:coordinate].MLNCoordinateValue;

    XCTAssertEqual(coordinate.latitude, roundTrippedCoordinate.latitude, @"Latitude should round-trip.");
    XCTAssertEqual(coordinate.longitude, roundTrippedCoordinate.longitude, @"Longitude should round-trip.");

    MLNCoordinateSpan span = MLNCoordinateSpanMake(4.383333333333335, -4.299999999999997);
    MLNCoordinateSpan roundTrippedSpan = [NSValue valueWithMLNCoordinateSpan:span].MLNCoordinateSpanValue;

    XCTAssertEqual(span.latitudeDelta, roundTrippedSpan.latitudeDelta, @"Latitude delta should round-trip.");
    XCTAssertEqual(span.longitudeDelta, roundTrippedSpan.longitudeDelta, @"Longitude delta should round-trip.");

    MLNCoordinateBounds bounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(38.9131982, -77.0325453144239),
                                                         CLLocationCoordinate2DMake(37.7757368, -122.4135302));
    MLNCoordinateBounds roundTrippedBounds = [NSValue valueWithMLNCoordinateBounds:bounds].MLNCoordinateBoundsValue;

    XCTAssertEqualObjects([NSValue valueWithMLNCoordinate:bounds.sw],
                          [NSValue valueWithMLNCoordinate:roundTrippedBounds.sw],
                          @"Southwest should round-trip.");
    XCTAssertEqualObjects([NSValue valueWithMLNCoordinate:bounds.ne],
                          [NSValue valueWithMLNCoordinate:roundTrippedBounds.ne],
                          @"Northeast should round-trip.");
}

- (void)testCoordinateInCoordinateBounds {
    CLLocationCoordinate2D ne = CLLocationCoordinate2DMake(45, -104);
    CLLocationCoordinate2D sw = CLLocationCoordinate2DMake(41, -111);
    MLNCoordinateBounds wyoming = MLNCoordinateBoundsMake(sw, ne);

    CLLocationCoordinate2D centerOfWyoming = CLLocationCoordinate2DMake(43, -107.5);

    XCTAssertTrue(MLNCoordinateInCoordinateBounds(ne, wyoming));
    XCTAssertTrue(MLNCoordinateInCoordinateBounds(sw, wyoming));
    XCTAssertTrue(MLNCoordinateInCoordinateBounds(centerOfWyoming, wyoming));

    CLLocationCoordinate2D australia = CLLocationCoordinate2DMake(-25, 135);
    CLLocationCoordinate2D brazil = CLLocationCoordinate2DMake(-12, -50);
    CLLocationCoordinate2D china = CLLocationCoordinate2DMake(35, 100);

    XCTAssertFalse(MLNCoordinateInCoordinateBounds(australia, wyoming));
    XCTAssertFalse(MLNCoordinateInCoordinateBounds(brazil, wyoming));
    XCTAssertFalse(MLNCoordinateInCoordinateBounds(china, wyoming));
    XCTAssertFalse(MLNCoordinateInCoordinateBounds(kCLLocationCoordinate2DInvalid, wyoming));
}

- (void)testGeoJSONDeserialization {
    NSData *data = [@"{\"type\": \"Feature\", \"geometry\": {\"type\": \"Point\", \"coordinates\": [0, 0]}, \"properties\": {}}" dataUsingEncoding:NSUTF8StringEncoding];
    NSError *error;
    MLNPointFeature *feature = (MLNPointFeature *)[MLNShape shapeWithData:data encoding:NSUTF8StringEncoding error:&error];
    XCTAssertNil(error, @"Valid GeoJSON data should produce no error on deserialization.");
    XCTAssertNotNil(feature, @"Valid GeoJSON data should produce an object on deserialization.");
    XCTAssertTrue([feature isKindOfClass:[MLNPointFeature class]], @"Valid GeoJSON point feature data should produce an MLNPointFeature.");
    XCTAssertEqual(feature.attributes.count, 0UL);
    XCTAssertEqual(feature.coordinate.latitude, 0);
    XCTAssertEqual(feature.coordinate.longitude, 0);

    data = [@"{\"type\": \"Feature\", \"feature\": {\"type\": \"Point\", \"coordinates\": [0, 0]}}" dataUsingEncoding:NSUTF8StringEncoding];
    error = nil;
    MLNShape *shape = [MLNShape shapeWithData:data encoding:NSUTF8StringEncoding error:&error];
    XCTAssertNotNil(error, @"Invalid GeoJSON data should produce an error on deserialization.");
    XCTAssertNil(shape, @"Invalid GeoJSON data should produce no object on deserialization.");
}

- (void)testGeoJSONSerialization {
    MLNPointFeature *feature = [[MLNPointFeature alloc] init];
    feature.identifier = @504;
    feature.coordinate = CLLocationCoordinate2DMake(29.95, -90.066667);

    NSData *data = [feature geoJSONDataUsingEncoding:NSUTF8StringEncoding];
    XCTAssertNotNil(data, @"MLNPointFeature should serialize as an UTF-8 string data object.");
    NSError *error;
    NSDictionary *serializedGeoJSON = [NSJSONSerialization JSONObjectWithData:data options:0 error:&error];
    XCTAssertNil(error, @"Serialized GeoJSON data should be deserializable JSON.");
    XCTAssertNotNil(serializedGeoJSON, @"Serialized GeoJSON data should be valid JSON.");
    XCTAssertTrue([serializedGeoJSON isKindOfClass:[NSDictionary class]], @"Serialized GeoJSON data should be a JSON object.");
    NSDictionary *geoJSON = @{
        @"type": @"Feature",
        @"id": @504,
        @"geometry": @{
            @"type": @"Point",
            @"coordinates": @[
                @(-90.066667),
                @29.95,
            ],
        },
        @"properties": @{},
    };
    XCTAssertEqualObjects(serializedGeoJSON, geoJSON, @"MLNPointFeature should serialize as a GeoJSON point feature.");
}

- (void)testMLNCoordinateBoundsToMLNCoordinateQuad {
    MLNCoordinateBounds bounds = MLNCoordinateBoundsMake(CLLocationCoordinate2DMake(37.936, -80.425),
                                                         CLLocationCoordinate2DMake(46.437, -71.516));

    MLNCoordinateQuad quad = MLNCoordinateQuadFromCoordinateBounds(bounds);
    XCTAssertEqualObjects([NSValue valueWithMLNCoordinate:bounds.sw],
                          [NSValue valueWithMLNCoordinate:quad.bottomLeft],
                          @"Bounds southwest should be bottom left of quad.");
    XCTAssertEqualObjects([NSValue valueWithMLNCoordinate:bounds.ne],
                          [NSValue valueWithMLNCoordinate:quad.topRight],
                          @"Bounds northeast should be top right of quad.");

    XCTAssertEqualObjects([NSValue valueWithMLNCoordinate:CLLocationCoordinate2DMake(46.437, -80.425)],
                          [NSValue valueWithMLNCoordinate:quad.topLeft],
                          @"Quad top left should be computed correctly.");
    XCTAssertEqualObjects([NSValue valueWithMLNCoordinate:CLLocationCoordinate2DMake(37.936, -71.516)],
                          [NSValue valueWithMLNCoordinate:quad.bottomRight],
                          @"Quad bottom right should be computed correctly.");
}

- (void)testMLNMapPoint {
    MLNMapPoint point = MLNMapPointForCoordinate(CLLocationCoordinate2DMake(37.936, -80.425), 0.0);
    
    MLNMapPoint roundTrippedPoint = [NSValue valueWithMLNMapPoint:point].MLNMapPointValue;
    XCTAssertEqual(point.x, roundTrippedPoint.x);
    XCTAssertEqual(point.y, roundTrippedPoint.y);
    XCTAssertEqual(point.zoomLevel, roundTrippedPoint.zoomLevel);
}

- (void)testMLNLocationCoordinate2DIsValid {
    {
        CLLocationCoordinate2D coordinate = CLLocationCoordinate2DMake(37.936, -71.516);
        XCTAssertTrue(MLNLocationCoordinate2DIsValid(coordinate));
    }
    {
        CLLocationCoordinate2D coordinate = CLLocationCoordinate2DMake(46.816368, 5.844469);
        XCTAssertTrue(MLNLocationCoordinate2DIsValid(coordinate));
    }
    {
        CLLocationCoordinate2D coordinate = CLLocationCoordinate2DMake(-21.512680, 23.334703);
        XCTAssertTrue(MLNLocationCoordinate2DIsValid(coordinate));
    }
    {
        CLLocationCoordinate2D coordinate = CLLocationCoordinate2DMake(-44.947936, -73.081313);
        XCTAssertTrue(MLNLocationCoordinate2DIsValid(coordinate));
    }
    {
        CLLocationCoordinate2D coordinate = CLLocationCoordinate2DMake(19.333630, 203.555405);
        XCTAssertTrue(MLNLocationCoordinate2DIsValid(coordinate));
    }
    {
        CLLocationCoordinate2D coordinate = CLLocationCoordinate2DMake(23.254696, -240.795323);
        XCTAssertTrue(MLNLocationCoordinate2DIsValid(coordinate));
    }
    {
        CLLocationCoordinate2D coordinate = CLLocationCoordinate2DMake(91, 361);
        XCTAssertFalse(MLNLocationCoordinate2DIsValid(coordinate));
    }
    {
        CLLocationCoordinate2D coordinate = CLLocationCoordinate2DMake(-91, -361);
        XCTAssertFalse(MLNLocationCoordinate2DIsValid(coordinate));
    }
}

@end
