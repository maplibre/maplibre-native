#import <Mapbox.h>

#import "NSBundle+MLNAdditions.h"
#import "MLNVectorTileSource_Private.h"

#import <XCTest/XCTest.h>
#if TARGET_OS_IPHONE
    #import <UIKit/UIKit.h>
#else
    #import <Cocoa/Cocoa.h>
#endif
#import <objc/runtime.h>

@interface MLNStyleTests : XCTestCase <MLNMapViewDelegate>

@property (nonatomic) MLNMapView *mapView;
@property (nonatomic) MLNStyle *style;

@end

@implementation MLNStyleTests {
    XCTestExpectation *_styleLoadingExpectation;
}

- (void)setUp {
    [super setUp];
    
    [MLNSettings useWellKnownTileServer:MLNMapTiler];
    [MLNSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
    
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    self.mapView = [[MLNMapView alloc] initWithFrame:CGRectMake(0, 0, 100, 100) styleURL:styleURL];
    self.mapView.delegate = self;
    if (!self.mapView.style) {
        _styleLoadingExpectation = [self expectationWithDescription:@"Map view should finish loading style."];
        [self waitForExpectationsWithTimeout:10 handler:nil];
    }
}

- (void)mapView:(MLNMapView *)mapView didFinishLoadingStyle:(MLNStyle *)style {
    XCTAssertNotNil(mapView.style);
    XCTAssertEqual(mapView.style, style);

    [_styleLoadingExpectation fulfill];
}

- (void)tearDown {
    _styleLoadingExpectation = nil;
    self.mapView = nil;

    [super tearDown];
}

// TODO: remove backed property _style
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-property-ivar"
- (MLNStyle *)style {
    return self.mapView.style;
}
#pragma clang diagnostic pop

- (void)testName {
    XCTAssertNil(self.style.name);
}

- (void)testSources {
    NSSet<MLNSource *> *initialSources = self.style.sources;
    if ([initialSources.anyObject.identifier isEqualToString:@"com.mapbox.annotations"]) {
        XCTAssertEqual(self.style.sources.count, 1UL);
    } else {
        XCTAssertEqual(self.style.sources.count, 0UL);
    }
    MLNShapeSource *shapeSource = [[MLNShapeSource alloc] initWithIdentifier:@"shapeSource" shape:nil options:nil];
    [self.style addSource:shapeSource];
    XCTAssertEqual(self.style.sources.count, initialSources.count + 1);
    XCTAssertEqual(shapeSource, [self.style sourceWithIdentifier:@"shapeSource"]);
    [self.style removeSource:shapeSource];
    XCTAssertEqual(self.style.sources.count, initialSources.count);
}

- (void)testAddingSourcesTwice {
    MLNShapeSource *shapeSource = [[MLNShapeSource alloc] initWithIdentifier:@"shapeSource" shape:nil options:nil];
    [self.style addSource:shapeSource];
    XCTAssertThrowsSpecificNamed([self.style addSource:shapeSource], NSException, MLNRedundantSourceException);

    MLNRasterTileSource *rasterTileSource = [[MLNRasterTileSource alloc] initWithIdentifier:@"rasterTileSource" configurationURL:[NSURL URLWithString:@".json"] tileSize:42];
    [self.style addSource:rasterTileSource];
    XCTAssertThrowsSpecificNamed([self.style addSource:rasterTileSource], NSException, MLNRedundantSourceException);

    MLNVectorTileSource *vectorTileSource = [[MLNVectorTileSource alloc] initWithIdentifier:@"vectorTileSource" configurationURL:[NSURL URLWithString:@".json"]];
    [self.style addSource:vectorTileSource];
    XCTAssertThrowsSpecificNamed([self.style addSource:vectorTileSource], NSException, MLNRedundantSourceException);
}

- (void)testAddingSourcesWithDuplicateIdentifiers {
    MLNVectorTileSource *source1 = [[MLNVectorTileSource alloc] initWithIdentifier:@"my-source" configurationURL:[NSURL URLWithString:@"maptiler://sources/hillshades"]];
    MLNVectorTileSource *source2 = [[MLNVectorTileSource alloc] initWithIdentifier:@"my-source" configurationURL:[NSURL URLWithString:@"maptiler://sources/hillshades"]];

    [self.style addSource: source1];
    XCTAssertThrowsSpecificNamed([self.style addSource: source2], NSException, MLNRedundantSourceIdentifierException);
}

- (void)testRemovingSourcesBeforeAddingThem {
    MLNRasterTileSource *rasterTileSource = [[MLNRasterTileSource alloc] initWithIdentifier:@"raster-tile-source" tileURLTemplates:@[] options:nil];
    [self.style removeSource:rasterTileSource];
    [self.style addSource:rasterTileSource];
    XCTAssertNotNil([self.style sourceWithIdentifier:rasterTileSource.identifier]);

    MLNShapeSource *shapeSource = [[MLNShapeSource alloc] initWithIdentifier:@"shape-source" shape:nil options:nil];
    [self.style removeSource:shapeSource];
    [self.style addSource:shapeSource];
    XCTAssertNotNil([self.style sourceWithIdentifier:shapeSource.identifier]);

    MLNVectorTileSource *vectorTileSource = [[MLNVectorTileSource alloc] initWithIdentifier:@"vector-tile-source" tileURLTemplates:@[] options:nil];
    [self.style removeSource:vectorTileSource];
    [self.style addSource:vectorTileSource];
    XCTAssertNotNil([self.style sourceWithIdentifier:vectorTileSource.identifier]);
}

- (void)testAddingSourceOfTypeABeforeSourceOfTypeBWithSameIdentifier {
    // Add a raster tile source
    MLNRasterTileSource *rasterTileSource = [[MLNRasterTileSource alloc] initWithIdentifier:@"some-identifier" tileURLTemplates:@[] options:nil];
    [self.style addSource:rasterTileSource];

    // Attempt to remove an image source with the same identifier as the raster tile source
    MLNImageSource *imageSource = [[MLNImageSource alloc] initWithIdentifier:@"some-identifier" coordinateQuad: { } URL:[NSURL URLWithString:@"http://host/image.png"]];
    [self.style removeSource:imageSource];
    // The raster tile source should still be added
    XCTAssertTrue([[self.style sourceWithIdentifier:rasterTileSource.identifier] isMemberOfClass:[MLNRasterTileSource class]]);

    // Remove the raster tile source
    [self.style removeSource:rasterTileSource];

    // Add the shape source
    [self.style addSource:imageSource];

    // Attempt to remove a vector tile source with the same identifer as the shape source
    MLNVectorTileSource *vectorTileSource = [[MLNVectorTileSource alloc] initWithIdentifier:@"some-identifier" tileURLTemplates:@[] options:nil];
    [self.style removeSource:vectorTileSource];
    // The image source should still be added
    XCTAssertTrue([[self.style sourceWithIdentifier:imageSource.identifier] isMemberOfClass:[MLNImageSource class]]);

    // Remove the image source
    [self.style removeSource:imageSource];

    // Add the vector tile source
    [self.style addSource:vectorTileSource];

    // Attempt to remove the previously created raster tile source that has the same identifer as the shape source
    [self.style removeSource:rasterTileSource];
    // The vector tile source should still be added
    XCTAssertTrue([[self.style sourceWithIdentifier:imageSource.identifier] isMemberOfClass:[MLNVectorTileSource class]]);
}

- (void)testRemovingSourceInUse {
    // Add a raster tile source
    MLNVectorTileSource *vectorTileSource = [[MLNVectorTileSource alloc] initWithIdentifier:@"some-identifier" tileURLTemplates:@[] options:nil];
    [self.style addSource:vectorTileSource];
    
    // Add a layer using it
    MLNFillStyleLayer *fillLayer = [[MLNFillStyleLayer alloc] initWithIdentifier:@"fillLayer" source:vectorTileSource];
    [self.style addLayer:fillLayer];

    // Attempt to remove the raster tile source
    NSError *error;
    BOOL result = [self.style removeSource:vectorTileSource error:&error];
    
    XCTAssertFalse(result);
    XCTAssertEqualObjects(error.domain, MLNErrorDomain);
    XCTAssertEqual(error.code, MLNErrorCodeSourceIsInUseCannotRemove);
    
    // Ensure it is still there
    XCTAssertTrue([[self.style sourceWithIdentifier:vectorTileSource.identifier] isMemberOfClass:[MLNVectorTileSource class]]);
}

- (void)testLayers {
    NSArray<MLNStyleLayer *> *initialLayers = self.style.layers;
    if ([initialLayers.firstObject.identifier isEqualToString:@"com.mapbox.annotations.points"]) {
        XCTAssertEqual(self.style.layers.count, 1UL);
    } else {
        XCTAssertEqual(self.style.layers.count, 0UL);
    }
    MLNShapeSource *shapeSource = [[MLNShapeSource alloc] initWithIdentifier:@"shapeSource" shape:nil options:nil];
    [self.style addSource:shapeSource];
    MLNFillStyleLayer *fillLayer = [[MLNFillStyleLayer alloc] initWithIdentifier:@"fillLayer" source:shapeSource];
    [self.style addLayer:fillLayer];
    XCTAssertEqual(self.style.layers.count, initialLayers.count + 1);
    XCTAssertEqual(fillLayer, [self.style layerWithIdentifier:@"fillLayer"]);
    [self.style removeLayer:fillLayer];
    XCTAssertEqual(self.style.layers.count, initialLayers.count);
}

- (void)testAddingLayersTwice {
    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"shapeSource" shape:nil options:nil];

    MLNBackgroundStyleLayer *backgroundLayer = [[MLNBackgroundStyleLayer alloc] initWithIdentifier:@"backgroundLayer"];
    [self.style addLayer:backgroundLayer];
    XCTAssertThrowsSpecificNamed([self.style addLayer:backgroundLayer], NSException, MLNRedundantLayerException);

    MLNCircleStyleLayer *circleLayer = [[MLNCircleStyleLayer alloc] initWithIdentifier:@"circleLayer" source:source];
    [self.style addLayer:circleLayer];
    XCTAssertThrowsSpecificNamed([self.style addLayer:circleLayer], NSException, MLNRedundantLayerException);

    MLNFillStyleLayer *fillLayer = [[MLNFillStyleLayer alloc] initWithIdentifier:@"fillLayer" source:source];
    [self.style addLayer:fillLayer];
    XCTAssertThrowsSpecificNamed([self.style addLayer:fillLayer], NSException, MLNRedundantLayerException);

    MLNLineStyleLayer *lineLayer = [[MLNLineStyleLayer alloc] initWithIdentifier:@"lineLayer" source:source];
    [self.style addLayer:lineLayer];
    XCTAssertThrowsSpecificNamed([self.style addLayer:lineLayer], NSException, MLNRedundantLayerException);

    MLNRasterStyleLayer *rasterLayer = [[MLNRasterStyleLayer alloc] initWithIdentifier:@"rasterLayer" source:source];
    [self.style addLayer:rasterLayer];
    XCTAssertThrowsSpecificNamed([self.style addLayer:rasterLayer], NSException, MLNRedundantLayerException);

    MLNSymbolStyleLayer *symbolLayer = [[MLNSymbolStyleLayer alloc] initWithIdentifier:@"symbolLayer" source:source];
    [self.style addLayer:symbolLayer];
    XCTAssertThrowsSpecificNamed([self.style addLayer:symbolLayer], NSException, MLNRedundantLayerException);
}

