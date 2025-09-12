#include <mbgl/storage/file_source_request.hpp>

#include <mbgl/actor/mailbox.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/logging.hpp>
#include <thread>

namespace mbgl {

FileSourceRequest::FileSourceRequest(FileSource::Callback&& callback)
    : responseCallback(callback),
      mailbox(std::make_shared<Mailbox>(*Scheduler::GetCurrent())) {}

FileSourceRequest::~FileSourceRequest() {
    if (cancelCallback) {
        cancelCallback();
    }

    mailbox->close();
}

void FileSourceRequest::onCancel(std::function<void()>&& callback) {
    cancelCallback = std::move(callback);
}

void FileSourceRequest::setResponse(const Response& response) {
    mbgl::Log::Info(mbgl::Event::General, "FileSourceRequest::setResponse called, has callback: " + 
                    std::to_string(responseCallback != nullptr) + ", has data: " + 
                    std::to_string(response.data != nullptr));
    // Copy, because calling the callback will sometimes self
    // destroy this object. We cannot move because this method
    // can be called more than once.
    auto callback = responseCallback;
    if (callback) {
        mbgl::Log::Info(mbgl::Event::General, "FileSourceRequest::setResponse invoking callback on thread: " + 
                       std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())));
        callback(response);
        mbgl::Log::Info(mbgl::Event::General, "FileSourceRequest::setResponse callback returned");
    } else {
        mbgl::Log::Warning(mbgl::Event::General, "FileSourceRequest::setResponse no callback!");
    }
}

ActorRef<FileSourceRequest> FileSourceRequest::actor() {
    return ActorRef<FileSourceRequest>(*this, mailbox);
}

} // namespace mbgl
