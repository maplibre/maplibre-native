#pragma once

#include <mln/actor/actor_ref.hpp>
#include <mln/storage/file_source.hpp>
#include <mln/util/async_request.hpp>

#include <memory>
#include <functional>

namespace mln {

class Mailbox;

class FileSourceRequest final : public AsyncRequest {
public:
    FileSourceRequest(FileSource::Callback&& callback);
    ~FileSourceRequest() final;

    void onCancel(std::function<void()>&& callback);
    void setResponse(const Response& res);

    ActorRef<FileSourceRequest> actor();

private:
    FileSource::Callback responseCallback = nullptr;
    std::function<void()> cancelCallback = nullptr;

    std::shared_ptr<Mailbox> mailbox;
};

} // namespace mln
