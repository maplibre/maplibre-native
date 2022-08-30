#pragma once

#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/resource.hpp>

namespace mbgl {

class ClientOptions;
class ResourceOptions;

class HTTPFileSource : public FileSource {
public:
    HTTPFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions);
    ~HTTPFileSource() override;

    std::unique_ptr<AsyncRequest> request(const Resource&, Callback) override;
    bool canRequest(const Resource& resource) const override {
        return resource.hasLoadingMethod(Resource::LoadingMethod::Network);
    }

    void setResourceOptions(ResourceOptions) override;
    ResourceOptions getResourceOptions() override;

    void setClientOptions(ClientOptions) override;
    ClientOptions getClientOptions() override;

    class Impl;

private:
    const std::unique_ptr<Impl> impl;
};

} // namespace mbgl
