#import "MGLOfflineStorage_Private.h"

#import "MGLFoundation_Private.h"
#import "MGLSettings_Private.h"
#import "MGLGeometry_Private.h"
#import "MGLOfflinePack_Private.h"
#import "MGLOfflineRegion_Private.h"
#import "MGLTilePyramidOfflineRegion.h"
#import "MGLShapeOfflineRegion.h"
#import "NSBundle+MGLAdditions.h"
#import "NSValue+MGLAdditions.h"
#import "NSDate+MGLAdditions.h"
#import "MGLLoggingConfiguration_Private.h"
#import "MGLNetworkConfiguration_Private.h"

#include <mbgl/actor/actor.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/storage/file_source_manager.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/storage/resource_transform.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/string.hpp>

#include <memory>

static NSString * const MGLOfflineStorageDatabasePathInfoDictionaryKey = @"MGLOfflineStorageDatabasePath";
static NSString * const MGLOfflineStorageFileName = @"cache.db";
static NSString * const MGLOfflineStorageFileName3_2_0_beta_1 = @"offline.db";

const NSNotificationName MGLOfflinePackProgressChangedNotification = @"MGLOfflinePackProgressChanged";
const NSNotificationName MGLOfflinePackErrorNotification = @"MGLOfflinePackError";
const NSNotificationName MGLOfflinePackMaximumMapboxTilesReachedNotification = @"MGLOfflinePackMaximumMapboxTilesReached";

const MGLOfflinePackUserInfoKey MGLOfflinePackUserInfoKeyState = @"State";
const MGLOfflinePackUserInfoKey MGLOfflinePackUserInfoKeyProgress = @"Progress";
const MGLOfflinePackUserInfoKey MGLOfflinePackUserInfoKeyError = @"Error";
const MGLOfflinePackUserInfoKey MGLOfflinePackUserInfoKeyMaximumCount = @"MaximumCount";

const MGLExceptionName MGLUnsupportedRegionTypeException = @"MGLUnsupportedRegionTypeException";

@interface MGLOfflineStorage ()

@property (nonatomic, strong, readwrite) NSMutableArray<MGLOfflinePack *> *packs;
@property (nonatomic) std::shared_ptr<mbgl::DatabaseFileSource> mbglDatabaseFileSource;
@property (nonatomic) std::shared_ptr<mbgl::FileSource> mbglOnlineFileSource;
@property (nonatomic) std::shared_ptr<mbgl::FileSource> mbglFileSource;
@property (nonatomic, getter=isPaused) BOOL paused;
@end

@implementation MGLOfflineStorage {
    NSURL *_databaseURL;
    std::unique_ptr<mbgl::Actor<mbgl::ResourceTransform::TransformCallback>> _mbglResourceTransform;
}

+ (instancetype)sharedOfflineStorage {
    static dispatch_once_t onceToken;
    static MGLOfflineStorage *sharedOfflineStorage;
    dispatch_once(&onceToken, ^{
        sharedOfflineStorage = [[self alloc] init];
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
        [[NSNotificationCenter defaultCenter] addObserver:sharedOfflineStorage selector:@selector(unpauseFileSource:) name:UIApplicationWillEnterForegroundNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:sharedOfflineStorage selector:@selector(pauseFileSource:) name:UIApplicationDidEnterBackgroundNotification object:nil];
#endif
        [sharedOfflineStorage reloadPacks];
    });

    // Always ensure the MGLNativeNetworkManager delegate is setup. Calling
    // `resetNativeNetworkManagerDelegate` is not necessary here, since the shared
    // manager already calls it.
    //
    // TODO: Consider only calling this for testing?
    [MGLNetworkConfiguration sharedManager];

    return sharedOfflineStorage;
}

#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
- (void)pauseFileSource:(__unused NSNotification *)notification {
    if (self.isPaused) {
        return;
    }

    _mbglOnlineFileSource->pause();
    _mbglDatabaseFileSource->pause();
    self.paused = YES;
}

