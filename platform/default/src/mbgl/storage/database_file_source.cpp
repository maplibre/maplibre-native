#include <mbgl/platform/settings.hpp>
#include <mbgl/storage/database_file_source.hpp>
#include <mbgl/storage/file_source_manager.hpp>
#include <mbgl/storage/file_source_request.hpp>
#include <mbgl/storage/offline_database.hpp>
#include <mbgl/storage/offline_download.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/util/client_options.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/thread.hpp>

#include <map>
#include <utility>

namespace mbgl {
class DatabaseFileSourceThread {
public:
    DatabaseFileSourceThread(std::shared_ptr<FileSource> onlineFileSource_, const std::string& cachePath)
        : db(std::make_unique<OfflineDatabase>(cachePath, onlineFileSource_->getResourceOptions().tileServerOptions())),
          onlineFileSource(std::move(onlineFileSource_)) {}

    void request(const Resource& resource, const ActorRef<FileSourceRequest>& req) {
        std::optional<Response> offlineResponse = (resource.storagePolicy != Resource::StoragePolicy::Volatile)
                                                      ? db->get(resource)
                                                      : std::nullopt;
        if (!offlineResponse) {
            offlineResponse.emplace();
            offlineResponse->noContent = true;
            offlineResponse->error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound,
                                                                       "Not found in offline database");
        } else if (!offlineResponse->isUsable()) {
            offlineResponse->error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound,
                                                                       "Cached resource is unusable");
        }
        req.invoke(&FileSourceRequest::setResponse, *offlineResponse);
    }

    void setDatabasePath(const std::string& path, std23::move_only_function<void()>&& callback) {
        db->changePath(path);
        if (callback) {
            callback();
        }
    }

    void forward(const Resource& resource, const Response& response, Scheduler::Task&& callback) {
        db->put(resource, response);
        if (callback) {
            callback();
        }
    }

    void resetDatabase(std23::move_only_function<void(std::exception_ptr)>&& callback) {
        callback(db->resetDatabase());
    }

    void packDatabase(std23::move_only_function<void(std::exception_ptr)>&& callback) { callback(db->pack()); }

    void runPackDatabaseAutomatically(bool autopack) { db->runPackDatabaseAutomatically(autopack); }

    void put(const Resource& resource, const Response& response) { db->put(resource, response); }

    void invalidateAmbientCache(std23::move_only_function<void(std::exception_ptr)>&& callback) {
        callback(db->invalidateAmbientCache());
    }

    void clearAmbientCache(std23::move_only_function<void(std::exception_ptr)>&& callback) {
        callback(db->clearAmbientCache());
    }

    void setMaximumAmbientCacheSize(uint64_t size, std23::move_only_function<void(std::exception_ptr)>&& callback) {
        callback(db->setMaximumAmbientCacheSize(size));
    }

    void listRegions(std23::move_only_function<void(expected<OfflineRegions, std::exception_ptr>)>&& callback) {
        callback(db->listRegions());
    }

    void getRegion(
        const int64_t regionID,
        std23::move_only_function<void(expected<std::optional<OfflineRegion>, std::exception_ptr>)>&& callback) {
        callback(db->getRegion(regionID));
    }

    void createRegion(const OfflineRegionDefinition& definition,
                      const OfflineRegionMetadata& metadata,
                      std23::move_only_function<void(expected<OfflineRegion, std::exception_ptr>)>&& callback) {
        callback(db->createRegion(definition, metadata));
    }

    void mergeOfflineRegions(const std::string& sideDatabasePath,
                             std23::move_only_function<void(expected<OfflineRegions, std::exception_ptr>)>&& callback) {
        callback(db->mergeDatabase(sideDatabasePath));
    }

    void updateMetadata(
        const int64_t regionID,
        const OfflineRegionMetadata& metadata,
        std23::move_only_function<void(expected<OfflineRegionMetadata, std::exception_ptr>)>&& callback) {
        callback(db->updateMetadata(regionID, metadata));
    }

    void getRegionStatus(
        int64_t regionID,
        std23::move_only_function<void(expected<OfflineRegionStatus, std::exception_ptr>)>&& callback) {
        if (auto download = getDownload(regionID)) {
            callback(download.value()->getStatus());
        } else {
            callback(unexpected<std::exception_ptr>(download.error()));
        }
    }

    void deleteRegion(OfflineRegion region, std23::move_only_function<void(std::exception_ptr)>&& callback) {
        downloads.erase(region.getID());
        callback(db->deleteRegion(std::move(region)));
    }

    void invalidateRegion(int64_t regionID, std23::move_only_function<void(std::exception_ptr)>&& callback) {
        callback(db->invalidateRegion(regionID));
    }

    void setRegionObserver(int64_t regionID, std::unique_ptr<OfflineRegionObserver> observer) {
        if (auto download = getDownload(regionID)) {
            download.value()->setObserver(std::move(observer));
        }
    }

    void setRegionDownloadState(int64_t regionID, OfflineRegionDownloadState state) {
        if (auto download = getDownload(regionID)) {
            download.value()->setState(state);
        }
    }

    void setOfflineMapboxTileCountLimit(uint64_t limit) { db->setOfflineMapboxTileCountLimit(limit); }

    void reopenDatabaseReadOnly(bool readOnly) { db->reopenDatabaseReadOnly(readOnly); }

