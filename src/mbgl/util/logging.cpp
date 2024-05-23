#include <mbgl/actor/scheduler.hpp>
#include <mbgl/platform/settings.hpp>
#include <mbgl/util/enum.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/traits.hpp>

#include <cstdio>
#include <cstdarg>
#include <sstream>

namespace mbgl {

namespace {

std::unique_ptr<Log::Observer> currentObserver;
std::atomic<bool> useThread[underlying_type(EventSeverity::SeverityCount)] = {
        true,
        true,
        true,
        false
};
std::mutex mutex;

} // namespace

class Log::Impl {
public:
    Impl() : scheduler(Scheduler::GetSequenced()) {}

    void record(EventSeverity severity, Event event, int64_t code, const std::string& msg) {
        if (useThread[underlying_type(severity)]) {
            auto threadName = platform::getCurrentThreadName();
            scheduler->schedule([=]() { Log::record(severity, event, code, msg, threadName); });
        } else {
            Log::record(severity, event, code, msg, {});
        }
    }

private:
    const std::shared_ptr<Scheduler> scheduler;
};

Log::Log() : impl(std::make_unique<Impl>()) {}

Log::~Log() = default;

Log* Log::get() noexcept {
    static Log instance;
    return &instance;
}

void Log::useLogThread(bool enable, optional<EventSeverity> severity) {
    if (severity) {
        useThread[underlying_type(*severity)] = enable;
    } else {
        useLogThread(enable, EventSeverity::Debug);
        useLogThread(enable, EventSeverity::Info);
        useLogThread(enable, EventSeverity::Warning);
        useLogThread(enable, EventSeverity::Error);
    }
}

void Log::setObserver(std::unique_ptr<Observer> observer) {
    std::lock_guard<std::mutex> lock(mutex);
    currentObserver = std::move(observer);
}

std::unique_ptr<Log::Observer> Log::removeObserver() {
    std::lock_guard<std::mutex> lock(mutex);
    std::unique_ptr<Observer> observer;
    std::swap(observer, currentObserver);
    return observer;
}

void Log::record(EventSeverity severity, Event event, const std::string &msg) {
    get()->impl->record(severity, event, -1, msg);
}

void Log::record(EventSeverity severity, Event event, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char msg[4096];
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);

    get()->impl->record(severity, event, -1, std::string{msg});
}

void Log::record(EventSeverity severity, Event event, int64_t code, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char msg[4096];
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);

    get()->impl->record(severity, event, code, std::string{msg});
}

void Log::record(EventSeverity severity,
                 Event event,
                 int64_t code,
                 const std::string& msg,
                 const optional<std::string>& threadName) {
    std::lock_guard<std::mutex> lock(mutex);
    if (currentObserver && severity != EventSeverity::Debug &&
        currentObserver->onRecord(severity, event, code, msg)) {
        return;
    }

    std::stringstream logStream;

    logStream << "{" << threadName.value_or(platform::getCurrentThreadName()) << "}";
    logStream << "[" << Enum<Event>::toString(event) << "]";

    if (code >= 0) {
        logStream << "(" << code << ")";
    }

    if (!msg.empty()) {
        logStream << ": " << msg;
    }

    platformRecord(severity, logStream.str());
}

} // namespace mbgl
