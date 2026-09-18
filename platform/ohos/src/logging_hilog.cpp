#include <mln/util/enum.hpp>
#include <mln/util/logging.hpp>

#include <hilog/log.h>

#include <string>

namespace mln {
namespace {

constexpr unsigned int kMapLibreHilogDomain = 0x4d4c;
constexpr char kMapLibreHilogTag[] = "MapLibreNative";

LogLevel logLevelForSeverity(EventSeverity severity) {
    switch (severity) {
        case EventSeverity::Debug:
            return LOG_DEBUG;
        case EventSeverity::Info:
            return LOG_INFO;
        case EventSeverity::Warning:
            return LOG_WARN;
        case EventSeverity::Error:
            return LOG_ERROR;
        case EventSeverity::SeverityCount:
            break;
    }
    return LOG_INFO;
}

} // namespace

void Log::platformRecord(EventSeverity severity, const std::string& msg) {
    const auto message = std::string("[") + Enum<EventSeverity>::toString(severity) + "] " + msg;
    // Pass the message as an argument to a constant format string so that '%' in
    // log text is never interpreted as a hilog format specifier. {public} keeps
    // the text from being redacted as private data.
    OH_LOG_Print(
        LOG_APP, logLevelForSeverity(severity), kMapLibreHilogDomain, kMapLibreHilogTag, "%{public}s", message.c_str());
}

} // namespace mln
