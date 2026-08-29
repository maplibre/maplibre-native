#include "node_logging.hpp"

#include <mln/util/enum.hpp>

namespace node_mbgl {

struct NodeLogObserver::LogMessage {
    mln::EventSeverity severity;
    mln::Event event;
    int64_t code;
    std::string text;

    LogMessage(mln::EventSeverity severity_, mln::Event event_, int64_t code_, std::string text_)
        : severity(severity_),
          event(event_),
          code(code_),
          text(std::move(text_)) {}
};

NodeLogObserver::NodeLogObserver(v8::Local<v8::Object> target)
    : queue(new util::AsyncQueue<LogMessage>(uv_default_loop(), [this](LogMessage &message) {
          Nan::HandleScope scope;

          auto msg = Nan::New<v8::Object>();

          Nan::Set(msg,
                   Nan::New("class").ToLocalChecked(),
                   Nan::New(mln::Enum<mln::Event>::toString(message.event)).ToLocalChecked());

          Nan::Set(msg,
                   Nan::New("severity").ToLocalChecked(),
                   Nan::New(mln::Enum<mln::EventSeverity>::toString(message.severity)).ToLocalChecked());

          if (message.code != -1) {
              Nan::Set(msg, Nan::New("code").ToLocalChecked(), Nan::New<v8::Number>(message.code));
          }

          if (!message.text.empty()) {
              Nan::Set(msg, Nan::New("text").ToLocalChecked(), Nan::New(message.text).ToLocalChecked());
          }

          v8::Local<v8::Value> argv[] = {Nan::New("message").ToLocalChecked(), msg};
          auto handle = Nan::New<v8::Object>(module);
          auto emit = Nan::To<v8::Object>(Nan::Get(handle, Nan::New("emit").ToLocalChecked()).ToLocalChecked())
                          .ToLocalChecked();
          Nan::CallAsFunction(emit, handle, 2, argv);
      })) {
    Nan::HandleScope scope;
    module.Reset(target);

    // Don't keep the event loop alive.
    queue->unref();
}

NodeLogObserver::~NodeLogObserver() {
    queue->stop();
}

bool NodeLogObserver::onRecord(mln::EventSeverity severity, mln::Event event, int64_t code, const std::string &text) {
    queue->send({severity, event, code, text});
    return true;
}

} // namespace node_mbgl
