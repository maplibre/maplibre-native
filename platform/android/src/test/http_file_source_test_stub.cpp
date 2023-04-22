#include <mbgl/storage/http_file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/util/async_request.hpp>
#include <mbgl/util/async_task.hpp>

namespace mbgl {

class HTTPFileSource::Impl {};

HTTPFileSource::HTTPFileSource(const ResourceOptions& /*resourceOptions*/, const ClientOptions& /*clientOptions*/)
    : impl(std::make_unique<Impl>()) {
}

HTTPFileSource::~HTTPFileSource() = default;

void HTTPFileSource::setResourceOptions(ResourceOptions /*options*/) {
}

ResourceOptions HTTPFileSource::getResourceOptions() {
    return ResourceOptions::Default();
}

void HTTPFileSource::setClientOptions(ClientOptions /*options*/) {
}

ClientOptions HTTPFileSource::getClientOptions() {
    return ClientOptions();
}

std::unique_ptr<AsyncRequest> HTTPFileSource::request(const Resource&, Callback) {
    return std::make_unique<AsyncRequest>();
}

} // namespace mbgl
