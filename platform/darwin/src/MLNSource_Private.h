#import "MLNSource.h"

#include <memory>

NS_ASSUME_NONNULL_BEGIN

@protocol MLNStylable;

namespace mbgl {
namespace style {
class Source;
}
}  // namespace mbgl

// A struct to be stored in the `peer` member of mbgl::style::Source, in order to implement
// object identity. We don't store a MLNSource pointer directly because that doesn't
// interoperate with ARC. The inner pointer is weak in order to avoid a reference cycle for
// "pending" MLNSources, which have a strong owning pointer to the mbgl::style::Source.
struct SourceWrapper {
  __weak MLNSource *source;
};

/**
 Assert that the style source is valid.

 This macro should be used at the beginning of any public-facing instance method
 of ``MLNSource`` and its subclasses. For private methods, an assertion is more appropriate.
 */
#define MLNAssertStyleSourceIsValid()                                            \
  do {                                                                           \
    if (!self.rawSource) {                                                       \
      [NSException raise:MLNInvalidStyleSourceException                          \
                  format:@"This source got invalidated after the style change"]; \
    }                                                                            \
  } while (NO);

@class MLNMapView;

@interface MLNSource (Private)

/**
 Initializes and returns a source with a raw pointer to the backing store,
 associated with a style.
 */
- (instancetype)initWithRawSource:(mbgl::style::Source *)rawSource
                         stylable:(nullable id<MLNStylable>)stylable;

/**
 Initializes and returns a source with an owning pointer to the backing store,
 unassociated from a style.
 */
- (instancetype)initWithPendingSource:(std::unique_ptr<mbgl::style::Source>)pendingSource;

/**
 A raw pointer to the mbgl object, which is always initialized, either to the
 value returned by `mbgl::Map getSource`, or for independently created objects,
 to the pointer value held in `pendingSource`. In the latter case, this raw
 pointer value stays even after ownership of the object is transferred via
 `mbgl::Map addSource`.
 */
@property (nonatomic, readonly) mbgl::style::Source *rawSource;

/**
 The stylable object whose style currently contains the source.

 If the source is not currently part of any stylable object’s style, this
 property is set to `nil`.
 */
@property (nonatomic, readonly, weak) id<MLNStylable> stylable;

/**
 Adds the mbgl source that this object represents to the mbgl map.
 Once a mbgl source is added, ownership of the object is transferred to the
 `mbgl::Map` and this object no longer has an active unique_ptr reference to the
 `mbgl::Source`. If this object's mbgl source is in that state, the mbgl source
 can still be changed but the changes will not be visible until the ``MLNSource``
 is added back to the map via ``MLNStyle/addSource:`` and styled with a
 ``MLNLayer``.
 */
- (void)addToStylable:(id<MLNStylable>)mapView;

/**
 Removes the mbgl source that this object represents from the mbgl map.
 When a mbgl source is removed, ownership of the object is transferred back
 to the ``MLNSource`` instance and the unique_ptr reference is valid again. It is
 safe to add the source back to the style after it is removed.
 */
- (BOOL)removeFromStylable:(id<MLNStylable>)mapView error:(NSError *__nullable *__nullable)outError;

@end

NS_ASSUME_NONNULL_END
