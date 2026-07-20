#include <mbgl/storage/http_file_source.hpp>
#include <mbgl/rust/http_response.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/async_request.hpp>
#include <mbgl/util/async_task.hpp>
#include <mbgl/util/client_options.hpp>
#include <mbgl/util/util.hpp>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>

namespace mln::bridge {

using RustHttpRequestFn = bool (*)(const std::string* url, uint8_t kind, mbgl::HttpResponse* out_response);

} // namespace mln::bridge

extern "C" {
MBGL_EXPORT void mbgl_rust_http_set_bridge(mln::bridge::RustHttpRequestFn request_fn);
}

namespace {
std::mutex rust_http_bridge_mutex;
mln::bridge::RustHttpRequestFn rust_http_request = nullptr;
} // namespace

extern "C" MBGL_EXPORT void mbgl_rust_http_set_bridge(mln::bridge::RustHttpRequestFn request_fn) {
    std::scoped_lock lock(rust_http_bridge_mutex);
    rust_http_request = request_fn;
}

namespace mbgl {

class HTTPFileSource::Impl {
public:
    Impl(const ResourceOptions& resourceOptions_, const ClientOptions& clientOptions_)
        : resourceOptions(resourceOptions_.clone()),
          clientOptions(clientOptions_.clone()) {}

    void setResourceOptions(ResourceOptions options) { resourceOptions = options.clone(); }
    ResourceOptions getResourceOptions() { return resourceOptions.clone(); }

    void setClientOptions(ClientOptions options) { clientOptions = options.clone(); }
    ClientOptions getClientOptions() { return clientOptions.clone(); }

private:
    ResourceOptions resourceOptions;
    ClientOptions clientOptions;
};

namespace {
class HTTPNoopRequest final : public AsyncRequest {};

class HTTPRequest final : public AsyncRequest {
public:
    HTTPRequest(Resource resource_,
                FileSource::Callback callback_,
                                mln::bridge::RustHttpRequestFn request_fn_)
        : resource(std::move(resource_)),
          callback(std::move(callback_)),
          request_fn(request_fn_),
          async([this] {
              // Calling callback may destroy this object, so copy first.
              auto callback_ = callback;
              auto response_ = response;
              callback_(response_);
          }) {
        worker = std::thread([this] {
            HttpResponse rust_response(&response);
            const bool handled = request_fn(&resource.url, static_cast<uint8_t>(resource.kind), &rust_response);

            if (cancelled.load()) {
                return;
            }

            if (!handled) {
                response.error = std::make_unique<Response::Error>(Response::Error::Reason::Other,
                                                                   "Rust HTTP bridge did not handle request");
            }

            if (!cancelled.load()) {
                async.send();
            }
        });
    }

    ~HTTPRequest() override {
        cancelled.store(true);
        if (worker.joinable()) {
            worker.join();
        }
    }

private:
    Resource resource;
    FileSource::Callback callback;
    mln::bridge::RustHttpRequestFn request_fn = nullptr;
    Response response;
    std::atomic<bool> cancelled{false};
    util::AsyncTask async;
    std::thread worker;
};
} // namespace

HTTPFileSource::HTTPFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions)
    : impl(std::make_unique<Impl>(resourceOptions, clientOptions)) {}

HTTPFileSource::~HTTPFileSource() = default;

std::unique_ptr<AsyncRequest> HTTPFileSource::request(const Resource& resource, Callback callback) {
    mln::bridge::RustHttpRequestFn request_fn = nullptr;
    {
        std::scoped_lock lock(rust_http_bridge_mutex);
        request_fn = rust_http_request;
    }

    Response response;
    if (!request_fn) {
        response.error =
            std::make_unique<Response::Error>(Response::Error::Reason::Other, "Rust HTTP bridge is not registered");
        callback(response);
        return std::make_unique<HTTPNoopRequest>();
    }

    return std::make_unique<HTTPRequest>(resource, std::move(callback), request_fn);
}

void HTTPFileSource::setResourceOptions(ResourceOptions options) {
    impl->setResourceOptions(options.clone());
}

ResourceOptions HTTPFileSource::getResourceOptions() {
    return impl->getResourceOptions();
}

void HTTPFileSource::setClientOptions(ClientOptions options) {
    impl->setClientOptions(options.clone());
}

ClientOptions HTTPFileSource::getClientOptions() {
    return impl->getClientOptions();
}

} // namespace mbgl
