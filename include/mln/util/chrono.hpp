#pragma once

#include <mln/platform/time.hpp>

#include <chrono>
#include <string>

namespace mln {

using Clock = std::chrono::steady_clock;

using Seconds = std::chrono::seconds;
using Milliseconds = std::chrono::milliseconds;

using TimePoint = Clock::time_point;
using Duration = Clock::duration;

// Used to measure second-precision times, such as times gathered from HTTP responses.
using Timestamp = std::chrono::time_point<std::chrono::system_clock, Seconds>;

namespace util {

inline Timestamp now() {
    return platform::now();
}

// Returns the RFC1123 formatted date. E.g. "Tue, 04 Nov 2014 02:13:24 GMT"
std::string rfc1123(Timestamp);

// YYYY-mm-dd HH:MM:SS e.g. "2015-11-26 16:11:23"
std::string iso8601(Timestamp);

// YYYY-mm-ddTHH:MM:SS.MS e.g. "2015-11-26T16:11:23.324Z"
std::string iso8601(std::chrono::time_point<std::chrono::system_clock, Milliseconds>);

Timestamp parseTimestamp(const char *);

Timestamp parseTimestamp(int32_t timestamp);

} // namespace util

} // namespace mln
