#include <mbgl/storage/file_source_request.hpp>
#include <mbgl/storage/pmtiles_file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/util/async_request.hpp>

namespace mbgl {

class PMTilesFileSource::Impl {
public:
    Impl() = default;
    ~Impl() = default;
};

PMTilesFileSource::PMTilesFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions) {}

std::unique_ptr<AsyncRequest> PMTilesFileSource::request(const Resource& resource, FileSource::Callback callback) {
    return nullptr;
}

bool PMTilesFileSource::canRequest(const Resource& resource) const {
    return false;
}

PMTilesFileSource::~PMTilesFileSource() = default;

void PMTilesFileSource::setResourceOptions(ResourceOptions options) {}

ResourceOptions PMTilesFileSource::getResourceOptions() {
    return {};
}

void PMTilesFileSource::setClientOptions(ClientOptions options) {}

ClientOptions PMTilesFileSource::getClientOptions() {
    return {};
}

} // namespace mbgl
