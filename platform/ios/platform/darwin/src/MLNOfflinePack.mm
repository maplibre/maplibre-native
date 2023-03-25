#import "MLNOfflinePack_Private.h"

#import "MLNOfflineStorage_Private.h"
#import "MLNOfflineRegion_Private.h"
#import "MLNTilePyramidOfflineRegion.h"
#import "MLNTilePyramidOfflineRegion_Private.h"
#import "MLNShapeOfflineRegion.h"
#import "MLNShapeOfflineRegion_Private.h"
#import "MLNLoggingConfiguration_Private.h"

#import "NSValue+MLNAdditions.h"

#include <mbgl/map/map_options.hpp>
#include <mbgl/storage/database_file_source.hpp>

const MLNExceptionName MLNInvalidOfflinePackException = @"MLNInvalidOfflinePackException";

/**
 Assert that the current offline pack is valid.

 This macro should be used at the beginning of any public-facing instance method
 of `MLNOfflinePack`. For private methods, an assertion is more appropriate.
 */
#define MLNAssertOfflinePackIsValid() \
    do { \
        if (_state == MLNOfflinePackStateInvalid) { \
            [NSException raise:MLNInvalidOfflinePackException \
                        format: \
             @"-[MLNOfflineStorage removePack:withCompletionHandler:] has been called " \
             @"on this instance of MLNOfflinePack, rendering it invalid. It is an " \
             @"error to send any message to this pack."]; \
        } \
    } while (NO);

@interface MLNTilePyramidOfflineRegion () <MLNOfflineRegion_Private, MLNTilePyramidOfflineRegion_Private>
@end

@interface MLNShapeOfflineRegion () <MLNOfflineRegion_Private, MLNShapeOfflineRegion_Private>
@end

class MBGLOfflineRegionObserver : public mbgl::OfflineRegionObserver {
public:
    MBGLOfflineRegionObserver(MLNOfflinePack *pack_) : pack(pack_) {}

    void statusChanged(mbgl::OfflineRegionStatus status) override;
    void responseError(mbgl::Response::Error error) override;
    void mapboxTileCountLimitExceeded(uint64_t limit) override;

private:
    __weak MLNOfflinePack *pack = nullptr;
};

@interface MLNOfflinePack ()

@property (nonatomic, nullable, readwrite) mbgl::OfflineRegion *mbglOfflineRegion;
@property (nonatomic, readwrite) MLNOfflinePackProgress progress;

@end

@implementation MLNOfflinePack {
    BOOL _isSuspending;
    std::shared_ptr<mbgl::DatabaseFileSource> _mbglDatabaseFileSource;
}

- (instancetype)init {
    MLNLogInfo(@"Calling this initializer is not allowed.");
    if (self = [super init]) {
        _state = MLNOfflinePackStateInvalid;
        NSLog(@"%s called; did you mean to call +[MLNOfflineStorage addPackForRegion:withContext:completionHandler:] instead?", __PRETTY_FUNCTION__);
    }
    return self;
}

- (instancetype)initWithMBGLRegion:(mbgl::OfflineRegion *)region {
    if (self = [super init]) {
        _mbglOfflineRegion = region;
        _state = MLNOfflinePackStateUnknown;

        _mbglDatabaseFileSource = [[MLNOfflineStorage sharedOfflineStorage] mbglDatabaseFileSource];
        _mbglDatabaseFileSource->setOfflineRegionObserver(*_mbglOfflineRegion, std::make_unique<MBGLOfflineRegionObserver>(self));
    }
    return self;
}

- (void)dealloc {
    MLNAssert(_state == MLNOfflinePackStateInvalid, @"MLNOfflinePack was not invalided prior to deallocation.");
}

- (id <MLNOfflineRegion>)region {
    MLNAssertOfflinePackIsValid();

    const mbgl::OfflineRegionDefinition &regionDefinition = _mbglOfflineRegion->getDefinition();
    MLNAssert([MLNTilePyramidOfflineRegion conformsToProtocol:@protocol(MLNOfflineRegion_Private)], @"MLNTilePyramidOfflineRegion should conform to MLNOfflineRegion_Private.");
    MLNAssert([MLNShapeOfflineRegion conformsToProtocol:@protocol(MLNOfflineRegion_Private)], @"MLNShapeOfflineRegion should conform to MLNOfflineRegion_Private.");
    
    
    
    return regionDefinition.match(
                           [&] (const mbgl::OfflineTilePyramidRegionDefinition def){
                               return (id <MLNOfflineRegion>)[[MLNTilePyramidOfflineRegion alloc] initWithOfflineRegionDefinition:def];
                           },
                           [&] (const mbgl::OfflineGeometryRegionDefinition& def){
                               return (id <MLNOfflineRegion>)[[MLNShapeOfflineRegion alloc] initWithOfflineRegionDefinition:def];
                           });
}

