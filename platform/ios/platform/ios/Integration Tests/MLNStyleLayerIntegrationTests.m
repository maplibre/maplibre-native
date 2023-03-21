#import "MLNMapViewIntegrationTest.h"

@interface MLNStyleLayerIntegrationTests : MLNMapViewIntegrationTest
@end

@implementation MLNStyleLayerIntegrationTests

- (MLNCircleStyleLayer*)setupCircleStyleLayer {
    // Adapted from https://docs.mapbox.com/ios/examples/dds-circle-layer/

    // "mapbox://examples.2uf7qges" is a tileset ID. For more
    // more information, see docs.mapbox.com/help/glossary/tileset-id/
    MLNSource *source = [[MLNVectorTileSource alloc] initWithIdentifier:@"trees" configurationURL:[NSURL URLWithString:@"mapbox://examples.2uf7qges"]];
    [self.mapView.style addSource:source];

    MLNCircleStyleLayer *layer = [[MLNCircleStyleLayer alloc] initWithIdentifier: @"tree-style" source:source];

    // The source name from the source's TileJSON metadata: mapbox.com/api-documentation/maps/#retrieve-tilejson-metadata
    layer.sourceLayerIdentifier = @"yoshino-trees-a0puw5";

    return layer;
}

- (void)testForInterpolatingExpressionRenderCrashWithEmptyStops {
    // Tests: https://github.com/mapbox/mapbox-gl-native/issues/9539
    // Adapted from https://docs.mapbox.com/ios/examples/dds-circle-layer/
    self.mapView.centerCoordinate = CLLocationCoordinate2DMake(38.897,-77.039);
    self.mapView.zoomLevel = 10.5;

    MLNCircleStyleLayer *layer = [self setupCircleStyleLayer];

    NSExpression *interpExpression = [NSExpression mgl_expressionForInterpolatingExpression:NSExpression.zoomLevelVariableExpression
                                                                              withCurveType:MLNExpressionInterpolationModeLinear
                                                                                 parameters:nil
                                                                                      stops:[NSExpression expressionForConstantValue:@{}]];

    XCTAssertThrowsSpecificNamed((layer.circleColor = interpExpression), NSException, NSInvalidArgumentException);

    [self.mapView.style addLayer:layer];
    [self waitForMapViewToBeRenderedWithTimeout:10];
}

- (void)testForSteppingExpressionRenderCrashWithEmptyStops {
    // Tests: https://github.com/mapbox/mapbox-gl-native/issues/9539
    // Adapted from https://docs.mapbox.com/ios/examples/dds-circle-layer/
    self.mapView.centerCoordinate = CLLocationCoordinate2DMake(38.897,-77.039);
    self.mapView.zoomLevel = 10.5;

    MLNCircleStyleLayer *layer = [self setupCircleStyleLayer];

    NSExpression *steppingExpression = [NSExpression mgl_expressionForSteppingExpression:NSExpression.zoomLevelVariableExpression
                                                                          fromExpression:[NSExpression expressionForConstantValue:[UIColor greenColor]]
                                                                                   stops:[NSExpression expressionForConstantValue:@{}]];

    XCTAssertThrowsSpecificNamed((layer.circleColor = steppingExpression), NSException, NSInvalidArgumentException);

    [self.mapView.style addLayer:layer];
    [self waitForMapViewToBeRenderedWithTimeout:10];
}

- (void)testForRaisingExceptionsOnStaleStyleObjects {
    self.mapView.centerCoordinate = CLLocationCoordinate2DMake(38.897,-77.039);
    self.mapView.zoomLevel = 10.5;
    
    MLNVectorTileSource *source = [[MLNVectorTileSource alloc] initWithIdentifier:@"trees" configurationURL:[NSURL URLWithString:@"mapbox://examples.2uf7qges"]];
    [self.mapView.style addSource:source];

    self.styleLoadingExpectation = nil;
    [self.mapView setStyleURL:[[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"]];
    [self waitForMapViewToFinishLoadingStyleWithTimeout:10];

    XCTAssertNotNil(source.description);
    XCTAssertThrowsSpecificNamed(source.configurationURL, NSException, MLNInvalidStyleSourceException, @"MLNSource should raise an exception if its core peer got invalidated");
}

- (void)testForRaisingExceptionsOnStaleLayerObject {
    self.mapView.centerCoordinate = CLLocationCoordinate2DMake(38.897,-77.039);
    self.mapView.zoomLevel = 10.5;
    
    MLNPointFeature *feature = [[MLNPointFeature alloc] init];
    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"sourceID" shape:feature options:nil];
    
    // Testing generated layers
    MLNLineStyleLayer *lineLayer = [[MLNLineStyleLayer alloc] initWithIdentifier:@"lineLayerID" source:source];
    MLNCircleStyleLayer *circleLayer = [[MLNCircleStyleLayer alloc] initWithIdentifier:@"circleLayerID" source:source];
    
    [self.mapView.style addSource:source];
    [self.mapView.style addLayer:lineLayer];
    [self.mapView.style addLayer:circleLayer];

    XCTAssertNoThrow(lineLayer.isVisible);
    XCTAssertNoThrow(circleLayer.isVisible);
    
    XCTAssert(![source.description containsString:@"<unknown>"]);
    XCTAssert(![lineLayer.description containsString:@"<unknown>"]);
    XCTAssert(![circleLayer.description containsString:@"<unknown>"]);

    self.styleLoadingExpectation = nil;
    [self.mapView setStyleURL:[[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"]];
    [self waitForMapViewToFinishLoadingStyleWithTimeout:10];

    XCTAssert([source.description containsString:@"<unknown>"]);
    XCTAssert([lineLayer.description containsString:@"<unknown>"]);
    XCTAssert([circleLayer.description containsString:@"<unknown>"]);

    XCTAssertThrowsSpecificNamed(lineLayer.isVisible, NSException, MLNInvalidStyleLayerException, @"Layer should raise an exception if its core peer got invalidated");
    XCTAssertThrowsSpecificNamed(circleLayer.isVisible, NSException, MLNInvalidStyleLayerException, @"Layer should raise an exception if its core peer got invalidated");
    
    XCTAssertThrowsSpecificNamed([self.mapView.style removeLayer:lineLayer], NSException, NSInvalidArgumentException, @"Style should raise an exception when attempting to remove an invalid layer (e.g. if its core peer got invalidated)");
    XCTAssertThrowsSpecificNamed([self.mapView.style removeLayer:circleLayer], NSException, NSInvalidArgumentException, @"Style should raise an exception when attempting to remove an invalid layer (e.g. if its core peer got invalidated)");
}
@end
