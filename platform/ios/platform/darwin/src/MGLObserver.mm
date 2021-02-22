#import "MGLObserver_Private.h"
#import "MGLLoggingConfiguration_Private.h"
#import "MGLEvent_Private.h"

#pragma mark - Native C++ peer object

namespace mbgl {
namespace darwin {

void Observer::notify(const ObservableEvent& event) {

    if (!observer) {
        MGLLogWarning(@"Platform observer has been deallocated");
        return;
    }

    [observer notifyWithEvent:[[MGLEvent alloc] initWithEvent:event]];
}

std::size_t Observer::id() const {
    if (!observer) {
        MGLLogWarning(@"Platform observer has been deallocated");
        return 0;
    }

    return static_cast<std::size_t>(observer.identifier);
}

}
}

#pragma mark - Cocoa observer

@implementation MGLObserver
+ (NSUInteger)nextIdentifier {
    static NSUInteger identifier;
    return ++identifier;
}

- (void)dealloc {
    MGLAssert(!_observing, @"Ensure the observer is unsubscribed before deallocating");
}

- (instancetype)init {
    self = [super init];

    if (!self) return nil;

    _identifier = [MGLObserver nextIdentifier];
    _peer = std::make_shared<mbgl::darwin::Observer>(self);

    return self;
}

- (void)notifyWithEvent:(MGLEvent*)event {
    if (self.notificationHandler) {
        self.notificationHandler(event);
    }
}

- (BOOL)isEqualToObserver:(MGLObserver *)other {
    if (self == other)
        return YES;

    if (self.identifier != other.identifier)
        return NO;

    return (self.peer == other.peer);
}

- (BOOL)isEqual:(id)other {
    if (self == other)
        return YES;

    if (![other isKindOfClass:[MGLObserver class]]) {
        return NO;
    }

    return [self isEqualToObserver:(MGLObserver*)other];
}

- (NSUInteger)hash {
    // Rotate the address
    NSUInteger peerHash = reinterpret_cast<NSUInteger>(self.peer.get());

    NSUInteger width = (sizeof(NSUInteger) * __CHAR_BIT__);
    NSUInteger shift = self.identifier % width;

    NSUInteger newHash = (peerHash << shift) | (peerHash >> (width - shift));
    return newHash;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@: %p; identifier = %lu, peer = %p, hash = %lu>",
            NSStringFromClass([self class]), (void *)self,
            (unsigned long)self.identifier,
            self.peer.get(),
            (unsigned long)[self hash]];
}

@end