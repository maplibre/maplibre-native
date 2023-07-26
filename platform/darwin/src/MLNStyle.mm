#import "MLNStyle_Private.h"

#import "MLNMapView_Private.h"
#import "MLNStyleLayer.h"
#import "MLNStyleLayer_Private.h"
#import "MLNFillStyleLayer.h"
#import "MLNFillExtrusionStyleLayer.h"
#import "MLNLineStyleLayer.h"
#import "MLNCircleStyleLayer.h"
#import "MLNSymbolStyleLayer.h"
#import "MLNHeatmapStyleLayer.h"
#import "MLNHillshadeStyleLayer.h"
#import "MLNRasterStyleLayer.h"
#import "MLNBackgroundStyleLayer.h"
#import "MLNOpenGLStyleLayer.h"
#import "MLNStyleLayerManager.h"

#import "MLNSource.h"
#import "MLNSource_Private.h"
#import "MLNLight_Private.h"
#import "MLNTileSource_Private.h"
#import "MLNVectorTileSource_Private.h"
#import "MLNRasterTileSource.h"
#import "MLNRasterDEMSource.h"
#import "MLNShapeSource.h"
#import "MLNImageSource.h"

#import "MLNAttributionInfo_Private.h"
#import "MLNLoggingConfiguration_Private.h"

#include <mbgl/map/map.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/style/image.hpp>
#include <mbgl/style/light.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/style/sources/vector_source.hpp>
#include <mbgl/style/sources/raster_source.hpp>
#include <mbgl/style/sources/raster_dem_source.hpp>
#include <mbgl/style/sources/image_source.hpp>

#import "NSDate+MLNAdditions.h"

#if TARGET_OS_IPHONE
    #import "UIImage+MLNAdditions.h"
#else
    #import "NSImage+MLNAdditions.h"
#endif

const MLNExceptionName MLNInvalidStyleURLException = @"MLNInvalidStyleURLException";
const MLNExceptionName MLNRedundantLayerException = @"MLNRedundantLayerException";
const MLNExceptionName MLNRedundantLayerIdentifierException = @"MLNRedundantLayerIdentifierException";
const MLNExceptionName MLNRedundantSourceException = @"MLNRedundantSourceException";
const MLNExceptionName MLNRedundantSourceIdentifierException = @"MLNRedundantSourceIdentifierException";

/**
 Model class for localization changes.
 */
@interface MLNTextLanguage: NSObject
@property (strong, nonatomic) NSString *originalTextField;
@property (strong, nonatomic) NSString *updatedTextField;

- (instancetype)initWithTextLanguage:(NSString *)originalTextField updatedTextField:(NSString *)updatedTextField;

@end

@implementation MLNTextLanguage
- (instancetype)initWithTextLanguage:(NSString *)originalTextField updatedTextField:(NSString *)updatedTextField
{
    if (self = [super init]) {
        _originalTextField = originalTextField;
        _updatedTextField = updatedTextField;
    }
    return self;
}
@end

@interface MLNStyle()

@property (nonatomic, readonly, weak) id <MLNStylable> stylable;
@property (nonatomic, readonly) mbgl::style::Style *rawStyle;
@property (readonly, copy, nullable) NSURL *URL;
@property (nonatomic, readwrite, strong) NSMutableDictionary<NSString *, MLNOpenGLStyleLayer *> *openGLLayers;
@property (nonatomic) NSMutableDictionary<NSString *, NSDictionary<NSObject *, MLNTextLanguage *> *> *localizedLayersByIdentifier;

@end

@implementation MLNStyle

// MARK: Predefined style URLs

+ (NSArray<MLNDefaultStyle*>*) predefinedStyles {
    return MLNSettings.tileServerOptions.defaultStyles;
}

+ (MLNDefaultStyle*) defaultStyle {
    MLNTileServerOptions* opts = MLNSettings.tileServerOptions;
    return opts.defaultStyle;
}

+ (NSURL*) defaultStyleURL {
    MLNDefaultStyle* styleDefinition = [MLNStyle defaultStyle];
    if (styleDefinition != nil){
        return styleDefinition.url;
    }
    
    return nil;
}

+ (MLNDefaultStyle*) predefinedStyle:(NSString*)withStyleName {
    for (MLNDefaultStyle* style in MLNSettings.tileServerOptions.defaultStyles) {
        if ([style.name isEqualToString:withStyleName]) {
            return style;
        }
    }
    return nil;
}

