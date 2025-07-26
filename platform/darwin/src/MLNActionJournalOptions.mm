#import "MLNActionJournalOptions.h"
#import <mbgl/util/action_journal_options.hpp>

@implementation MLNActionJournalOptions {
    std::unique_ptr<mbgl::util::ActionJournalOptions> _actionJournalOptionsInternal;
}

- (instancetype)init {
    if (self = [super init]) {
        _actionJournalOptionsInternal = std::make_unique<mbgl::util::ActionJournalOptions>();

        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
        _actionJournalOptionsInternal->withPath([paths.firstObject stringByAppendingPathComponent:@""].UTF8String);
    }
    return self;
}

- (mbgl::util::ActionJournalOptions)getCoreOptions {
    return *_actionJournalOptionsInternal;
}

- (void)setEnabled:(BOOL)value {
    _actionJournalOptionsInternal->enable(value);
}

- (BOOL)enabled {
    return _actionJournalOptionsInternal->enabled();
}

- (void)setPath:(NSString*)value {
    _actionJournalOptionsInternal->withPath(value.UTF8String);
}

- (NSString*)path {
    return [NSString stringWithUTF8String:_actionJournalOptionsInternal->path().c_str()];
}

- (void)setLogFileSize:(NSInteger)value {
    _actionJournalOptionsInternal->withLogFileSize(static_cast<uint32_t>(value));
}

- (NSInteger)logFileSize {
    return _actionJournalOptionsInternal->logFileSize();
}

- (void)setLogFileCount:(NSInteger)value {
    _actionJournalOptionsInternal->withLogFileCount(static_cast<uint32_t>(value));
}

- (NSInteger)logFileCount {
    return _actionJournalOptionsInternal->logFileCount();
}

- (void)setRenderingStatsReportInterval:(NSInteger)value {
    _actionJournalOptionsInternal->withRenderingStatsReportInterval(static_cast<uint32_t>(value));
}

- (NSInteger)renderingStatsReportInterval {
    return _actionJournalOptionsInternal->renderingStatsReportInterval();
}

@end
