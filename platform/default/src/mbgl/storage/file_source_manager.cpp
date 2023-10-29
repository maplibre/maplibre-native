#include <mbgl/storage/asset_file_source.hpp>
#include <mbgl/storage/database_file_source.hpp>
#include <mbgl/storage/file_source_manager.hpp>
#include <mbgl/storage/local_file_source.hpp>
#include <mbgl/storage/main_resource_loader.hpp>
#include <mbgl/storage/online_file_source.hpp>
#include <mbgl/storage/mbtiles_file_source.hpp>
#include <mbgl/storage/resource_options.hpp>

namespace mbgl {

class DefaultFileSourceManagerImpl final : public FileSourceManager {
public:
    DefaultFileSourceManagerImpl() {
        registerFileSourceFactory(FileSourceType::ResourceLoader,
                                  [](const ResourceOptions& resourceOptions, const ClientOptions& clientOptions) {
                                      return std::make_unique<MainResourceLoader>(resourceOptions, clientOptions);
                                  });

        registerFileSourceFactory(FileSourceType::Asset,
                                  [](const ResourceOptions& resourceOptions, const ClientOptions& clientOptions) {
                                      return std::make_unique<AssetFileSource>(resourceOptions, clientOptions);
                                  });

        registerFileSourceFactory(FileSourceType::Database,
                                  [](const ResourceOptions& resourceOptions, const ClientOptions& clientOptions) {
                                      return std::make_unique<DatabaseFileSource>(resourceOptions, clientOptions);
                                  });

        registerFileSourceFactory(FileSourceType::FileSystem,
                                  [](const ResourceOptions& resourceOptions, const ClientOptions& clientOptions) {
                                      return std::make_unique<LocalFileSource>(resourceOptions, clientOptions);
                                  });

        registerFileSourceFactory(FileSourceType::Mbtiles,
                                  [](const ResourceOptions& resourceOptions, const ClientOptions& clientOptions) {
                                      return std::make_unique<MBTilesFileSource>(resourceOptions, clientOptions);
                                  });

        registerFileSourceFactory(FileSourceType::Network,
                                  [](const ResourceOptions& resourceOptions, const ClientOptions& clientOptions) {
                                      return std::make_unique<OnlineFileSource>(resourceOptions, clientOptions);
                                  });
    }
};

FileSourceManager* FileSourceManager::get() noexcept {
    static DefaultFileSourceManagerImpl instance;
    return &instance;
}

} // namespace mbgl
