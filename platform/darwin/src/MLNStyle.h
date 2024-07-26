#import <Foundation/Foundation.h>

#import "MLNFoundation.h"
#import "MLNStyleLayer.h"

#import "MLNTypes.h"

#import "MLNDefaultStyle.h"
#import "MLNSettings.h"
#import "MLNTileServerOptions.h"

@class MLNSource;
@class MLNLight;

NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXTERN MLN_EXPORT MLNExceptionName const MLNInvalidStyleURLException;
FOUNDATION_EXTERN MLN_EXPORT MLNExceptionName const MLNRedundantLayerException;
FOUNDATION_EXTERN MLN_EXPORT MLNExceptionName const MLNRedundantLayerIdentifierException;
FOUNDATION_EXTERN MLN_EXPORT MLNExceptionName const MLNRedundantSourceException;
FOUNDATION_EXTERN MLN_EXPORT MLNExceptionName const MLNRedundantSourceIdentifierException;

/**
 The proxy object for the current map style.

 MLNStyle provides a set of convenience methods for changing default styles included
 with MapLibre.

 It is also possible to directly manipulate the current map style
 via `MLNMapView.style` by updating the style's data sources or layers.

 @note Wait until the map style has finished loading before modifying a map's
    style via any of the `MLNStyle` instance methods below. You can use the
    `-[MLNMapViewDelegate mapView:didFinishLoadingStyle:]` or
    `-[MLNMapViewDelegate mapViewDidFinishLoadingMap:]` methods as indicators
    that it's safe to modify the map's style.
 */
MLN_EXPORT
@interface MLNStyle : NSObject

// MARK: Accessing Default Styles

/**
 Returns list of predefined styles
 */
+ (NSArray<MLNDefaultStyle *> *)predefinedStyles;

/**
 Returns default style
 */
+ (MLNDefaultStyle *)defaultStyle;

/**
 Returns default style as NSURL
 */
+ (nullable NSURL *)defaultStyleURL;

/** Get predefined style by name

 @param withStyleName style name.
 */
+ (nullable MLNDefaultStyle *)predefinedStyle:(NSString *)withStyleName;

// MARK: Accessing Metadata About the Style

/**
 The name of the style.

 You can customize the style’s name in Mapbox Studio.
 */
@property (readonly, copy, nullable) NSString *name;

// MARK: Managing Sources

/**
 A set containing the style’s sources.
 */
@property (nonatomic, strong) NSSet<__kindof MLNSource *> *sources;

/**
 Values describing animated transitions to changes on a style's individual
 paint properties.
 */
@property (nonatomic) MLNTransition transition;

/**
 A boolean value indicating whether label placement transitions are enabled.

 The default value of this property is `YES`.
 */
@property (nonatomic, assign) BOOL performsPlacementTransitions;

/**
 Returns a source with the given identifier in the current style.

 @note Source identifiers are not guaranteed to exist across styles or different
    versions of the same style. Applications that use this API must first set the
    style URL to an explicitly versioned style using a convenience method like
    `+[MLNStyle outdoorsStyleURLWithVersion:]`, `MLNMapView`’s “Style URL”
    inspectable in Interface Builder, or a manually constructed `NSURL`. This
    approach also avoids source identifer name changes that will occur in the default
    style’s sources over time.

 @return An instance of a concrete subclass of `MLNSource` associated with the
    given identifier, or `nil` if the current style contains no such source.
 */
- (nullable MLNSource *)sourceWithIdentifier:(NSString *)identifier;

/**
 Adds a new source to the current style.

 @note Adding the same source instance more than once will result in a
    `MLNRedundantSourceException`. Reusing the same source identifier, even with
    different source instances, will result in a
    `MLNRedundantSourceIdentifierException`.

 @note Sources should be added in
    `-[MLNMapViewDelegate mapView:didFinishLoadingStyle:]` or
    `-[MLNMapViewDelegate mapViewDidFinishLoadingMap:]` to ensure that the map
    has loaded the style and is ready to accept a new source.

 @param source The source to add to the current style.
 */
- (void)addSource:(MLNSource *)source;

/**
 Removes a source from the current style.

 @note Source identifiers are not guaranteed to exist across styles or different
    versions of the same style. Applications that use this API must first set the
    style URL to an explicitly versioned style using a convenience method like
    `+[MLNStyle outdoorsStyleURLWithVersion:]`, `MLNMapView`’s “Style URL”
    inspectable in Interface Builder, or a manually constructed `NSURL`. This
    approach also avoids source identifer name changes that will occur in the default
    style’s sources over time.

 @param source The source to remove from the current style.
 */