private:
    expected<OfflineDownload*, std::exception_ptr> getDownload(int64_t regionID) {
        if (!onlineFileSource) {
            return unexpected<std::exception_ptr>(
                std::make_exception_ptr(std::runtime_error("Network file source unavailable.")));
        }

        auto it = downloads.find(regionID);
        if (it != downloads.end()) {
            return it->second.get();
        }
        auto definition = db->getRegionDefinition(regionID);
        if (!definition) {
            return unexpected<std::exception_ptr>(definition.error());
        }
        auto download = std::make_unique<OfflineDownload>(
            regionID, std::move(definition.value()), *db, *onlineFileSource);
        return downloads.emplace(regionID, std::move(download)).first->second.get();
    }

    std::unique_ptr<OfflineDatabase> db;
    std::map<int64_t, std::unique_ptr<OfflineDownload>> downloads;
    std::shared_ptr<FileSource> onlineFileSource;
};

class DatabaseFileSource::Impl {
public:
    Impl(std::shared_ptr<FileSource> onlineFileSource,
         const ResourceOptions& resourceOptions_,
         const ClientOptions& clientOptions_)
        : thread(std::make_unique<util::Thread<DatabaseFileSourceThread>>(
              util::makeThreadPrioritySetter(platform::EXPERIMENTAL_THREAD_PRIORITY_DATABASE),
              "DatabaseFileSource",
              std::move(onlineFileSource),
              resourceOptions_.cachePath())),
          resourceOptions(resourceOptions_.clone()),
          clientOptions(clientOptions_.clone()) {}

    ActorRef<DatabaseFileSourceThread> actor() const { return thread->actor(); }

    void pause() { thread->pause(); }
    void resume() { thread->resume(); }

    void setResourceOptions(ResourceOptions options) {
        std::lock_guard<std::mutex> lock(resourceOptionsMutex);
        resourceOptions = options;
    }

    ResourceOptions getResourceOptions() {
        std::lock_guard<std::mutex> lock(resourceOptionsMutex);
        return resourceOptions.clone();
    }

    void setClientOptions(ClientOptions options) {
        std::lock_guard<std::mutex> lock(clientOptionsMutex);
        clientOptions = options;
    }

    ClientOptions getClientOptions() {
        std::lock_guard<std::mutex> lock(clientOptionsMutex);
        return clientOptions.clone();
    }

private:
    const std::unique_ptr<util::Thread<DatabaseFileSourceThread>> thread;
    mutable std::mutex resourceOptionsMutex;
    mutable std::mutex clientOptionsMutex;
    ResourceOptions resourceOptions;
    ClientOptions clientOptions;
};

DatabaseFileSource::DatabaseFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions)
    : impl(std::make_unique<Impl>(
          FileSourceManager::get()->getFileSource(FileSourceType::Network, resourceOptions, clientOptions),
          resourceOptions,
          clientOptions)) {}

DatabaseFileSource::~DatabaseFileSource() = default;

std::unique_ptr<AsyncRequest> DatabaseFileSource::request(const Resource& resource,
                                                          std::function<void(Response)> callback) {
    auto req = std::make_unique<FileSourceRequest>(std::move(callback));
    impl->actor().invoke(&DatabaseFileSourceThread::request, resource, req->actor());
    return req;
}

void DatabaseFileSource::forward(const Resource& res, const Response& response, Scheduler::Task&& callback) {
    if (res.storagePolicy == Resource::StoragePolicy::Volatile) return;

    Scheduler::Task wrapper;
    if (callback) {
        wrapper = Scheduler::GetCurrent()->bindOnce(std::move(callback));
    }
    impl->actor().invoke(&DatabaseFileSourceThread::forward, res, response, std::move(wrapper));
}

bool DatabaseFileSource::canRequest(const Resource& resource) const {
    return resource.hasLoadingMethod(Resource::LoadingMethod::Cache) &&
           resource.url.rfind(mbgl::util::ASSET_PROTOCOL, 0) == std::string::npos &&
           resource.url.rfind(mbgl::util::FILE_PROTOCOL, 0) == std::string::npos;
}

void DatabaseFileSource::setDatabasePath(const std::string& path, std23::move_only_function<void()>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::setDatabasePath, path, std::move(callback));
}

void DatabaseFileSource::resetDatabase(std23::move_only_function<void(std::exception_ptr)>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::resetDatabase, std::move(callback));
}

