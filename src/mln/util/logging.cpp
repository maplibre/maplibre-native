#include <mln/actor/scheduler.hpp>
#include <mln/platform/settings.hpp>
#include <mln/util/enum.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/platform.hpp>
#include <mln/util/traits.hpp>

#include <cstdio>
#include <cstdarg>
#include <exception>
#include <sstream>
#include <mutex>

namespace mln {

namespace {

constexpr auto SeverityCount = underlying_type(EventSeverity::SeverityCount);
std::atomic<bool> useThread[SeverityCount] = {true, true, true, false};

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

    std::unique_ptr<Observer> observer;
    std::mutex mutex;

private:
    const std::shared_ptr<Scheduler> scheduler;
};

Log::Log()
    : impl(std::make_unique<Impl>()) {}

Log::~Log() = default;

Log* Log::get() noexcept {
    // Never destroyed: the worker thread may be attached to a host VM (JNI in
    // MapLibre Android, Java FFM upcalls elsewhere) that is gone by static
    // destruction, and joining it then deadlocks. The worker never drained its
    // queue on termination, so nothing is lost.
    static auto* const instance = new Log();
    // Releasing the observer at exit keeps host cleanup that ran from its
    // destructor; the logger and its thread stay alive.
    static const struct ObserverRelease {
        ~ObserverRelease() { removeObserver(); }
    } observerRelease;
    return instance;
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
    auto& state = *get()->impl;
    std::scoped_lock lock(state.mutex);
    state.observer = std::move(observer);
}

std::unique_ptr<Log::Observer> Log::removeObserver() {
    auto& state = *get()->impl;
    std::scoped_lock lock(state.mutex);
    std::unique_ptr<Observer> observer;
    std::swap(observer, state.observer);
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
    auto& state = *get()->impl;
    std::scoped_lock lock(state.mutex);
    if (state.observer && severity != EventSeverity::Debug && state.observer->onRecord(severity, event, code, msg)) {
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

} // namespace mln
