#import "MLNOfflineStorage.h"

#import "MLNOfflinePack.h"

#include <mln/storage/database_file_source.hpp>
#include <mln/storage/online_file_source.hpp>

#include "MLNSettings_Private.h"

#include <memory>

NS_ASSUME_NONNULL_BEGIN

@interface MLNOfflineStorage (Private)

/**
 The shared database file source object owned by the shared offline storage object.
 */
@property (nonatomic) std::shared_ptr<mln::DatabaseFileSource> mbglDatabaseFileSource;

/**
 The shared online file source object owned by the shared offline storage object.
 */
@property (nonatomic) std::shared_ptr<mln::FileSource> mbglOnlineFileSource;

/**
 The shared resource loader file source object owned by the shared offline storage object.
 */
@property (nonatomic) std::shared_ptr<mln::FileSource> mbglFileSource;

- (void)getPacksWithCompletionHandler:(void (^)(NSArray<MLNOfflinePack *> *packs,
                                                NSError *_Nullable error))completion;

@end

NS_ASSUME_NONNULL_END
