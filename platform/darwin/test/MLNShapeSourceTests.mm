#import <XCTest/XCTest.h>

#import <Mapbox.h>
#import "MLNFeature_Private.h"
#import "MLNShapeSource_Private.h"
#import "MLNSource_Private.h"

#include <mbgl/style/sources/geojson_source.hpp>

@interface MLNShapeSourceTests : XCTestCase
@end

@implementation MLNShapeSourceTests

- (void)testGeoJSONOptionsFromDictionary {
    NSExpression *reduceExpression = [NSExpression expressionForFunction:@"sum:" arguments:@[[NSExpression expressionForKeyPath:@"featureAccumulated"], [NSExpression expressionForKeyPath:@"sumValue"]]];
    NSExpression *mapExpression = [NSExpression expressionForKeyPath:@"mag"];
    NSArray *clusterPropertyArray = @[reduceExpression, mapExpression];
    NSDictionary *options = @{MLNShapeSourceOptionClustered: @YES,
                              MLNShapeSourceOptionClusterRadius: @42,
                              MLNShapeSourceOptionClusterMinPoints: @3,
                              MLNShapeSourceOptionClusterProperties: @{@"sumValue": clusterPropertyArray},
                              MLNShapeSourceOptionMaximumZoomLevelForClustering: @98,
                              MLNShapeSourceOptionMaximumZoomLevel: @99,
                              MLNShapeSourceOptionBuffer: @1976,
                              MLNShapeSourceOptionSimplificationTolerance: @0.42,
                              MLNShapeSourceOptionLineDistanceMetrics: @YES,
                              MLNShapeSourceOptionSynchronousUpdate: @YES};

    auto mbglOptions = MLNGeoJSONOptionsFromDictionary(options);
    XCTAssertTrue(mbglOptions->cluster);
    XCTAssertEqual(mbglOptions->clusterRadius, 42);
    XCTAssertEqual(mbglOptions->clusterMaxZoom, 98);
    XCTAssertEqual(mbglOptions->clusterMinPoints, 3UL);
    XCTAssertEqual(mbglOptions->maxzoom, 99);
    XCTAssertEqual(mbglOptions->buffer, 1976);
    XCTAssertEqual(mbglOptions->tolerance, 0.42);
    XCTAssertTrue(mbglOptions->lineMetrics);
    XCTAssertTrue(!mbglOptions->clusterProperties.empty());
    XCTAssertTrue(mbglOptions->synchronousUpdate);

    options = @{MLNShapeSourceOptionClustered: @"number 1"};
    XCTAssertThrows(MLNGeoJSONOptionsFromDictionary(options));
}

- (void)testNilShape {
    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"id" shape:nil options:nil];
    XCTAssertNil(source.shape);
}

- (void)testUnclusterableShape {
    NSDictionary *options = @{
        MLNShapeSourceOptionClustered: @YES,
    };

    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"id" shape:[[MLNPointFeature alloc] init] options:options];
    XCTAssertTrue([source.shape isKindOfClass:[MLNPointFeature class]]);

    MLNShapeCollectionFeature *feature = [MLNShapeCollectionFeature shapeCollectionWithShapes:@[]];
    source = [[MLNShapeSource alloc] initWithIdentifier:@"id" shape:feature options:options];
    XCTAssertTrue([source.shape isKindOfClass:[MLNShapeCollectionFeature class]]);
}

- (void)testMLNShapeSourceWithDataMultipleFeatures {

    NSString *geoJSON = @"{\"type\": \"FeatureCollection\",\"features\": [{\"type\": \"Feature\",\"properties\": {},\"geometry\": {\"type\": \"LineString\",\"coordinates\": [[-107.75390625,40.329795743702064],[-104.34814453125,37.64903402157866]]}}]}";

    NSData *data = [geoJSON dataUsingEncoding:NSUTF8StringEncoding];
    NSError *error;
    MLNShape *shape = [MLNShape shapeWithData:data encoding:NSUTF8StringEncoding error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(shape);
    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"source-id" shape:shape options:nil];

    MLNShapeCollection *collection = (MLNShapeCollection *)source.shape;
    XCTAssertNotNil(collection);
    XCTAssertEqual(collection.shapes.count, 1UL);
    XCTAssertTrue([collection.shapes.firstObject isMemberOfClass:[MLNPolylineFeature class]]);
}

