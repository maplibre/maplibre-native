#include <mbgl/test/delayed_file_source.hpp>

namespace mbgl {

DelayedFileSource::AsyncRequestImpl::AsyncRequestImpl(DelayedFileSource& fs, Callback callback_)
    : fileSource(fs),
      callback(std::move(callback_)) {}

void DelayedFileSource::AsyncRequestImpl::respond() {
    if (callback) {
        callback(*fileSource.response);
    }
}

DelayedFileSource::DelayedFileSource()
    : StubFileSource(StubFileSource::ResponseType::Synchronous) {}

std::unique_ptr<AsyncRequest> DelayedFileSource::request(const Resource&, Callback callback) {
    auto req = std::make_unique<AsyncRequestImpl>(*this, std::move(callback));
    pendingRequest = req.get();
    return req;
}

void DelayedFileSource::respondToRequest(const std::string& data) {
    response = Response();
    response->data = std::make_shared<std::string>(data);
    if (pendingRequest) {
        pendingRequest->respond();
    }
}

} // namespace mbgl
