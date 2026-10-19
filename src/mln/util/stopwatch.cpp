#ifndef DISABLE_STOPWATCH
#include <mln/util/stopwatch.hpp>
#include <mln/util/string.hpp>
#include <mln/util/chrono.hpp>
#include <mln/util/logging.hpp>

#include <atomic>

namespace mln {
namespace util {

stopwatch::stopwatch(Event event_)
    : event(event_),
      start(Clock::now()) {}

stopwatch::stopwatch(EventSeverity severity_, Event event_)
    : severity(severity_),
      event(event_),
      start(Clock::now()) {}

stopwatch::stopwatch(std::string name_, Event event_)
    : name(std::move(name_)),
      event(event_),
      start(Clock::now()) {}

stopwatch::stopwatch(std::string name_, EventSeverity severity_, Event event_)
    : name(std::move(name_)),
      severity(severity_),
      event(event_),
      start(Clock::now()) {}

void stopwatch::report(const std::string &name_) {
    Duration duration = Clock::now() - start;

    const auto logMsg = name_ + ": " +
                        std::to_string(
                            std::chrono::duration<float, std::chrono::milliseconds::period>(duration).count());

    Log::Record(severity, event, logMsg);
    start += duration;
}

stopwatch::~stopwatch() {
    if (!name.empty()) {
        report(name);
    }
}

} // namespace util
} // namespace mln

#endif