- (void)testAddingLayersWithDuplicateIdentifiers {
    // Just some source
    MLNVectorTileSource *source = [[MLNVectorTileSource alloc] initWithIdentifier:@"my-source" configurationURL:[NSURL URLWithString:@"maptiler://sources/hillshades"]];
    [self.style addSource: source];

    // Add initial layer
    MLNFillStyleLayer *initial = [[MLNFillStyleLayer alloc] initWithIdentifier:@"my-layer" source:source];
    [self.style addLayer:initial];

    // Try to add the duplicate
    XCTAssertThrowsSpecificNamed([self.style addLayer:[[MLNFillStyleLayer alloc] initWithIdentifier:@"my-layer" source:source]], NSException, @"MLNRedundantLayerIdentifierException");
    XCTAssertThrowsSpecificNamed([self.style insertLayer:[[MLNFillStyleLayer alloc] initWithIdentifier:@"my-layer" source:source] belowLayer:initial],NSException, @"MLNRedundantLayerIdentifierException");
    XCTAssertThrowsSpecificNamed([self.style insertLayer:[[MLNFillStyleLayer alloc] initWithIdentifier:@"my-layer" source:source] aboveLayer:initial], NSException, @"MLNRedundantLayerIdentifierException");
    XCTAssertThrowsSpecificNamed([self.style insertLayer:[[MLNFillStyleLayer alloc] initWithIdentifier:@"my-layer" source:source] atIndex:0], NSException, @"MLNRedundantLayerIdentifierException");
    XCTAssertThrowsSpecificNamed([self.style insertLayer:[[MLNOpenGLStyleLayer alloc] initWithIdentifier:@"my-layer"] atIndex:0], NSException, @"MLNRedundantLayerIdentifierException");
}