// MARK: -

- (instancetype)initWithRawStyle:(mbgl::style::Style *)rawStyle stylable:(id <MLNStylable>)stylable {
    MLNLogInfo(@"Initializing %@ with stylable: %@", NSStringFromClass([self class]), stylable);
    if (self = [super init]) {
        _stylable = stylable;
        _rawStyle = rawStyle;
        _openGLLayers = [NSMutableDictionary dictionary];
        _localizedLayersByIdentifier = [NSMutableDictionary dictionary];
        MLNLogDebug(@"Initializing with style name: %@ stylable: %@", self.name, stylable);
    }
    return self;
}

- (NSURL *)URL {
    return [NSURL URLWithString:@(self.rawStyle->getURL().c_str())];
}

- (NSString *)name {
    std::string name = self.rawStyle->getName();
    return name.empty() ? nil : @(name.c_str());
}

// MARK: Sources

- (NSSet<__kindof MLNSource *> *)sources {
    auto rawSources = self.rawStyle->getSources();
    NSMutableSet<__kindof MLNSource *> *sources = [NSMutableSet setWithCapacity:rawSources.size()];
    for (auto rawSource = rawSources.begin(); rawSource != rawSources.end(); ++rawSource) {
        MLNSource *source = [self sourceFromMBGLSource:*rawSource];
        [sources addObject:source];
    }
    return sources;
}

- (void)setSources:(NSSet<__kindof MLNSource *> *)sources {
    MLNLogDebug(@"Setting: %lu sources", sources.count);
    for (MLNSource *source in self.sources) {
        [self removeSource:source];
    }
    for (MLNSource *source in sources) {
        [self addSource:source];
    }
}

- (NSUInteger)countOfSources {
    return self.rawStyle->getSources().size();
}

- (MLNSource *)memberOfSources:(MLNSource *)object {
    return [self sourceWithIdentifier:object.identifier];
}

- (MLNSource *)sourceWithIdentifier:(NSString *)identifier
{
    MLNLogDebug(@"Querying source with identifier: %@", identifier);
    auto rawSource = self.rawStyle->getSource(identifier.UTF8String);
    
    return rawSource ? [self sourceFromMBGLSource:rawSource] : nil;
}

- (MLNSource *)sourceFromMBGLSource:(mbgl::style::Source *)rawSource {
    if (MLNSource *source = rawSource->peer.has_value() ? rawSource->peer.get<SourceWrapper>().source : nil) {
        return source;
    }

    // TODO: Fill in options specific to the respective source classes
    // https://github.com/mapbox/mapbox-gl-native/issues/6584
    if (auto vectorSource = rawSource->as<mbgl::style::VectorSource>()) {
        return [[MLNVectorTileSource alloc] initWithRawSource:vectorSource stylable:self.stylable];
    } else if (auto geoJSONSource = rawSource->as<mbgl::style::GeoJSONSource>()) {
        return [[MLNShapeSource alloc] initWithRawSource:geoJSONSource stylable:self.stylable];
    } else if (auto rasterSource = rawSource->as<mbgl::style::RasterSource>()) {
        return [[MLNRasterTileSource alloc] initWithRawSource:rasterSource stylable:self.stylable];
    } else if (auto rasterDEMSource = rawSource->as<mbgl::style::RasterDEMSource>()) {
        return [[MLNRasterDEMSource alloc] initWithRawSource:rasterDEMSource stylable:self.stylable];
    } else if (auto imageSource = rawSource->as<mbgl::style::ImageSource>()) {
        return [[MLNImageSource alloc] initWithRawSource:imageSource stylable:self.stylable];
    } else {
        return [[MLNSource alloc] initWithRawSource:rawSource stylable:self.stylable];
    }
}

- (void)addSource:(MLNSource *)source
{
    MLNLogDebug(@"Adding source: %@", source);
    if (!source.rawSource) {
        [NSException raise:NSInvalidArgumentException format:
         @"The source %@ cannot be added to the style. "
         @"Make sure the source was created as a member of a concrete subclass of MLNSource.",
         source];
    }

    try {
        [source addToStylable:self.stylable];
    } catch (std::runtime_error & err) {
        [NSException raise:MLNRedundantSourceIdentifierException format:@"%s", err.what()];
    }
}

- (void)removeSource:(MLNSource *)source
{
    [self removeSource:source error:nil];
}