- (void)unpauseFileSource:(__unused NSNotification *)notification {
    if (!self.isPaused) {
        return;
    }

    _mbglOnlineFileSource->resume();
    _mbglDatabaseFileSource->resume();
    self.paused = NO;
}
#endif

- (void)setDelegate:(id<MGLOfflineStorageDelegate>)newValue {
    MGLLogDebug(@"Setting delegate: %@", newValue);
    _delegate = newValue;
    if ([self.delegate respondsToSelector:@selector(offlineStorage:URLForResourceOfKind:withURL:)]) {
        _mbglResourceTransform = std::make_unique<mbgl::Actor<mbgl::ResourceTransform::TransformCallback>>(*mbgl::Scheduler::GetCurrent(), [offlineStorage = self](auto kind_, const std::string& url_, mbgl::ResourceTransform::FinishedCallback cb) {
            NSURL* url =
            [NSURL URLWithString:[[NSString alloc] initWithBytes:url_.data()
                                                          length:url_.length()
                                                        encoding:NSUTF8StringEncoding]];
            MGLResourceKind kind = MGLResourceKindUnknown;
            switch (kind_) {
                case mbgl::Resource::Kind::Tile:
                    kind = MGLResourceKindTile;
                    break;
                case mbgl::Resource::Kind::Glyphs:
                    kind = MGLResourceKindGlyphs;
                    break;
                case mbgl::Resource::Kind::Style:
                    kind = MGLResourceKindStyle;
                    break;
                case mbgl::Resource::Kind::Source:
                    kind = MGLResourceKindSource;
                    break;
                case mbgl::Resource::Kind::SpriteImage:
                    kind = MGLResourceKindSpriteImage;
                    break;
                case mbgl::Resource::Kind::SpriteJSON:
                    kind = MGLResourceKindSpriteJSON;
                    break;
                case mbgl::Resource::Kind::Image:
                    kind = MGLResourceKindImage;
                    break;
                case mbgl::Resource::Kind::Unknown:
                    kind = MGLResourceKindUnknown;
                    break;

            }
            url = [offlineStorage.delegate offlineStorage:offlineStorage
                                     URLForResourceOfKind:kind
                                                  withURL:url];
            cb(url.absoluteString.UTF8String);
        });

        _mbglOnlineFileSource->setResourceTransform({[actorRef = _mbglResourceTransform->self()](auto kind_, const std::string& url_, mbgl::ResourceTransform::FinishedCallback cb_){
            actorRef.invoke(&mbgl::ResourceTransform::TransformCallback::operator(), kind_, url_, std::move(cb_));
        }});
    } else {
        _mbglResourceTransform.reset();
        _mbglOnlineFileSource->setResourceTransform({});
    }
}

