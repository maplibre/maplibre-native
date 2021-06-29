#import <Mapbox/Mapbox.h>

#import "NSBundle+MGLAdditions.h"
#import "MGLVectorTileSource_Private.h"

#import <XCTest/XCTest.h>
#if TARGET_OS_IPHONE
    #import <UIKit/UIKit.h>
#else
    #import <Cocoa/Cocoa.h>
#endif
#import <objc/runtime.h>

@interface MGLStyleTests : XCTestCase <MGLMapViewDelegate>

@property (nonatomic) MGLMapView *mapView;
@property (nonatomic) MGLStyle *style;

@end

@implementation MGLStyleTests {
    XCTestExpectation *_styleLoadingExpectation;
}

- (void)setUp {
    [super setUp];
    
    [MGLSettings useWellKnownTileServer:MGLMapTiler];
    [MGLSettings setApiKey:@"pk.feedcafedeadbeefbadebede"];
    
    NSURL *styleURL = [[NSBundle bundleForClass:[self class]] URLForResource:@"one-liner" withExtension:@"json"];
    self.mapView = [[MGLMapView alloc] initWithFrame:CGRectMake(0, 0, 100, 100) styleURL:styleURL];
    self.mapView.delegate = self;
    if (!self.mapView.style) {
        _styleLoadingExpectation = [self expectationWithDescription:@"Map view should finish loading style."];
        [self waitForExpectationsWithTimeout:10 handler:nil];
    }
}

- (void)mapView:(MGLMapView *)mapView didFinishLoadingStyle:(MGLStyle *)style {
    XCTAssertNotNil(mapView.style);
    XCTAssertEqual(mapView.style, style);

    [_styleLoadingExpectation fulfill];
}

- (void)tearDown {
    _styleLoadingExpectation = nil;
    self.mapView = nil;

    [super tearDown];
}

- (MGLStyle *)style {
    return self.mapView.style;
}

- (void)testName {
    XCTAssertNil(self.style.name);
}

- (void)testSources {
    NSSet<MGLSource *> *initialSources = self.style.sources;
    if ([initialSources.anyObject.identifier isEqualToString:@"com.mapbox.annotations"]) {
        XCTAssertEqual(self.style.sources.count, 1UL);
    } else {
        XCTAssertEqual(self.style.sources.count, 0UL);
    }
    MGLShapeSource *shapeSource = [[MGLShapeSource alloc] initWithIdentifier:@"shapeSource" shape:nil options:nil];
    [self.style addSource:shapeSource];
    XCTAssertEqual(self.style.sources.count, initialSources.count + 1);
    XCTAssertEqual(shapeSource, [self.style sourceWithIdentifier:@"shapeSource"]);
    [self.style removeSource:shapeSource];
    XCTAssertEqual(self.style.sources.count, initialSources.count);
}

- (void)testAddingSourcesTwice {
    MGLShapeSource *shapeSource = [[MGLShapeSource alloc] initWithIdentifier:@"shapeSource" shape:nil options:nil];
    [self.style addSource:shapeSource];
    XCTAssertThrowsSpecificNamed([self.style addSource:shapeSource], NSException, MGLRedundantSourceException);

    MGLRasterTileSource *rasterTileSource = [[MGLRasterTileSource alloc] initWithIdentifier:@"rasterTileSource" configurationURL:[NSURL URLWithString:@".json"] tileSize:42];
    [self.style addSource:rasterTileSource];
    XCTAssertThrowsSpecificNamed([self.style addSource:rasterTileSource], NSException, MGLRedundantSourceException);

    MGLVectorTileSource *vectorTileSource = [[MGLVectorTileSource alloc] initWithIdentifier:@"vectorTileSource" configurationURL:[NSURL URLWithString:@".json"]];
    [self.style addSource:vectorTileSource];
    XCTAssertThrowsSpecificNamed([self.style addSource:vectorTileSource], NSException, MGLRedundantSourceException);
}

- (void)testAddingSourcesWithDuplicateIdentifiers {
    MGLVectorTileSource *source1 = [[MGLVectorTileSource alloc] initWithIdentifier:@"my-source" configurationURL:[NSURL URLWithString:@"maptiler://sources/hillshades"]];
    MGLVectorTileSource *source2 = [[MGLVectorTileSource alloc] initWithIdentifier:@"my-source" configurationURL:[NSURL URLWithString:@"maptiler://sources/hillshades"]];

    [self.style addSource: source1];
    XCTAssertThrowsSpecificNamed([self.style addSource: source2], NSException, MGLRedundantSourceIdentifierException);
}

