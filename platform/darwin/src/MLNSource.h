#import <Foundation/Foundation.h>

#import "MLNFoundation.h"
#import "MLNTypes.h"

NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXTERN MLN_EXPORT MLNExceptionName const MLNInvalidStyleSourceException;

/**
 ``MLNSource`` is an abstract base class for map content sources. A map content
 source supplies content to be shown on the map. A source is added to an
 ``MLNStyle`` object along with an ``MLNStyle`` object. The
 foreground style layer defines the appearance of any content supplied by the
 source.

 Each source defined by the style JSON file is represented at runtime by an
 ``MLNSource`` object that you can use to refine the map’s content. You can also
 add and remove sources dynamically using methods such as
 ``MLNStyle/addSource:`` and ``MLNStyle/sourceWithIdentifier:``.

 Create instances of ``MLNShapeSource``, ``MLNShapeSource``,
 ``MLNImageSource``, and the concrete subclasses of ``MLNImageSource``
 (``MLNVectorTileSource`` and ``MLNRasterTileSource``) in order to use ``MLNRasterTileSource``’s
 properties and methods. Do not create instances of ``MLNSource`` directly, and do
 not create your own subclasses of this class.
 */
MLN_EXPORT
@interface MLNSource : NSObject

// MARK: Initializing a Source

- (instancetype)init __attribute__((unavailable("Use -initWithIdentifier: instead.")));

/**
 Returns a source initialized with an identifier.

 After initializing and configuring the source, add it to a map view’s style
 using the ``MLNStyle/addSource:`` method.

 @param identifier A string that uniquely identifies the source in the style to
    which it is added.
 @return An initialized source.
 */
- (instancetype)initWithIdentifier:(NSString *)identifier;

// MARK: Identifying a Source

/**
 A string that uniquely identifies the source in the style to which it is added.
 */
@property (nonatomic, copy) NSString *identifier;

@end

NS_ASSUME_NONNULL_END
