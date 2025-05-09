#import <mbgl/util/action_journal_options.hpp>
#import "MLNActionJournalOptions.h"

NS_ASSUME_NONNULL_BEGIN

@interface MLNActionJournalOptions (Private)

- (mbgl::util::ActionJournalOptions)getCoreOptions;

@end

NS_ASSUME_NONNULL_END
