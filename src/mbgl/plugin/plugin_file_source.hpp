#pragma once

#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/client_options.hpp>

namespace mbgl {

namespace util {
template <typename T>
class Thread;
} // namespace util

class PluginFileSourceRequest {};

class PluginFileSourceResponse {};

class PluginFileSource : public FileSource {
public:
    PluginFileSource(const ResourceOptions &resourceOptions, const ClientOptions &clientOptions);
    ~PluginFileSource() override;

    using OnCanRequestResource = std::function<bool(const Resource &)>;
    using OnRequestResource = std::function<Response(const Resource &)>;

    void setOnCanRequestFunction(OnCanRequestResource requestFunction);
    void setOnRequestResourceFunction(OnRequestResource requestFunction);

    std::unique_ptr<AsyncRequest> request(const Resource &, Callback) override;
    bool canRequest(const Resource &) const override;
    void pause() override;
    void resume() override;

    void setProtocolPrefix(const std::string &);

    void setResourceOptions(ResourceOptions) override;
    ResourceOptions getResourceOptions() override;

    void setClientOptions(ClientOptions) override;
    ClientOptions getClientOptions() override;

private:
    OnCanRequestResource _onCanRequestResourceFunction;
    class Impl;
    std::unique_ptr<util::Thread<Impl>> impl;
};

} // namespace mbgl