void DatabaseFileSource::packDatabase(std23::move_only_function<void(std::exception_ptr)>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::packDatabase, std::move(callback));
}

void DatabaseFileSource::runPackDatabaseAutomatically(bool autopack) {
    impl->actor().invoke(&DatabaseFileSourceThread::runPackDatabaseAutomatically, autopack);
}

void DatabaseFileSource::put(const Resource& resource, const Response& response) {
    impl->actor().invoke(&DatabaseFileSourceThread::put, resource, response);
}

void DatabaseFileSource::invalidateAmbientCache(std23::move_only_function<void(std::exception_ptr)>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::invalidateAmbientCache, std::move(callback));
}

void DatabaseFileSource::clearAmbientCache(std23::move_only_function<void(std::exception_ptr)>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::clearAmbientCache, std::move(callback));
}

void DatabaseFileSource::setMaximumAmbientCacheSize(uint64_t size,
                                                    std23::move_only_function<void(std::exception_ptr)>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::setMaximumAmbientCacheSize, size, std::move(callback));
}

void DatabaseFileSource::listOfflineRegions(
    std23::move_only_function<void(expected<OfflineRegions, std::exception_ptr>)>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::listRegions, std::move(callback));
}

void DatabaseFileSource::getOfflineRegion(
    const int64_t regionID,
    std23::move_only_function<void(expected<std::optional<OfflineRegion>, std::exception_ptr>)>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::getRegion, regionID, std::move(callback));
}

void DatabaseFileSource::createOfflineRegion(
    const OfflineRegionDefinition& definition,
    const OfflineRegionMetadata& metadata,
    std23::move_only_function<void(expected<OfflineRegion, std::exception_ptr>)>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::createRegion, definition, metadata, std::move(callback));
}

void DatabaseFileSource::mergeOfflineRegions(
    const std::string& sideDatabasePath,
    std23::move_only_function<void(expected<OfflineRegions, std::exception_ptr>)>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::mergeOfflineRegions, sideDatabasePath, std::move(callback));
}

void DatabaseFileSource::updateOfflineMetadata(
    const int64_t regionID,
    const OfflineRegionMetadata& metadata,
    std23::move_only_function<void(expected<OfflineRegionMetadata, std::exception_ptr>)>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::updateMetadata, regionID, metadata, std::move(callback));
}

void DatabaseFileSource::deleteOfflineRegion(const OfflineRegion& region,
                                             std23::move_only_function<void(std::exception_ptr)>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::deleteRegion, region, std::move(callback));
}

void DatabaseFileSource::invalidateOfflineRegion(const OfflineRegion& region,
                                                 std23::move_only_function<void(std::exception_ptr)>&& callback) {
    impl->actor().invoke(&DatabaseFileSourceThread::invalidateRegion, region.getID(), std::move(callback));
}

void DatabaseFileSource::setOfflineRegionObserver(const OfflineRegion& region,
                                                  std::unique_ptr<OfflineRegionObserver>&& observer) {
    impl->actor().invoke(&DatabaseFileSourceThread::setRegionObserver, region.getID(), std::move(observer));
}

void DatabaseFileSource::setOfflineRegionDownloadState(const OfflineRegion& region, OfflineRegionDownloadState state) {
    impl->actor().invoke(&DatabaseFileSourceThread::setRegionDownloadState, region.getID(), state);
}

void DatabaseFileSource::getOfflineRegionStatus(
    const OfflineRegion& region,
    std23::move_only_function<void(expected<OfflineRegionStatus, std::exception_ptr>)>&& callback) const {
    impl->actor().invoke(&DatabaseFileSourceThread::getRegionStatus, region.getID(), std::move(callback));
}

void DatabaseFileSource::setOfflineMapboxTileCountLimit(uint64_t limit) const {
    impl->actor().invoke(&DatabaseFileSourceThread::setOfflineMapboxTileCountLimit, limit);
}

void DatabaseFileSource::setProperty(const std::string& key, const mapbox::base::Value& value) {
    if (key == READ_ONLY_MODE_KEY && value.getBool()) {
        impl->actor().invoke(&DatabaseFileSourceThread::reopenDatabaseReadOnly, *value.getBool());
    } else {
        std::string message = "Resource provider does not support property " + key;
        Log::Error(Event::General, message.c_str());
    }
}

void DatabaseFileSource::pause() {
    impl->pause();
}

void DatabaseFileSource::resume() {
    impl->resume();
}

void DatabaseFileSource::setResourceOptions(ResourceOptions options) {
    impl->setResourceOptions(options.clone());
}

ResourceOptions DatabaseFileSource::getResourceOptions() {
    return impl->getResourceOptions();
}

void DatabaseFileSource::setClientOptions(ClientOptions options) {
    impl->setClientOptions(options.clone());
}

ClientOptions DatabaseFileSource::getClientOptions() {
    return impl->getClientOptions();
}

} // namespace mbgl
