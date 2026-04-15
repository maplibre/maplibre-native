#include <mbgl/actor/scheduler.hpp>
#include <mbgl/platform/settings.hpp>
#include <mbgl/util/enum.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/traits.hpp>

#include <cstdio>
#include <cstdarg>
#include <exception>
#include <sstream>
#include <mutex>

namespace mbgl {

namespace {

std::unique_ptr<Log::Observer> currentObserver;
constexpr auto SeverityCount = underlying_type(EventSeverity::SeverityCount);
std::atomic<bool> useThread[SeverityCount] = {true, true, true, false};
std::mutex mutex;

} // namespace

class Log::Impl {
public:
    Impl()
        : scheduler(Scheduler::GetSequenced()) {}

    void record(EventSeverity severity, Event event, int64_t code, const std::string& msg) try {
        if (useThread[underlying_type(severity)]) {
            auto threadName = platform::getCurrentThreadName();
            scheduler->schedule([=]() { Log::record(severity, event, code, msg, threadName); });
        } else {
            Log::record(severity, event, code, msg, {});
        }
    } catch (...) { // NOLINT(bugprone-empty-catch)
                    // ignore exceptions during logging
                    // What would we do, log them?
#if !defined(NDEBUG)
        [[maybe_unused]] auto ex = std::current_exception();
        assert(!"unhandled exception while logging");
#endif
    }

private:
    const std::shared_ptr<Scheduler> scheduler;
};

Log::Log()
    : impl(std::make_unique<Impl>()) {}

Log::~Log() = default;

Log* Log::get() noexcept {
    static Log instance;
    return &instance;
}

void Log::useLogThread(bool enable, std::optional<EventSeverity> severity) {
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
    std::scoped_lock lock(mutex);
    currentObserver = std::move(observer);
}

std::unique_ptr<Log::Observer> Log::removeObserver() {
    std::scoped_lock lock(mutex);
    std::unique_ptr<Observer> observer;
    std::swap(observer, currentObserver);
    return observer;
}

void Log::record(EventSeverity severity, Event event, const std::string& msg) noexcept {
    get()->impl->record(severity, event, -1, msg);
}

void Log::record(EventSeverity severity, Event event, int64_t code, const std::string& msg) noexcept {
    get()->impl->record(severity, event, code, msg);
}

void Log::record(EventSeverity severity,
                 Event event,
                 int64_t code,
                 const std::string& msg,
                 const std::optional<std::string>& threadName) {
    std::scoped_lock lock(mutex);
    if (currentObserver && severity != EventSeverity::Debug && currentObserver->onRecord(severity, event, code, msg)) {
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