- (void)testRemovingSourcesBeforeAddingThem {
    MGLRasterTileSource *rasterTileSource = [[MGLRasterTileSource alloc] initWithIdentifier:@"raster-tile-source" tileURLTemplates:@[] options:nil];
    [self.style removeSource:rasterTileSource];
    [self.style addSource:rasterTileSource];
    XCTAssertNotNil([self.style sourceWithIdentifier:rasterTileSource.identifier]);

    MGLShapeSource *shapeSource = [[MGLShapeSource alloc] initWithIdentifier:@"shape-source" shape:nil options:nil];
    [self.style removeSource:shapeSource];
    [self.style addSource:shapeSource];
    XCTAssertNotNil([self.style sourceWithIdentifier:shapeSource.identifier]);

    MGLVectorTileSource *vectorTileSource = [[MGLVectorTileSource alloc] initWithIdentifier:@"vector-tile-source" tileURLTemplates:@[] options:nil];
    [self.style removeSource:vectorTileSource];
    [self.style addSource:vectorTileSource];
    XCTAssertNotNil([self.style sourceWithIdentifier:vectorTileSource.identifier]);
}

- (void)testAddingSourceOfTypeABeforeSourceOfTypeBWithSameIdentifier {
    // Add a raster tile source
    MGLRasterTileSource *rasterTileSource = [[MGLRasterTileSource alloc] initWithIdentifier:@"some-identifier" tileURLTemplates:@[] options:nil];
    [self.style addSource:rasterTileSource];

    // Attempt to remove an image source with the same identifier as the raster tile source
    MGLImageSource *imageSource = [[MGLImageSource alloc] initWithIdentifier:@"some-identifier" coordinateQuad: { } URL:[NSURL URLWithString:@"http://host/image.png"]];
    [self.style removeSource:imageSource];
    // The raster tile source should still be added
    XCTAssertTrue([[self.style sourceWithIdentifier:rasterTileSource.identifier] isMemberOfClass:[MGLRasterTileSource class]]);

    // Remove the raster tile source
    [self.style removeSource:rasterTileSource];

    // Add the shape source
    [self.style addSource:imageSource];

    // Attempt to remove a vector tile source with the same identifer as the shape source
    MGLVectorTileSource *vectorTileSource = [[MGLVectorTileSource alloc] initWithIdentifier:@"some-identifier" tileURLTemplates:@[] options:nil];
    [self.style removeSource:vectorTileSource];
    // The image source should still be added
    XCTAssertTrue([[self.style sourceWithIdentifier:imageSource.identifier] isMemberOfClass:[MGLImageSource class]]);

    // Remove the image source
    [self.style removeSource:imageSource];

    // Add the vector tile source
    [self.style addSource:vectorTileSource];

    // Attempt to remove the previously created raster tile source that has the same identifer as the shape source
    [self.style removeSource:rasterTileSource];
    // The vector tile source should still be added
    XCTAssertTrue([[self.style sourceWithIdentifier:imageSource.identifier] isMemberOfClass:[MGLVectorTileSource class]]);
}

- (void)testRemovingSourceInUse {
    // Add a raster tile source
    MGLVectorTileSource *vectorTileSource = [[MGLVectorTileSource alloc] initWithIdentifier:@"some-identifier" tileURLTemplates:@[] options:nil];
    [self.style addSource:vectorTileSource];
    
    // Add a layer using it
    MGLFillStyleLayer *fillLayer = [[MGLFillStyleLayer alloc] initWithIdentifier:@"fillLayer" source:vectorTileSource];
    [self.style addLayer:fillLayer];

    // Attempt to remove the raster tile source
    NSError *error;
    BOOL result = [self.style removeSource:vectorTileSource error:&error];
    
    XCTAssertFalse(result);
    XCTAssertEqualObjects(error.domain, MGLErrorDomain);
    XCTAssertEqual(error.code, MGLErrorCodeSourceIsInUseCannotRemove);
    
    // Ensure it is still there
    XCTAssertTrue([[self.style sourceWithIdentifier:vectorTileSource.identifier] isMemberOfClass:[MGLVectorTileSource class]]);
}