- (BOOL)removeSource:(MLNSource *)source error:(NSError * __nullable * __nullable)outError {
    MLNLogDebug(@"Removing source: %@", source);
    
    if (!source.rawSource) {
        NSString *errorMessage = [NSString stringWithFormat:
                                  @"The source %@ cannot be removed from the style. "
                                  @"Make sure the source was created as a member of a concrete subclass of MLNSource."
                                  @"Automatic re-addition of sources after style changes is not currently supported.",
                                  source];
        
        if (outError) {
            *outError = [NSError errorWithDomain:MLNErrorDomain
                                            code:MLNErrorCodeSourceCannotBeRemovedFromStyle
                                        userInfo:@{ NSLocalizedDescriptionKey : errorMessage }];
            return NO;
        }
        else {
            [NSException raise:NSInvalidArgumentException format:@"%@", errorMessage];
        }
    }
    
    return [source removeFromStylable:self.stylable error:outError];
}


- (nullable NSArray<MLNAttributionInfo *> *)attributionInfosWithFontSize:(CGFloat)fontSize linkColor:(nullable MLNColor *)linkColor {
    // It’d be incredibly convenient to use -sources here, but this operation
    // depends on the sources being sorted in ascending order by creation, as
    // with the std::vector used in mbgl.
    auto rawSources = self.rawStyle->getSources();
    NSMutableArray *infos = [NSMutableArray arrayWithCapacity:rawSources.size()];
    for (auto rawSource = rawSources.begin(); rawSource != rawSources.end(); ++rawSource) {
        MLNTileSource *source = (MLNTileSource *)[self sourceFromMBGLSource:*rawSource];
        if (![source isKindOfClass:[MLNTileSource class]]) {
            continue;
        }

        NSArray *tileSetInfos = [source attributionInfosWithFontSize:fontSize linkColor:linkColor];
        [infos growArrayByAddingAttributionInfosFromArray:tileSetInfos];
    }
    return infos;
}

// MARK: Style layers

- (NSArray<__kindof MLNStyleLayer *> *)layers
{
    auto layers = self.rawStyle->getLayers();
    NSMutableArray<__kindof MLNStyleLayer *> *styleLayers = [NSMutableArray arrayWithCapacity:layers.size()];
    for (auto layer : layers) {
        MLNStyleLayer *styleLayer = [self layerFromMBGLLayer:layer];
        [styleLayers addObject:styleLayer];
    }
    return styleLayers;
}

- (void)setLayers:(NSArray<__kindof MLNStyleLayer *> *)layers {
    MLNLogDebug(@"Setting: %lu layers", layers.count);
    for (MLNStyleLayer *layer in self.layers) {
        [self removeLayer:layer];
    }
    for (MLNStyleLayer *layer in layers) {
        [self addLayer:layer];
    }
}

- (NSUInteger)countOfLayers
{
    return self.rawStyle->getLayers().size();
}

- (MLNStyleLayer *)objectInLayersAtIndex:(NSUInteger)index
{
    auto layers = self.rawStyle->getLayers();
    if (index >= layers.size()) {
        [NSException raise:NSRangeException
                    format:@"No style layer at index %lu.", (unsigned long)index];
        return nil;
    }
    auto layer = layers.at(index);
    return [self layerFromMBGLLayer:layer];
}

- (void)getLayers:(MLNStyleLayer **)buffer range:(NSRange)inRange
{
    auto layers = self.rawStyle->getLayers();
    if (NSMaxRange(inRange) > layers.size()) {
        [NSException raise:NSRangeException
                    format:@"Style layer range %@ is out of bounds.", NSStringFromRange(inRange)];
    }
    NSUInteger i = 0;
    for (auto layer = *(layers.rbegin() + inRange.location); i < inRange.length; ++layer, ++i) {
        MLNStyleLayer *styleLayer = [self layerFromMBGLLayer:layer];
        buffer[i] = styleLayer;
    }
}

