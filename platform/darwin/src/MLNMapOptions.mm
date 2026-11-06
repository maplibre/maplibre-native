#import "MLNMapOptions.h"

@interface MLNMapOptions ()

@end

@implementation MLNMapOptions

- (instancetype _Nonnull)init {
  self = [super init];
  if (self) {
    _styleURL = nil;
    _styleJSON = nil;
    _pluginLayers = nil;
    _actionJournalOptions = [[MLNActionJournalOptions alloc] init];
    _featureInfoEnabled = NO;
  }

  return self;
}

- (bool)fastPFOREnabled {
  return true;
}

- (void)setFastPFOREnabled:(bool)fastPFOREnabled {
}

@end