- (void)testLayers {
    NSArray<MGLStyleLayer *> *initialLayers = self.style.layers;
    if ([initialLayers.firstObject.identifier isEqualToString:@"com.mapbox.annotations.points"]) {
        XCTAssertEqual(self.style.layers.count, 1UL);
    } else {
        XCTAssertEqual(self.style.layers.count, 0UL);
    }
    MGLShapeSource *shapeSource = [[MGLShapeSource alloc] initWithIdentifier:@"shapeSource" shape:nil options:nil];
    [self.style addSource:shapeSource];
    MGLFillStyleLayer *fillLayer = [[MGLFillStyleLayer alloc] initWithIdentifier:@"fillLayer" source:shapeSource];
    [self.style addLayer:fillLayer];
    XCTAssertEqual(self.style.layers.count, initialLayers.count + 1);
    XCTAssertEqual(fillLayer, [self.style layerWithIdentifier:@"fillLayer"]);
    [self.style removeLayer:fillLayer];
    XCTAssertEqual(self.style.layers.count, initialLayers.count);
}

- (void)testAddingLayersTwice {
    MGLShapeSource *source = [[MGLShapeSource alloc] initWithIdentifier:@"shapeSource" shape:nil options:nil];

    MGLBackgroundStyleLayer *backgroundLayer = [[MGLBackgroundStyleLayer alloc] initWithIdentifier:@"backgroundLayer"];
    [self.style addLayer:backgroundLayer];
    XCTAssertThrowsSpecificNamed([self.style addLayer:backgroundLayer], NSException, MGLRedundantLayerException);

    MGLCircleStyleLayer *circleLayer = [[MGLCircleStyleLayer alloc] initWithIdentifier:@"circleLayer" source:source];
    [self.style addLayer:circleLayer];
    XCTAssertThrowsSpecificNamed([self.style addLayer:circleLayer], NSException, MGLRedundantLayerException);

    MGLFillStyleLayer *fillLayer = [[MGLFillStyleLayer alloc] initWithIdentifier:@"fillLayer" source:source];
    [self.style addLayer:fillLayer];
    XCTAssertThrowsSpecificNamed([self.style addLayer:fillLayer], NSException, MGLRedundantLayerException);

    MGLLineStyleLayer *lineLayer = [[MGLLineStyleLayer alloc] initWithIdentifier:@"lineLayer" source:source];
    [self.style addLayer:lineLayer];
    XCTAssertThrowsSpecificNamed([self.style addLayer:lineLayer], NSException, MGLRedundantLayerException);

    MGLRasterStyleLayer *rasterLayer = [[MGLRasterStyleLayer alloc] initWithIdentifier:@"rasterLayer" source:source];
    [self.style addLayer:rasterLayer];
    XCTAssertThrowsSpecificNamed([self.style addLayer:rasterLayer], NSException, MGLRedundantLayerException);

    MGLSymbolStyleLayer *symbolLayer = [[MGLSymbolStyleLayer alloc] initWithIdentifier:@"symbolLayer" source:source];
    [self.style addLayer:symbolLayer];
    XCTAssertThrowsSpecificNamed([self.style addLayer:symbolLayer], NSException, MGLRedundantLayerException);
}

- (void)testAddingLayersWithDuplicateIdentifiers {
    // Just some source
    MGLVectorTileSource *source = [[MGLVectorTileSource alloc] initWithIdentifier:@"my-source" configurationURL:[NSURL URLWithString:@"maptiler://sources/hillshades"]];
    [self.style addSource: source];

    // Add initial layer
    MGLFillStyleLayer *initial = [[MGLFillStyleLayer alloc] initWithIdentifier:@"my-layer" source:source];
    [self.style addLayer:initial];

    // Try to add the duplicate
    XCTAssertThrowsSpecificNamed([self.style addLayer:[[MGLFillStyleLayer alloc] initWithIdentifier:@"my-layer" source:source]], NSException, @"MGLRedundantLayerIdentifierException");
    XCTAssertThrowsSpecificNamed([self.style insertLayer:[[MGLFillStyleLayer alloc] initWithIdentifier:@"my-layer" source:source] belowLayer:initial],NSException, @"MGLRedundantLayerIdentifierException");
    XCTAssertThrowsSpecificNamed([self.style insertLayer:[[MGLFillStyleLayer alloc] initWithIdentifier:@"my-layer" source:source] aboveLayer:initial], NSException, @"MGLRedundantLayerIdentifierException");
    XCTAssertThrowsSpecificNamed([self.style insertLayer:[[MGLFillStyleLayer alloc] initWithIdentifier:@"my-layer" source:source] atIndex:0], NSException, @"MGLRedundantLayerIdentifierException");
    XCTAssertThrowsSpecificNamed([self.style insertLayer:[[MGLOpenGLStyleLayer alloc] initWithIdentifier:@"my-layer"] atIndex:0], NSException, @"MGLRedundantLayerIdentifierException");
}

