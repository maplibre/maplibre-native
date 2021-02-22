#import "MGLFoundation.h"

NS_ASSUME_NONNULL_BEGIN

/**
 Type of event used when subscribing to and unsubscribing from an `MGLObservable`.
 */
typedef NSString *MGLEventType NS_TYPED_EXTENSIBLE_ENUM;

// TODO: Doc
FOUNDATION_EXPORT MGL_EXPORT MGLEventType const MGLEventTypeResourceRequest;


/**
 Generic Event used when notifying an `MGLObserver`. This is not intended nor
 expected to be created by the application developer. It will be provided as
 part of an `MGLObservable` notification.
 */
MGL_EXPORT
@interface MGLEvent: NSObject

/// Type of an event. Matches an original event type used for a subscription.
@property (nonatomic, readonly, copy) MGLEventType type;

/// Timestamp taken at the time of an event creation, relative to the Unix epoch.
@property (nonatomic, readonly) NSTimeInterval begin;

/// Timestamp taken at the time of an event completion. For a non-interval
/// (single-shot) events, migth be equal to an event's `begin` timestamp.
/// Relative to the Unix epoch.
@property (nonatomic, readonly) NSTimeInterval end;

/// Generic property for the event's data. Supported types are: `NSNumber` (int64,
/// uint64, bool, double), `NSString`, `NSArray`, `NSDictionary`.
@property (nonatomic, readonly, copy) id data;

/// Test for equality. Note this compares all properties except `data`.
- (BOOL)isEqualToEvent:(MGLEvent *)otherEvent;
@end

NS_ASSUME_NONNULL_END