- (void)testMLNShapeSourceWithSingleGeometry {
    NSData *data = [@"{\"type\": \"Point\", \"coordinates\": [0, 0]}" dataUsingEncoding:NSUTF8StringEncoding];
    NSError *error;
    MLNShape *shape = [MLNShape shapeWithData:data encoding:NSUTF8StringEncoding error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(shape);
    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"geojson" shape:shape options:nil];
    XCTAssertNotNil(source.shape);
    XCTAssert([source.shape isKindOfClass:[MLNPointAnnotation class]]);
}

- (void)testMLNGeoJSONSourceWithSingleFeature {
    NSString *geoJSON = @"{\"type\": \"Feature\", \"properties\": {\"color\": \"green\"}, \"geometry\": { \"type\": \"Point\", \"coordinates\": [ -114.06847000122069, 51.050459433092655 ] }}";
    NSData *data = [geoJSON dataUsingEncoding:NSUTF8StringEncoding];
    NSError *error;
    MLNShape *shape = [MLNShape shapeWithData:data encoding:NSUTF8StringEncoding error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(shape);
    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"geojson" shape:shape options:nil];
    XCTAssertNotNil(source.shape);
    XCTAssert([source.shape isKindOfClass:[MLNPointFeature class]]);
    MLNPointFeature *feature = (MLNPointFeature *)source.shape;
    XCTAssert([feature.attributes.allKeys containsObject:@"color"]);
}

- (void)testMLNShapeSourceWithPolylineFeatures {
    CLLocationCoordinate2D coordinates[] = { CLLocationCoordinate2DMake(0, 0), CLLocationCoordinate2DMake(10, 10)};
    MLNPolylineFeature *polylineFeature = [MLNPolylineFeature polylineWithCoordinates:coordinates count:2];

    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"source-id" shape:polylineFeature options:nil];

    XCTAssertNotNil(source.shape);
    XCTAssertTrue([source.shape isMemberOfClass:[MLNPolylineFeature class]]);
}

- (void)testMLNShapeSourceWithPolygonFeatures {
    CLLocationCoordinate2D coordinates[] = {
        CLLocationCoordinate2DMake(0.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 100.0)};

    MLNPolygonFeature *polygonFeature = [MLNPolygonFeature polygonWithCoordinates:coordinates count:5];
    polygonFeature.identifier = @"feature-id";
    NSString *stringAttribute = @"string";
    NSNumber *boolAttribute = [NSNumber numberWithBool:YES];
    NSNumber *doubleAttribute = [NSNumber numberWithDouble:1.23];
    NSDictionary *nestedDictionaryValue = @{@"nested-key-1": @"nested-string-value"};
    NSArray *arrayValue = @[@"string-value", @2];
    NSDictionary *dictionaryValue = @{@"key-1": @"string-value",
                                      @"key-2": @1,
                                      @"key-3": nestedDictionaryValue,
                                      @"key-4": arrayValue};
    NSArray *arrayOfArrays = @[@[@1, @"string-value", @[@"jagged"]]];
    NSArray *arrayOfDictionaries = @[@{@"key": @"value"}];

    polygonFeature.attributes = @{@"name": stringAttribute,
                                  @"bool": boolAttribute,
                                  @"double": doubleAttribute,
                                  @"dictionary-attribute": dictionaryValue,
                                  @"array-attribute": arrayValue,
                                  @"array-of-array-attribute": arrayOfArrays,
                                  @"array-of-dictionary-attribute": arrayOfDictionaries};

    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"source-id" shape:polygonFeature options:nil];

    XCTAssertNotNil(source.shape);
    MLNPolygonFeature *expectedPolygonFeature = (MLNPolygonFeature *)source.shape;
    XCTAssertEqualObjects(expectedPolygonFeature.identifier, polygonFeature.identifier);
    XCTAssertTrue([expectedPolygonFeature isMemberOfClass:[MLNPolygonFeature class]]);
    XCTAssertEqualObjects(expectedPolygonFeature.identifier, polygonFeature.identifier);
    XCTAssertEqualObjects(expectedPolygonFeature.attributes[@"name"], stringAttribute);
    XCTAssertEqualObjects(expectedPolygonFeature.attributes[@"bool"], boolAttribute);
    XCTAssertEqualObjects(expectedPolygonFeature.attributes[@"double"], doubleAttribute);
    XCTAssertEqualObjects(expectedPolygonFeature.attributes[@"dictionary-attribute"], dictionaryValue);
    XCTAssertEqualObjects(expectedPolygonFeature.attributes[@"array-attribute"], arrayValue);
    XCTAssertEqualObjects(expectedPolygonFeature.attributes[@"array-of-array-attribute"], arrayOfArrays);
    XCTAssertEqualObjects(expectedPolygonFeature.attributes[@"array-of-dictionary-attribute"], arrayOfDictionaries);
}

