#pragma once

#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/client_options.hpp>

namespace mbgl {

namespace util {
template <typename T>
class Thread;
} // namespace util

class LocalFileSource : public FileSource {
public:
    LocalFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions);
    ~LocalFileSource() override;

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
    std::unique_ptr<util::Thread<Impl>> impl;
};

} // namespace mbgl
