#include <mbgl/platform/settings.hpp>
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

namespace {
bool acceptsURL(const std::string& url) {
    return url.starts_with(mbgl::util::FILE_PROTOCOL);
}
} // namespace

namespace mbgl {

class LocalFileSource::Impl {
public:
    explicit Impl(const ActorRef<Impl>&, const ResourceOptions& resourceOptions_, const ClientOptions& clientOptions_)
        : resourceOptions(resourceOptions_.clone()),
          clientOptions(clientOptions_.clone()) {}

    void request(const Resource& resource, const ActorRef<FileSourceRequest>& req) {
        if (!acceptsURL(resource.url)) {
            Response response;
            response.error = std::make_unique<Response::Error>(Response::Error::Reason::Other, "Invalid file URL");
            req.invoke(&FileSourceRequest::setResponse, response);
            return;
        }

        // Cut off the protocol and prefix with path.
        const auto path = mbgl::util::percentDecode(
            resource.url.substr(std::char_traits<char>::length(util::FILE_PROTOCOL)));
        requestLocalFile(path, req, resource.dataRange);
    }

    void setResourceOptions(ResourceOptions options) {
        std::scoped_lock lock(resourceOptionsMutex);
        resourceOptions = options;
    }

    ResourceOptions getResourceOptions() {
        std::scoped_lock lock(resourceOptionsMutex);
        return resourceOptions.clone();
    }

    void setClientOptions(ClientOptions options) {
        std::scoped_lock lock(clientOptionsMutex);
        clientOptions = options;
    }

    ClientOptions getClientOptions() {
        std::scoped_lock lock(clientOptionsMutex);
        return clientOptions.clone();
    }

private:
    mutable std::mutex resourceOptionsMutex;
    mutable std::mutex clientOptionsMutex;
    ResourceOptions resourceOptions;
    ClientOptions clientOptions;
};

LocalFileSource::LocalFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions)
    : impl(std::make_unique<util::Thread<Impl>>(
          util::makeThreadPrioritySetter(platform::EXPERIMENTAL_THREAD_PRIORITY_FILE),
          "LocalFileSource",
          resourceOptions.clone(),
          clientOptions.clone())) {}

LocalFileSource::~LocalFileSource() = default;

std::unique_ptr<AsyncRequest> LocalFileSource::request(const Resource& resource, Callback callback) {
    auto req = std::make_unique<FileSourceRequest>(std::move(callback));

    impl->actor().invoke(&Impl::request, resource, req->actor());

    return req;
}

bool LocalFileSource::canRequest(const Resource& resource) const {
    return acceptsURL(resource.url);
}

void LocalFileSource::pause() {
    impl->pause();
}

void LocalFileSource::resume() {
    impl->resume();
}

void LocalFileSource::setResourceOptions(ResourceOptions options) {
    impl->actor().invoke(&Impl::setResourceOptions, options.clone());
}

ResourceOptions LocalFileSource::getResourceOptions() {
    return impl->actor().ask(&Impl::getResourceOptions).get();
}

void LocalFileSource::setClientOptions(ClientOptions options) {
    impl->actor().invoke(&Impl::setClientOptions, options.clone());
}

ClientOptions LocalFileSource::getClientOptions() {
    return impl->actor().ask(&Impl::getClientOptions).get();
}

} // namespace mbgl