- (void)testRemovingLayerBeforeAddingSameLayer {
    {
        MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"shape-source-removing-before-adding" shape:nil options:nil];
        
        // Attempting to find a layer with identifier will trigger an exception if the source associated with the layer is not added
        [self.style addSource:source];
        
        MLNFillStyleLayer *fillLayer = [[MLNFillStyleLayer alloc] initWithIdentifier:@"fill-layer" source:source];
        [self.style removeLayer:fillLayer];
        [self.style addLayer:fillLayer];
        XCTAssertNotNil([self.style layerWithIdentifier:fillLayer.identifier]);
        
        MLNSymbolStyleLayer *symbolLayer = [[MLNSymbolStyleLayer alloc] initWithIdentifier:@"symbol-layer" source:source];
        [self.style removeLayer:symbolLayer];
        [self.style addLayer:symbolLayer];
        XCTAssertNotNil([self.style layerWithIdentifier:symbolLayer.identifier]);
        
        MLNLineStyleLayer *lineLayer = [[MLNLineStyleLayer alloc] initWithIdentifier:@"line-layer" source:source];
        [self.style removeLayer:lineLayer];
        [self.style addLayer:lineLayer];
        XCTAssertNotNil([self.style layerWithIdentifier:lineLayer.identifier]);
        
        MLNCircleStyleLayer *circleLayer = [[MLNCircleStyleLayer alloc] initWithIdentifier:@"circle-layer" source:source];
        [self.style removeLayer:circleLayer];
        [self.style addLayer:circleLayer];
        XCTAssertNotNil([self.style layerWithIdentifier:circleLayer.identifier]);
        
        MLNBackgroundStyleLayer *backgroundLayer = [[MLNBackgroundStyleLayer alloc] initWithIdentifier:@"background-layer"];
        [self.style removeLayer:backgroundLayer];
        [self.style addLayer:backgroundLayer];
        XCTAssertNotNil([self.style layerWithIdentifier:backgroundLayer.identifier]);
    }
    
    {
        MLNRasterTileSource *rasterSource = [[MLNRasterTileSource alloc] initWithIdentifier:@"raster-tile-source" tileURLTemplates:@[] options:nil];
        [self.style addSource:rasterSource];
        
        MLNRasterStyleLayer *rasterLayer = [[MLNRasterStyleLayer alloc] initWithIdentifier:@"raster-layer" source:rasterSource];
        [self.style removeLayer:rasterLayer];
        [self.style addLayer:rasterLayer];
        XCTAssertNotNil([self.style layerWithIdentifier:rasterLayer.identifier]);
    }
}

