#pragma once

#include <mbgl/storage/file_source.hpp>
#include <mbgl/util/async_request.hpp>
#include <mbgl/test/stub_file_source.hpp>
#include <memory>
#include <string>
#include <optional>

namespace mbgl {

/*
  This is a file source that allows control of when a request is responded to.
*/
class DelayedFileSource : public StubFileSource {
public:
    DelayedFileSource();
    ~DelayedFileSource() override = default;

    std::unique_ptr<AsyncRequest> request(const Resource&, Callback) override;

    // Custom method to respond to pending request
    void respondToRequest(const std::string& data);

private:
    class AsyncRequestImpl : public AsyncRequest {
    public:
        using Callback = std::function<void(const Response&)>;

        AsyncRequestImpl(DelayedFileSource& fs, Callback callback_);
        void respond();

    private:
        DelayedFileSource& fileSource;
        Callback callback;
    };

    friend class AsyncRequestImpl;
    std::optional<Response> response;
    AsyncRequestImpl* pendingRequest = nullptr;
};

} // namespace mbgl