- (NSData *)context {
    MLNAssertOfflinePackIsValid();

    const mbgl::OfflineRegionMetadata &metadata = _mbglOfflineRegion->getMetadata();
    return [NSData dataWithBytes:&metadata[0] length:metadata.size()];
}

- (void)setContext:(NSData *)context completionHandler:(void (^_Nullable)(NSError * _Nullable error))completion {
    MLNAssertOfflinePackIsValid();
    
    mbgl::OfflineRegionMetadata metadata(context.length);
    [context getBytes:&metadata[0] length:metadata.size()];
    
    [self willChangeValueForKey:@"context"];
    __weak MLNOfflinePack *weakSelf = self;
    _mbglDatabaseFileSource->updateOfflineMetadata(_mbglOfflineRegion->getID(), metadata, [&, completion, weakSelf](mbgl::expected<mbgl::OfflineRegionMetadata, std::exception_ptr> mbglOfflineRegionMetadata) {
        NSError *error;
        if (!mbglOfflineRegionMetadata) {
            NSString *errorDescription = @(mbgl::util::toString(mbglOfflineRegionMetadata.error()).c_str());
            error = [NSError errorWithDomain:MLNErrorDomain code:MLNErrorCodeModifyingOfflineStorageFailed userInfo:errorDescription ? @{
                NSLocalizedDescriptionKey: errorDescription,
            } : nil];
        }
        dispatch_async(dispatch_get_main_queue(), [&, completion, weakSelf, error](void) {
            [weakSelf reloadWithCompletionHandler:^(NSError * _Nullable reloadingError) {
                MLNOfflinePack *strongSelf = weakSelf;
                [strongSelf didChangeValueForKey:@"context"];
                if (completion) {
                    completion(error ?: reloadingError);
                }
            }];
        });
    });
}

- (void)reloadWithCompletionHandler:(void (^)(NSError * _Nullable error))completion {
    auto regionID = _mbglOfflineRegion->getID();
    MLNOfflineStorage *sharedOfflineStorage = [MLNOfflineStorage sharedOfflineStorage];
    __weak MLNOfflinePack *weakSelf = self;
    [sharedOfflineStorage getPacksWithCompletionHandler:^(NSArray<MLNOfflinePack *> *packs, __unused NSError * _Nullable error) {
        MLNOfflinePack *strongSelf = weakSelf;
        for (MLNOfflinePack *pack in packs) {
            if (pack.mbglOfflineRegion->getID() == regionID) {
                strongSelf.mbglOfflineRegion = pack.mbglOfflineRegion;
            }
            [pack invalidate];
        }
        completion(error);
    }];
}

- (void)resume {
    MLNLogInfo(@"Resuming pack download.");
    MLNAssertOfflinePackIsValid();

    self.state = MLNOfflinePackStateActive;

    _mbglDatabaseFileSource->setOfflineRegionDownloadState(*_mbglOfflineRegion, mbgl::OfflineRegionDownloadState::Active);
}

- (void)suspend {
    MLNLogInfo(@"Suspending pack download.");
    MLNAssertOfflinePackIsValid();

    if (self.state == MLNOfflinePackStateActive) {
        self.state = MLNOfflinePackStateInactive;
        _isSuspending = YES;
    }

    _mbglDatabaseFileSource->setOfflineRegionDownloadState(*_mbglOfflineRegion, mbgl::OfflineRegionDownloadState::Inactive);
}

- (void)invalidate {
    MLNLogInfo(@"Invalidating pack.");
    MLNAssert(_state != MLNOfflinePackStateInvalid, @"Cannot invalidate an already invalid offline pack.");
    MLNAssert(self.mbglOfflineRegion, @"Should have a valid region");

    @synchronized (self) {
        self.state = MLNOfflinePackStateInvalid;
        if (self.mbglOfflineRegion) {
            _mbglDatabaseFileSource->setOfflineRegionObserver(*self.mbglOfflineRegion, nullptr);
        }
        self.mbglOfflineRegion = nil;
    }
}

- (void)setState:(MLNOfflinePackState)state {
    MLNLogDebug(@"Setting state: %ld", (long)state);
    if (!self.mbglOfflineRegion) {
        // A progress update has arrived after the call to
        // -[MLNOfflineStorage removePack:withCompletionHandler:] but before the
        // removal is complete and the completion handler is called.
        MLNAssert(_state == MLNOfflinePackStateInvalid, @"A valid MLNOfflinePack has no mbgl::OfflineRegion.");
        return;
    }

    MLNAssert(_state != MLNOfflinePackStateInvalid, @"Cannot change the state of an invalid offline pack.");

    if (!_isSuspending || state != MLNOfflinePackStateActive) {
        _isSuspending = NO;
        _state = state;
    }
}