- (void)testRemovingLayerBeforeAddingSameLayer {
    {
        MGLShapeSource *source = [[MGLShapeSource alloc] initWithIdentifier:@"shape-source-removing-before-adding" shape:nil options:nil];
        
        // Attempting to find a layer with identifier will trigger an exception if the source associated with the layer is not added
        [self.style addSource:source];
        
        MGLFillStyleLayer *fillLayer = [[MGLFillStyleLayer alloc] initWithIdentifier:@"fill-layer" source:source];
        [self.style removeLayer:fillLayer];
        [self.style addLayer:fillLayer];
        XCTAssertNotNil([self.style layerWithIdentifier:fillLayer.identifier]);
        
        MGLSymbolStyleLayer *symbolLayer = [[MGLSymbolStyleLayer alloc] initWithIdentifier:@"symbol-layer" source:source];
        [self.style removeLayer:symbolLayer];
        [self.style addLayer:symbolLayer];
        XCTAssertNotNil([self.style layerWithIdentifier:symbolLayer.identifier]);
        
        MGLLineStyleLayer *lineLayer = [[MGLLineStyleLayer alloc] initWithIdentifier:@"line-layer" source:source];
        [self.style removeLayer:lineLayer];
        [self.style addLayer:lineLayer];
        XCTAssertNotNil([self.style layerWithIdentifier:lineLayer.identifier]);
        
        MGLCircleStyleLayer *circleLayer = [[MGLCircleStyleLayer alloc] initWithIdentifier:@"circle-layer" source:source];
        [self.style removeLayer:circleLayer];
        [self.style addLayer:circleLayer];
        XCTAssertNotNil([self.style layerWithIdentifier:circleLayer.identifier]);
        
        MGLBackgroundStyleLayer *backgroundLayer = [[MGLBackgroundStyleLayer alloc] initWithIdentifier:@"background-layer"];
        [self.style removeLayer:backgroundLayer];
        [self.style addLayer:backgroundLayer];
        XCTAssertNotNil([self.style layerWithIdentifier:backgroundLayer.identifier]);
    }
    
    {
        MGLRasterTileSource *rasterSource = [[MGLRasterTileSource alloc] initWithIdentifier:@"raster-tile-source" tileURLTemplates:@[] options:nil];
        [self.style addSource:rasterSource];
        
        MGLRasterStyleLayer *rasterLayer = [[MGLRasterStyleLayer alloc] initWithIdentifier:@"raster-layer" source:rasterSource];
        [self.style removeLayer:rasterLayer];
        [self.style addLayer:rasterLayer];
        XCTAssertNotNil([self.style layerWithIdentifier:rasterLayer.identifier]);
    }
}

- (void)testAddingLayerOfTypeABeforeRemovingLayerOfTypeBWithSameIdentifier {
    MGLShapeSource *source = [[MGLShapeSource alloc] initWithIdentifier:@"shape-source-identifier" shape:nil options:nil];
    [self.style addSource:source];
    
    // Add a fill layer
    MGLFillStyleLayer *fillLayer = [[MGLFillStyleLayer alloc] initWithIdentifier:@"some-identifier" source:source];
    [self.style addLayer:fillLayer];
    
    // Attempt to remove a line layer with the same identifier as the fill layer
    MGLLineStyleLayer *lineLayer = [[MGLLineStyleLayer alloc] initWithIdentifier:fillLayer.identifier source:source];
    [self.style removeLayer:lineLayer];
    
    XCTAssertTrue([[self.style layerWithIdentifier:fillLayer.identifier] isMemberOfClass:[MGLFillStyleLayer class]]);
}

