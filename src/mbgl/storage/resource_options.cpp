#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/tile_server_options.hpp>

namespace mbgl {

class ResourceOptions::Impl {
public:
    std::string apiKey;
    TileServerOptions tileServerOptions;
    std::string cachePath = ":memory:";
    std::string assetPath = ".";
    uint64_t maximumSize = mbgl::util::DEFAULT_MAX_CACHE_SIZE;
    void* platformContext = nullptr;
};

// These requires the complete type of Impl.
ResourceOptions::ResourceOptions()
    : impl_(std::make_unique<Impl>()) {}
ResourceOptions::~ResourceOptions() = default;
ResourceOptions::ResourceOptions(ResourceOptions&&) noexcept = default;
ResourceOptions::ResourceOptions(const ResourceOptions& other)
    : impl_(std::make_unique<Impl>(*other.impl_)) {}
ResourceOptions& ResourceOptions::operator=(const ResourceOptions& other) {
    impl_ = std::make_unique<Impl>(*other.impl_);
    return *this;
}
ResourceOptions& ResourceOptions::operator=(ResourceOptions&& options) {
    swap(impl_, options.impl_);
    return *this;
}

ResourceOptions ResourceOptions::clone() const {
    return ResourceOptions(*this);
}

ResourceOptions& ResourceOptions::withApiKey(std::string token) {
    impl_->apiKey = std::move(token);
    return *this;
}

const std::string& ResourceOptions::apiKey() const {
    return impl_->apiKey;
}

ResourceOptions& ResourceOptions::withTileServerOptions(TileServerOptions tileServerOptions) {
    impl_->tileServerOptions = std::move(tileServerOptions);
    return *this;
}

const TileServerOptions ResourceOptions::tileServerOptions() const {
    return impl_->tileServerOptions.clone();
}

ResourceOptions& ResourceOptions::withCachePath(std::string path) {
    impl_->cachePath = std::move(path);
    return *this;
}

const std::string& ResourceOptions::cachePath() const {
    return impl_->cachePath;
}

ResourceOptions& ResourceOptions::withAssetPath(std::string path) {
    impl_->assetPath = std::move(path);
    return *this;
}

const std::string& ResourceOptions::assetPath() const {
    return impl_->assetPath;
}

ResourceOptions& ResourceOptions::withMaximumCacheSize(uint64_t size) {
    impl_->maximumSize = size;
    return *this;
}

uint64_t ResourceOptions::maximumCacheSize() const {
    return impl_->maximumSize;
}

ResourceOptions& ResourceOptions::withPlatformContext(void* context) {
    impl_->platformContext = context;
    return *this;
}

void* ResourceOptions::platformContext() const {
    return impl_->platformContext;
}

ResourceOptions ResourceOptions::Default() {
    ResourceOptions resourceOptions = ResourceOptions().withTileServerOptions(
        TileServerOptions::DefaultConfiguration());
    return resourceOptions;
}

} // namespace mbgl