- (void)requestProgress {
    MLNLogInfo(@"Requesting pack progress.");
    MLNAssertOfflinePackIsValid();

    __weak MLNOfflinePack *weakSelf = self;
    _mbglDatabaseFileSource->getOfflineRegionStatus(*_mbglOfflineRegion, [&, weakSelf](mbgl::expected<mbgl::OfflineRegionStatus, std::exception_ptr> status) {
        if (status) {
            mbgl::OfflineRegionStatus checkedStatus = *status;
            dispatch_async(dispatch_get_main_queue(), ^{
                MLNOfflinePack *strongSelf = weakSelf;
                [strongSelf offlineRegionStatusDidChange:checkedStatus];
            });
        }
    });
}

- (void)offlineRegionStatusDidChange:(mbgl::OfflineRegionStatus)status {
    MLNAssert(_state != MLNOfflinePackStateInvalid, @"Cannot change update progress of an invalid offline pack.");

    switch (status.downloadState) {
        case mbgl::OfflineRegionDownloadState::Inactive:
            self.state = status.complete() ? MLNOfflinePackStateComplete : MLNOfflinePackStateInactive;
            break;

        case mbgl::OfflineRegionDownloadState::Active:
            self.state = MLNOfflinePackStateActive;
            break;
    }

    if (_isSuspending) {
        return;
    }

    MLNOfflinePackProgress progress;
    progress.countOfResourcesCompleted = status.completedResourceCount;
    progress.countOfBytesCompleted = status.completedResourceSize;
    progress.countOfTilesCompleted = status.completedTileCount;
    progress.countOfTileBytesCompleted = status.completedTileSize;
    progress.countOfResourcesExpected = status.requiredResourceCount;
    progress.maximumResourcesExpected = status.requiredResourceCountIsPrecise ? status.requiredResourceCount : UINT64_MAX;
    self.progress = progress;

    NSDictionary *userInfo = @{MLNOfflinePackUserInfoKeyState: @(self.state),
                               MLNOfflinePackUserInfoKeyProgress: [NSValue valueWithMLNOfflinePackProgress:progress]};

    NSNotificationCenter *noteCenter = [NSNotificationCenter defaultCenter];
    [noteCenter postNotificationName:MLNOfflinePackProgressChangedNotification
                              object:self
                            userInfo:userInfo];
}

- (void)didReceiveError:(NSError *)error {
    MLNLogError(@"Error: %@", error.localizedDescription);
    MLNLogInfo(@"Notifying about pack error.");
    
    NSDictionary *userInfo = @{ MLNOfflinePackUserInfoKeyError: error };
    NSNotificationCenter *noteCenter = [NSNotificationCenter defaultCenter];
    [noteCenter postNotificationName:MLNOfflinePackErrorNotification
                              object:self
                            userInfo:userInfo];
}

- (void)didReceiveMaximumAllowedMapboxTiles:(uint64_t)limit {
    MLNLogInfo(@"Notifying reached maximum allowed Mapbox tiles: %lu", (unsigned long)limit);
    NSDictionary *userInfo = @{ MLNOfflinePackUserInfoKeyMaximumCount: @(limit) };
    NSNotificationCenter *noteCenter = [NSNotificationCenter defaultCenter];
    [noteCenter postNotificationName:MLNOfflinePackMaximumMapboxTilesReachedNotification
                              object:self
                            userInfo:userInfo];
}

NSError *MLNErrorFromResponseError(mbgl::Response::Error error) {
    NSInteger errorCode = MLNErrorCodeUnknown;
    switch (error.reason) {
        case mbgl::Response::Error::Reason::NotFound:
            errorCode = MLNErrorCodeNotFound;
            break;

        case mbgl::Response::Error::Reason::Server:
            errorCode = MLNErrorCodeBadServerResponse;
            break;

        case mbgl::Response::Error::Reason::Connection:
            errorCode = MLNErrorCodeConnectionFailed;
            break;

        default:
            break;
    }
    return [NSError errorWithDomain:MLNErrorDomain code:errorCode userInfo:@{
        NSLocalizedFailureReasonErrorKey: @(error.message.c_str())
    }];
}

@end

void MBGLOfflineRegionObserver::statusChanged(mbgl::OfflineRegionStatus status) {
    __weak MLNOfflinePack *weakPack = pack;
    dispatch_async(dispatch_get_main_queue(), ^{
        [weakPack offlineRegionStatusDidChange:status];
    });
}

void MBGLOfflineRegionObserver::responseError(mbgl::Response::Error error) {
    __weak MLNOfflinePack *weakPack = pack;
    dispatch_async(dispatch_get_main_queue(), ^{
        [weakPack didReceiveError:MLNErrorFromResponseError(error)];
    });
}

void MBGLOfflineRegionObserver::mapboxTileCountLimitExceeded(uint64_t limit) {
    __weak MLNOfflinePack *weakPack = pack;
    dispatch_async(dispatch_get_main_queue(), ^{
        [weakPack didReceiveMaximumAllowedMapboxTiles:limit];
    });
}