- (void)insertObject:(MLNStyleLayer *)styleLayer inLayersAtIndex:(NSUInteger)index
{
    if (!styleLayer.rawLayer) {
        [NSException raise:NSInvalidArgumentException format:
         @"The style layer %@ cannot be inserted into the style. "
         @"Make sure the style layer was created as a member of a concrete subclass of MLNStyleLayer.",
         styleLayer];
    }
    auto layers = self.rawStyle->getLayers();
    if (index > layers.size()) {
        [NSException raise:NSRangeException
                    format:@"Cannot insert style layer at out-of-bounds index %lu.", (unsigned long)index];
    } else if (index == 0) {
        try {
            MLNStyleLayer *sibling = layers.size() ? [self layerFromMBGLLayer:layers.at(0)] : nil;
            [styleLayer addToStyle:self belowLayer:sibling];
        } catch (const std::runtime_error & err) {
            [NSException raise:MLNRedundantLayerIdentifierException format:@"%s", err.what()];
        }
    } else {
        try {
            MLNStyleLayer *sibling = [self layerFromMBGLLayer:layers.at(index)];
            [styleLayer addToStyle:self belowLayer:sibling];
        } catch (std::runtime_error & err) {
            [NSException raise:MLNRedundantLayerIdentifierException format:@"%s", err.what()];
        }
    }
}

- (void)removeObjectFromLayersAtIndex:(NSUInteger)index
{
    auto layers = self.rawStyle->getLayers();
    if (index >= layers.size()) {
        [NSException raise:NSRangeException
                    format:@"Cannot remove style layer at out-of-bounds index %lu.", (unsigned long)index];
    }
    auto layer = layers.at(index);
    MLNStyleLayer *styleLayer = [self layerFromMBGLLayer:layer];
    [styleLayer removeFromStyle:self];
}

- (MLNStyleLayer *)layerFromMBGLLayer:(mbgl::style::Layer *)rawLayer
{
    NSParameterAssert(rawLayer);

    if (MLNStyleLayer *layer = rawLayer->peer.has_value() ? rawLayer->peer.get<LayerWrapper>().layer : nil) {
        return layer;
    }

    return mbgl::LayerManagerDarwin::get()->createPeer(rawLayer);
}

- (MLNStyleLayer *)layerWithIdentifier:(NSString *)identifier
{
    MLNLogDebug(@"Querying layerWithIdentifier: %@", identifier);
    auto mbglLayer = self.rawStyle->getLayer(identifier.UTF8String);
    return mbglLayer ? [self layerFromMBGLLayer:mbglLayer] : nil;
}

- (void)removeLayer:(MLNStyleLayer *)layer
{
    MLNLogDebug(@"Removing layer: %@", layer);
    if (!layer.rawLayer) {
        [NSException raise:NSInvalidArgumentException format:
         @"The style layer %@ cannot be removed from the style. "
         @"Make sure the style layer was created as a member of a concrete subclass of MLNStyleLayer.",
         layer];
    }
    [self willChangeValueForKey:@"layers"];
    [layer removeFromStyle:self];
    [self didChangeValueForKey:@"layers"];
}

- (void)addLayer:(MLNStyleLayer *)layer
{
    MLNLogDebug(@"Adding layer: %@", layer);
    if (!layer.rawLayer) {
        [NSException raise:NSInvalidArgumentException format:
         @"The style layer %@ cannot be added to the style. "
         @"Make sure the style layer was created as a member of a concrete subclass of MLNStyleLayer.",
         layer];
    }
    [self willChangeValueForKey:@"layers"];
    try {
        [layer addToStyle:self belowLayer:nil];
    } catch (std::runtime_error & err) {
        [NSException raise:MLNRedundantLayerIdentifierException format:@"%s", err.what()];
    }
    [self didChangeValueForKey:@"layers"];
}

- (void)insertLayer:(MLNStyleLayer *)layer atIndex:(NSUInteger)index {
    [self insertObject:layer inLayersAtIndex:index];
}

- (void)insertLayer:(MLNStyleLayer *)layer belowLayer:(MLNStyleLayer *)sibling
{
    MLNLogDebug(@"Inseting layer: %@ belowLayer: %@", layer, sibling);
    if (!layer.rawLayer) {
        [NSException raise:NSInvalidArgumentException
                    format:
         @"The style layer %@ cannot be added to the style. "
         @"Make sure the style layer was created as a member of a concrete subclass of MLNStyleLayer.",
         layer];
    }
    if (!sibling.rawLayer) {
        [NSException raise:NSInvalidArgumentException
                    format:
         @"A style layer cannot be placed below %@ in the style. "
         @"Make sure sibling was obtained using -[MLNStyle layerWithIdentifier:].",
         sibling];
    }
    [self willChangeValueForKey:@"layers"];
    try {
        [layer addToStyle:self belowLayer:sibling];
    } catch (std::runtime_error & err) {
        [NSException raise:MLNRedundantLayerIdentifierException format:@"%s", err.what()];
    }
    [self didChangeValueForKey:@"layers"];
}

