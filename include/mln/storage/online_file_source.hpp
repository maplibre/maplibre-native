#pragma once

#include <mln/storage/file_source.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/util/client_options.hpp>

#include <optional>

namespace mln {

class OnlineFileSource : public FileSource {
public:
    OnlineFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions);
    ~OnlineFileSource() override;
    void setResourceOptions(ResourceOptions) override;
    ResourceOptions getResourceOptions() override;
    void setClientOptions(ClientOptions) override;
    ClientOptions getClientOptions() override;

private:
    // FileSource overrides
    std::unique_ptr<AsyncRequest> request(const Resource&, Callback) override;
    bool canRequest(const Resource&) const override;
    void pause() override;
    void resume() override;
    void setProperty(const std::string&, const mapbox::base::Value&) override;
    mapbox::base::Value getProperty(const std::string&) const override;
    void setResourceTransform(ResourceTransform) override;

    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace mln
