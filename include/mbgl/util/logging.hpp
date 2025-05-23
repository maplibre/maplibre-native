#pragma once

#include <mbgl/util/event.hpp>

#include <mbgl/util/noncopyable.hpp>

#include <memory>
#include <string>
#include <optional>

namespace mbgl {

class Log {
public:
    class Observer : private util::noncopyable {
    public:
        virtual ~Observer() = default;

        /// When an observer is set, this function will be called for every log
        /// message. Returning true will consume the message.
        virtual bool onRecord(EventSeverity severity, Event event, int64_t code, const std::string& msg) = 0;
    };

    class NullObserver : public Observer {
        bool onRecord(EventSeverity, Event, int64_t, const std::string&) override { return true; }
    };

    static void setObserver(std::unique_ptr<Observer> Observer);
    static std::unique_ptr<Observer> removeObserver();

private:
    template <typename T, size_t N>
    constexpr static bool includes(const T e, const T (&l)[N], const size_t i = 0) {
        return i < N && (l[i] == e || includes(e, l, i + 1));
    }

public:
    Log();
    ~Log();

    /// @brief Determines whether messages of a given severity level are logged asynchronously.
    ///
    /// In a crash or other unexpected termination, pending asynchronous log entries will be lost.
    /// The default is true (asynchronous) for all levels except `Error`.
    static void useLogThread(bool enable, std::optional<EventSeverity> = {});

    template <typename... Args>
    static void Debug(Event event, Args&&... args) noexcept {
        Record(EventSeverity::Debug, event, ::std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void Info(Event event, Args&&... args) noexcept {
        Record(EventSeverity::Info, event, ::std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void Warning(Event event, Args&&... args) noexcept {
        Record(EventSeverity::Warning, event, ::std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void Error(Event event, Args&&... args) noexcept {
        Record(EventSeverity::Error, event, ::std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void Record(EventSeverity severity, Event event, Args&&... args) noexcept {
        if (!includes(severity, disabledEventSeverities) && !includes(event, disabledEvents) &&
            !includes({.severity = severity, .event = event}, disabledEventPermutations)) {
            record(severity, event, ::std::forward<Args>(args)...);
        }
    }

private:
    static Log* get() noexcept;

    static void record(EventSeverity severity, Event event, const std::string& msg) noexcept;
    static void record(EventSeverity severity, Event event, int64_t code, const std::string& msg) noexcept;
    static void record(EventSeverity severity,
                       Event event,
                       int64_t code,
                       const std::string& msg,
                       const std::optional<std::string>& threadName);

    // This method is the data sink that must be implemented by each platform we
    // support. It should ideally output the error message in a human readable
    // format to the developer.
    static void platformRecord(EventSeverity severity, const std::string& msg);
    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace mbgl