- (void)insertLayer:(MLNStyleLayer *)layer aboveLayer:(MLNStyleLayer *)sibling {
    MLNLogDebug(@"Inseting layer: %@ aboveLayer: %@", layer, sibling);
    if (!layer.rawLayer) {
        [NSException raise:NSInvalidArgumentException
                    format:
         @"The style layer %@ cannot be added to the style. "
         @"Make sure the style layer was created as a member of a concrete subclass of MLNStyleLayer.",
         layer];
    }
    if (!sibling.rawLayer) {
        [NSException raise:NSInvalidArgumentException
                    format:
         @"A style layer cannot be placed above %@ in the style. "
         @"Make sure sibling was obtained using -[MLNStyle layerWithIdentifier:].",
         sibling];
    }

    auto layers = self.rawStyle->getLayers();
    std::string siblingIdentifier = sibling.identifier.UTF8String;
    NSUInteger index = 0;
    for (auto siblingLayer : layers) {
        if (siblingLayer->getID() == siblingIdentifier) {
            break;
        }
        index++;
    }

    [self willChangeValueForKey:@"layers"];
    if (index + 1 > layers.size()) {
        [NSException raise:NSInvalidArgumentException
                    format:
         @"A style layer cannot be placed above %@ in the style. "
         @"Make sure sibling was obtained using -[MLNStyle layerWithIdentifier:].",
         sibling];
    } else if (index + 1 == layers.size()) {
        try {
            [layer addToStyle:self belowLayer:nil];
        } catch (std::runtime_error & err) {
            [NSException raise:MLNRedundantLayerIdentifierException format:@"%s", err.what()];
        }
    } else {
        MLNStyleLayer *nextSibling = [self layerFromMBGLLayer:layers.at(index + 1)];
        try {
            [layer addToStyle:self belowLayer:nextSibling];
        } catch (std::runtime_error & err) {
            [NSException raise:MLNRedundantLayerIdentifierException format:@"%s", err.what()];
        }
    }
    [self didChangeValueForKey:@"layers"];
}

// MARK: Style images

- (void)setImage:(MLNImage *)image forName:(NSString *)name
{
    MLNLogDebug(@"Setting image: %@ forName: %@", image, name);
    if (!image) {
        [NSException raise:NSInvalidArgumentException
                    format:@"Cannot assign nil image to “%@”.", name];
    }
    if (!name) {
        [NSException raise:NSInvalidArgumentException
                    format:@"Cannot assign image %@ to a nil name.", image];
    }

    self.rawStyle->addImage([image mgl_styleImageWithIdentifier:name]);
}

- (void)removeImageForName:(NSString *)name
{
    MLNLogDebug(@"Removing imageForName: %@", name);
    if (!name) {
        [NSException raise:NSInvalidArgumentException
                    format:@"Cannot remove image with nil name."];
    }

    self.rawStyle->removeImage([name UTF8String]);
}

- (MLNImage *)imageForName:(NSString *)name
{
    MLNLogDebug(@"Querying imageForName: %@", name);
    if (!name) {
        [NSException raise:NSInvalidArgumentException
                    format:@"Cannot get image with nil name."];
    }

    auto styleImage = self.rawStyle->getImage([name UTF8String]);
    return styleImage ? [[MLNImage alloc] initWithMLNStyleImage:*styleImage] : nil;
}

// MARK: Style transitions

- (void)setTransition:(MLNTransition)transition
{
    self.rawStyle->setTransitionOptions(MLNOptionsFromTransition(transition));
}

- (MLNTransition)transition
{
    const mbgl::style::TransitionOptions transitionOptions = self.rawStyle->getTransitionOptions();
    
    return MLNTransitionFromOptions(transitionOptions);
}

- (void)setPerformsPlacementTransitions:(BOOL)performsPlacementTransitions
{
    mbgl::style::TransitionOptions transitionOptions = self.rawStyle->getTransitionOptions();
    transitionOptions.enablePlacementTransitions = static_cast<bool>(performsPlacementTransitions);
    self.rawStyle->setTransitionOptions(transitionOptions);
}

