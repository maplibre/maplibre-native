#import <Foundation/Foundation.h>

#import "MLNActionJournalOptions.h"
#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

/**
 The ``MLNMapOptions`` object provides a way to set map properties for each instance
 */
MLN_EXPORT
@interface MLNMapOptions : NSObject

/**
 Action journal  options
 */
@property (nonatomic, nonnull) MLNActionJournalOptions* actionJournalOptions;

@end

NS_ASSUME_NONNULL_END