- (void)removeSource:(MLNSource *)source;

/**
 Removes a source from the current style.

 @note Source identifiers are not guaranteed to exist across styles or different
 versions of the same style. Applications that use this API must first set the
 style URL to an explicitly versioned style using a convenience method like
 `+[MLNStyle outdoorsStyleURLWithVersion:]`, `MLNMapView`’s “Style URL”
 inspectable in Interface Builder, or a manually constructed `NSURL`. This
 approach also avoids source identifer name changes that will occur in the default
 style’s sources over time.

 @param source The source to remove from the current style.
 @param outError Upon return, if an error has occurred, a pointer to an `NSError`
 object describing the error. Pass in `NULL` to ignore any error.

 @return `YES` if `source` was removed successfully. If `NO`, `outError` contains
 an `NSError` object describing the problem.
 */
- (BOOL)removeSource:(MLNSource *)source error:(NSError *__nullable *__nullable)outError;

// MARK: Managing Style Layers

/**
 The layers included in the style, arranged according to their back-to-front
 ordering on the screen.
 */
@property (nonatomic, strong) NSArray<__kindof MLNStyleLayer *> *layers;

/**
 Returns a style layer with the given identifier in the current style.

 @note Layer identifiers are not guaranteed to exist across styles or different
    versions of the same style. Applications that use this API must first set
    the style URL to an explicitly versioned style using a convenience method like
    `+[MLNStyle outdoorsStyleURLWithVersion:]`, `MLNMapView`’s “Style URL”
    inspectable in Interface Builder, or a manually constructed `NSURL`. This
    approach also avoids layer identifer name changes that will occur in the default
    style’s layers over time.

 @return An instance of a concrete subclass of `MLNStyleLayer` associated with
    the given identifier, or `nil` if the current style contains no such style
    layer.
 */
- (nullable MLNStyleLayer *)layerWithIdentifier:(NSString *)identifier;

/**
 Adds a new layer on top of existing layers.

 @note Adding the same layer instance more than once will result in a
    `MLNRedundantLayerException`. Reusing the same layer identifer, even with
    different layer instances, will also result in an exception.

 @note Layers should be added in
    `-[MLNMapViewDelegate mapView:didFinishLoadingStyle:]` or
    `-[MLNMapViewDelegate mapViewDidFinishLoadingMap:]` to ensure that the map
    has loaded the style and is ready to accept a new layer.

 @param layer The layer object to add to the map view. This object must be an
    instance of a concrete subclass of `MLNStyleLayer`.
 */
- (void)addLayer:(MLNStyleLayer *)layer;

/**
 Inserts a new layer into the style at the given index.

 @note Adding the same layer instance more than once will result in a
    `MLNRedundantLayerException`. Reusing the same layer identifer, even with
    different layer instances, will also result in an exception.

 @note Layers should be added in
    `-[MLNMapViewDelegate mapView:didFinishLoadingStyle:]` or
    `-[MLNMapViewDelegate mapViewDidFinishLoadingMap:]` to ensure that the map
    has loaded the style and is ready to accept a new layer.

 @param layer The layer to insert.
 @param index The index at which to insert the layer. An index of 0 would send
    the layer to the back; an index equal to the number of objects in the
    `layers` property would bring the layer to the front.
 */
- (void)insertLayer:(MLNStyleLayer *)layer atIndex:(NSUInteger)index;

/**
 Inserts a new layer below another layer.

 @note Layer identifiers are not guaranteed to exist across styles or different
    versions of the same style. Applications that use this API must first set
    the style URL to an explicitly versioned style using a convenience method like
    `+[MLNStyle outdoorsStyleURLWithVersion:]`, `MLNMapView`’s “Style URL”
    inspectable in Interface Builder, or a manually constructed `NSURL`. This
    approach also avoids layer identifer name changes that will occur in the default
    style’s layers over time.

    Inserting the same layer instance more than once will result in a
    `MLNRedundantLayerException`. Reusing the same layer identifer, even with
    different layer instances, will also result in an exception.

 @param layer The layer to insert.
 @param sibling An existing layer in the style.

 #### Related examples
 TODO: Add multiple shapes from a single shape source, learn how to
 add a layer to your map below an existing layer.
 */
- (void)insertLayer:(MLNStyleLayer *)layer belowLayer:(MLNStyleLayer *)sibling;

