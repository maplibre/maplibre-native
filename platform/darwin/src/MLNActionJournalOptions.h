#import <Foundation/Foundation.h>

#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

/**
 The ``MLNActionJournalOptions`` defines action journal properties such as path, log size, etc...
 */
MLN_EXPORT
@interface MLNActionJournalOptions : NSObject

/**
 * Enable/disable journal logging
 */
@property (nonatomic) BOOL enabled;

/**
 * Local file path.
 */
@property (nonatomic, nonnull) NSString* path;

/**
 * Log file size (total journal size is equal to `logFileSize * logFileCount`)
 */
@property (nonatomic) NSInteger logFileSize;

/**
 * Maximum number of log files
 */
@property (nonatomic) NSInteger logFileCount;

/**
 * The wait time (seconds) between rendering reports
 */
@property (nonatomic) NSInteger renderingStatsReportInterval;

@end

NS_ASSUME_NONNULL_END