- (void)testMLNShapeSourceWithPolygonFeaturesInculdingInteriorPolygons {
    CLLocationCoordinate2D coordinates[] = {
        CLLocationCoordinate2DMake(0.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 100.0)};

    CLLocationCoordinate2D interiorCoordinates[] = {
        CLLocationCoordinate2DMake(0.2, 100.2),
        CLLocationCoordinate2DMake(0.2, 100.8),
        CLLocationCoordinate2DMake(0.8, 100.8),
        CLLocationCoordinate2DMake(0.8, 100.2),
        CLLocationCoordinate2DMake(0.2, 100.2)};

    MLNPolygon *polygon = [MLNPolygon polygonWithCoordinates:interiorCoordinates count:5];

    MLNPolygonFeature *polygonFeature = [MLNPolygonFeature polygonWithCoordinates:coordinates count:5 interiorPolygons:@[polygon]];

    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"source-id" shape:polygonFeature options:nil];

    XCTAssertNotNil(source.shape);
    XCTAssertTrue([source.shape isMemberOfClass:[MLNPolygonFeature class]]);
}

- (void)testMLNShapeSourceWithMultiPolylineFeatures {
    CLLocationCoordinate2D firstCoordinates[] = { CLLocationCoordinate2DMake(0, 0), CLLocationCoordinate2DMake(10, 10)};
    MLNPolylineFeature *firstPolylineFeature = [MLNPolylineFeature polylineWithCoordinates:firstCoordinates count:2];
    CLLocationCoordinate2D secondCoordinates[] = { CLLocationCoordinate2DMake(0, 0), CLLocationCoordinate2DMake(10, 10)};
    MLNPolylineFeature *secondPolylineFeature = [MLNPolylineFeature polylineWithCoordinates:secondCoordinates count:2];
    MLNMultiPolylineFeature *multiPolylineFeature = [MLNMultiPolylineFeature multiPolylineWithPolylines:@[firstPolylineFeature, secondPolylineFeature]];

    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"source-id" shape:multiPolylineFeature options:nil];

    XCTAssertNotNil(source.shape);
    XCTAssertTrue([source.shape isMemberOfClass:[MLNMultiPolylineFeature class]]);
}

- (void)testMLNShapeSourceWithMultiPolygonFeatures {
    CLLocationCoordinate2D coordinates[] = {
        CLLocationCoordinate2DMake(0.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 100.0)};

    CLLocationCoordinate2D interiorCoordinates[] = {
        CLLocationCoordinate2DMake(0.2, 100.2),
        CLLocationCoordinate2DMake(0.2, 100.8),
        CLLocationCoordinate2DMake(0.8, 100.8),
        CLLocationCoordinate2DMake(0.8, 100.2),
        CLLocationCoordinate2DMake(0.2, 100.2)};

    MLNPolygon *polygon = [MLNPolygon polygonWithCoordinates:interiorCoordinates count:5];

    MLNPolygonFeature *firstPolygon = [MLNPolygonFeature polygonWithCoordinates:coordinates count:5 interiorPolygons:@[polygon]];
    MLNPolygonFeature *secondPolygon = [MLNPolygonFeature polygonWithCoordinates:coordinates count:5 interiorPolygons:@[polygon]];

    MLNMultiPolygonFeature *multiPolygonFeature = [MLNMultiPolygonFeature multiPolygonWithPolygons:@[firstPolygon, secondPolygon]];

    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"source-id" shape:multiPolygonFeature options:nil];

    XCTAssertNotNil(source.shape);
    XCTAssertTrue([source.shape isMemberOfClass:[MLNMultiPolygonFeature class]]);
}

- (void)testMLNShapeSourceWithPointFeature {
    MLNPointFeature *pointFeature = [MLNPointFeature new];
    pointFeature.coordinate = CLLocationCoordinate2DMake(0.2, 100.2);

    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"souce-id" shape:pointFeature options:nil];

    XCTAssertNotNil(source.shape);
    XCTAssertTrue([source.shape isMemberOfClass:[MLNPointFeature class]]);
}

- (void)testMLNShapeSourceWithPointCollectionFeature {
    CLLocationCoordinate2D coordinates[] = {
        CLLocationCoordinate2DMake(0.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 100.0)};
    MLNPointCollectionFeature *pointCollectionFeature = [MLNPointCollectionFeature pointCollectionWithCoordinates:coordinates count:5];
    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"souce-id" shape:pointCollectionFeature options:nil];

    XCTAssertNotNil(source.shape);
    XCTAssertTrue([source.shape isMemberOfClass:[MLNPointCollectionFeature class]]);
}

