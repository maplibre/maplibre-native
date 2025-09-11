#include <mbgl/util/chrono.hpp>

#include <parsedate/parsedate.hpp>

#include <cstdio>
#include <ctime>

#if defined(_WIN32)
#define _gmtime(t, i) gmtime_s(i, t)
#else
#define _gmtime(t, i) gmtime_r(t, i)
#endif

namespace mbgl {
namespace util {

namespace {
const char* week[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
} // namespace

std::string rfc1123(Timestamp timestamp) {
    std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm info;
    _gmtime(&time, &info);

    // Buffer size 30 is OK assuming the year has 4 digits. However, In theory,
    // it might have more digits. Under gcc 8.3.0 with -Os optimization flag,
    // there is compiler warning complaining about the buffer size might be too
    // small. Inceasing the buffer to 32 fixes the warning.
    char buffer[32];
    snprintf(buffer,
             32,
             "%s, %02d %s %4d %02d:%02d:%02d GMT",
             week[info.tm_wday],
             info.tm_mday,
             months[info.tm_mon],
             1900 + info.tm_year,
             info.tm_hour,
             info.tm_min,
             info.tm_sec);
    return buffer;
}

std::string iso8601(Timestamp timestamp) {
    std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm info;
    _gmtime(&time, &info);
    char buffer[30];
    // %F and %T are not supported in MinGW (https://sourceforge.net/p/mingw-w64/bugs/793/)
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &info);
    return buffer;
}

std::string iso8601(std::chrono::time_point<std::chrono::system_clock, Milliseconds> timestamp) {
    std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm info;
    _gmtime(&time, &info);

    long long ms =
        std::chrono::duration_cast<Milliseconds>(timestamp - std::chrono::system_clock::from_time_t(time)).count() %
        1000;

    char buffer[sizeof("yyyy-mm-ddThh:mm:ss.000Z")];
    // %F and %T are not supported in MinGW (https://sourceforge.net/p/mingw-w64/bugs/793/)
    const std::size_t offset = std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &info);
    snprintf(buffer + offset, sizeof(buffer) - offset, ".%03lldZ", ms);

    return buffer;
}

Timestamp parseTimestamp(const char* timestamp) {
    return std::chrono::time_point_cast<Seconds>(std::chrono::system_clock::from_time_t(parse_date(timestamp)));
}

Timestamp parseTimestamp(const int32_t timestamp) {
    return std::chrono::time_point_cast<Seconds>(std::chrono::system_clock::from_time_t(timestamp));
}

} // namespace util

} // namespace mbgl