- (instancetype)init {
    // Ensure network configuration & appropriate delegate prior to starting the
    // run loop. Calling `resetNativeNetworkManagerDelegate` is not necessary here,
    // since the shared manager already calls it.
    [MGLNetworkConfiguration sharedManager];

    MGLInitializeRunLoop();

    if (self = [super init]) {
        mbgl::TileServerOptions* tileServerOptions = [[MGLSettings sharedSettings] tileServerOptionsInternal];
        mbgl::ResourceOptions options;
        options.withCachePath(self.databasePath.UTF8String)
               .withAssetPath([NSBundle mainBundle].resourceURL.path.UTF8String)
               .withTileServerOptions(*tileServerOptions);
        
        _mbglFileSource = mbgl::FileSourceManager::get()->getFileSource(mbgl::FileSourceType::ResourceLoader, options);
        _mbglOnlineFileSource = mbgl::FileSourceManager::get()->getFileSource(mbgl::FileSourceType::Network, options);
        _mbglDatabaseFileSource = std::static_pointer_cast<mbgl::DatabaseFileSource>(std::shared_ptr<mbgl::FileSource>(mbgl::FileSourceManager::get()->getFileSource(mbgl::FileSourceType::Database, options)));
        
        // Observe for changes to the tile server options (and find out the current one).
        [[MGLSettings sharedSettings] addObserver:self
                                            forKeyPath:@"tileServerOptionsChangeToken"
                                               options:(NSKeyValueObservingOptionInitial |
                                                              NSKeyValueObservingOptionNew)
                                               context:NULL];

        // Observe for changes to the global access token (and find out the current one).
        [[MGLSettings sharedSettings] addObserver:self
                                            forKeyPath:@"apiKey"
                                               options:(NSKeyValueObservingOptionInitial |
                                                        NSKeyValueObservingOptionNew)
                                               context:NULL];
    }
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    [[MGLSettings sharedSettings] removeObserver:self forKeyPath:@"tileServerOptionsChangeToken"];
    [[MGLSettings sharedSettings] removeObserver:self forKeyPath:@"apiKey"];

    for (MGLOfflinePack *pack in self.packs) {
        [pack invalidate];
    }
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *, id> *)change context:(void *)context {
    // Synchronize the file source’s access token with the global one in MGLSettings.
    if ([keyPath isEqualToString:@"apiKey"] && object == [MGLSettings sharedSettings]) {
        NSString *apiKey = change[NSKeyValueChangeNewKey];
        if (![apiKey isKindOfClass:[NSNull class]]) {
            _mbglOnlineFileSource->setProperty(mbgl::API_KEY_KEY, apiKey.UTF8String);
        }
    } else if ([keyPath isEqualToString:@"tileServerOptionsChangeToken"] && object == [MGLSettings sharedSettings]) {
        auto tileServerOptions = [[MGLSettings sharedSettings] tileServerOptionsInternal];
        auto apiBaseURL = tileServerOptions->baseURL();
        auto resourceOptions = _mbglOnlineFileSource->getResourceOptions().clone();
         _mbglOnlineFileSource->setResourceOptions(resourceOptions.withTileServerOptions(*tileServerOptions).clone());
    } else {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

#pragma mark Database management methods

- (NSString *)databasePath {
    return self.databaseURL.path;
}

- (NSURL *)databaseURL {
    if (!_databaseURL) {
        NSString *customPath = [NSBundle.mainBundle objectForInfoDictionaryKey:MGLOfflineStorageDatabasePathInfoDictionaryKey];
        if ([customPath isKindOfClass:[NSString class]]) {
            _databaseURL = [NSURL fileURLWithPath:customPath.stringByStandardizingPath isDirectory:NO relativeToURL:NSBundle.mainBundle.resourceURL];
            
            NSURL *directoryURL = _databaseURL.URLByDeletingLastPathComponent;
            [NSFileManager.defaultManager createDirectoryAtURL:directoryURL
                                   withIntermediateDirectories:YES
                                                    attributes:nil
                                                         error:nil];
        } else {
            _databaseURL = [[self class] defaultDatabaseURLIncludingSubdirectory:YES];
        }
        NSString *databasePath = self.databasePath;
        NSAssert(databasePath, @"Offline pack database URL “%@” is not a valid file URL.", _databaseURL);
        
        // Move the offline database from v3.2.0-beta.1 to a location that can
        // also be used for ambient caching.
        if (![[NSFileManager defaultManager] fileExistsAtPath:databasePath]) {
            NSString *legacyDatabasePath = [[self class] legacyDatabasePath];
            [[NSFileManager defaultManager] moveItemAtPath:legacyDatabasePath toPath:databasePath error:NULL];
        }
        
        // Move the offline database from v3.2.x path to a subdirectory that can
        // be reliably excluded from backups.
        if (![[NSFileManager defaultManager] fileExistsAtPath:databasePath]) {
            NSURL *subdirectorylessDatabaseURL = [[self class] defaultDatabaseURLIncludingSubdirectory:NO];
            [[NSFileManager defaultManager] moveItemAtPath:subdirectorylessDatabaseURL.path toPath:databasePath error:NULL];
        }
    }
    return _databaseURL;
}

/**
 Returns the default file URL to the offline pack database, with the option to
 omit the private subdirectory for legacy (v3.2.0–v3.2.3) migration purposes.

 The database is located in a directory specific to the application, so that
 packs downloaded by other applications don’t count toward this application’s
 limits.

 The database is located at:
 ~/Library/Application Support/tld.app.bundle.id/.mapbox/cache.db

 The subdirectory-less database was located at:
 ~/Library/Application Support/tld.app.bundle.id/cache.db
 */
+ (NSURL *)defaultDatabaseURLIncludingSubdirectory:(BOOL)useSubdirectory {
    NSURL *databaseDirectoryURL = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory
                                                                         inDomain:NSUserDomainMask
                                                                appropriateForURL:nil
                                                                           create:YES
                                                                            error:nil];
    NSString *bundleIdentifier = [NSBundle mgl_applicationBundleIdentifier];
    if (!bundleIdentifier) {
        // There’s no main bundle identifier when running in a unit test bundle.
        bundleIdentifier = [[NSUUID UUID] UUIDString];
    }
    databaseDirectoryURL = [databaseDirectoryURL URLByAppendingPathComponent:bundleIdentifier];
    if (useSubdirectory) {
        databaseDirectoryURL = [databaseDirectoryURL URLByAppendingPathComponent:@".mapbox"];
    }
    [[NSFileManager defaultManager] createDirectoryAtURL:databaseDirectoryURL
                             withIntermediateDirectories:YES
                                              attributes:nil
                                                   error:nil];
    if (useSubdirectory) {
        // Avoid backing up the database onto iCloud, because it can be
        // redownloaded. Ideally, we’d even put the ambient cache in Caches, so
        // it can be reclaimed by the system when disk space runs low. But
        // unfortunately it has to live in the same file as offline resources.
        [databaseDirectoryURL setResourceValue:@YES forKey:NSURLIsExcludedFromBackupKey error:NULL];
    }
    return [databaseDirectoryURL URLByAppendingPathComponent:MGLOfflineStorageFileName];
}

/**
 Returns the absolute path to the location where v3.2.0-beta.1 placed the
 offline pack database.
 */
+ (NSString *)legacyDatabasePath {
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
    // ~/Documents/offline.db
    NSArray *legacyPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *legacyDatabasePath = [legacyPaths.firstObject stringByAppendingPathComponent:MGLOfflineStorageFileName3_2_0_beta_1];
#elif TARGET_OS_MAC
    // ~/Library/Caches/tld.app.bundle.id/offline.db
    NSString *bundleIdentifier = [NSBundle mgl_applicationBundleIdentifier];
    NSURL *legacyDatabaseDirectoryURL = [[NSFileManager defaultManager] URLForDirectory:NSCachesDirectory
                                                                               inDomain:NSUserDomainMask
                                                                      appropriateForURL:nil
                                                                                 create:NO
                                                                                  error:nil];
    legacyDatabaseDirectoryURL = [legacyDatabaseDirectoryURL URLByAppendingPathComponent:bundleIdentifier];
    NSURL *legacyDatabaseURL = [legacyDatabaseDirectoryURL URLByAppendingPathComponent:MGLOfflineStorageFileName3_2_0_beta_1];
    NSString *legacyDatabasePath = legacyDatabaseURL ? legacyDatabaseURL.path : @"";
#endif
    return legacyDatabasePath;
}

- (void)addContentsOfFile:(NSString *)filePath withCompletionHandler:(MGLBatchedOfflinePackAdditionCompletionHandler)completion {
    MGLLogDebug(@"Adding contentsOfFile: %@ completionHandler: %@", filePath, completion);
    NSURL *fileURL = [NSURL fileURLWithPath:filePath];
    
    [self addContentsOfURL:fileURL withCompletionHandler:completion];

}

- (void)addContentsOfURL:(NSURL *)fileURL withCompletionHandler:(MGLBatchedOfflinePackAdditionCompletionHandler)completion {
    MGLLogDebug(@"Adding contentsOfURL: %@ completionHandler: %@", fileURL, completion);
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    if (!fileURL.isFileURL) {
        [NSException raise:NSInvalidArgumentException format:@"%@ must be a valid file path", fileURL.absoluteString];
    }
    if (![fileManager isWritableFileAtPath:fileURL.path]) {
        [NSException raise:NSInvalidArgumentException format:@"The file path: %@ must be writable", fileURL.absoluteString];
    }
    
    __weak MGLOfflineStorage *weakSelf = self;
    [self _addContentsOfFile:fileURL.path withCompletionHandler:^(NSArray<MGLOfflinePack *> * _Nullable packs, NSError * _Nullable error) {
        if (packs) {
            NSMutableDictionary *packsByIdentifier = [NSMutableDictionary dictionary];
            
            MGLOfflineStorage *strongSelf = weakSelf;
            for (MGLOfflinePack *pack in packs) {
                [packsByIdentifier setObject:pack forKey:@(pack.mbglOfflineRegion->getID())];
            }
            
            id mutablePacks = [strongSelf mutableArrayValueForKey:@"packs"];
            NSMutableIndexSet *replaceIndexSet = [NSMutableIndexSet indexSet];
            NSMutableArray *replacePacksArray = [NSMutableArray array];
            [strongSelf.packs enumerateObjectsUsingBlock:^(MGLOfflinePack * _Nonnull pack, NSUInteger idx, BOOL * _Nonnull stop) {
                MGLOfflinePack *newPack = packsByIdentifier[@(pack.mbglOfflineRegion->getID())];
                if (newPack) {
                    MGLOfflinePack *previousPack = [mutablePacks objectAtIndex:idx];
                    [previousPack invalidate];
                    [replaceIndexSet addIndex:idx];
                    [replacePacksArray addObject:[packsByIdentifier objectForKey:@(newPack.mbglOfflineRegion->getID())]];
                    [packsByIdentifier removeObjectForKey:@(newPack.mbglOfflineRegion->getID())];
                }

            }];
            
            if (replaceIndexSet.count > 0) {
                [mutablePacks replaceObjectsAtIndexes:replaceIndexSet withObjects:replacePacksArray];
            }
            
            [mutablePacks addObjectsFromArray:packsByIdentifier.allValues];
        }
        if (completion) {
            completion(fileURL, packs, error);
        }
    }];
}

- (void)_addContentsOfFile:(NSString *)filePath withCompletionHandler:(void (^)(NSArray<MGLOfflinePack *> * _Nullable packs, NSError * _Nullable error))completion {
    _mbglDatabaseFileSource->mergeOfflineRegions(std::string(static_cast<const char *>([filePath UTF8String])), [&, completion, filePath](mbgl::expected<mbgl::OfflineRegions, std::exception_ptr> result) {
        NSError *error;
        NSMutableArray *packs;
        if (!result) {
            NSString *description = [NSString stringWithFormat:NSLocalizedStringWithDefaultValue(@"ADD_FILE_CONTENTS_FAILED_DESC", @"Foundation", nil, @"Unable to add offline packs from the file at %@.", @"User-friendly error description"), filePath];
            error = [NSError errorWithDomain:MGLErrorDomain code:MGLErrorCodeModifyingOfflineStorageFailed
                                    userInfo:@{
                                               NSLocalizedDescriptionKey: description,
                                               NSLocalizedFailureReasonErrorKey: @(mbgl::util::toString(result.error()).c_str())
                                               }];
        } else {
            auto& regions = result.value();
            packs = [NSMutableArray arrayWithCapacity:regions.size()];
            for (auto &region : regions) {
                MGLOfflinePack *pack = [[MGLOfflinePack alloc] initWithMBGLRegion:new mbgl::OfflineRegion(std::move(region))];
                [packs addObject:pack];
            }
        }
        if (completion) {
            dispatch_async(dispatch_get_main_queue(), [&, completion, error, packs](void) {
                completion(packs, error);
            });
        }
    });
}

#pragma mark Pack management methods

- (void)addPackForRegion:(id <MGLOfflineRegion>)region withContext:(NSData *)context completionHandler:(MGLOfflinePackAdditionCompletionHandler)completion {
    MGLLogDebug(@"Adding packForRegion: %@ contextLength: %lu completionHandler: %@", region, (unsigned long)context.length, completion);
    __weak MGLOfflineStorage *weakSelf = self;
    [self _addPackForRegion:region withContext:context completionHandler:^(MGLOfflinePack * _Nullable pack, NSError * _Nullable error) {
        pack.state = MGLOfflinePackStateInactive;
        MGLOfflineStorage *strongSelf = weakSelf;
        [[strongSelf mutableArrayValueForKey:@"packs"] addObject:pack];
        if (completion) {
            completion(pack, error);
        }
    }];
}

- (void)_addPackForRegion:(id <MGLOfflineRegion>)region withContext:(NSData *)context completionHandler:(MGLOfflinePackAdditionCompletionHandler)completion {
    if (![region conformsToProtocol:@protocol(MGLOfflineRegion_Private)]) {
        [NSException raise:MGLUnsupportedRegionTypeException
                    format:@"Regions of type %@ are unsupported.", NSStringFromClass([region class])];
        return;
    }

    const mbgl::OfflineRegionDefinition regionDefinition = [(id <MGLOfflineRegion_Private>)region offlineRegionDefinition];
    mbgl::OfflineRegionMetadata metadata(context.length);
    [context getBytes:&metadata[0] length:metadata.size()];
    _mbglDatabaseFileSource->createOfflineRegion(regionDefinition, metadata, [&, completion](mbgl::expected<mbgl::OfflineRegion, std::exception_ptr> mbglOfflineRegion) {
        NSError *error;
        if (!mbglOfflineRegion) {
            NSString *errorDescription = @(mbgl::util::toString(mbglOfflineRegion.error()).c_str());
            error = [NSError errorWithDomain:MGLErrorDomain code:MGLErrorCodeModifyingOfflineStorageFailed userInfo:errorDescription ? @{
                NSLocalizedDescriptionKey: errorDescription,
            } : nil];
        }
        if (completion) {
            MGLOfflinePack *pack = mbglOfflineRegion ? [[MGLOfflinePack alloc] initWithMBGLRegion:new mbgl::OfflineRegion(std::move(mbglOfflineRegion.value()))] : nil;
            dispatch_async(dispatch_get_main_queue(), [&, completion, error, pack](void) {
                completion(pack, error);
            });
        }
    });
}

- (void)removePack:(MGLOfflinePack *)pack withCompletionHandler:(MGLOfflinePackRemovalCompletionHandler)completion {
    MGLLogDebug(@"Removing pack: %@ completionHandler: %@", pack, completion);
    [[self mutableArrayValueForKey:@"packs"] removeObject:pack];
    [self _removePack:pack withCompletionHandler:^(NSError * _Nullable error) {
        if (completion) {
            completion(error);
        }
    }];
}

- (void)_removePack:(MGLOfflinePack *)pack withCompletionHandler:(MGLOfflinePackRemovalCompletionHandler)completion {
    mbgl::OfflineRegion *mbglOfflineRegion = pack.mbglOfflineRegion;

    [pack invalidate];

    if (!mbglOfflineRegion) {
        MGLAssert(pack.state == MGLOfflinePackStateInvalid, @"State should be invalid");
        completion(nil);
        return;
    }
    
    _mbglDatabaseFileSource->deleteOfflineRegion(std::move(*mbglOfflineRegion), [&, completion](std::exception_ptr exception) {
        NSError *error;
        if (exception) {
            error = [NSError errorWithDomain:MGLErrorDomain code:MGLErrorCodeModifyingOfflineStorageFailed userInfo:@{
                NSLocalizedDescriptionKey: @(mbgl::util::toString(exception).c_str()),
            }];
        }
        if (completion) {
            dispatch_async(dispatch_get_main_queue(), [&, completion, error](void) {
                completion(error);
            });
        }
    });

}

- (void)invalidatePack:(MGLOfflinePack *)pack withCompletionHandler:(void (^)(NSError * _Nullable))completion {
    mbgl::OfflineRegion& region = *pack.mbglOfflineRegion;
    NSError *error;
    if (!pack.mbglOfflineRegion) {
        completion(nil);
        return;
    }

    _mbglDatabaseFileSource->invalidateOfflineRegion(region, [&](std::exception_ptr exception) {
        if (exception) {
            error = [NSError errorWithDomain:MGLErrorDomain code:MGLErrorCodeModifyingOfflineStorageFailed userInfo:@{
                NSLocalizedDescriptionKey: @(mbgl::util::toString(exception).c_str()),
            }];
        }
    });
    if (completion) {
        dispatch_async(dispatch_get_main_queue(), [&, completion, error](void) {
            completion(error);
        });
    }
}

- (void)reloadPacks {
    MGLLogInfo(@"Reloading packs.");
    [self getPacksWithCompletionHandler:^(NSArray<MGLOfflinePack *> *packs, __unused NSError * _Nullable error) {
        for (MGLOfflinePack *pack in self.packs) {
            [pack invalidate];
        }
        self.packs = [packs mutableCopy];
    }];
}

- (void)getPacksWithCompletionHandler:(void (^)(NSArray<MGLOfflinePack *> *packs, NSError * _Nullable error))completion {
    _mbglDatabaseFileSource->listOfflineRegions([&, completion](mbgl::expected<mbgl::OfflineRegions, std::exception_ptr> result) {
        NSError *error;
        NSMutableArray *packs;
        if (!result) {
            error = [NSError errorWithDomain:MGLErrorDomain code:MGLErrorCodeUnknown userInfo:@{
                NSLocalizedDescriptionKey: @(mbgl::util::toString(result.error()).c_str()),
            }];
        } else {
            auto& regions = result.value();
            packs = [NSMutableArray arrayWithCapacity:regions.size()];
            for (auto &region : regions) {
                MGLOfflinePack *pack = [[MGLOfflinePack alloc] initWithMBGLRegion:new mbgl::OfflineRegion(std::move(region))];
                [packs addObject:pack];
            }
        }
        if (completion) {
            dispatch_async(dispatch_get_main_queue(), [&, completion, error, packs](void) {
                completion(packs, error);
            });
        }
    });
}

- (void)setMaximumAllowedMapboxTiles:(uint64_t)maximumCount {
    MGLLogDebug(@"Setting maximumAllowedMapboxTiles: %lu", (unsigned long)maximumCount);
    _mbglDatabaseFileSource->setOfflineMapboxTileCountLimit(maximumCount);
}

#pragma mark - Ambient cache management

- (void)setMaximumAmbientCacheSize:(NSUInteger)cacheSize withCompletionHandler:(void (^)(NSError  * _Nullable))completion {
    _mbglDatabaseFileSource->setMaximumAmbientCacheSize(cacheSize, [&, completion](std::exception_ptr exception) {
        NSError *error;
        if (completion) {
            if (exception) {
                error = [NSError errorWithDomain:MGLErrorDomain code:MGLErrorCodeModifyingOfflineStorageFailed userInfo:@{
                    NSLocalizedDescriptionKey: @(mbgl::util::toString(exception).c_str()),
                }];
            }
            dispatch_sync(dispatch_get_main_queue(), ^ {
                completion(error);
            });
        }
    });
}

- (void)invalidateAmbientCacheWithCompletionHandler:(void (^)(NSError *_Nullable))completion {
    _mbglDatabaseFileSource->invalidateAmbientCache([&, completion](std::exception_ptr exception){
        NSError *error;
        if (completion) {
            if (exception) {
                // Convert std::exception_ptr to an NSError.
                error = [NSError errorWithDomain:MGLErrorDomain code:MGLErrorCodeModifyingOfflineStorageFailed userInfo:@{
                    NSLocalizedDescriptionKey: @(mbgl::util::toString(exception).c_str()),
                }];
            }
            dispatch_async(dispatch_get_main_queue(), ^ {
                completion(error);
            });
        }
    });
}

- (void)clearAmbientCacheWithCompletionHandler:(void (^)(NSError *_Nullable error))completion {
    _mbglDatabaseFileSource->clearAmbientCache([&, completion](std::exception_ptr exception){
        NSError *error;
        if (completion) {
            if (exception) {
                error = [NSError errorWithDomain:MGLErrorDomain code:MGLErrorCodeModifyingOfflineStorageFailed userInfo:@{
                    NSLocalizedDescriptionKey: @(mbgl::util::toString(exception).c_str()),
                }];
            }
            dispatch_async(dispatch_get_main_queue(), [&, completion, error](void) {
                completion(error);
            });
        }
    });
}

- (void)resetDatabaseWithCompletionHandler:(void (^)(NSError *_Nullable error))completion {
    _mbglDatabaseFileSource->resetDatabase([&, completion](std::exception_ptr exception) {
        NSError *error;
        if (completion) {
            if (exception) {
                error = [NSError errorWithDomain:MGLErrorDomain code:MGLErrorCodeUnknown userInfo:@{
                    NSLocalizedDescriptionKey: @(mbgl::util::toString(exception).c_str()),
                }];
            }
            dispatch_async(dispatch_get_main_queue(), ^{
                completion(error);
            });
        }
    });
}
#pragma mark -

- (unsigned long long)countOfBytesCompleted {
    NSDictionary *attributes = [[NSFileManager defaultManager] attributesOfItemAtPath:self.databasePath error:NULL];
    return attributes.fileSize;
}

- (void)preloadData:(NSData *)data forURL:(NSURL *)url modificationDate:(nullable NSDate *)modified expirationDate:(nullable NSDate *)expires eTag:(nullable NSString *)eTag mustRevalidate:(BOOL)mustRevalidate {
    [self preloadData:data forURL:url modificationDate:modified expirationDate:expires eTag:eTag mustRevalidate:mustRevalidate completionHandler:nil];
}

- (void)preloadData:(NSData *)data forURL:(NSURL *)url modificationDate:(nullable NSDate *)modified expirationDate:(nullable NSDate *)expires eTag:(nullable NSString *)eTag mustRevalidate:(BOOL)mustRevalidate
    completionHandler:(nullable MGLOfflinePreloadDataCompletionHandler)completion {
    mbgl::Resource resource(mbgl::Resource::Kind::Unknown, url.absoluteString.UTF8String);
    mbgl::Response response;
    response.data = std::make_shared<std::string>(static_cast<const char*>(data.bytes), data.length);
    response.mustRevalidate = mustRevalidate;
    
    if (eTag) {
        response.etag = std::string(eTag.UTF8String);
    }
    
    if (modified) {
        response.modified = mbgl::Timestamp() + std::chrono::duration_cast<mbgl::Seconds>(MGLDurationFromTimeInterval(modified.timeIntervalSince1970));
    }
    
    if (expires) {
        response.expires = mbgl::Timestamp() + std::chrono::duration_cast<mbgl::Seconds>(MGLDurationFromTimeInterval(expires.timeIntervalSince1970));
    }

    std::function<void()> callback;

    if (completion) {
        callback = [completion, url] {
            dispatch_async(dispatch_get_main_queue(), [completion, url] { completion(url, nil); });
        };
    }

    _mbglDatabaseFileSource->forward(resource, response, callback);
}

- (void)putResourceWithUrl:(NSURL *)url data:(NSData *)data modified:(nullable NSDate *)modified expires:(nullable NSDate *)expires etag:(nullable NSString *)etag mustRevalidate:(BOOL)mustRevalidate {
    [self preloadData:data forURL:url modificationDate:modified expirationDate:expires eTag:etag mustRevalidate:mustRevalidate];
}

@end
