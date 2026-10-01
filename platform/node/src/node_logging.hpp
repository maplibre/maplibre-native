#pragma once

#include "util/async_queue.hpp"

#include <mln/util/logging.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#include <nan.h>
#pragma GCC diagnostic pop

namespace node_mbgl {

namespace util {
template <typename T>
class AsyncQueue;
}

class NodeLogObserver : public mln::Log::Observer {
public:
    NodeLogObserver(v8::Local<v8::Object> target);
    ~NodeLogObserver() override;

    // Log::Observer implementation
    bool onRecord(mln::EventSeverity severity, mln::Event event, int64_t code, const std::string& text) override;

private:
    Nan::Persistent<v8::Object> module;

    struct LogMessage;
    util::AsyncQueue<LogMessage>* queue;
};

} // namespace node_mbgl
