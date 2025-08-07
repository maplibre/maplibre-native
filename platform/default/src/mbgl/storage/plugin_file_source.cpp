#include <mbgl/platform/settings.hpp>
#include <mbgl/plugin/plugin_file_source.hpp>
#include <mbgl/storage/file_source_request.hpp>
#include <mbgl/storage/local_file_request.hpp>
#include <mbgl/storage/local_file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/util/client_options.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/thread.hpp>
#include <mbgl/util/url.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/platform/settings.hpp>

namespace mbgl {

void PluginFileSource::setProtocolPrefix([[maybe_unused]] const std::string& protocolPrefix) {}

class PluginFileSource::Impl {
public:
    explicit Impl(const ActorRef<Impl>&, const ResourceOptions& resourceOptions_, const ClientOptions& clientOptions_)
        : resourceOptions(resourceOptions_.clone()),
          clientOptions(clientOptions_.clone()) {}

    OnRequestResource _requestFunction;
    void setOnRequestResourceFunction(OnRequestResource requestFunction) { _requestFunction = requestFunction; }

    void request(const Resource& resource, const ActorRef<FileSourceRequest>& req) {
        Response response;
        if (_requestFunction) {
            response = _requestFunction(resource);
        } else {
            response.error = std::make_unique<Response::Error>(
                Response::Error::Reason::Other, std::string("Custom Protocol Handler Not Configured Correctly"));
        }
        req.invoke(&FileSourceRequest::setResponse, response);
    }

    void setResourceOptions(ResourceOptions options) {
        std::lock_guard<std::mutex> lock(resourceOptionsMutex);
        resourceOptions = options;
    }

    ResourceOptions getResourceOptions() {
        std::lock_guard<std::mutex> lock(resourceOptionsMutex);
        return resourceOptions.clone();
    }

    void setClientOptions(ClientOptions options) {
        std::lock_guard<std::mutex> lock(clientOptionsMutex);
        clientOptions = options;
    }

    ClientOptions getClientOptions() {
        std::lock_guard<std::mutex> lock(clientOptionsMutex);
        return clientOptions.clone();
    }

private:
    mutable std::mutex resourceOptionsMutex;
    mutable std::mutex clientOptionsMutex;
    ResourceOptions resourceOptions;
    ClientOptions clientOptions;
};

void PluginFileSource::setOnCanRequestFunction(OnCanRequestResource requestFunction) {
    _onCanRequestResourceFunction = requestFunction;
}

void PluginFileSource::setOnRequestResourceFunction(OnRequestResource requestFunction) {
    impl.get()->actor().invoke(&Impl::setOnRequestResourceFunction, requestFunction);
}

PluginFileSource::PluginFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions)
    : impl(std::make_unique<util::Thread<Impl>>(
          util::makeThreadPrioritySetter(platform::EXPERIMENTAL_THREAD_PRIORITY_FILE),
          "PluginFileSource",
          resourceOptions.clone(),
          clientOptions.clone())) {}

PluginFileSource::~PluginFileSource() = default;

std::unique_ptr<AsyncRequest> PluginFileSource::request(const Resource& resource, Callback callback) {
    auto req = std::make_unique<FileSourceRequest>(std::move(callback));

    impl->actor().invoke(&Impl::request, resource, req->actor());

    return req;
}

bool PluginFileSource::canRequest(const Resource& resource) const {
    if (_onCanRequestResourceFunction) {
        return _onCanRequestResourceFunction(resource);
    }
    return false;
}

void PluginFileSource::pause() {
    impl->pause();
}

void PluginFileSource::resume() {
    impl->resume();
}

void PluginFileSource::setResourceOptions(ResourceOptions options) {
    impl->actor().invoke(&Impl::setResourceOptions, options.clone());
}

ResourceOptions PluginFileSource::getResourceOptions() {
    return impl->actor().ask(&Impl::getResourceOptions).get();
}

void PluginFileSource::setClientOptions(ClientOptions options) {
    impl->actor().invoke(&Impl::setClientOptions, options.clone());
}

ClientOptions PluginFileSource::getClientOptions() {
    return impl->actor().ask(&Impl::getClientOptions).get();
}

} // namespace mbgl
