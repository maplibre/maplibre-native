#pragma once

#include <mbgl/storage/file_source.hpp>

namespace mbgl {

class ClientOptions;
class ResourceTransform;
class ResourceOptions;

class MainResourceLoader final : public FileSource {
public:
    explicit MainResourceLoader(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions);
    ~MainResourceLoader() override;

    bool supportsCacheOnlyRequests() const override;
    std::unique_ptr<AsyncRequest> request(const Resource&, Callback) override;
    bool canRequest(const Resource&) const override;
    void pause() override;
    void resume() override;

    void setResourceOptions(ResourceOptions) override;
    ResourceOptions getResourceOptions() override;

    void setClientOptions(ClientOptions) override;
    ClientOptions getClientOptions() override;

private:
    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace mbgl
