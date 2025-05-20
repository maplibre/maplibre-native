#import "MLNMapOptions.h"

@interface MLNMapOptions ()

@end

@implementation MLNMapOptions

-(instancetype _Nonnull)init
{
    self = [super init];
    if (self)
    {
        _actionJournalOptions = [[MLNActionJournalOptions alloc] init];
    }

    return self;
}

@end
