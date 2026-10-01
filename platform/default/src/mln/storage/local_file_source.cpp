#include <mln/platform/settings.hpp>
#include <mln/storage/file_source_request.hpp>
#include <mln/storage/local_file_request.hpp>
#include <mln/storage/local_file_source.hpp>
#include <mln/storage/resource.hpp>
#include <mln/storage/response.hpp>
#include <mln/util/client_options.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/string.hpp>
#include <mln/util/thread.hpp>
#include <mln/util/url.hpp>
#include <mln/storage/resource_options.hpp>

namespace {
bool acceptsURL(const std::string& url) {
    return url.starts_with(mln::util::FILE_PROTOCOL);
}
} // namespace

namespace mln {

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
        const auto path = mln::util::percentDecode(
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

} // namespace mln
