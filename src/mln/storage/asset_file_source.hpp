#pragma once

#include <mln/storage/file_source.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/util/client_options.hpp>

namespace mln {

namespace util {
template <typename T>
class Thread;
} // namespace util

class AssetFileSource : public FileSource {
public:
    AssetFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions);
    ~AssetFileSource() override;

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

} // namespace mln