- (void)testMLNShapeSourceWithShapeCollectionFeatures {
    CLLocationCoordinate2D coordinates[] = {
        CLLocationCoordinate2DMake(0.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 100.0)};

    CLLocationCoordinate2D interiorCoordinates[] = {
        CLLocationCoordinate2DMake(0.2, 100.2),
        CLLocationCoordinate2DMake(0.2, 100.8),
        CLLocationCoordinate2DMake(0.8, 100.8),
        CLLocationCoordinate2DMake(0.8, 100.2),
        CLLocationCoordinate2DMake(0.2, 100.2)};

    MLNPolygon *polygon = [MLNPolygon polygonWithCoordinates:interiorCoordinates count:5];

    MLNPolygonFeature *polygonFeature = [MLNPolygonFeature polygonWithCoordinates:coordinates count:5 interiorPolygons:@[polygon]];

    CLLocationCoordinate2D coordinates_2[] = { CLLocationCoordinate2DMake(0, 0), CLLocationCoordinate2DMake(10, 10)};
    MLNPolylineFeature *polylineFeature = [MLNPolylineFeature polylineWithCoordinates:coordinates_2 count:2];

    MLNMultiPolygonFeature *multiPolygonFeature = [MLNMultiPolygonFeature multiPolygonWithPolygons:@[polygonFeature, polygonFeature]];

    MLNMultiPolylineFeature *multiPolylineFeature = [MLNMultiPolylineFeature multiPolylineWithPolylines:@[polylineFeature, polylineFeature]];

    MLNPointCollectionFeature *pointCollectionFeature = [MLNPointCollectionFeature pointCollectionWithCoordinates:coordinates count:5];

    MLNPointFeature *pointFeature = [MLNPointFeature new];
    pointFeature.coordinate = CLLocationCoordinate2DMake(0.2, 100.2);

    MLNShapeCollectionFeature *shapeCollectionFeature = [MLNShapeCollectionFeature shapeCollectionWithShapes:@[polygonFeature, polylineFeature, multiPolygonFeature, multiPolylineFeature, pointCollectionFeature, pointFeature]];

    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"source-id" shape:shapeCollectionFeature options:nil];

    MLNShapeCollectionFeature *shape = (MLNShapeCollectionFeature *)source.shape;
    XCTAssertNotNil(shape);
    XCTAssert(shape.shapes.count == 6, @"Shape collection should contain 6 shapes");
}

- (void)testMLNShapeSourceWithFeaturesConvenienceInitializer {
    CLLocationCoordinate2D coordinates[] = {
        CLLocationCoordinate2DMake(0.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 100.0)};

    MLNPolygonFeature *polygonFeature = [MLNPolygonFeature polygonWithCoordinates:coordinates count:sizeof(coordinates)/sizeof(coordinates[0]) interiorPolygons:nil];

    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"source-id" features:@[polygonFeature] options:nil];
    MLNShapeCollectionFeature *shape = (MLNShapeCollectionFeature *)source.shape;

    XCTAssertTrue([shape isKindOfClass:[MLNShapeCollectionFeature class]]);
    XCTAssertEqual(shape.shapes.count, 1UL, @"Shape collection should contain 1 shape");

    // when a shape is included in the features array
    MLNPolygon *polygon = [MLNPolygon polygonWithCoordinates:coordinates count:sizeof(coordinates)/sizeof(coordinates[0]) interiorPolygons:nil];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-literal-conversion"
    XCTAssertThrowsSpecificNamed([[MLNShapeSource alloc] initWithIdentifier:@"source-id-invalid" features:@[polygon] options:nil], NSException, NSInvalidArgumentException, @"Shape source should raise an exception if a shape is sent to the features initializer");
#pragma clang diagnostic pop
}

- (void)testMLNShapeSourceWithShapesConvenienceInitializer {
    CLLocationCoordinate2D coordinates[] = {
        CLLocationCoordinate2DMake(0.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 101.0),
        CLLocationCoordinate2DMake(1.0, 100.0),
        CLLocationCoordinate2DMake(0.0, 100.0)};

    MLNPolygon *polygon = [MLNPolygon polygonWithCoordinates:coordinates count:sizeof(coordinates)/sizeof(coordinates[0]) interiorPolygons:nil];

    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"source-id" shapes:@[polygon] options:nil];
    MLNShapeCollectionFeature *shape = (MLNShapeCollectionFeature *)source.shape;

    XCTAssertTrue([shape isKindOfClass:[MLNShapeCollection class]]);
    XCTAssertEqual(shape.shapes.count, 1UL, @"Shape collection should contain 1 shape");
}

@end