- (BOOL)performsPlacementTransitions
{
    mbgl::style::TransitionOptions transitionOptions = self.rawStyle->getTransitionOptions();
    return transitionOptions.enablePlacementTransitions;
}

// MARK: Style light

- (void)setLight:(MLNLight *)light
{
    std::unique_ptr<mbgl::style::Light> mbglLight = std::make_unique<mbgl::style::Light>([light mbglLight]);
    self.rawStyle->setLight(std::move(mbglLight));
}

- (MLNLight *)light
{
    auto mbglLight = self.rawStyle->getLight();
    MLNLight *light = [[MLNLight alloc] initWithMBGLLight:mbglLight];
    return light;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; name = %@, URL = %@>",
            NSStringFromClass([self class]), (void *)self,
            self.name ? [NSString stringWithFormat:@"\"%@\"", self.name] : self.name,
            self.URL ? [NSString stringWithFormat:@"\"%@\"", self.URL] : self.URL];
}

// MARK: Mapbox Streets source introspection

- (void)localizeLabelsIntoLocale:(nullable NSLocale *)locale {
    NSSet<MLNVectorTileSource *> *streetsSources =
        [self.sources filteredSetUsingPredicate:
         [NSPredicate predicateWithBlock:^BOOL(MLNVectorTileSource * _Nullable source, NSDictionary<NSString *, id> * _Nullable bindings) {
            return [source isKindOfClass:[MLNVectorTileSource class]] && [source isMapboxStreets];
        }]];
    NSSet<NSString *> *streetsSourceIdentifiers = [streetsSources valueForKey:@"identifier"];
    
    for (MLNSymbolStyleLayer *layer in self.layers) {
        if (![layer isKindOfClass:[MLNSymbolStyleLayer class]]) {
            continue;
        }
        if (![streetsSourceIdentifiers containsObject:layer.sourceIdentifier]) {
            continue;
        }
        
        NSExpression *text = layer.text;
        NSExpression *localizedText = [text mgl_expressionLocalizedIntoLocale:locale];
        if (![localizedText isEqual:text]) {
            layer.text = localizedText;
        }
    }
}

- (NSSet<MLNVectorTileSource *> *)mapboxStreetsSources {
    return [self.sources objectsPassingTest:^BOOL (__kindof MLNVectorTileSource * _Nonnull source, BOOL * _Nonnull stop) {
        return [source isKindOfClass:[MLNVectorTileSource class]] && source.mapboxStreets;
    }];
}

- (NSArray<MLNStyleLayer *> *)placeStyleLayers {
    NSSet *streetsSourceIdentifiers = [self.mapboxStreetsSources valueForKey:@"identifier"];
    
    NSSet *placeSourceLayerIdentifiers = [NSSet setWithObjects:@"marine_label", @"country_label", @"state_label", @"place_label", @"water_label", @"poi_label", @"rail_station_label", @"mountain_peak_label", @"natural_label", @"transit_stop_label", nil];
    NSPredicate *isPlacePredicate = [NSPredicate predicateWithBlock:^BOOL (MLNVectorStyleLayer * _Nullable layer, NSDictionary<NSString *, id> * _Nullable bindings) {
        return [layer isKindOfClass:[MLNVectorStyleLayer class]] && [streetsSourceIdentifiers containsObject:layer.sourceIdentifier] && [placeSourceLayerIdentifiers containsObject:layer.sourceLayerIdentifier];
    }];
    return [self.layers filteredArrayUsingPredicate:isPlacePredicate];
}

- (NSArray<MLNStyleLayer *> *)roadStyleLayers {
    NSSet *streetsSourceIdentifiers = [self.mapboxStreetsSources valueForKey:@"identifier"];

    NSSet *roadStyleLayerIdentifiers = [NSSet setWithObjects:@"road_label", @"road", nil];
    NSPredicate *isPlacePredicate = [NSPredicate predicateWithBlock:^BOOL (MLNVectorStyleLayer * _Nullable layer, NSDictionary<NSString *, id> * _Nullable bindings) {
        return [layer isKindOfClass:[MLNVectorStyleLayer class]] && [streetsSourceIdentifiers containsObject:layer.sourceIdentifier] && [roadStyleLayerIdentifiers containsObject:layer.sourceLayerIdentifier];
    }];
    return [self.layers filteredArrayUsingPredicate:isPlacePredicate];
}

@end
