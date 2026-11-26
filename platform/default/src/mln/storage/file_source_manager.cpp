#include <mln/storage/asset_file_source.hpp>
#include <mln/storage/database_file_source.hpp>
#include <mln/storage/file_source_manager.hpp>
#include <mln/storage/local_file_source.hpp>
#include <mln/storage/main_resource_loader.hpp>
#include <mln/storage/online_file_source.hpp>
#include <mln/storage/mbtiles_file_source.hpp>
#include <mln/storage/pmtiles_file_source.hpp>
#include <mln/storage/resource_options.hpp>

namespace mln {

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

        registerFileSourceFactory(FileSourceType::Pmtiles,
                                  [](const ResourceOptions& resourceOptions, const ClientOptions& clientOptions) {
                                      return std::make_unique<PMTilesFileSource>(resourceOptions, clientOptions);
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

} // namespace mln
