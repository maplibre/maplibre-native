#import "MGLEvent_Private.h"
#import "MGLStyleValue_Private.h"

const MGLEventType MGLEventTypeResourceRequest = @"resource-request";

#pragma mark - Event

@implementation MGLEvent

- (instancetype)init {
    [self doesNotRecognizeSelector:_cmd];
    return nil;
}

- (instancetype)initWithEvent:(const mbgl::ObservableEvent&)event {
    self = [super init];

    if (!self)
        return nil;

    // mbgl::ObservableEvent timestamps do not use the system_clock, i.e. they
    // are not relative to the Unix epoch, so we'll need to offset appropriately
    auto systemClockNow = std::chrono::system_clock::now();
    auto steadyClockNow = std::chrono::steady_clock::now();
    auto begin = std::chrono::time_point_cast<std::chrono::system_clock::duration>(systemClockNow +
                                                                                   (event.begin - steadyClockNow));
    auto end = std::chrono::time_point_cast<std::chrono::system_clock::duration>(systemClockNow +
                                                                                 (event.end - steadyClockNow));

    auto beginTime  = std::chrono::duration<double, std::ratio<1>>(begin.time_since_epoch()).count();
    auto endTime    = std::chrono::duration<double, std::ratio<1>>(end.time_since_epoch()).count();

    _type   = (MGLEventType)[NSString stringWithUTF8String:event.type.c_str()];
    _begin  = beginTime;
    _end    = endTime;

    // From value.md
    // Supported types are `int`, `uint`, `bool`, `double`, `array`, `object` and `string`.
    _data = MGLJSONObjectFromMBGLValue(event.data);

    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@: %p; type = %@, begin = %f, end = %f, data = %@>",
            NSStringFromClass([self class]), (void *)self,
            self.type,
            self.begin,
            self.end,
            self.data];
}

#pragma mark - Equality

- (NSUInteger)hash {
    return self.type.hash ^ @(self.begin).hash ^ @(self.end).hash;
}

- (BOOL)isEqualToEvent:(MGLEvent *)other {
    if (self == other)
        return YES;

    // Ignore the value at this moment.
    return ((self.type == other.type) &&
            (self.begin == other.begin) &&
            (self.end == other.end));
}

- (BOOL)isEqual:(id)other {
    if (self == other)
        return YES;

    if (![other isKindOfClass:[MGLEvent class]]) {
        return NO;
    }

    return [self isEqualToEvent:(MGLEvent*)other];
}

@end