/**
 Inserts a new layer above another layer.

 @note Layer identifiers are not guaranteed to exist across styles or different
    versions of the same style. Applications that use this API must first set
    the style URL to an explicitly versioned style using a convenience method like
    `+[MLNStyle outdoorsStyleURLWithVersion:]`, `MLNMapView`’s “Style URL”
    inspectable in Interface Builder, or a manually constructed `NSURL`. This
    approach also avoids layer identifer name changes that will occur in the default
    style’s layers over time.

    Inserting the same layer instance more than once will result in a
    `MLNRedundantLayerException`. Reusing the same layer identifer, even with
    different layer instances, will also result in an exception.

 @param layer The layer to insert.
 @param sibling An existing layer in the style.

 #### Related examples
 TODO: Add an image, learn how to add a layer to your map above an
 existing layer.
 */
- (void)insertLayer:(MLNStyleLayer *)layer aboveLayer:(MLNStyleLayer *)sibling;

/**
 Removes a layer from the map view.

 @note Layer identifiers are not guaranteed to exist across styles or different
    versions of the same style. Applications that use this API must first set
    the style URL to an explicitly versioned style using a convenience method like
    `+[MLNStyle outdoorsStyleURLWithVersion:]`, `MLNMapView`’s “Style URL”
    inspectable in Interface Builder, or a manually constructed `NSURL`. This
    approach also avoids layer identifer name changes that will occur in the default
    style’s layers over time.

 @param layer The layer object to remove from the map view. This object
 must conform to the `MLNStyleLayer` protocol.
 */
- (void)removeLayer:(MLNStyleLayer *)layer;

// MARK: Managing a Style’s Images

/**
 Returns the image associated with the given name in the style.

 @note Names and their associated images are not guaranteed to exist across
    styles or different versions of the same style. Applications that use this
    API must first set the style URL to an explicitly versioned style using a
    convenience method like `+[MLNStyle outdoorsStyleURLWithVersion:]`,
    `MLNMapView`’s “Style URL” inspectable in Interface Builder, or a manually
    constructed `NSURL`. This approach also avoids image name changes that will
    occur in the default style over time.

 @param name The name associated with the image you want to obtain.
 @return The image associated with the given name, or `nil` if no image is
    associated with that name.
 */
- (nullable MLNImage *)imageForName:(NSString *)name;

/**
 Adds or overrides an image used by the style’s layers.

 To use an image in a style layer, give it a unique name using this method, then
 set the `iconImageName` property of an `MLNSymbolStyleLayer` object to that
 name.

 @param image The image for the name.
 @param name The name of the image to set to the style.

 #### Related examples
 TODO: Use images to cluster point data
 TODO: Cluster point data
 Learn how to add images to your map using an `MLNStyle` object.
 */
- (void)setImage:(MLNImage *)image forName:(NSString *)name;

/**
 Removes a name and its associated image from the style.

 @note Names and their associated images are not guaranteed to exist across
    styles or different versions of the same style. Applications that use this
    API must first set the style URL to an explicitly versioned style using a
    convenience method like `+[MLNStyle outdoorsStyleURLWithVersion:]`,
    `MLNMapView`’s “Style URL” inspectable in Interface Builder, or a manually
    constructed `NSURL`. This approach also avoids image name changes that will
    occur in the default style over time.

 @param name The name of the image to remove.
 */
- (void)removeImageForName:(NSString *)name;

// MARK: Managing the Style's Light

/**
 Provides global light source for the style.
 */
@property (nonatomic, strong) MLNLight *light;

// MARK: Localizing Map Content

/**
 Attempts to localize labels in the style into the given locale.

 This method automatically modifies the text property of any symbol style layer
 in the style whose source is the
 <a href="https://www.mapbox.com/vector-tiles/mapbox-streets-v8/#overview">Mapbox Streets
 source</a>. On iOS, the user can set the system’s preferred language in Settings, General Settings,
 Language & Region. On macOS, the user can set the system’s preferred language in the Language &
 Region pane of System Preferences.

 @param locale The locale into which labels should be localized. To use the
    system’s preferred language, if supported, specify `nil`. To use the local
    language, specify a locale with the identifier `mul`.
 */
- (void)localizeLabelsIntoLocale:(nullable NSLocale *)locale;

@end

/**
 An object whose contents are represented by an `MLNStyle` object that you
 configure.
 */
@protocol MLNStylable <NSObject>

/**
 The style currently displayed in the receiver.

 @note The default styles provided by Mapbox contain sources and layers with
    identifiers that will change over time. Applications that use APIs that
    manipulate a style’s sources and layers must first set the style URL to an
    explicitly versioned style using a convenience method like
    `+[MLNStyle outdoorsStyleURLWithVersion:]` or a manually constructed
     `NSURL`.
 */
@property (nonatomic, readonly, nullable) MLNStyle *style;

@end

NS_ASSUME_NONNULL_END