- (void)testAddingLayerOfTypeABeforeRemovingLayerOfTypeBWithSameIdentifier {
    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"shape-source-identifier" shape:nil options:nil];
    [self.style addSource:source];
    
    // Add a fill layer
    MLNFillStyleLayer *fillLayer = [[MLNFillStyleLayer alloc] initWithIdentifier:@"some-identifier" source:source];
    [self.style addLayer:fillLayer];
    
    // Attempt to remove a line layer with the same identifier as the fill layer
    MLNLineStyleLayer *lineLayer = [[MLNLineStyleLayer alloc] initWithIdentifier:fillLayer.identifier source:source];
    [self.style removeLayer:lineLayer];
    
    XCTAssertTrue([[self.style layerWithIdentifier:fillLayer.identifier] isMemberOfClass:[MLNFillStyleLayer class]]);
}

- (NSString *)stringWithContentsOfStyleHeader {
    NSURL *styleHeaderURL = [[[NSBundle mgl_frameworkBundle].bundleURL
                              URLByAppendingPathComponent:@"Headers" isDirectory:YES]
                             URLByAppendingPathComponent:@"MLNStyle.h"];
    NSError *styleHeaderError;
    NSString *styleHeader = [NSString stringWithContentsOfURL:styleHeaderURL usedEncoding:nil error:&styleHeaderError];
    XCTAssertNil(styleHeaderError, @"Error getting contents of MLNStyle.h.");
    return styleHeader;
}

