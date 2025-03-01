#pragma once

#include <mbgl/util/event.hpp>
#include <mbgl/util/chrono.hpp>

#include <string>

#ifdef MBGL_TIMING
#include <sstream>
#endif

namespace mbgl {
namespace util {

#ifdef MBGL_TIMING
// Declare 'watch' as a shared_ptr so it can be captured by value in a lambda function
#define MBGL_TIMING_START(watch) \
    std::shared_ptr<util::stopwatch> watch = std::make_unique<util::stopwatch>(Event::Timing);
#define MBGL_TIMING_FINISH(watch, message)  \
    do {                                    \
        std::stringstream messageStream;    \
        messageStream << message;           \
        watch->report(messageStream.str()); \
    } while (0)
#else
#define MBGL_TIMING_START(watch) ((void)0)
#define MBGL_TIMING_FINISH(watch, message) ((void)0)
#endif

#ifndef DISABLE_STOPWATCH
class stopwatch {
public:
    stopwatch(Event event = Event::General);
    stopwatch(EventSeverity severity, Event event = Event::General);
    stopwatch(std::string name, Event event = Event::General);
    stopwatch(std::string name, EventSeverity severity, Event event = Event::General);
    void report(const std::string &name);
    ~stopwatch();

private:
    const std::string name;
    EventSeverity severity = EventSeverity::Debug;
    Event event = Event::General;
    TimePoint start;
};
#else
class stopwatch {
    stopwatch(Event event = Event::General);
    stopwatch(EventSeverity severity, Event event = Event::General);
    stopwatch(const std::string &name, Event event = Event::General);
    stopwatch(const std::string &name, EventSeverity severity, Event event = Event::General);
    void report(const std::string &name) {}
    ~stopwatch() {}
};
#endif
} // namespace util
} // namespace mbgl