- (NSString *)stringWithContentsOfStyleHeader {
    NSURL *styleHeaderURL = [[[NSBundle mgl_frameworkBundle].bundleURL
                              URLByAppendingPathComponent:@"Headers" isDirectory:YES]
                             URLByAppendingPathComponent:@"MGLStyle.h"];
    NSError *styleHeaderError;
    NSString *styleHeader = [NSString stringWithContentsOfURL:styleHeaderURL usedEncoding:nil error:&styleHeaderError];
    XCTAssertNil(styleHeaderError, @"Error getting contents of MGLStyle.h.");
    return styleHeader;
}

- (void)testImages {
    NSString *imageName = @"TrackingLocationMask";
#if TARGET_OS_IPHONE
    MGLImage *image = [MGLImage imageNamed:imageName
                                  inBundle:[NSBundle bundleForClass:[self class]]
             compatibleWithTraitCollection:nil];
#else
    MGLImage *image = [[NSBundle bundleForClass:[self class]] imageForResource:imageName];
#endif
    XCTAssertNotNil(image);
    
    [self.style setImage:image forName:imageName];
    MGLImage *styleImage = [self.style imageForName:imageName];
    
    XCTAssertNotNil(styleImage);
    XCTAssertEqual(image.size.width, styleImage.size.width);
    XCTAssertEqual(image.size.height, styleImage.size.height);
}

- (void)testLayersOrder {
    NSString *filePath = [[NSBundle bundleForClass:self.class] pathForResource:@"amsterdam" ofType:@"geojson"];
    NSURL *url = [NSURL fileURLWithPath:filePath];
    MGLShapeSource *source = [[MGLShapeSource alloc] initWithIdentifier:@"sourceID" URL:url options:nil];
    [self.style addSource:source];

    MGLCircleStyleLayer *layer1 = [[MGLCircleStyleLayer alloc] initWithIdentifier:@"layer1" source:source];
    [self.style addLayer:layer1];

    MGLCircleStyleLayer *layer3 = [[MGLCircleStyleLayer alloc] initWithIdentifier:@"layer3" source:source];
    [self.style addLayer:layer3];

    MGLCircleStyleLayer *layer2 = [[MGLCircleStyleLayer alloc] initWithIdentifier:@"layer2" source:source];
    [self.style insertLayer:layer2 aboveLayer:layer1];

    MGLCircleStyleLayer *layer4 = [[MGLCircleStyleLayer alloc] initWithIdentifier:@"layer4" source:source];
    [self.style insertLayer:layer4 aboveLayer:layer3];

    MGLCircleStyleLayer *layer0 = [[MGLCircleStyleLayer alloc] initWithIdentifier:@"layer0" source:source];
    [self.style insertLayer:layer0 belowLayer:layer1];

    NSArray<MGLStyleLayer *> *layers = [self.style layers];
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

#pragma mark Localization tests

- (void)testLanguageMatching {
    {
        NSArray *preferences = @[@"en"];
        XCTAssertEqualObjects([MGLVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"en");
    }
    {
        NSArray *preferences = @[@"en-US"];
        XCTAssertEqualObjects([MGLVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"en");
    }
    {
        NSArray *preferences = @[@"fr"];
        XCTAssertEqualObjects([MGLVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"fr");
    }
    {
        NSArray *preferences = @[@"zh-Hans"];
        XCTAssertEqualObjects([MGLVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"zh-Hans");
    }
    {
        NSArray *preferences = @[@"zh-Hans", @"en"];
        XCTAssertEqualObjects([MGLVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"zh-Hans");
    }
    {
        NSArray *preferences = @[@"zh-Hant"];
        XCTAssertEqualObjects([MGLVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"zh-Hant");
    }
    {
        NSArray *preferences = @[@"en", @"fr", @"el"];
        XCTAssertEqualObjects([MGLVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"en");
    }
    {
        NSArray *preferences = @[@"tlh"];
        XCTAssertNil([MGLVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences]);
    }
    {
        NSArray *preferences = @[@"tlh", @"en"];
        XCTAssertEqualObjects([MGLVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences], @"en");
    }
    {
        NSArray *preferences = @[@"mul"];
        XCTAssertNil([MGLVectorTileSource preferredMapboxStreetsLanguageForPreferences:preferences]);
    }
}

#pragma mark Transition tests

- (void)testTransition
{
    MGLTransition transitionTest = MGLTransitionMake(5, 4);
    
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