- (void)testImages {
    NSString *imageName = @"TrackingLocationMask";
#if TARGET_OS_IPHONE
    MLNImage *image = [MLNImage imageNamed:imageName
                                  inBundle:[NSBundle bundleForClass:[self class]]
             compatibleWithTraitCollection:nil];
#else
    MLNImage *image = [[NSBundle bundleForClass:[self class]] imageForResource:imageName];
#endif
    XCTAssertNotNil(image);
    
    [self.style setImage:image forName:imageName];
    MLNImage *styleImage = [self.style imageForName:imageName];
    
    XCTAssertNotNil(styleImage);
    XCTAssertEqual(image.size.width, styleImage.size.width);
    XCTAssertEqual(image.size.height, styleImage.size.height);
}

- (void)testLayersOrder {
    NSString *filePath = [[NSBundle bundleForClass:self.class] pathForResource:@"amsterdam" ofType:@"geojson"];
    NSURL *url = [NSURL fileURLWithPath:filePath];
    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"sourceID" URL:url options:nil];
    [self.style addSource:source];

    MLNCircleStyleLayer *layer1 = [[MLNCircleStyleLayer alloc] initWithIdentifier:@"layer1" source:source];
    [self.style addLayer:layer1];

    MLNCircleStyleLayer *layer3 = [[MLNCircleStyleLayer alloc] initWithIdentifier:@"layer3" source:source];
    [self.style addLayer:layer3];

    MLNCircleStyleLayer *layer2 = [[MLNCircleStyleLayer alloc] initWithIdentifier:@"layer2" source:source];
    [self.style insertLayer:layer2 aboveLayer:layer1];

    MLNCircleStyleLayer *layer4 = [[MLNCircleStyleLayer alloc] initWithIdentifier:@"layer4" source:source];
    [self.style insertLayer:layer4 aboveLayer:layer3];

    MLNCircleStyleLayer *layer0 = [[MLNCircleStyleLayer alloc] initWithIdentifier:@"layer0" source:source];
    [self.style insertLayer:layer0 belowLayer:layer1];

    NSArray<MLNStyleLayer *> *layers = [self.style layers];
    NSUInteger startIndex = 0;
    if ([layers.firstObject.identifier isEqualToString:@"com.mapbox.annotations.points"]) {
        startIndex++;
    }

    XCTAssertEqualObjects(layers[startIndex++].identifier, layer0.identifier);
    XCTAssertEqualObjects(layers[startIndex++].identifier, layer1.identifier);
    XCTAssertEqualObjects(layers[startIndex++].identifier, layer2.identifier);
    XCTAssertEqualObjects(layers[startIndex++].identifier, layer3.identifier);
    XCTAssertEqualObjects(layers[startIndex++].identifier, layer4.identifier);
}

// MARK: Localization tests

- (void)testLanguageMatching {
    {
        NSArray *preferences = @[@"en"];
        XCTAssertEqualObjects([MLNVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"en");
    }
    {
        NSArray *preferences = @[@"en-US"];
        XCTAssertEqualObjects([MLNVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"en");
    }
    {
        NSArray *preferences = @[@"fr"];
        XCTAssertEqualObjects([MLNVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"fr");
    }
    {
        NSArray *preferences = @[@"zh-Hans"];
        XCTAssertEqualObjects([MLNVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"zh-Hans");
    }
    {
        NSArray *preferences = @[@"zh-Hans", @"en"];
        XCTAssertEqualObjects([MLNVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"zh-Hans");
    }
    {
        NSArray *preferences = @[@"zh-Hant"];
        XCTAssertEqualObjects([MLNVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"zh-Hant");
    }
    {
        NSArray *preferences = @[@"en", @"fr", @"el"];
        XCTAssertEqualObjects([MLNVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"en");
    }
    {
        NSArray *preferences = @[@"tlh"];
        XCTAssertNil([MLNVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences]);
    }
    {
        NSArray *preferences = @[@"tlh", @"en"];
        XCTAssertEqualObjects([MLNVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"en");
    }
    {
        NSArray *preferences = @[@"mul"];
        XCTAssertNil([MLNVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences]);
    }
}

// MARK: Transition tests

- (void)testTransition
{
    MLNTransition transitionTest = MLNTransitionMake(5, 4);
    
    self.style.transition = transitionTest;
    
    XCTAssert(self.style.transition.delay == transitionTest.delay);
    XCTAssert(self.style.transition.duration == transitionTest.duration);
}

- (void)testPerformsPlacementTransitions
{
    XCTAssertTrue(self.style.performsPlacementTransitions, @"The default value for enabling placement transitions should be YES.");
    
    self.style.performsPlacementTransitions = NO;
    XCTAssertFalse(self.style.performsPlacementTransitions, @"Enabling placement transitions should be NO.");
}

@end
